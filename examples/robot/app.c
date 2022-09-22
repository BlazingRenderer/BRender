/****************************************************************************
 *                                                                          *
 * SOURCE FILE NAME: App.c                                                  *
 *                                                                          *
 * DESCRIPTIVE NAME: Simple test harness for using BRender under SDL        *
 *                                                                          *
 * COMPONENT: Robot                                                         *
 *                                                                          *
 * COPYRIGHT: (C) Copyright Argonaut Software. 1994.  All rights reserved.  *
 *            (C) Copyright IBM, 1995.  All rights reserved.                *
 *                                                                          *
 * FUNCTIONS:                                                               *
 *    main                                                                  *
 *    BrPMWarning                                                           *
 *    BrPMError                                                             *
 *                                                                          *
 * BRENDER FUNCTIONS                                                        *
 *    BrBegin                                                               *
 *    BrEnd                                                                 *
 *    BrErrorHandlerSet                                                     *
 *    BrPixelmapFree                                                        *
 *    BrPixelmapLoad                                                        *
 *                                                                          *
 * DIVE FUNCTIONS:                                                          *
 *    DiveClose                                                             *
 *    DiveOpen                                                              *
 *    DiveQueryCaps                                                         *
 *    DiveSetupBlitter                                                      *
 *                                                                          *
 ****************************************************************************/

#include <stdlib.h>               /* Standard C information                 */
#include <SDL2/SDL.h>             /* Master include file for SDL            */
#include <brender.h>              /* Master include file for BRender        */

#include "world.h"                /* 3D world and views into it             */
#include "app.h"                  /* Application prototypes and defines     */

/*
 * Globals
 */
SDL_Window *sdlWindow;                   /* SDL window handle         */

/*
 * Rendering Thread Globals
 */
static brwin_world *BrenderWorld;
static brwin_view  *BrenderView;

/*
 * Mouse tracking info
 */
int    TrackingFlags = 0;
POINTL TrackingValues[4];

#ifdef _DEBUG
int trianglesInHierarchyCount = 0;
int trianglesRenderedCount    = 0;
int verticesRenderedCount     = 0;
int trianglesDrawnCount       = 0;
int faceGroupCount            = 0;
int modelsDrawnCount          = 0;
#endif

static void BR_CALLBACK BrPMError(const char *message);

static void BR_CALLBACK BrPMWarning(const char *message);

br_diaghandler BrPMErrorHandler = {
    "Presentation Manager ErrorHandler",
    BrPMWarning,
    BrPMError,
};

/*
 * Primitive heap - used by z-buffered renderer to defer drawing of blended primitives
 */
static char primitive_heap[1500 * 1024];

/*
 * FUNCTION NAME: main
 *
 * DESCRIPTION:   Main routine...initializes window and message queue
 *
 */
int main(int argc, char **argv)
{
    SDL_GLContext *sdlContext;                  /* SDL OpenGL context        */

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0);

    /*
     * Init SDL
     */
    if(SDL_Init(SDL_INIT_VIDEO) < 0) {
        BrLogError("SDL", "Initialisation error: %s", SDL_GetError());
        goto sdl_init_failed;
    }

    /* Ask for a 3.2 Core context with no multisampling on the window. */
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0);

    const int width  = 1280; // 450
    const int height = 720; // 350

    if((sdlWindow = SDL_CreateWindow("BRender Sample Application", 10, 10, width, height, SDL_WINDOW_OPENGL)) == NULL) {
        SDL_ShowSimpleMessageBox(
            SDL_MESSAGEBOX_INFORMATION,
            "ROBOT.EXE - Robot Sample",
            "usage:  The sample can not run on this system environment.",
            NULL
        );
        goto sdl_createwindow_failed;
    }

    sdlContext = SDL_GL_CreateContext(sdlWindow);
    SDL_GL_MakeCurrent(sdlWindow, sdlContext);

    /*
     * Initialize the renderer
     */
    BrBegin();

    {
        char args[256];
        snprintf(args, BR_ASIZE(args), "WINDOW_HANDLE=0x%" PRIxPTR, (intptr_t)sdlWindow);

        struct br_device *BR_EXPORT BrDrvGLBegin(const char *arguments);

        BrDevAddStatic(NULL, BrDrvGLBegin, args);
    }

    if(getenv("BRENDER_PATH") == NULL)
        putenv("BRENDER_PATH=dat;../dat;../../dat;../../../dat");

    BrErrorHandlerSet(&BrPMErrorHandler);

    /*
     * Create the 3D world
     */
    BrenderWorld = WorldAllocate();
    BrenderView  = calloc(sizeof(*BrenderView), 1);
    BrenderView->world    = BrenderWorld;
    BrenderView->ulWidth  = width;
    BrenderView->ulHeight = height;

    ViewBufferSet(BrenderView);

    BrRendererBegin(BrenderView->colour_buffer, NULL, NULL, primitive_heap, sizeof(primitive_heap));

    memset(&TrackingFlags, 0, sizeof(TrackingFlags));

    for(;;) {
        SDL_Event evt;
        while(SDL_PollEvent(&evt) > 0) {
            switch(evt.type) {
                case SDL_QUIT:
                    goto done;
                case SDL_MOUSEBUTTONDOWN:
                case SDL_MOUSEBUTTONUP: {
                    int flag = 0;
                    if(evt.button.button == SDL_BUTTON_LEFT)
                        flag |= MOUSETRACK_LEFT;
                    else if(evt.button.button == SDL_BUTTON_RIGHT)
                        flag |= MOUSETRACK_RIGHT;

                    if(evt.button.state == SDL_RELEASED) {
                        TrackingFlags &= ~flag;
                    }
                    else {
                        TrackingFlags |= flag;
                    }
                    break;
                }
                case SDL_MOUSEMOTION: {
                    if(TrackingFlags) {
                        // FIXME: This is completely wrong
                        //fprintf(stderr, "%d, %d, %d, %d\n", evt.motion.x, evt.motion.y, evt.motion.xrel, evt.motion.yrel);
                        TrackingValues[TrackingFlags].x = evt.motion.x - BrenderView->ulWidth / 2;
                        TrackingValues[TrackingFlags].y = (BrenderView->ulHeight - evt.motion.y) - BrenderView->ulHeight / 2;
                    }
                    break;
                }
            }
        }

        WorldUpdate(BrenderView->world);
        ViewRender(BrenderView);
        ViewScreenUpdate(BrenderView);
    }

done:
    BrRendererEnd();
    free(BrenderView);

    /*
     * Close down renderer
     */
    BrEnd();

    SDL_DestroyWindow(sdlWindow);

sdl_createwindow_failed:
    /*
     * Close down SDL
     */
    SDL_Quit();
sdl_init_failed:
    return 1;
}

/*
 * FUNCTION NAME: BrPMWarning
 *
 * DESCRIPTION:   BRender Error handling function
 *
 */
static void BR_CALLBACK BrPMWarning(const char *message)
{
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_WARNING, "WARNING", message, NULL);
    exit(10);
} /* End of BrPMWarning() */


/*
 * FUNCTION NAME: BrPMError
 *
 * DESCRIPTION:   BRender Error handling function
 *
 */
static void BR_CALLBACK BrPMError(const char *message)
{
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "ERROR", message, NULL);
    exit(10);
} /* End of BrPMError() */
