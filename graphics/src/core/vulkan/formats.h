#ifndef FORMATS_H
#define FORMATS_H

#include "types.h"
#include "vulkan/vulkan.h"

namespace undicht {

	namespace vulkan {

		// functions for translating between vulkan and undicht formats

        VkFormat translate(const FixedType& t);
        FixedType translate(const VkFormat& format);

	} // vulkan

} // undicht

#endif // FORMATS_H
