/*
 * Copyright (c) 1993-1995 Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: template.h 1.1 1997/12/10 16:52:57 jon Exp $
 * $Locker: $
 *
 * per-device store of allocated token-value templates
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
    br_tv_template *rendererFacilityTemplate;
    br_tv_template *rendererTemplate;
    br_tv_template *rendererStateStoredTemplate;
    br_tv_template *geometryV1ModelTemplate;
    br_tv_template *geometryV1BucketsTemplate;
    br_tv_template *geometryPrimitivesTemplate;
    br_tv_template *geometryLightingTemplate;

    /*
     * Argument processing templates
     */
    br_tv_template *rendererNewTemplate;

    /*
     * Parts templates
     */
    br_tv_template *partCullTemplate;
    br_tv_template *partSurfaceTemplate;
    br_tv_template *partMatrixTemplate;
    br_tv_template *partEnableTemplate;
    br_tv_template *partHiddenSurfaceTemplate;
    br_tv_template *partBoundsTemplate;
    br_tv_template *partLightTemplate;
    br_tv_template *partClipTemplate;
};

#ifdef __cplusplus
};
#endif
#endif
