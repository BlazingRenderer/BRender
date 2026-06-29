/**
 * SPDX-License-Identifier: (WTFPL OR CC0-1.0) AND Apache-2.0
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glad/gl.h>

#ifndef GLAD_IMPL_UTIL_C_
#define GLAD_IMPL_UTIL_C_

#ifdef _MSC_VER
#define GLAD_IMPL_UTIL_SSCANF sscanf_s
#else
#define GLAD_IMPL_UTIL_SSCANF sscanf
#endif

#endif /* GLAD_IMPL_UTIL_C_ */

#ifdef __cplusplus
extern "C" {
#endif








static void glad_gl_load_GL_VERSION_1_0(GladGLContext *context, GLADuserptrloadfunc load, void* userptr) {
    if(!context->VERSION_1_0) return;
    context->Accum = (PFNGLACCUMPROC) load(userptr, "glAccum");
    context->AlphaFunc = (PFNGLALPHAFUNCPROC) load(userptr, "glAlphaFunc");
    context->Begin = (PFNGLBEGINPROC) load(userptr, "glBegin");
    context->Bitmap = (PFNGLBITMAPPROC) load(userptr, "glBitmap");
    context->BlendFunc = (PFNGLBLENDFUNCPROC) load(userptr, "glBlendFunc");
    context->CallList = (PFNGLCALLLISTPROC) load(userptr, "glCallList");
    context->CallLists = (PFNGLCALLLISTSPROC) load(userptr, "glCallLists");
    context->Clear = (PFNGLCLEARPROC) load(userptr, "glClear");
    context->ClearAccum = (PFNGLCLEARACCUMPROC) load(userptr, "glClearAccum");
    context->ClearColor = (PFNGLCLEARCOLORPROC) load(userptr, "glClearColor");
    context->ClearDepth = (PFNGLCLEARDEPTHPROC) load(userptr, "glClearDepth");
    context->ClearIndex = (PFNGLCLEARINDEXPROC) load(userptr, "glClearIndex");
    context->ClearStencil = (PFNGLCLEARSTENCILPROC) load(userptr, "glClearStencil");
    context->ClipPlane = (PFNGLCLIPPLANEPROC) load(userptr, "glClipPlane");
    context->Color3b = (PFNGLCOLOR3BPROC) load(userptr, "glColor3b");
    context->Color3bv = (PFNGLCOLOR3BVPROC) load(userptr, "glColor3bv");
    context->Color3d = (PFNGLCOLOR3DPROC) load(userptr, "glColor3d");
    context->Color3dv = (PFNGLCOLOR3DVPROC) load(userptr, "glColor3dv");
    context->Color3f = (PFNGLCOLOR3FPROC) load(userptr, "glColor3f");
    context->Color3fv = (PFNGLCOLOR3FVPROC) load(userptr, "glColor3fv");
    context->Color3i = (PFNGLCOLOR3IPROC) load(userptr, "glColor3i");
    context->Color3iv = (PFNGLCOLOR3IVPROC) load(userptr, "glColor3iv");
    context->Color3s = (PFNGLCOLOR3SPROC) load(userptr, "glColor3s");
    context->Color3sv = (PFNGLCOLOR3SVPROC) load(userptr, "glColor3sv");
    context->Color3ub = (PFNGLCOLOR3UBPROC) load(userptr, "glColor3ub");
    context->Color3ubv = (PFNGLCOLOR3UBVPROC) load(userptr, "glColor3ubv");
    context->Color3ui = (PFNGLCOLOR3UIPROC) load(userptr, "glColor3ui");
    context->Color3uiv = (PFNGLCOLOR3UIVPROC) load(userptr, "glColor3uiv");
    context->Color3us = (PFNGLCOLOR3USPROC) load(userptr, "glColor3us");
    context->Color3usv = (PFNGLCOLOR3USVPROC) load(userptr, "glColor3usv");
    context->Color4b = (PFNGLCOLOR4BPROC) load(userptr, "glColor4b");
    context->Color4bv = (PFNGLCOLOR4BVPROC) load(userptr, "glColor4bv");
    context->Color4d = (PFNGLCOLOR4DPROC) load(userptr, "glColor4d");
    context->Color4dv = (PFNGLCOLOR4DVPROC) load(userptr, "glColor4dv");
    context->Color4f = (PFNGLCOLOR4FPROC) load(userptr, "glColor4f");
    context->Color4fv = (PFNGLCOLOR4FVPROC) load(userptr, "glColor4fv");
    context->Color4i = (PFNGLCOLOR4IPROC) load(userptr, "glColor4i");
    context->Color4iv = (PFNGLCOLOR4IVPROC) load(userptr, "glColor4iv");
    context->Color4s = (PFNGLCOLOR4SPROC) load(userptr, "glColor4s");
    context->Color4sv = (PFNGLCOLOR4SVPROC) load(userptr, "glColor4sv");
    context->Color4ub = (PFNGLCOLOR4UBPROC) load(userptr, "glColor4ub");
    context->Color4ubv = (PFNGLCOLOR4UBVPROC) load(userptr, "glColor4ubv");
    context->Color4ui = (PFNGLCOLOR4UIPROC) load(userptr, "glColor4ui");
    context->Color4uiv = (PFNGLCOLOR4UIVPROC) load(userptr, "glColor4uiv");
    context->Color4us = (PFNGLCOLOR4USPROC) load(userptr, "glColor4us");
    context->Color4usv = (PFNGLCOLOR4USVPROC) load(userptr, "glColor4usv");
    context->ColorMask = (PFNGLCOLORMASKPROC) load(userptr, "glColorMask");
    context->ColorMaterial = (PFNGLCOLORMATERIALPROC) load(userptr, "glColorMaterial");
    context->CopyPixels = (PFNGLCOPYPIXELSPROC) load(userptr, "glCopyPixels");
    context->CullFace = (PFNGLCULLFACEPROC) load(userptr, "glCullFace");
    context->DeleteLists = (PFNGLDELETELISTSPROC) load(userptr, "glDeleteLists");
    context->DepthFunc = (PFNGLDEPTHFUNCPROC) load(userptr, "glDepthFunc");
    context->DepthMask = (PFNGLDEPTHMASKPROC) load(userptr, "glDepthMask");
    context->DepthRange = (PFNGLDEPTHRANGEPROC) load(userptr, "glDepthRange");
    context->Disable = (PFNGLDISABLEPROC) load(userptr, "glDisable");
    context->DrawBuffer = (PFNGLDRAWBUFFERPROC) load(userptr, "glDrawBuffer");
    context->DrawPixels = (PFNGLDRAWPIXELSPROC) load(userptr, "glDrawPixels");
    context->EdgeFlag = (PFNGLEDGEFLAGPROC) load(userptr, "glEdgeFlag");
    context->EdgeFlagv = (PFNGLEDGEFLAGVPROC) load(userptr, "glEdgeFlagv");
    context->Enable = (PFNGLENABLEPROC) load(userptr, "glEnable");
    context->End = (PFNGLENDPROC) load(userptr, "glEnd");
    context->EndList = (PFNGLENDLISTPROC) load(userptr, "glEndList");
    context->EvalCoord1d = (PFNGLEVALCOORD1DPROC) load(userptr, "glEvalCoord1d");
    context->EvalCoord1dv = (PFNGLEVALCOORD1DVPROC) load(userptr, "glEvalCoord1dv");
    context->EvalCoord1f = (PFNGLEVALCOORD1FPROC) load(userptr, "glEvalCoord1f");
    context->EvalCoord1fv = (PFNGLEVALCOORD1FVPROC) load(userptr, "glEvalCoord1fv");
    context->EvalCoord2d = (PFNGLEVALCOORD2DPROC) load(userptr, "glEvalCoord2d");
    context->EvalCoord2dv = (PFNGLEVALCOORD2DVPROC) load(userptr, "glEvalCoord2dv");
    context->EvalCoord2f = (PFNGLEVALCOORD2FPROC) load(userptr, "glEvalCoord2f");
    context->EvalCoord2fv = (PFNGLEVALCOORD2FVPROC) load(userptr, "glEvalCoord2fv");
    context->EvalMesh1 = (PFNGLEVALMESH1PROC) load(userptr, "glEvalMesh1");
    context->EvalMesh2 = (PFNGLEVALMESH2PROC) load(userptr, "glEvalMesh2");
    context->EvalPoint1 = (PFNGLEVALPOINT1PROC) load(userptr, "glEvalPoint1");
    context->EvalPoint2 = (PFNGLEVALPOINT2PROC) load(userptr, "glEvalPoint2");
    context->FeedbackBuffer = (PFNGLFEEDBACKBUFFERPROC) load(userptr, "glFeedbackBuffer");
    context->Finish = (PFNGLFINISHPROC) load(userptr, "glFinish");
    context->Flush = (PFNGLFLUSHPROC) load(userptr, "glFlush");
    context->Fogf = (PFNGLFOGFPROC) load(userptr, "glFogf");
    context->Fogfv = (PFNGLFOGFVPROC) load(userptr, "glFogfv");
    context->Fogi = (PFNGLFOGIPROC) load(userptr, "glFogi");
    context->Fogiv = (PFNGLFOGIVPROC) load(userptr, "glFogiv");
    context->FrontFace = (PFNGLFRONTFACEPROC) load(userptr, "glFrontFace");
    context->Frustum = (PFNGLFRUSTUMPROC) load(userptr, "glFrustum");
    context->GenLists = (PFNGLGENLISTSPROC) load(userptr, "glGenLists");
    context->GetBooleanv = (PFNGLGETBOOLEANVPROC) load(userptr, "glGetBooleanv");
    context->GetClipPlane = (PFNGLGETCLIPPLANEPROC) load(userptr, "glGetClipPlane");
    context->GetDoublev = (PFNGLGETDOUBLEVPROC) load(userptr, "glGetDoublev");
    context->GetError = (PFNGLGETERRORPROC) load(userptr, "glGetError");
    context->GetFloatv = (PFNGLGETFLOATVPROC) load(userptr, "glGetFloatv");
    context->GetIntegerv = (PFNGLGETINTEGERVPROC) load(userptr, "glGetIntegerv");
    context->GetLightfv = (PFNGLGETLIGHTFVPROC) load(userptr, "glGetLightfv");
    context->GetLightiv = (PFNGLGETLIGHTIVPROC) load(userptr, "glGetLightiv");
    context->GetMapdv = (PFNGLGETMAPDVPROC) load(userptr, "glGetMapdv");
    context->GetMapfv = (PFNGLGETMAPFVPROC) load(userptr, "glGetMapfv");
    context->GetMapiv = (PFNGLGETMAPIVPROC) load(userptr, "glGetMapiv");
    context->GetMaterialfv = (PFNGLGETMATERIALFVPROC) load(userptr, "glGetMaterialfv");
    context->GetMaterialiv = (PFNGLGETMATERIALIVPROC) load(userptr, "glGetMaterialiv");
    context->GetPixelMapfv = (PFNGLGETPIXELMAPFVPROC) load(userptr, "glGetPixelMapfv");
    context->GetPixelMapuiv = (PFNGLGETPIXELMAPUIVPROC) load(userptr, "glGetPixelMapuiv");
    context->GetPixelMapusv = (PFNGLGETPIXELMAPUSVPROC) load(userptr, "glGetPixelMapusv");
    context->GetPolygonStipple = (PFNGLGETPOLYGONSTIPPLEPROC) load(userptr, "glGetPolygonStipple");
    context->GetString = (PFNGLGETSTRINGPROC) load(userptr, "glGetString");
    context->GetTexEnvfv = (PFNGLGETTEXENVFVPROC) load(userptr, "glGetTexEnvfv");
    context->GetTexEnviv = (PFNGLGETTEXENVIVPROC) load(userptr, "glGetTexEnviv");
    context->GetTexGendv = (PFNGLGETTEXGENDVPROC) load(userptr, "glGetTexGendv");
    context->GetTexGenfv = (PFNGLGETTEXGENFVPROC) load(userptr, "glGetTexGenfv");
    context->GetTexGeniv = (PFNGLGETTEXGENIVPROC) load(userptr, "glGetTexGeniv");
    context->GetTexImage = (PFNGLGETTEXIMAGEPROC) load(userptr, "glGetTexImage");
    context->GetTexLevelParameterfv = (PFNGLGETTEXLEVELPARAMETERFVPROC) load(userptr, "glGetTexLevelParameterfv");
    context->GetTexLevelParameteriv = (PFNGLGETTEXLEVELPARAMETERIVPROC) load(userptr, "glGetTexLevelParameteriv");
    context->GetTexParameterfv = (PFNGLGETTEXPARAMETERFVPROC) load(userptr, "glGetTexParameterfv");
    context->GetTexParameteriv = (PFNGLGETTEXPARAMETERIVPROC) load(userptr, "glGetTexParameteriv");
    context->Hint = (PFNGLHINTPROC) load(userptr, "glHint");
    context->IndexMask = (PFNGLINDEXMASKPROC) load(userptr, "glIndexMask");
    context->Indexd = (PFNGLINDEXDPROC) load(userptr, "glIndexd");
    context->Indexdv = (PFNGLINDEXDVPROC) load(userptr, "glIndexdv");
    context->Indexf = (PFNGLINDEXFPROC) load(userptr, "glIndexf");
    context->Indexfv = (PFNGLINDEXFVPROC) load(userptr, "glIndexfv");
    context->Indexi = (PFNGLINDEXIPROC) load(userptr, "glIndexi");
    context->Indexiv = (PFNGLINDEXIVPROC) load(userptr, "glIndexiv");
    context->Indexs = (PFNGLINDEXSPROC) load(userptr, "glIndexs");
    context->Indexsv = (PFNGLINDEXSVPROC) load(userptr, "glIndexsv");
    context->InitNames = (PFNGLINITNAMESPROC) load(userptr, "glInitNames");
    context->IsEnabled = (PFNGLISENABLEDPROC) load(userptr, "glIsEnabled");
    context->IsList = (PFNGLISLISTPROC) load(userptr, "glIsList");
    context->LightModelf = (PFNGLLIGHTMODELFPROC) load(userptr, "glLightModelf");
    context->LightModelfv = (PFNGLLIGHTMODELFVPROC) load(userptr, "glLightModelfv");
    context->LightModeli = (PFNGLLIGHTMODELIPROC) load(userptr, "glLightModeli");
    context->LightModeliv = (PFNGLLIGHTMODELIVPROC) load(userptr, "glLightModeliv");
    context->Lightf = (PFNGLLIGHTFPROC) load(userptr, "glLightf");
    context->Lightfv = (PFNGLLIGHTFVPROC) load(userptr, "glLightfv");
    context->Lighti = (PFNGLLIGHTIPROC) load(userptr, "glLighti");
    context->Lightiv = (PFNGLLIGHTIVPROC) load(userptr, "glLightiv");
    context->LineStipple = (PFNGLLINESTIPPLEPROC) load(userptr, "glLineStipple");
    context->LineWidth = (PFNGLLINEWIDTHPROC) load(userptr, "glLineWidth");
    context->ListBase = (PFNGLLISTBASEPROC) load(userptr, "glListBase");
    context->LoadIdentity = (PFNGLLOADIDENTITYPROC) load(userptr, "glLoadIdentity");
    context->LoadMatrixd = (PFNGLLOADMATRIXDPROC) load(userptr, "glLoadMatrixd");
    context->LoadMatrixf = (PFNGLLOADMATRIXFPROC) load(userptr, "glLoadMatrixf");
    context->LoadName = (PFNGLLOADNAMEPROC) load(userptr, "glLoadName");
    context->LogicOp = (PFNGLLOGICOPPROC) load(userptr, "glLogicOp");
    context->Map1d = (PFNGLMAP1DPROC) load(userptr, "glMap1d");
    context->Map1f = (PFNGLMAP1FPROC) load(userptr, "glMap1f");
    context->Map2d = (PFNGLMAP2DPROC) load(userptr, "glMap2d");
    context->Map2f = (PFNGLMAP2FPROC) load(userptr, "glMap2f");
    context->MapGrid1d = (PFNGLMAPGRID1DPROC) load(userptr, "glMapGrid1d");
    context->MapGrid1f = (PFNGLMAPGRID1FPROC) load(userptr, "glMapGrid1f");
    context->MapGrid2d = (PFNGLMAPGRID2DPROC) load(userptr, "glMapGrid2d");
    context->MapGrid2f = (PFNGLMAPGRID2FPROC) load(userptr, "glMapGrid2f");
    context->Materialf = (PFNGLMATERIALFPROC) load(userptr, "glMaterialf");
    context->Materialfv = (PFNGLMATERIALFVPROC) load(userptr, "glMaterialfv");
    context->Materiali = (PFNGLMATERIALIPROC) load(userptr, "glMateriali");
    context->Materialiv = (PFNGLMATERIALIVPROC) load(userptr, "glMaterialiv");
    context->MatrixMode = (PFNGLMATRIXMODEPROC) load(userptr, "glMatrixMode");
    context->MultMatrixd = (PFNGLMULTMATRIXDPROC) load(userptr, "glMultMatrixd");
    context->MultMatrixf = (PFNGLMULTMATRIXFPROC) load(userptr, "glMultMatrixf");
    context->NewList = (PFNGLNEWLISTPROC) load(userptr, "glNewList");
    context->Normal3b = (PFNGLNORMAL3BPROC) load(userptr, "glNormal3b");
    context->Normal3bv = (PFNGLNORMAL3BVPROC) load(userptr, "glNormal3bv");
    context->Normal3d = (PFNGLNORMAL3DPROC) load(userptr, "glNormal3d");
    context->Normal3dv = (PFNGLNORMAL3DVPROC) load(userptr, "glNormal3dv");
    context->Normal3f = (PFNGLNORMAL3FPROC) load(userptr, "glNormal3f");
    context->Normal3fv = (PFNGLNORMAL3FVPROC) load(userptr, "glNormal3fv");
    context->Normal3i = (PFNGLNORMAL3IPROC) load(userptr, "glNormal3i");
    context->Normal3iv = (PFNGLNORMAL3IVPROC) load(userptr, "glNormal3iv");
    context->Normal3s = (PFNGLNORMAL3SPROC) load(userptr, "glNormal3s");
    context->Normal3sv = (PFNGLNORMAL3SVPROC) load(userptr, "glNormal3sv");
    context->Ortho = (PFNGLORTHOPROC) load(userptr, "glOrtho");
    context->PassThrough = (PFNGLPASSTHROUGHPROC) load(userptr, "glPassThrough");
    context->PixelMapfv = (PFNGLPIXELMAPFVPROC) load(userptr, "glPixelMapfv");
    context->PixelMapuiv = (PFNGLPIXELMAPUIVPROC) load(userptr, "glPixelMapuiv");
    context->PixelMapusv = (PFNGLPIXELMAPUSVPROC) load(userptr, "glPixelMapusv");
    context->PixelStoref = (PFNGLPIXELSTOREFPROC) load(userptr, "glPixelStoref");
    context->PixelStorei = (PFNGLPIXELSTOREIPROC) load(userptr, "glPixelStorei");
    context->PixelTransferf = (PFNGLPIXELTRANSFERFPROC) load(userptr, "glPixelTransferf");
    context->PixelTransferi = (PFNGLPIXELTRANSFERIPROC) load(userptr, "glPixelTransferi");
    context->PixelZoom = (PFNGLPIXELZOOMPROC) load(userptr, "glPixelZoom");
    context->PointSize = (PFNGLPOINTSIZEPROC) load(userptr, "glPointSize");
    context->PolygonMode = (PFNGLPOLYGONMODEPROC) load(userptr, "glPolygonMode");
    context->PolygonStipple = (PFNGLPOLYGONSTIPPLEPROC) load(userptr, "glPolygonStipple");
    context->PopAttrib = (PFNGLPOPATTRIBPROC) load(userptr, "glPopAttrib");
    context->PopMatrix = (PFNGLPOPMATRIXPROC) load(userptr, "glPopMatrix");
    context->PopName = (PFNGLPOPNAMEPROC) load(userptr, "glPopName");
    context->PushAttrib = (PFNGLPUSHATTRIBPROC) load(userptr, "glPushAttrib");
    context->PushMatrix = (PFNGLPUSHMATRIXPROC) load(userptr, "glPushMatrix");
    context->PushName = (PFNGLPUSHNAMEPROC) load(userptr, "glPushName");
    context->RasterPos2d = (PFNGLRASTERPOS2DPROC) load(userptr, "glRasterPos2d");
    context->RasterPos2dv = (PFNGLRASTERPOS2DVPROC) load(userptr, "glRasterPos2dv");
    context->RasterPos2f = (PFNGLRASTERPOS2FPROC) load(userptr, "glRasterPos2f");
    context->RasterPos2fv = (PFNGLRASTERPOS2FVPROC) load(userptr, "glRasterPos2fv");
    context->RasterPos2i = (PFNGLRASTERPOS2IPROC) load(userptr, "glRasterPos2i");
    context->RasterPos2iv = (PFNGLRASTERPOS2IVPROC) load(userptr, "glRasterPos2iv");
    context->RasterPos2s = (PFNGLRASTERPOS2SPROC) load(userptr, "glRasterPos2s");
    context->RasterPos2sv = (PFNGLRASTERPOS2SVPROC) load(userptr, "glRasterPos2sv");
    context->RasterPos3d = (PFNGLRASTERPOS3DPROC) load(userptr, "glRasterPos3d");
    context->RasterPos3dv = (PFNGLRASTERPOS3DVPROC) load(userptr, "glRasterPos3dv");
    context->RasterPos3f = (PFNGLRASTERPOS3FPROC) load(userptr, "glRasterPos3f");
    context->RasterPos3fv = (PFNGLRASTERPOS3FVPROC) load(userptr, "glRasterPos3fv");
    context->RasterPos3i = (PFNGLRASTERPOS3IPROC) load(userptr, "glRasterPos3i");
    context->RasterPos3iv = (PFNGLRASTERPOS3IVPROC) load(userptr, "glRasterPos3iv");
    context->RasterPos3s = (PFNGLRASTERPOS3SPROC) load(userptr, "glRasterPos3s");
    context->RasterPos3sv = (PFNGLRASTERPOS3SVPROC) load(userptr, "glRasterPos3sv");
    context->RasterPos4d = (PFNGLRASTERPOS4DPROC) load(userptr, "glRasterPos4d");
    context->RasterPos4dv = (PFNGLRASTERPOS4DVPROC) load(userptr, "glRasterPos4dv");
    context->RasterPos4f = (PFNGLRASTERPOS4FPROC) load(userptr, "glRasterPos4f");
    context->RasterPos4fv = (PFNGLRASTERPOS4FVPROC) load(userptr, "glRasterPos4fv");
    context->RasterPos4i = (PFNGLRASTERPOS4IPROC) load(userptr, "glRasterPos4i");
    context->RasterPos4iv = (PFNGLRASTERPOS4IVPROC) load(userptr, "glRasterPos4iv");
    context->RasterPos4s = (PFNGLRASTERPOS4SPROC) load(userptr, "glRasterPos4s");
    context->RasterPos4sv = (PFNGLRASTERPOS4SVPROC) load(userptr, "glRasterPos4sv");
    context->ReadBuffer = (PFNGLREADBUFFERPROC) load(userptr, "glReadBuffer");
    context->ReadPixels = (PFNGLREADPIXELSPROC) load(userptr, "glReadPixels");
    context->Rectd = (PFNGLRECTDPROC) load(userptr, "glRectd");
    context->Rectdv = (PFNGLRECTDVPROC) load(userptr, "glRectdv");
    context->Rectf = (PFNGLRECTFPROC) load(userptr, "glRectf");
    context->Rectfv = (PFNGLRECTFVPROC) load(userptr, "glRectfv");
    context->Recti = (PFNGLRECTIPROC) load(userptr, "glRecti");
    context->Rectiv = (PFNGLRECTIVPROC) load(userptr, "glRectiv");
    context->Rects = (PFNGLRECTSPROC) load(userptr, "glRects");
    context->Rectsv = (PFNGLRECTSVPROC) load(userptr, "glRectsv");
    context->RenderMode = (PFNGLRENDERMODEPROC) load(userptr, "glRenderMode");
    context->Rotated = (PFNGLROTATEDPROC) load(userptr, "glRotated");
    context->Rotatef = (PFNGLROTATEFPROC) load(userptr, "glRotatef");
    context->Scaled = (PFNGLSCALEDPROC) load(userptr, "glScaled");
    context->Scalef = (PFNGLSCALEFPROC) load(userptr, "glScalef");
    context->Scissor = (PFNGLSCISSORPROC) load(userptr, "glScissor");
    context->SelectBuffer = (PFNGLSELECTBUFFERPROC) load(userptr, "glSelectBuffer");
    context->ShadeModel = (PFNGLSHADEMODELPROC) load(userptr, "glShadeModel");
    context->StencilFunc = (PFNGLSTENCILFUNCPROC) load(userptr, "glStencilFunc");
    context->StencilMask = (PFNGLSTENCILMASKPROC) load(userptr, "glStencilMask");
    context->StencilOp = (PFNGLSTENCILOPPROC) load(userptr, "glStencilOp");
    context->TexCoord1d = (PFNGLTEXCOORD1DPROC) load(userptr, "glTexCoord1d");
    context->TexCoord1dv = (PFNGLTEXCOORD1DVPROC) load(userptr, "glTexCoord1dv");
    context->TexCoord1f = (PFNGLTEXCOORD1FPROC) load(userptr, "glTexCoord1f");
    context->TexCoord1fv = (PFNGLTEXCOORD1FVPROC) load(userptr, "glTexCoord1fv");
    context->TexCoord1i = (PFNGLTEXCOORD1IPROC) load(userptr, "glTexCoord1i");
    context->TexCoord1iv = (PFNGLTEXCOORD1IVPROC) load(userptr, "glTexCoord1iv");
    context->TexCoord1s = (PFNGLTEXCOORD1SPROC) load(userptr, "glTexCoord1s");
    context->TexCoord1sv = (PFNGLTEXCOORD1SVPROC) load(userptr, "glTexCoord1sv");
    context->TexCoord2d = (PFNGLTEXCOORD2DPROC) load(userptr, "glTexCoord2d");
    context->TexCoord2dv = (PFNGLTEXCOORD2DVPROC) load(userptr, "glTexCoord2dv");
    context->TexCoord2f = (PFNGLTEXCOORD2FPROC) load(userptr, "glTexCoord2f");
    context->TexCoord2fv = (PFNGLTEXCOORD2FVPROC) load(userptr, "glTexCoord2fv");
    context->TexCoord2i = (PFNGLTEXCOORD2IPROC) load(userptr, "glTexCoord2i");
    context->TexCoord2iv = (PFNGLTEXCOORD2IVPROC) load(userptr, "glTexCoord2iv");
    context->TexCoord2s = (PFNGLTEXCOORD2SPROC) load(userptr, "glTexCoord2s");
    context->TexCoord2sv = (PFNGLTEXCOORD2SVPROC) load(userptr, "glTexCoord2sv");
    context->TexCoord3d = (PFNGLTEXCOORD3DPROC) load(userptr, "glTexCoord3d");
    context->TexCoord3dv = (PFNGLTEXCOORD3DVPROC) load(userptr, "glTexCoord3dv");
    context->TexCoord3f = (PFNGLTEXCOORD3FPROC) load(userptr, "glTexCoord3f");
    context->TexCoord3fv = (PFNGLTEXCOORD3FVPROC) load(userptr, "glTexCoord3fv");
    context->TexCoord3i = (PFNGLTEXCOORD3IPROC) load(userptr, "glTexCoord3i");
    context->TexCoord3iv = (PFNGLTEXCOORD3IVPROC) load(userptr, "glTexCoord3iv");
    context->TexCoord3s = (PFNGLTEXCOORD3SPROC) load(userptr, "glTexCoord3s");
    context->TexCoord3sv = (PFNGLTEXCOORD3SVPROC) load(userptr, "glTexCoord3sv");
    context->TexCoord4d = (PFNGLTEXCOORD4DPROC) load(userptr, "glTexCoord4d");
    context->TexCoord4dv = (PFNGLTEXCOORD4DVPROC) load(userptr, "glTexCoord4dv");
    context->TexCoord4f = (PFNGLTEXCOORD4FPROC) load(userptr, "glTexCoord4f");
    context->TexCoord4fv = (PFNGLTEXCOORD4FVPROC) load(userptr, "glTexCoord4fv");
    context->TexCoord4i = (PFNGLTEXCOORD4IPROC) load(userptr, "glTexCoord4i");
    context->TexCoord4iv = (PFNGLTEXCOORD4IVPROC) load(userptr, "glTexCoord4iv");
    context->TexCoord4s = (PFNGLTEXCOORD4SPROC) load(userptr, "glTexCoord4s");
    context->TexCoord4sv = (PFNGLTEXCOORD4SVPROC) load(userptr, "glTexCoord4sv");
    context->TexEnvf = (PFNGLTEXENVFPROC) load(userptr, "glTexEnvf");
    context->TexEnvfv = (PFNGLTEXENVFVPROC) load(userptr, "glTexEnvfv");
    context->TexEnvi = (PFNGLTEXENVIPROC) load(userptr, "glTexEnvi");
    context->TexEnviv = (PFNGLTEXENVIVPROC) load(userptr, "glTexEnviv");
    context->TexGend = (PFNGLTEXGENDPROC) load(userptr, "glTexGend");
    context->TexGendv = (PFNGLTEXGENDVPROC) load(userptr, "glTexGendv");
    context->TexGenf = (PFNGLTEXGENFPROC) load(userptr, "glTexGenf");
    context->TexGenfv = (PFNGLTEXGENFVPROC) load(userptr, "glTexGenfv");
    context->TexGeni = (PFNGLTEXGENIPROC) load(userptr, "glTexGeni");
    context->TexGeniv = (PFNGLTEXGENIVPROC) load(userptr, "glTexGeniv");
    context->TexImage1D = (PFNGLTEXIMAGE1DPROC) load(userptr, "glTexImage1D");
    context->TexImage2D = (PFNGLTEXIMAGE2DPROC) load(userptr, "glTexImage2D");
    context->TexParameterf = (PFNGLTEXPARAMETERFPROC) load(userptr, "glTexParameterf");
    context->TexParameterfv = (PFNGLTEXPARAMETERFVPROC) load(userptr, "glTexParameterfv");
    context->TexParameteri = (PFNGLTEXPARAMETERIPROC) load(userptr, "glTexParameteri");
    context->TexParameteriv = (PFNGLTEXPARAMETERIVPROC) load(userptr, "glTexParameteriv");
    context->Translated = (PFNGLTRANSLATEDPROC) load(userptr, "glTranslated");
    context->Translatef = (PFNGLTRANSLATEFPROC) load(userptr, "glTranslatef");
    context->Vertex2d = (PFNGLVERTEX2DPROC) load(userptr, "glVertex2d");
    context->Vertex2dv = (PFNGLVERTEX2DVPROC) load(userptr, "glVertex2dv");
    context->Vertex2f = (PFNGLVERTEX2FPROC) load(userptr, "glVertex2f");
    context->Vertex2fv = (PFNGLVERTEX2FVPROC) load(userptr, "glVertex2fv");
    context->Vertex2i = (PFNGLVERTEX2IPROC) load(userptr, "glVertex2i");
    context->Vertex2iv = (PFNGLVERTEX2IVPROC) load(userptr, "glVertex2iv");
    context->Vertex2s = (PFNGLVERTEX2SPROC) load(userptr, "glVertex2s");
    context->Vertex2sv = (PFNGLVERTEX2SVPROC) load(userptr, "glVertex2sv");
    context->Vertex3d = (PFNGLVERTEX3DPROC) load(userptr, "glVertex3d");
    context->Vertex3dv = (PFNGLVERTEX3DVPROC) load(userptr, "glVertex3dv");
    context->Vertex3f = (PFNGLVERTEX3FPROC) load(userptr, "glVertex3f");
    context->Vertex3fv = (PFNGLVERTEX3FVPROC) load(userptr, "glVertex3fv");
    context->Vertex3i = (PFNGLVERTEX3IPROC) load(userptr, "glVertex3i");
    context->Vertex3iv = (PFNGLVERTEX3IVPROC) load(userptr, "glVertex3iv");
    context->Vertex3s = (PFNGLVERTEX3SPROC) load(userptr, "glVertex3s");
    context->Vertex3sv = (PFNGLVERTEX3SVPROC) load(userptr, "glVertex3sv");
    context->Vertex4d = (PFNGLVERTEX4DPROC) load(userptr, "glVertex4d");
    context->Vertex4dv = (PFNGLVERTEX4DVPROC) load(userptr, "glVertex4dv");
    context->Vertex4f = (PFNGLVERTEX4FPROC) load(userptr, "glVertex4f");
    context->Vertex4fv = (PFNGLVERTEX4FVPROC) load(userptr, "glVertex4fv");
    context->Vertex4i = (PFNGLVERTEX4IPROC) load(userptr, "glVertex4i");
    context->Vertex4iv = (PFNGLVERTEX4IVPROC) load(userptr, "glVertex4iv");
    context->Vertex4s = (PFNGLVERTEX4SPROC) load(userptr, "glVertex4s");
    context->Vertex4sv = (PFNGLVERTEX4SVPROC) load(userptr, "glVertex4sv");
    context->Viewport = (PFNGLVIEWPORTPROC) load(userptr, "glViewport");
}
static void glad_gl_load_GL_VERSION_1_1(GladGLContext *context, GLADuserptrloadfunc load, void* userptr) {
    if(!context->VERSION_1_1) return;
    context->AreTexturesResident = (PFNGLARETEXTURESRESIDENTPROC) load(userptr, "glAreTexturesResident");
    context->ArrayElement = (PFNGLARRAYELEMENTPROC) load(userptr, "glArrayElement");
    context->BindTexture = (PFNGLBINDTEXTUREPROC) load(userptr, "glBindTexture");
    context->ColorPointer = (PFNGLCOLORPOINTERPROC) load(userptr, "glColorPointer");
    context->CopyTexImage1D = (PFNGLCOPYTEXIMAGE1DPROC) load(userptr, "glCopyTexImage1D");
    context->CopyTexImage2D = (PFNGLCOPYTEXIMAGE2DPROC) load(userptr, "glCopyTexImage2D");
    context->CopyTexSubImage1D = (PFNGLCOPYTEXSUBIMAGE1DPROC) load(userptr, "glCopyTexSubImage1D");
    context->CopyTexSubImage2D = (PFNGLCOPYTEXSUBIMAGE2DPROC) load(userptr, "glCopyTexSubImage2D");
    context->DeleteTextures = (PFNGLDELETETEXTURESPROC) load(userptr, "glDeleteTextures");
    context->DisableClientState = (PFNGLDISABLECLIENTSTATEPROC) load(userptr, "glDisableClientState");
    context->DrawArrays = (PFNGLDRAWARRAYSPROC) load(userptr, "glDrawArrays");
    context->DrawElements = (PFNGLDRAWELEMENTSPROC) load(userptr, "glDrawElements");
    context->EdgeFlagPointer = (PFNGLEDGEFLAGPOINTERPROC) load(userptr, "glEdgeFlagPointer");
    context->EnableClientState = (PFNGLENABLECLIENTSTATEPROC) load(userptr, "glEnableClientState");
    context->GenTextures = (PFNGLGENTEXTURESPROC) load(userptr, "glGenTextures");
    context->GetPointerv = (PFNGLGETPOINTERVPROC) load(userptr, "glGetPointerv");
    context->IndexPointer = (PFNGLINDEXPOINTERPROC) load(userptr, "glIndexPointer");
    context->Indexub = (PFNGLINDEXUBPROC) load(userptr, "glIndexub");
    context->Indexubv = (PFNGLINDEXUBVPROC) load(userptr, "glIndexubv");
    context->InterleavedArrays = (PFNGLINTERLEAVEDARRAYSPROC) load(userptr, "glInterleavedArrays");
    context->IsTexture = (PFNGLISTEXTUREPROC) load(userptr, "glIsTexture");
    context->NormalPointer = (PFNGLNORMALPOINTERPROC) load(userptr, "glNormalPointer");
    context->PolygonOffset = (PFNGLPOLYGONOFFSETPROC) load(userptr, "glPolygonOffset");
    context->PopClientAttrib = (PFNGLPOPCLIENTATTRIBPROC) load(userptr, "glPopClientAttrib");
    context->PrioritizeTextures = (PFNGLPRIORITIZETEXTURESPROC) load(userptr, "glPrioritizeTextures");
    context->PushClientAttrib = (PFNGLPUSHCLIENTATTRIBPROC) load(userptr, "glPushClientAttrib");
    context->TexCoordPointer = (PFNGLTEXCOORDPOINTERPROC) load(userptr, "glTexCoordPointer");
    context->TexSubImage1D = (PFNGLTEXSUBIMAGE1DPROC) load(userptr, "glTexSubImage1D");
    context->TexSubImage2D = (PFNGLTEXSUBIMAGE2DPROC) load(userptr, "glTexSubImage2D");
    context->VertexPointer = (PFNGLVERTEXPOINTERPROC) load(userptr, "glVertexPointer");
}
static void glad_gl_load_GL_VERSION_1_2(GladGLContext *context, GLADuserptrloadfunc load, void* userptr) {
    if(!context->VERSION_1_2) return;
    context->CopyTexSubImage3D = (PFNGLCOPYTEXSUBIMAGE3DPROC) load(userptr, "glCopyTexSubImage3D");
    context->DrawRangeElements = (PFNGLDRAWRANGEELEMENTSPROC) load(userptr, "glDrawRangeElements");
    context->TexImage3D = (PFNGLTEXIMAGE3DPROC) load(userptr, "glTexImage3D");
    context->TexSubImage3D = (PFNGLTEXSUBIMAGE3DPROC) load(userptr, "glTexSubImage3D");
}
static void glad_gl_load_GL_EXT_fog_coord(GladGLContext *context, GLADuserptrloadfunc load, void* userptr) {
    if(!context->EXT_fog_coord) return;
    context->FogCoordPointerEXT = (PFNGLFOGCOORDPOINTEREXTPROC) load(userptr, "glFogCoordPointerEXT");
    context->FogCoorddEXT = (PFNGLFOGCOORDDEXTPROC) load(userptr, "glFogCoorddEXT");
    context->FogCoorddvEXT = (PFNGLFOGCOORDDVEXTPROC) load(userptr, "glFogCoorddvEXT");
    context->FogCoordfEXT = (PFNGLFOGCOORDFEXTPROC) load(userptr, "glFogCoordfEXT");
    context->FogCoordfvEXT = (PFNGLFOGCOORDFVEXTPROC) load(userptr, "glFogCoordfvEXT");
}
static void glad_gl_load_GL_KHR_debug(GladGLContext *context, GLADuserptrloadfunc load, void* userptr) {
    if(!context->KHR_debug) return;
    context->DebugMessageCallback = (PFNGLDEBUGMESSAGECALLBACKPROC) load(userptr, "glDebugMessageCallback");
    context->DebugMessageControl = (PFNGLDEBUGMESSAGECONTROLPROC) load(userptr, "glDebugMessageControl");
    context->DebugMessageInsert = (PFNGLDEBUGMESSAGEINSERTPROC) load(userptr, "glDebugMessageInsert");
    context->GetDebugMessageLog = (PFNGLGETDEBUGMESSAGELOGPROC) load(userptr, "glGetDebugMessageLog");
    context->GetObjectLabel = (PFNGLGETOBJECTLABELPROC) load(userptr, "glGetObjectLabel");
    context->GetObjectPtrLabel = (PFNGLGETOBJECTPTRLABELPROC) load(userptr, "glGetObjectPtrLabel");
    context->GetPointerv = (PFNGLGETPOINTERVPROC) load(userptr, "glGetPointerv");
    context->ObjectLabel = (PFNGLOBJECTLABELPROC) load(userptr, "glObjectLabel");
    context->ObjectPtrLabel = (PFNGLOBJECTPTRLABELPROC) load(userptr, "glObjectPtrLabel");
    context->PopDebugGroup = (PFNGLPOPDEBUGGROUPPROC) load(userptr, "glPopDebugGroup");
    context->PushDebugGroup = (PFNGLPUSHDEBUGGROUPPROC) load(userptr, "glPushDebugGroup");
}



static void glad_gl_free_extensions(char **exts_i) {
    if (exts_i != NULL) {
        unsigned int index;
        for(index = 0; exts_i[index]; index++) {
            free((void *) (exts_i[index]));
        }
        free((void *)exts_i);
        exts_i = NULL;
    }
}
static int glad_gl_get_extensions(GladGLContext *context, const char **out_exts, char ***out_exts_i) {
#if defined(GL_ES_VERSION_3_0) || defined(GL_VERSION_3_0)
    if (context->GetStringi != NULL && context->GetIntegerv != NULL) {
        unsigned int index = 0;
        unsigned int num_exts_i = 0;
        char **exts_i = NULL;
        context->GetIntegerv(GL_NUM_EXTENSIONS, (int*) &num_exts_i);
        exts_i = (char **) malloc((num_exts_i + 1) * (sizeof *exts_i));
        if (exts_i == NULL) {
            return 0;
        }
        for(index = 0; index < num_exts_i; index++) {
            const char *gl_str_tmp = (const char*) context->GetStringi(GL_EXTENSIONS, index);
            size_t len = strlen(gl_str_tmp) + 1;

            char *local_str = (char*) malloc(len * sizeof(char));
            if(local_str == NULL) {
                exts_i[index] = NULL;
                glad_gl_free_extensions(exts_i);
                return 0;
            }

            memcpy(local_str, gl_str_tmp, len * sizeof(char));
            exts_i[index] = local_str;
        }
        exts_i[index] = NULL;

        *out_exts_i = exts_i;

        return 1;
    }
#else
    GLAD_UNUSED(out_exts_i);
#endif
    if (context->GetString == NULL) {
        return 0;
    }
    *out_exts = (const char *)context->GetString(GL_EXTENSIONS);
    return 1;
}
static int glad_gl_has_extension(const char *exts, char **exts_i, const char *ext) {
    if(exts_i) {
        unsigned int index;
        for(index = 0; exts_i[index]; index++) {
            const char *e = exts_i[index];
            if(strcmp(e, ext) == 0) {
                return 1;
            }
        }
    } else {
        const char *extensions;
        const char *loc;
        const char *terminator;
        extensions = exts;
        if(extensions == NULL || ext == NULL) {
            return 0;
        }
        while(1) {
            loc = strstr(extensions, ext);
            if(loc == NULL) {
                return 0;
            }
            terminator = loc + strlen(ext);
            if((loc == extensions || *(loc - 1) == ' ') &&
                (*terminator == ' ' || *terminator == '\0')) {
                return 1;
            }
            extensions = terminator;
        }
    }
    return 0;
}

static GLADapiproc glad_gl_get_proc_from_userptr(void *userptr, const char* name) {
    return (GLAD_GNUC_EXTENSION (GLADapiproc (*)(const char *name)) userptr)(name);
}

static int glad_gl_find_extensions_gl(GladGLContext *context) {
    const char *exts = NULL;
    char **exts_i = NULL;
    if (!glad_gl_get_extensions(context, &exts, &exts_i)) return 0;

    context->EXT_fog_coord = glad_gl_has_extension(exts, exts_i, "GL_EXT_fog_coord");
    context->KHR_debug = glad_gl_has_extension(exts, exts_i, "GL_KHR_debug");

    glad_gl_free_extensions(exts_i);

    return 1;
}

static int glad_gl_find_core_gl(GladGLContext *context) {
    int i;
    const char* version;
    const char* prefixes[] = {
        "OpenGL ES-CM ",
        "OpenGL ES-CL ",
        "OpenGL ES ",
        "OpenGL SC ",
        NULL
    };
    int major = 0;
    int minor = 0;
    version = (const char*) context->GetString(GL_VERSION);
    if (!version) return 0;
    for (i = 0;  prefixes[i];  i++) {
        const size_t length = strlen(prefixes[i]);
        if (strncmp(version, prefixes[i], length) == 0) {
            version += length;
            break;
        }
    }

    GLAD_IMPL_UTIL_SSCANF(version, "%d.%d", &major, &minor);

    context->VERSION_1_0 = (major == 1 && minor >= 0) || major > 1;
    context->VERSION_1_1 = (major == 1 && minor >= 1) || major > 1;
    context->VERSION_1_2 = (major == 1 && minor >= 2) || major > 1;

    return GLAD_MAKE_VERSION(major, minor);
}

int gladLoadGLContextUserPtr(GladGLContext *context, GLADuserptrloadfunc load, void *userptr) {
    int version;

    context->GetString = (PFNGLGETSTRINGPROC) load(userptr, "glGetString");
    if(context->GetString == NULL) return 0;
    version = glad_gl_find_core_gl(context);

    glad_gl_load_GL_VERSION_1_0(context, load, userptr);
    glad_gl_load_GL_VERSION_1_1(context, load, userptr);
    glad_gl_load_GL_VERSION_1_2(context, load, userptr);

    if (!glad_gl_find_extensions_gl(context)) return 0;
    glad_gl_load_GL_EXT_fog_coord(context, load, userptr);
    glad_gl_load_GL_KHR_debug(context, load, userptr);



    return version;
}


int gladLoadGLContext(GladGLContext *context, GLADloadfunc load) {
    return gladLoadGLContextUserPtr(context, glad_gl_get_proc_from_userptr, GLAD_GNUC_EXTENSION (void*) load);
}



 


#ifdef __cplusplus
}
#endif
