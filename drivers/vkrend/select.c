/*
 * Device Selection Routines
 */
#include <inttypes.h>
#include "drv.h"

typedef struct br_device_vk_select_state {
    uint32_t          phy_count;
    VkPhysicalDevice *phy;
    br_device_vk_phy *phy_info;
} br_device_vk_select_state;

static const char *type2str(VkPhysicalDeviceType type)
{
    switch(type) {
        case VK_PHYSICAL_DEVICE_TYPE_OTHER:
            return "Other";
        case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
            return "Integrated";
        case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
            return "Discrete";
        case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
            return "Virtual";
        case VK_PHYSICAL_DEVICE_TYPE_CPU:
            return "Software";
        default:
            return "Unknown";
    }
}

static void log_phy(const br_device_vk_select_state *state)
{
    BrLogInfo("VKREND", "Found %u devices:", state->phy_count);
    for(uint32_t i = 0; i < state->phy_count; ++i) {
        char              uu_device[DEVICE_VK_UUID_STRING_SIZE];
        char              uu_driver[DEVICE_VK_UUID_STRING_SIZE];
        br_device_vk_phy *phy = state->phy_info + i;

        DeviceVkRawUUIDToString(uu_device, phy->id_props.deviceUUID);
        DeviceVkRawUUIDToString(uu_driver, phy->id_props.driverUUID);

        BrLogInfo("VKREND", "[%u] Device Name:    %s", i, phy->props2.properties.deviceName);
        BrLogInfo("VKREND", "    Device UUID:    %s", uu_device);
        BrLogInfo("VKREND", "    Device Vendor:  %" PRIx32, phy->props2.properties.vendorID);
        BrLogInfo("VKREND", "    Device Type:    %s", type2str(phy->props2.properties.deviceType));
        BrLogInfo("VKREND", "    Driver UUID:    %s", uu_driver);
        BrLogInfo("VKREND", "    Queue Families: %u", phy->qf_count);

        for(uint32_t q = 0; q < phy->qf_count; ++q) {
            const VkQueueFamilyProperties *qp = phy->qf + q;
            BrLogInfo("VKREND", "      Family %d: %d queue(s), graphics = %s, transfer = %s, compute = %s", q, qp->queueCount,
                      qp->queueFlags & VK_QUEUE_GRAPHICS_BIT ? "Y" : "N", qp->queueFlags & VK_QUEUE_TRANSFER_BIT ? "Y" : "N",
                      qp->queueFlags & VK_QUEUE_COMPUTE_BIT ? "Y" : "N");
        }
        BrLogInfo("VKREND", "    Rank:           %d", phy->rank);
    }
}

static VkResult query_phy(const GladVulkanContext *vk, br_device_vk_select_state *state, VkInstance instance)
{
    VkResult vr;

    if((vr = vk->EnumeratePhysicalDevices(instance, &state->phy_count, NULL)) != VK_SUCCESS)
        return vr;

    state->phy      = BrResAllocate(state, sizeof(VkPhysicalDevice) * state->phy_count, BR_MEMORY_SCRATCH);
    state->phy_info = BrResAllocate(state, sizeof(br_device_vk_phy) * state->phy_count, BR_MEMORY_SCRATCH);

    if((vr = vk->EnumeratePhysicalDevices(instance, &state->phy_count, state->phy)) != VK_SUCCESS)
        return vr;

    for(uint32_t i = 0; i < state->phy_count; ++i) {
        br_device_vk_phy *phy = state->phy_info + i;

        phy->dev      = state->phy[i];
        phy->index    = i;
        phy->id_props = (VkPhysicalDeviceIDProperties){
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ID_PROPERTIES,
        };
        phy->props2 = (VkPhysicalDeviceProperties2){
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2,
            .pNext = &phy->id_props,
        };
        phy->qf_gfx = UINT32_MAX;
        phy->rank   = 0;

        vk->GetPhysicalDeviceProperties2(phy->dev, &phy->props2);

        vk->GetPhysicalDeviceQueueFamilyProperties(phy->dev, &phy->qf_count, NULL);

        phy->qf = BrResAllocate(state->phy_info, sizeof(VkQueueFamilyProperties) * phy->qf_count, BR_MEMORY_SCRATCH);

        vk->GetPhysicalDeviceQueueFamilyProperties(phy->dev, &phy->qf_count, phy->qf);

        /*
         * Assign a score/rank to each device.
         */
        switch(phy->props2.properties.deviceType) {
            case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
                phy->rank += 10000;
                break;
            case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
                phy->rank += 1000;
                break;
            case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
                phy->rank += 100;
                break;
            case VK_PHYSICAL_DEVICE_TYPE_CPU:
                phy->rank += 10;
                break;
            default:
                break;
        }

        for(uint32_t q = 0; q < phy->qf_count; ++q) {
            const VkQueueFamilyProperties *qp = phy->qf + q;

            if(qp->queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                phy->rank += 1;
                phy->qf_gfx = q;
            }
        }
    }

    return VK_SUCCESS;
}

