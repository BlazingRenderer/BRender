/*
 * Per-device store of allocated token-value templates
 */
#ifndef _TEMPLATE_H_
#define _TEMPLATE_H_

#ifdef __cplusplus
extern "C" {
#endif

struct device_templates {
    struct br_tv_template *deviceTemplate;
    struct br_tv_template *devicePixelmapTemplate;
    struct br_tv_template *deviceClutTemplate;
    struct br_tv_template *outputFacilityTemplate;
    struct br_tv_template *pixelmapNewTemplate;
    struct br_tv_template *matchOffTemplate;
};

#ifdef __cplusplus
};
#endif
#endif
