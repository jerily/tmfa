#include "library.h"
#include "cotp.h"

#include <stdio.h>

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

static int tmfa_GetTotp(ClientData  clientData, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] ) {
    DBG(fprintf(stderr,"GetTotpCmd\n"));

    CheckArgs(5,5,1,"base32_encoded_secret digits period algo");

    const char *base32_encoded_secret = Tcl_GetString(objv[1]);
    int digits = atoi(Tcl_GetString(objv[2]));

    if (digits < MIN_DIGITS || digits > MAX_DIGITS) {
        Tcl_SetObjResult(interp, Tcl_NewStringObj("digits must be between 6 and 8", -1));
        return TCL_ERROR;
    }

    int         period                = atoi(Tcl_GetString(objv[3]));

    if (period <= 0 || period > 120) {
        Tcl_SetObjResult(interp, Tcl_NewStringObj("period must be between 1 and 120", -1));
        return TCL_ERROR;
    }

    const char *algo_str = Tcl_GetString(objv[4]);
    int algo = strcmp(algo_str, "SHA1") == 0 ? SHA1 : strcmp(algo_str, "SHA256") == 0 ? SHA256 : strcmp(algo_str, "SHA512") == 0 ? SHA512 : -1;

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
        Tcl_SetObjResult(interp, Tcl_NewIntObj((int) err));
        return TCL_ERROR;
    }

    Tcl_SetObjResult(interp, Tcl_NewStringObj(totp, -1));
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

    return Tcl_PkgProvide(interp, "tmfa", "0.1");
}

#ifdef USE_NAVISERVER
int Ns_ModuleInit(const char *server, const char *module) {
    Ns_TclRegisterTrace(server, (Ns_TclTraceProc *) Tmfa_Init, server, NS_TCL_TRACE_CREATE);
    return NS_OK;
}
#endif
