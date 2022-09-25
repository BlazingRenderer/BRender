#ifndef _GLSTATE_IP_H
#define _GLSTATE_IP_H

#include "glstate.h"

#if defined(__cplusplus)
extern "C" {
#endif

void GLSTATEI_InitMatrix(HGLSTATE hState);
void GLSTATEI_InitCull(HGLSTATE hState);
void GLSTATEI_InitSurface(HGLSTATE hState);
void GLSTATEI_InitPrimitive(HGLSTATE hState);
void GLSTATEI_InitOutput(HGLSTATE hState);
void GLSTATEI_InitHidden(HGLSTATE hState);
void GLSTATEI_InitLight(HGLSTATE hState);

#if defined(__cplusplus)
}
#endif

#endif /* _GLSTATE_IP_H */
