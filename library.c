#include "library.h"

#include <openssl/crypto.h>
#include <openssl/evp.h>
#include <limits.h>
#include <stdint.h>
#include <string.h>

#if TCL_MAJOR_VERSION < 9
typedef int Tcl_Size;
#define TCL_SIZE_MAX INT_MAX
#define MIN_TCL_VERSION "8.6"
#else
#define MIN_TCL_VERSION "9.0"
#endif

static int tmfa_Error(Tcl_Interp *interp, const char *code, const char *message) {
    Tcl_SetObjResult(interp, Tcl_NewStringObj(message, -1));
    Tcl_SetErrorCode(interp, "TMFA", code, NULL);
    return TCL_ERROR;
}

static int tmfa_Base32Value(unsigned char c) {
    if (c >= 'a' && c <= 'z') c -= 'a' - 'A';
    if (c >= 'A' && c <= 'Z') return c - 'A';
    if (c >= '2' && c <= '7') return c - '2' + 26;
    return -1;
}

/* RFC 4648: accept padded or unpadded input, with zero unused bits. */
static int tmfa_Decode(Tcl_Interp *interp, Tcl_Obj *obj,
                       unsigned char **bytes, size_t *length) {
    Tcl_Size input_length;
    const char *input = Tcl_GetStringFromObj(obj, &input_length);
    size_t total = (size_t)input_length;
    size_t symbols = total;
    while (symbols > 0 && input[symbols - 1] == '=') --symbols;
    size_t remainder = symbols % 8;
    if (remainder == 1 || remainder == 3 || remainder == 6 ||
        (symbols != total && (symbols == 0 || remainder == 0 ||
         total % 8 != 0 || total - symbols != 8 - remainder))) {
        return tmfa_Error(interp, "BASE32", "invalid Base32 input");
    }
    size_t size = (symbols / 8) * 5 + (remainder * 5) / 8;
    unsigned char *output = OPENSSL_malloc(size ? size : 1);
    if (!output) return tmfa_Error(interp, "MEMORY", "cannot allocate Base32 buffer");

    uint32_t buffer = 0;
    unsigned int bits = 0;
    size_t pos = 0;
    for (size_t i = 0; i < symbols; ++i) {
        int value = tmfa_Base32Value((unsigned char)input[i]);
        if (value < 0) goto invalid;
        buffer = (buffer << 5) | (unsigned int)value;
        bits += 5;
        if (bits >= 8) {
            bits -= 8;
            output[pos++] = (unsigned char)(buffer >> bits);
        }
    }
    if (bits && (buffer & ((1U << bits) - 1U))) goto invalid;
    *bytes = output;
    *length = size;
    return TCL_OK;

invalid:
    OPENSSL_clear_free(output, size);
    return tmfa_Error(interp, "BASE32", "invalid Base32 input");
}

static int tmfa_Base32Encode(ClientData data, Tcl_Interp *interp,
                              int objc, Tcl_Obj *const objv[]) {
    (void)data;
    if (objc != 2) {
        Tcl_WrongNumArgs(interp, 1, objv, "bytes");
        return TCL_ERROR;
    }
    Tcl_Size input_length;
    const unsigned char *input = Tcl_GetByteArrayFromObj(objv[1], &input_length);
    size_t length = (size_t)input_length;
    if (length > ((size_t)TCL_SIZE_MAX / 8) * 5) {
        return tmfa_Error(interp, "SIZE", "input is too large to Base32 encode");
    }
    size_t size = ((length + 4) / 5) * 8;
    char *output = OPENSSL_malloc(size + 1);
    if (!output) return tmfa_Error(interp, "MEMORY", "cannot allocate Base32 buffer");

    static const char alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567";
    uint32_t buffer = 0;
    unsigned int bits = 0;
    size_t pos = 0;
    for (size_t i = 0; i < length; ++i) {
        buffer = (buffer << 8) | input[i];
        bits += 8;
        while (bits >= 5) {
            bits -= 5;
            output[pos++] = alphabet[(buffer >> bits) & 31U];
        }
    }
    if (bits) output[pos++] = alphabet[(buffer << (5 - bits)) & 31U];
    while (pos < size) output[pos++] = '=';
    Tcl_SetObjResult(interp, Tcl_NewStringObj(output, (Tcl_Size)size));
    OPENSSL_clear_free(output, size + 1);
    return TCL_OK;
}

