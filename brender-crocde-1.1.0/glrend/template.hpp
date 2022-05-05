#ifndef _TEMPLATE_HPP_
#define _TEMPLATE_HPP_

/* Driver-wide template store */
struct device_templates_gl
{
	/*
	* Object query templates
	*/
	struct br_tv_template *device;
    struct br_tv_template *outputFacility;
    struct br_tv_template *devicePixelmap;
    struct br_tv_template *rendererFacility;
    struct br_tv_template *renderer;
    struct br_tv_template *rendererStateStored;
    struct br_tv_template *bufferStored;
    struct br_tv_template *geometryV1Model;
    struct br_tv_template *geometryV1Buckets;
    struct br_tv_template *geometryPrimitives;
    struct br_tv_template *geometryLighting;
    struct br_tv_template *geometryStored;

	//
	///*
	//* Argument processing templates
	//*/
	struct br_tv_template *pixelmapNew;
	struct br_tv_template *pixelmapMatch;
	struct br_tv_template *rendererNew;
	//
	///*
	//* Parts templates
	//*/
	//struct br_tv_template *partCullTemplate;
	//struct br_tv_template *partSurfaceTemplate;
	//struct br_tv_template *partMatrixTemplate;
	//struct br_tv_template *partEnableTemplate;
	//struct br_tv_template *partHiddenSurfaceTemplate;
	//struct br_tv_template *partBoundsTemplate;
	//struct br_tv_template *partLightTemplate;
	//struct br_tv_template *partClipTemplate;
	//struct br_tv_template *partPrimitiveTemplate;
	//struct br_tv_template *partOutputTemplate;
};


#endif /* _TEMPLATE_HPP_ */

