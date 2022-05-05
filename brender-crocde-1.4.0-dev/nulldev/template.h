/*
 * per-device store of allocated token-value templates
 */
#ifndef _TEMPLATE_H_
#define _TEMPLATE_H_

struct device_templates {
    /*
     * Object query templates
     */
    struct br_tv_template *deviceTemplate;
};

#endif /* _TEMPLATE_H_ */