static int tmfa_Base32Decode(ClientData data, Tcl_Interp *interp,
                              int objc, Tcl_Obj *const objv[]) {
    (void)data;
    if (objc != 2) {
        Tcl_WrongNumArgs(interp, 1, objv, "base32_text");
        return TCL_ERROR;
    }
    unsigned char *bytes;
    size_t length;
    if (tmfa_Decode(interp, objv[1], &bytes, &length) != TCL_OK) return TCL_ERROR;
    Tcl_SetObjResult(interp, Tcl_NewByteArrayObj(bytes, (Tcl_Size)length));
    OPENSSL_clear_free(bytes, length);
    return TCL_OK;
}

static int tmfa_GetUint64(Tcl_Interp *interp, Tcl_Obj *obj,
                          const char *name, uint64_t *result) {
    Tcl_Size length;
    const char *input = Tcl_GetStringFromObj(obj, &length);
    uint64_t value = 0;
    if (length == 0) goto invalid;
    for (Tcl_Size i = 0; i < length; ++i) {
        if (input[i] < '0' || input[i] > '9') goto invalid;
        unsigned int digit = (unsigned int)(input[i] - '0');
        if (value > (UINT64_MAX - digit) / 10) goto invalid;
        value = value * 10 + digit;
    }
    *result = value;
    return TCL_OK;

invalid:
    Tcl_SetObjResult(interp, Tcl_ObjPrintf("%s must be an unsigned 64-bit decimal integer", name));
    Tcl_SetErrorCode(interp, "TMFA", "INTEGER", NULL);
    return TCL_ERROR;
}

static int tmfa_GetDigits(Tcl_Interp *interp, Tcl_Obj *obj, int *digits) {
    if (Tcl_GetIntFromObj(interp, obj, digits) != TCL_OK) return TCL_ERROR;
    if (*digits < 4 || *digits > 10) {
        return tmfa_Error(interp, "DIGITS", "digits must be between 4 and 10");
    }
    return TCL_OK;
}

static int tmfa_Generate(Tcl_Interp *interp, Tcl_Obj *secret, uint64_t counter,
                         int digits, Tcl_Obj *algorithm) {
    static const char *const algorithms[] = {"SHA1", "SHA256", "SHA512", NULL};
    int index;
    if (Tcl_GetIndexFromObj(interp, algorithm, algorithms, "algo", TCL_EXACT, &index) != TCL_OK) {
        return TCL_ERROR;
    }
    unsigned char *key;
    size_t key_length;
    if (tmfa_Decode(interp, secret, &key, &key_length) != TCL_OK) return TCL_ERROR;
    if (key_length == 0) {
        OPENSSL_free(key);
        return tmfa_Error(interp, "SECRET", "secret must not be empty");
    }

    unsigned char message[8], digest[EVP_MAX_MD_SIZE];
    for (int i = 7; i >= 0; --i) {
        message[i] = (unsigned char)counter;
        counter >>= 8;
    }
    size_t digest_length = 0;
    unsigned char *mac = EVP_Q_mac(NULL, "HMAC", NULL, algorithms[index], NULL,
                                   key, key_length, message, sizeof(message),
                                   digest, sizeof(digest), &digest_length);
    OPENSSL_clear_free(key, key_length);
    if (!mac || digest_length < 20) {
        OPENSSL_cleanse(digest, sizeof(digest));
        return tmfa_Error(interp, "CRYPTO", "OpenSSL HMAC calculation failed");
    }

    unsigned int offset = digest[digest_length - 1] & 15U;
    uint32_t value = ((uint32_t)(digest[offset] & 127U) << 24) |
                     ((uint32_t)digest[offset + 1] << 16) |
                     ((uint32_t)digest[offset + 2] << 8) | digest[offset + 3];
    char token[10];
    for (int i = digits - 1; i >= 0; --i) {
        token[i] = (char)('0' + value % 10);
        value /= 10;
    }
    Tcl_SetObjResult(interp, Tcl_NewStringObj(token, digits));
    OPENSSL_cleanse(digest, sizeof(digest));
    OPENSSL_cleanse(token, sizeof(token));
    return TCL_OK;
}

