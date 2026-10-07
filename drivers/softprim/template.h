/*
 * softprim per-device store of allocated token-value templates
 */
#ifndef _SOFTPRIM_TEMPLATE_H_
#define _SOFTPRIM_TEMPLATE_H_

#ifdef __cplusplus
extern "C" {
#endif

struct device_templates {
    /*
     * Object query templates
     */
    struct br_tv_template *deviceTemplate;
    struct br_tv_template *primitiveLibraryTemplate;
    struct br_tv_template *primitiveStateTemplate;
    struct br_tv_template *bufferStoredTemplate;

    /*
     * Parts templates
     */
    struct br_tv_template *partPrimitiveTemplate;
    struct br_tv_template *partOutputTemplate;
};

#ifdef __cplusplus
};
#endif
#endif
