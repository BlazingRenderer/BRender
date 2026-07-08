/*
 * Per-device store of allocated token-value templates
 */
#ifndef _TEMPLATE_H_
#define _TEMPLATE_H_

#ifdef __cplusplus
extern "C" {
#endif

struct device_templates {
    /*
     * Object query templates
     */
    br_tv_template *deviceTemplate;
    br_tv_template *outputFacilityTemplate;
    br_tv_template *devicePixelmapTemplate;
    br_tv_template *devicePixelmapSubTemplate;
    br_tv_template *devicePixelmapFrontTemplate;
    br_tv_template *deviceClutTemplate;
    br_tv_template *primitiveStateTemplate;

    /*
     * Argument processing templates
     */
    br_tv_template *pixelmapNewTemplate;
    br_tv_template *pixelmapMatchTemplate;

    /*
     * Parts templates
     */
    br_tv_template *partPrimitiveTemplate;
    br_tv_template *partOutputTemplate;
};

#ifdef __cplusplus
};
#endif
#endif