static int tmfa_GetHotp(ClientData data, Tcl_Interp *interp,
                         int objc, Tcl_Obj *const objv[]) {
    (void)data;
    if (objc != 5) {
        Tcl_WrongNumArgs(interp, 1, objv, "base32_secret counter digits algo");
        return TCL_ERROR;
    }
    uint64_t counter;
    int digits;
    if (tmfa_GetUint64(interp, objv[2], "counter", &counter) != TCL_OK ||
        tmfa_GetDigits(interp, objv[3], &digits) != TCL_OK) return TCL_ERROR;
    return tmfa_Generate(interp, objv[1], counter, digits, objv[4]);
}

static int tmfa_GetTotp(ClientData data, Tcl_Interp *interp,
                         int objc, Tcl_Obj *const objv[]) {
    (void)data;
    if (objc != 5 && objc != 6) {
        Tcl_WrongNumArgs(interp, 1, objv, "base32_secret digits period algo ?timestamp?");
        return TCL_ERROR;
    }
    int digits, period;
    if (tmfa_GetDigits(interp, objv[2], &digits) != TCL_OK ||
        Tcl_GetIntFromObj(interp, objv[3], &period) != TCL_OK) return TCL_ERROR;
    if (period < 1 || period > 120) {
        return tmfa_Error(interp, "PERIOD", "period must be between 1 and 120");
    }
    uint64_t timestamp;
    if (objc == 6) {
        if (tmfa_GetUint64(interp, objv[5], "timestamp", &timestamp) != TCL_OK) return TCL_ERROR;
    } else {
        Tcl_Time now;
        Tcl_GetTime(&now);
        if (now.sec < 0) return tmfa_Error(interp, "TIME", "time must not precede the Unix epoch");
        timestamp = (uint64_t)now.sec;
    }
    return tmfa_Generate(interp, objv[1], timestamp / (unsigned int)period, digits, objv[4]);
}

DLLEXPORT int Tmfa_Init(Tcl_Interp *interp) {
    if (!Tcl_InitStubs(interp, MIN_TCL_VERSION, 0)) return TCL_ERROR;
    if (!Tcl_FindNamespace(interp, "::tmfa", NULL, 0) &&
        !Tcl_CreateNamespace(interp, "::tmfa", NULL, NULL)) return TCL_ERROR;
    Tcl_CreateObjCommand(interp, "::tmfa::get_totp", tmfa_GetTotp, NULL, NULL);
    Tcl_CreateObjCommand(interp, "::tmfa::get_hotp", tmfa_GetHotp, NULL, NULL);
    Tcl_CreateObjCommand(interp, "::tmfa::base32_encode", tmfa_Base32Encode, NULL, NULL);
    Tcl_CreateObjCommand(interp, "::tmfa::base32_decode", tmfa_Base32Decode, NULL, NULL);
    return Tcl_PkgProvide(interp, "tmfa", PROJECT_VERSION);
}

#ifdef USE_NAVISERVER
NS_EXPORT int Ns_ModuleVersion = 1;

static int tmfa_InterpInit(Tcl_Interp *interp, const void *arg) {
    (void)arg;
    return Tmfa_Init(interp);
}

NS_EXPORT int Ns_ModuleInit(const char *server, const char *module) {
    (void)module;
    return Ns_TclRegisterTrace(server, tmfa_InterpInit, NULL, NS_TCL_TRACE_CREATE);
}
#endif
