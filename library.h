#ifndef TBERT_LIBRARY_H
#define TBERT_LIBRARY_H

#include <tcl.h>

#ifdef USE_NAVISERVER
#include <cstdint>
#include "ns.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

extern int Tbert_Init(Tcl_Interp *interp);
#ifdef USE_NAVISERVER
NS_EXTERN int Ns_ModuleVersion = 1;
NS_EXTERN int Ns_ModuleInit(const char *server, const char *module);
#endif

#ifdef __cplusplus
}
#endif

#endif //TBERT_LIBRARY_H
