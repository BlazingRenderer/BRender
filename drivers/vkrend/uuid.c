#include "drv.h"

void DeviceVkUUIDNil(br_device_vk_uuid *uuid)
{
    BrMemSet(uuid, 0, VK_UUID_SIZE);
}

void DeviceVkUUIDMax(br_device_vk_uuid *uuid)
{
    BrMemSet(uuid, 0xFF, sizeof(uuid->b));
}

char *DeviceVkRawUUIDToString(char buf[DEVICE_VK_UUID_STRING_SIZE], const br_uint_8 uuid[VK_UUID_SIZE])
{
    static const char *alphabet = "0123456789abcdef";

    char *s = buf;
    for(int i = 0; i < VK_UUID_SIZE; ++i) {
        uint8_t ch = uuid[i];

        if(i == 4 || i == 6 || i == 8 || i == 10)
            *s++ = '-';

        *s++ = alphabet[ch >> 4];
        *s++ = alphabet[ch & 0xF];
    }

    *s++ = '\0';
    return buf;
}

char *DeviceVkUUIDToString(char buf[DEVICE_VK_UUID_STRING_SIZE], const br_device_vk_uuid *uuid)
{
    return DeviceVkRawUUIDToString(buf, uuid->b);
}

char *DeviceVkResUUIDToString(void *vparent, const br_device_vk_uuid *uuid)
{
    char *s;

    if((s = BrResAllocate(vparent, DEVICE_VK_UUID_STRING_SIZE, BR_MEMORY_STRING)) == NULL)
        return NULL;

    return DeviceVkUUIDToString(s, uuid);
}

br_error DeviceVkUUIDParse(br_device_vk_uuid *uuid, const char *s)
{
    br_size_t  len;
    br_uint_8 *u;

    /*
     * No need to fire up a br_lexer for this.
     */
    if((len = BrStrLen(s)) != 36)
        return BRE_INVALID;

    u = uuid->b;
    for(br_size_t i = 0; i < len; ++i) {
        unsigned long val;
        char          c[3] = {'\0', '\0', '\0'};
        char         *e    = NULL;

        if(i == 8 || i == 13 || i == 18 || i == 23) {
            if(s[i] != '-')
                return BRE_INVALID;

            continue;
        }

        c[0] = s[i++];
        c[1] = s[i];

        val = BrStrToUL(c, &e, 16);
        if(e != c + 2)
            return BRE_INVALID;

        *u++ = val;
    }

    return BRE_OK;
}

int DeviceVkRawUUIDCompare(const br_uint_8 a[VK_UUID_SIZE], const br_uint_8 b[VK_UUID_SIZE])
{
    return BrMemCmp(a, b, VK_UUID_SIZE);
}

int DeviceVkUUIDCompare(const br_device_vk_uuid *a, const br_device_vk_uuid *b)
{
    return DeviceVkRawUUIDCompare(a->b, b->b);
}

br_boolean DeviceVkUUIDIsNil(const br_device_vk_uuid *uuid)
{
    br_device_vk_uuid u;
    DeviceVkUUIDNil(&u);
    return DeviceVkUUIDCompare(uuid, &u) == 0 ? BR_TRUE : BR_FALSE;
}

br_boolean DeviceVkUUIDIsMax(const br_device_vk_uuid *uuid)
{
    br_device_vk_uuid u;
    DeviceVkUUIDMax(&u);
    return DeviceVkUUIDCompare(uuid, &u) == 0 ? BR_TRUE : BR_FALSE;
}

void DeviceVkUUIDCopy(br_device_vk_uuid *dst, const br_device_vk_uuid *src)
{
    BrMemCpy(dst, src, VK_UUID_SIZE);
}
