#ifndef TMFA_LIBRARY_H
#define TMFA_LIBRARY_H

#ifdef USE_NAVISERVER
#include <ns.h>
#else
#include <tcl.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

DLLEXPORT int Tmfa_Init(Tcl_Interp *interp);

#ifdef __cplusplus
}
#endif

#endif
