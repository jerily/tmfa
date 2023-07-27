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

typedef int32_t bert_vocab_id;

static int           tmfa_ModuleInitialized;

static int tmfa_GetTotp(ClientData  clientData, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] ) {
    DBG(fprintf(stderr,"GetTotpCmd\n"));

    CheckArgs(3,3,1,"handle_name filename");
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
