/*
 * Per-device store of allocated token-value templates
 */
#ifndef _TEMPLATE_H_
#define _TEMPLATE_H_

#ifdef __cplusplus
extern "C" {
#endif

struct device_templates {
    br_tv_template *deviceTemplate;
    br_tv_template *devicePixelmapTemplate;
    br_tv_template *deviceClutTemplate;
    br_tv_template *outputFacilityTemplate;
};

#ifdef __cplusplus
};
#endif
#endif
