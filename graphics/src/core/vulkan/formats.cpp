#include "formats.h"
#include "debug.h"

#include "vector"


namespace undicht {
	
	namespace vulkan {

		const int UND_VERTEX_SHADER = 100;
		const int UND_FRAGMENT_SHADER = 101;

        const std::vector<std::pair<FixedType, VkFormat>> FORMAT_DICTIONARY {
                {UND_UNDEFINED_TYPE, VK_FORMAT_UNDEFINED},

                // types with 1 component
                {UND_FLOAT32, VK_FORMAT_R32_SFLOAT},
                {UND_FLOAT64, VK_FORMAT_R64_SFLOAT},
                {UND_INT8, VK_FORMAT_R8_SINT},
                {UND_INT16, VK_FORMAT_R16_SINT},
                {UND_INT32, VK_FORMAT_R32_SINT},
                {UND_INT64, VK_FORMAT_R64_SINT},
                {UND_R8, VK_FORMAT_R8_SRGB},

                // types with 2 components
                {UND_VEC2F, VK_FORMAT_R32G32_SFLOAT},
                {FixedType(Type::FLOAT, 8, 2), VK_FORMAT_R64G64_SFLOAT},
                {FixedType(Type::INT, 1, 2), VK_FORMAT_R8G8_SINT},
                {FixedType(Type::INT, 2, 2), VK_FORMAT_R16G16_SINT},
                {FixedType(Type::INT, 4, 2), VK_FORMAT_R32G32_SINT},
                {FixedType(Type::INT, 8, 2), VK_FORMAT_R64G64_SINT},
                {UND_R8G8, VK_FORMAT_R8G8_SRGB},

                // types with 3 components
                {UND_VEC3F, VK_FORMAT_R32G32B32_SFLOAT},
                {FixedType(Type::FLOAT, 8, 3), VK_FORMAT_R64G64B64_SFLOAT},
                {FixedType(Type::INT, 1, 3), VK_FORMAT_R8G8B8_SINT},
                {FixedType(Type::INT, 2, 3), VK_FORMAT_R16G16B16_SINT},
                {FixedType(Type::INT, 4, 3), VK_FORMAT_R32G32B32_SINT},
                {FixedType(Type::INT, 8, 3), VK_FORMAT_R64G64B64_SINT},
                {UND_R8G8B8, VK_FORMAT_R8G8B8_SRGB},
                {UND_B8G8R8, VK_FORMAT_B8G8R8_SRGB},

                // types with 4 components
                {UND_VEC4F, VK_FORMAT_R32G32B32A32_SFLOAT},
                {FixedType(Type::FLOAT, 8, 4),VK_FORMAT_R64G64B64A64_SFLOAT},
                {FixedType(Type::INT, 1, 4), VK_FORMAT_R8G8B8A8_SINT},
                {FixedType(Type::INT, 2, 4), VK_FORMAT_R16G16B16A16_SINT},
                {FixedType(Type::INT, 4, 4), VK_FORMAT_R32G32B32A32_SINT},
                {FixedType(Type::INT, 8, 4), VK_FORMAT_R64G64B64A64_SINT},
                {UND_R8G8B8A8, VK_FORMAT_R8G8B8A8_SRGB},
                {UND_B8G8R8A8, VK_FORMAT_B8G8R8A8_SRGB},
        };

        VkFormat translate(const FixedType& type) {

            for(const std::pair<FixedType, VkFormat>& p : FORMAT_DICTIONARY) {

                if(p.first == type)
                    return p.second;
            }

            UND_ERROR << "failed to translate format\n";
            return VK_FORMAT_UNDEFINED;
        }

        FixedType translate(const VkFormat& format) {

            for(const std::pair<FixedType, VkFormat>& p : FORMAT_DICTIONARY) {

                if(p.second == format)
                    return p.first;
            }

            UND_ERROR << "failed to translate format\n";
            return UND_UNDEFINED_TYPE;
        }

	} // vulkan

} // undicht