/*
 * Per-device store of allocated token-value templates
 */
#ifndef TEMPLATE_H_
#define TEMPLATE_H_

#ifdef __cplusplus
extern "C" {
#endif

struct device_templates {
    /*
     * Object query templates
     */
    br_tv_template *deviceTemplate;
    br_tv_template *outputFacilityTemplate;
    br_tv_template *devicePixelmapFrontTemplate;
    br_tv_template *devicePixelmapTemplate;
    br_tv_template *rendererFacilityTemplate;

    br_tv_template *deviceAllocateTemplate;
    br_tv_template *pixelmapNewTemplate;
    br_tv_template *pixelmapMatchTemplate;
    br_tv_template *rendererNewTemplate;
};

#ifdef __cplusplus
};
#endif
#endif /* TEMPLATE_H_ */
