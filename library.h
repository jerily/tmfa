#ifndef TMFA_LIBRARY_H
#define TMFA_LIBRARY_H

#ifdef USE_NAVISERVER
#include "ns.h"
#include <stdint.h>
#else
#include <tcl.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

extern int Tmfa_Init(Tcl_Interp *interp);
#ifdef USE_NAVISERVER
NS_EXTERN int Ns_ModuleVersion = 1;
NS_EXTERN int Ns_ModuleInit(const char *server, const char *module);
#endif

#ifdef __cplusplus
}
#endif

#endif //TMFA_LIBRARY_H
