#include "library.h"
#include "cotp.h"

#include <stdio.h>
#include <string.h>

#define XSTR(s) STR(s)
#define STR(s) #s

#ifdef DEBUG
# define DBG(x) x
#else
# define DBG(x)
#endif

#define CheckArgs(min,max,n,msg) \
                 if ((objc < min) || (objc >max)) { \
                     Tcl_WrongNumArgs(interp, n, objv, msg); \
                     return TCL_ERROR; \
                 }

#define MIN_DIGITS 4
#define MAX_DIGITS 10

static int           tmfa_ModuleInitialized;

const char *errors[] = {
        "NO_ERROR",
        "VALID",
        "GCRYPT_VERSION_MISMATCH",
        "INVALID_B32_INPUT",
        "INVALID_ALGO",
        "INVALID_DIGITS",
        "INVALID_PERIOD",
        "MEMORY_ALLOCATION_ERROR",
        "INVALID_USER_INPUT",
        "EMPTY_STRING",
        "MISSING_LEADING_ZERO",
        "INVALID_COUNTER"
};

int get_algo(const char *algo_str) {
    return strcmp(algo_str, "SHA1") == 0 ? SHA1 : strcmp(algo_str, "SHA256") == 0 ? SHA256 : strcmp(algo_str, "SHA512") == 0 ? SHA512 : -1;
}

static int tmfa_GetTotp(ClientData  clientData, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] ) {
    DBG(fprintf(stderr,"GetTotpCmd\n"));

    CheckArgs(5,5,1,"base32_encoded_secret digits period algo");

    const char *base32_encoded_secret = Tcl_GetString(objv[1]);
    int digits = atoi(Tcl_GetString(objv[2]));

    if (digits < MIN_DIGITS || digits > MAX_DIGITS) {
        Tcl_SetObjResult(interp, Tcl_NewStringObj("digits must be between 4 and 10", -1));
        return TCL_ERROR;
    }

    int         period                = atoi(Tcl_GetString(objv[3]));

    if (period <= 0 || period > 120) {
        Tcl_SetObjResult(interp, Tcl_NewStringObj("period must be between 1 and 120", -1));
        return TCL_ERROR;
    }

    const char *algo_str = Tcl_GetString(objv[4]);
    int algo = get_algo(algo_str);

    if (algo == -1) {
        Tcl_SetObjResult(interp, Tcl_NewStringObj("algo must be SHA1, SHA256 or SHA512", -1));
        return TCL_ERROR;
    }

    cotp_error_t err;
    char *totp = get_totp(
            base32_encoded_secret,
            digits,
            period,
            algo,
            &err);
    if (totp == NULL) {
        Tcl_SetObjResult(interp, Tcl_NewStringObj(errors[(int) err], -1));
        return TCL_ERROR;
    }

    Tcl_SetObjResult(interp, Tcl_NewStringObj(totp, -1));
    return TCL_OK;
}

static int tmfa_GetHotp(ClientData  clientData, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] ) {
    DBG(fprintf(stderr,"GetHotpCmd\n"));

    CheckArgs(5,5,1,"secret counter digits algo");

    const char *secret = Tcl_GetString(objv[1]);

    int         counter                = atoi(Tcl_GetString(objv[2]));

    int digits = atoi(Tcl_GetString(objv[3]));

    if (digits < MIN_DIGITS || digits > MAX_DIGITS) {
        Tcl_SetObjResult(interp, Tcl_NewStringObj("digits must be between 4 and 10", -1));
        return TCL_ERROR;
    }

    const char *algo_str = Tcl_GetString(objv[4]);
    int algo = get_algo(algo_str);

    if (algo == -1) {
        Tcl_SetObjResult(interp, Tcl_NewStringObj("algo must be SHA1, SHA256 or SHA512", -1));
        return TCL_ERROR;
    }

    cotp_error_t err;
    char *hotp = get_hotp(
            secret,
            counter,
            digits,
            algo,
            &err);
    if (hotp == NULL) {
        Tcl_SetObjResult(interp, Tcl_NewStringObj(errors[(int) err], -1));
        return TCL_ERROR;
    }


    Tcl_SetObjResult(interp, Tcl_NewStringObj(hotp, -1));
    return TCL_OK;
}

static int tmfa_Base32Encode(ClientData  clientData, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] ) {
    DBG(fprintf(stderr,"Base32EncodeCmd\n"));

    CheckArgs(2,2,1,"text");

    int len;
    const char *user_data = Tcl_GetStringFromObj(objv[1], &len);

    cotp_error_t err;
    char *encoded_data = base32_encode(
            user_data,
            len,
            &err);
    if (encoded_data == NULL) {
        Tcl_SetObjResult(interp, Tcl_NewStringObj(errors[(int) err], -1));
        return TCL_ERROR;
    }


    Tcl_SetObjResult(interp, Tcl_NewStringObj(encoded_data, -1));
    return TCL_OK;
}


static int tmfa_Base32Decode(ClientData  clientData, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] ) {
    DBG(fprintf(stderr,"Base32DecodeCmd\n"));

    CheckArgs(2,2,1,"base32_encoded_text");

    int len;
    const char *user_data_untrimmed = Tcl_GetStringFromObj(objv[1], &len);

    cotp_error_t err;
    unsigned char *decoded_data = base32_decode(
            user_data_untrimmed,
            len,
            &err);
    if (decoded_data == NULL) {
        Tcl_SetObjResult(interp, Tcl_NewStringObj(errors[(int) err], -1));
        return TCL_ERROR;
    }


    Tcl_SetObjResult(interp, Tcl_NewStringObj(decoded_data, -1));
    return TCL_OK;
}


static void tmfa_ExitHandler(ClientData unused) {
}


void tmfa_InitModule() {
    if (!tmfa_ModuleInitialized) {
        Tcl_CreateThreadExitHandler(tmfa_ExitHandler, NULL);
        tmfa_ModuleInitialized = 1;
    }
}

int Tmfa_Init(Tcl_Interp *interp) {
    if (Tcl_InitStubs(interp, "8.6", 0) == NULL) {
        return TCL_ERROR;
    }

    tmfa_InitModule();

    Tcl_CreateNamespace(interp, "::tmfa", NULL, NULL);
    Tcl_CreateObjCommand(interp, "::tmfa::get_totp", tmfa_GetTotp, NULL, NULL);
    Tcl_CreateObjCommand(interp, "::tmfa::get_hotp", tmfa_GetHotp, NULL, NULL);
    Tcl_CreateObjCommand(interp, "::tmfa::base32_encode", tmfa_Base32Encode, NULL, NULL);
    Tcl_CreateObjCommand(interp, "::tmfa::base32_decode", tmfa_Base32Decode, NULL, NULL);

    return Tcl_PkgProvide(interp, "tmfa", XSTR(PROJECT_VERSION));
}

#ifdef USE_NAVISERVER
int Ns_ModuleInit(const char *server, const char *module) {
    Ns_TclRegisterTrace(server, (Ns_TclTraceProc *) Tmfa_Init, server, NS_TCL_TRACE_CREATE);
    return NS_OK;
}
#endif
