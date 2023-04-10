#include "drv.h"

br_error DevicePixelmapGLBindFramebuffer(GLenum target, br_device_pixelmap *pm)
{
    GLuint fbo;

    if(pm == NULL)
        return BRE_FAIL;

    switch(pm->use_type) {
        case BRT_OFFSCREEN:
            fbo = pm->asBack.glFbo;
            break;
        case BRT_DEPTH:
            if(pm->asDepth.backbuffer == NULL)
                return BRE_FAIL;
            fbo = pm->asDepth.backbuffer->asBack.glFbo;
            break;
        default:
            return BRE_FAIL;
    }

    glBindFramebuffer(target, fbo);
    return BRE_OK;
}
