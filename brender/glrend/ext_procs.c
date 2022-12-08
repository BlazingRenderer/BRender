#include "drv.h"

void *DeviceGLCreateContext(br_device *self)
{
    return self->ext_procs.create_context(self, self->ext_procs.user);
}

void DeviceGLDeleteContext(br_device *self, void *ctx)
{
    self->ext_procs.delete_context(self, ctx, self->ext_procs.user);
}

br_error DeviceGLMakeCurrent(br_device *self, void *ctx)
{
    return self->ext_procs.make_current(self, ctx, self->ext_procs.user);
}

void DeviceGLSwapBuffers(br_device *self, br_device_pixelmap *pm)
{
    self->ext_procs.swap_buffers(self, pm, self->ext_procs.user);
}

br_device_gl_getprocaddress_cbfn *DeviceGLGetGetProcAddress(br_device *self)
{
    return self->ext_procs.get_proc_address;
}

void DeviceGLPreSwap(br_device *self, GLuint fbo)
{
    if(self->ext_procs.preswap_hook != NULL)
        self->ext_procs.preswap_hook(self, fbo, self->ext_procs.user);
}
