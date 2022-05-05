#ifndef _TEMPLATE_HPP_
#define _TEMPLATE_HPP_

struct device_templates_gl
{
	///*
	//* Object query templates
	//*/
	struct br_tv_template *deviceTemplate;
	struct br_tv_template *outputFacilityTemplate;
	struct br_tv_template *devicePixelmapTemplate;
	struct br_tv_template *rendererFacilityTemplate;
	struct br_tv_template *rendererTemplate;
	struct br_tv_template *rendererStateStoredTemplate;
	struct br_tv_template *bufferStoredTemplate;
	struct br_tv_template *geometryV1ModelTemplate;
	struct br_tv_template *geometryV1BucketsTemplate;
	struct br_tv_template *geometryPrimitivesTemplate;
	struct br_tv_template *geometryLightingTemplate;
	//
	///*
	//* Argument processing templates
	//*/
	//struct br_tv_template *pixelmapNewTemplate;
	//struct br_tv_template *pixelmapMatchTemplate;
	struct br_tv_template *rendererNewTemplate;
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

