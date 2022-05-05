#include <cstddef>
#include <log.h>
#include "drv_ip.hpp"

extern "C" br_device *BrDrvGLBegin(const char *arguments)
{
	br_device_gl *dev = DeviceGLAllocate(arguments);
	if(!dev)
		return nullptr;

	/* List all the GLFW screen modes */
	if(OutputFacilityGLEnumerate(dev) != BRE_OK)
		return nullptr;

	return reinterpret_cast<br_device*>(dev);
}
