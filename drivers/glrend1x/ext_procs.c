#include "drv.h"
#include <brassert.h>

br_error DevicePixelmapGL1xExtCreateContext(br_device_pixelmap *self, br_device_gl_context_info *info)
{
    UASSERT(self->use_type == BRT_NONE);
    return self->asFront.ext_procs.create_context((br_pixelmap *)self, info, self->asFront.ext_procs.user);
}

void DevicePixelmapGL1xExtDeleteContext(br_device_pixelmap *self, void *ctx)
{
    UASSERT(self->use_type == BRT_NONE);
    self->asFront.ext_procs.delete_context((br_pixelmap *)self, ctx, self->asFront.ext_procs.user);
}

br_error DevicePixelmapGL1xExtMakeCurrent(br_device_pixelmap *self, void *ctx)
{
    UASSERT(self->use_type == BRT_NONE);
    return self->asFront.ext_procs.make_current((br_pixelmap *)self, ctx, self->asFront.ext_procs.user);
}

void DevicePixelmapGL1xExtSwapBuffers(br_device_pixelmap *self)
{
    UASSERT(self->use_type == BRT_NONE);
    self->asFront.ext_procs.swap_buffers((br_pixelmap *)self, self->asFront.ext_procs.user);
}

br_error DevicePixelmapGL1xExtResize(br_device_pixelmap *self, br_int_32 w, br_int_32 h)
{
    UASSERT(self->use_type == BRT_NONE);
    if(self->asFront.ext_procs.resize == NULL)
        return BRE_OK;

    return self->asFront.ext_procs.resize((br_pixelmap *)self, w, h, self->asFront.ext_procs.user);
}

static GLADapiproc GetProcAddressInternal(void *userptr, const char *name)
{
    br_device_pixelmap *self = userptr;
    UASSERT(self->use_type == BRT_NONE);
    return self->asFront.ext_procs.get_proc_address(name, self->asFront.ext_procs.user);
}

GLADuserptrloadfunc DevicePixelmapGL1xExtGetGetProcAddress(br_device_pixelmap *self)
{
    (void)self;
    return GetProcAddressInternal;
}

void DevicePixelmapGL1xExtPreSwap(br_device_pixelmap *self, GLuint fbo)
{
    UASSERT(self->use_type == BRT_NONE);
    if(self->asFront.ext_procs.preswap_hook != NULL)
        self->asFront.ext_procs.preswap_hook((br_pixelmap *)self, fbo, self->asFront.ext_procs.user);
}

void DevicePixelmapGL1xExtFree(br_device_pixelmap *self)
{
    UASSERT(self->use_type == BRT_NONE);
    if(self->asFront.ext_procs.free != NULL)
        self->asFront.ext_procs.free((br_pixelmap *)self, self->asFront.ext_procs.user);
}

br_error DevicePixelmapGL1xExtHandleWindowEvent(br_device_pixelmap *self, void *arg)
{
    UASSERT(self->use_type == BRT_NONE);
    if(self->asFront.ext_procs.handle_window_event == NULL)
        return BRE_OK;

    return self->asFront.ext_procs.handle_window_event((br_pixelmap *)self, arg, self->asFront.ext_procs.user);
}