static const br_device_vk_phy *select_device(const br_device_vk_select_state *state, const br_device_vk_select *select)
{
    const br_device_vk_phy *physical_device = NULL;
    int                     rank;
    char                    uu[DEVICE_VK_UUID_STRING_SIZE];

    /*
     * Special case - the user's explicitly requested a device. Bail if not found.
     */
    if(DeviceVkUUIDIsNil(&select->device_uuid) == BR_FALSE) {
        DeviceVkUUIDToString(uu, &select->device_uuid);
        BrLogInfo("VKREND", "Requested device: %s", uu);

        for(uint32_t i = 0; i < state->phy_count; ++i) {
            const br_device_vk_phy *phy = state->phy_info + i;

            if(DeviceVkRawUUIDCompare(select->device_uuid.b, phy->id_props.deviceUUID) == 0) {
                physical_device = phy;
                goto found_phy;
            }
        }

        BrLogError("VKREND", "Requested device not found.");
        return NULL;
    }

    /*
     * Get the device with the highest score.
     */
    rank = 0;
    for(uint32_t i = 0; i < state->phy_count; ++i) {
        const br_device_vk_phy *phy = state->phy_info + i;

        if(phy->rank > rank) {
            rank            = phy->rank;
            physical_device = phy;
        }
    }

    if(physical_device == NULL) {
        BrLogError("VKREND", "No candidate devices.");
        return NULL;
    }

found_phy:

    DeviceVkRawUUIDToString(uu, physical_device->id_props.deviceUUID);
    BrLogInfo("VKREND", "Selected physical device: %s, rank = %d", uu, physical_device->rank);

    if(physical_device->qf_gfx == UINT32_MAX) {
        BrLogError("VKREND", "Device has no graphics queues, cannot continue.");
        return NULL;
    }

    return physical_device;
}

br_device_vk_phy *DeviceVkSelect(void *res, const GladVulkanContext *vk, VkInstance instance, const br_device_vk_select *select)
{
    VkResult                   vr;
    br_device_vk_select_state *state;
    const br_device_vk_phy    *phy;
    br_device_vk_phy          *phy2;

    state = BrResAllocate(res, sizeof(br_device_vk_select_state), BR_MEMORY_SCRATCH);

    /*
     * Query the physical devices.
     */
    if(query_phy(vk, state, instance) != VK_SUCCESS) {
        BrResFree(state);
        return NULL;
    }

    /*
     * Log them for good measure.
     */
    log_phy(state);

    if((phy = select_device(state, select)) == NULL) {
        BrResFree(state);
        return NULL;
    }

    phy2                 = BrResAllocate(res, sizeof(br_device_vk_phy), BR_MEMORY_DRIVER);
    *phy2                = *phy;
    phy2->props2.pNext   = &phy2->id_props;
    phy2->id_props.pNext = NULL;
    BrResAdd(phy2, BrResRemove(phy->qf));

    BrResFree(state);
    return phy2;
}
