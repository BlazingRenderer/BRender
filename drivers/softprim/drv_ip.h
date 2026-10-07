/*
 * softprim internal prototypes
 */
#ifndef _SOFTPRIM_DRV_IP_H_
#define _SOFTPRIM_DRV_IP_H_

#ifndef NO_PROTOTYPES

#ifdef __cplusplus
extern "C" {
#endif

/*
 * object.c
 */
const char *BR_CMETHOD_DECL(br_object_softprim, identifier)(br_object *self);
br_device  *BR_CMETHOD_DECL(br_object_softprim, device)(br_object *self);

/*
 * device.c
 */
br_device *DeviceSoftPrimAllocate(const char *identifier);

/*
 * plib.c
 */
extern const br_token        PrimPartsTokens[];
struct br_primitive_library *PrimitiveLibrarySoftPrimAllocate(struct br_device *dev, const char *identifier, const char *arguments);

/*
 * pstate.c
 */
struct br_primitive_state *PrimitiveStateSoftPrimAllocate(struct br_primitive_library *plib);

/*
 * sbuffer.c
 */
struct br_buffer_stored *BufferStoredSoftPrimAllocate(struct br_primitive_library *plib, br_token use, struct br_device_pixelmap *pm,
                                                     br_token_value *tv);

/*
 * match.c
 */
br_error BR_CMETHOD_DECL(br_primitive_state_softprim, renderBegin)(struct br_primitive_state *self, struct brp_block **rpb, br_boolean *block_changed,
                                                                  br_boolean *ranges_changed, br_boolean no_render, br_token prim_type);

br_error BR_CMETHOD_DECL(br_primitive_state_softprim, renderEnd)(struct br_primitive_state *self, struct brp_block *pb);

br_error BR_CMETHOD_DECL(br_primitive_state_softprim, rangesQuery)(struct br_primitive_state *self, br_scalar *offset, br_scalar *scale,
                                                                  br_int_32 max_comp);

/*
 * Called by match.c with the current state's output buffers.
 */
void SoftPrimWorkUpdate(struct br_primitive_state *self);

/*
 * raster.cpp - draw the MMX tables' queued frames. pentprim's primitive library
 * flushes its rasteriser buffer here (plib.c), and the deferred triangles are
 * only drawn when it does.
 */
void SoftPrimMmxFlush(void);

#ifdef __cplusplus
};
#endif

#endif
#endif
