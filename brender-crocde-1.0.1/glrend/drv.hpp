#ifndef _DRV_HPP_
#define _DRV_HPP_

#define BR_DEVICE_PRIVATE
#define BR_OBJECT_PRIVATE

#include <brddi.h>
#include <token.h>
#include <tokenmap.h>
#include "device.hpp"
#include "object.hpp"

/*
* Macros that expand to the first two arguments of a template entry
* Builtin or device token
*/
#define BRT(t)	(br_token)BRT_##t,0
#define DEV(t)	0,#t


#endif /* _DRV_HPP_ */
