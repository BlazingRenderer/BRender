#ifndef _DRV_IP_HPP_
#define _DRV_IP_HPP_

#include "gl_core_3_2.h"
#include <GLFW/glfw3.h>

#include "drv.hpp"
#include "devpixmp.hpp"
#include "rendfcty.hpp"
#include "renderer.hpp"
#include "gv1buckt.hpp"
#include "gprim.hpp"
#include "lighting.hpp"
#include "gv1model.hpp"
#include "gprim.hpp"
#include "lighting.hpp"
#include "sstate.hpp"
#include "sbuffer.hpp"
#include "gv1model.hpp"

br_device_gl *DeviceGLAllocate(const char *arguments);
br_error OutputFacilityGLEnumerate(br_device_gl *device);
br_device_pixelmap_gl *DevicePixelmapGLAllocate(br_device_gl *device, br_output_facility_gl *facility, br_token_value *tv);
br_renderer_facility_gl *RendererFacilityGLInit(br_output_facility_gl *outfcty);
br_renderer_gl *RendererGLAllocate(br_device_gl *device, br_renderer_facility_gl *facility, br_device_pixelmap_gl *dest);
br_geometry_v1_buckets_gl *GeometryV1BucketsGLAllocate(br_renderer_facility_gl *type, const char *id);
br_geometry_primitives_gl * GeometryPrimitivesGLAllocate(br_renderer_facility_gl *type, const char *id);
br_geometry_lighting_gl *GeometryLightingGLAllocate(br_renderer_facility_gl *type, const char *id);
br_geometry_v1_model_gl *GeometryV1ModelGLAllocate(br_renderer_facility_gl *type, const char *id);
br_geometry_stored_gl *GeometryStoredGLAllocate(br_geometry_v1_model_gl *gv1model, const char *id, struct v11model_f *model);
br_renderer_state_stored_gl *RendererStateStoredGLAllocate(br_renderer_gl *renderer, HGLSTATE_STACK base_state, br_uint_32 m, br_token_value *tv);
br_buffer_stored_gl *BufferStoredGLAllocate(br_renderer_gl *renderer, br_token use, br_device_pixelmap *pm, br_token_value *tv);

void StoredGLRenderGroups(br_geometry_stored_gl *self, br_renderer_gl *renderer, grflags rflags, bool depthOverride);
#endif /* _DRV_IP_HPP_ */