#ifndef UUID_H_
#define UUID_H_

#define DEVICE_VK_UUID_STRING_SIZE 37

typedef struct br_device_vk_uuid {
    br_uint_8 b[VK_UUID_SIZE];
} br_device_vk_uuid;


#endif /* UUID_H_ */
