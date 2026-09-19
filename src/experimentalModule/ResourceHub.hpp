#ifndef RESOURCE_HUB_HPP_
#define RESOURCE_HUB_HPP_

#include <cstdint>

//! @brief The level of prioritisation of a resource
enum class ResourcePriority : uint8_t {
    UNLOADED, // No resources acquired: explicitly unloaded, or owned by an inner ModularBody (or child of it) while the camera is outside its area of influence
    LAZY, // Only minimal resources shall be loaded, in background (default)
    BACKGROUND, // High resolution will probably be needed (lower resolution in use)
    PRELOAD, // High resolution needed in the near future (preload request)
    ACTIVE, // Currently needed (ex: missing resolution expected, resource in use)
};

#endif /* end of include guard: RESOURCE_HUB_HPP_ */
