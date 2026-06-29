/*
 * Private primitive library structure
 */
#ifndef _PRIMLIB_H_
#define _PRIMLIB_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct br_primitive_library {
    const struct br_primitive_library_dispatch *dispatch;
    const char                                 *identifier;
    struct br_device                           *device;
    void                                       *object_list;
    const GladGLContext                        *gl;
    GLuint                                      tex_white;
} br_primitive_library;

#ifdef __cplusplus
};
#endif
#endif
