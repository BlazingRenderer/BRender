#ifndef _GLSTATE_IP_HPP
#define _GLSTATE_IP_HPP

#include "glstate.hpp"

void GLSTATEI_InitMatrix(HGLSTATE hState);
void GLSTATEI_InitCull(HGLSTATE hState);
void GLSTATEI_InitSurface(HGLSTATE hState);
void GLSTATEI_InitPrimitive(HGLSTATE hState);
void GLSTATEI_InitOutput(HGLSTATE hState);
void GLSTATEI_InitHidden(HGLSTATE hState);
void GLSTATEI_InitLight(HGLSTATE hState);
void GLSTATEI_InitGlobal(HGLSTATE hState);

#endif /* _GLSTATE_IP_HPP */
