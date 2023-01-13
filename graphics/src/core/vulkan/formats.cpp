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
                {UND_FLOAT16, VK_FORMAT_R16_SFLOAT},
                {UND_FLOAT32, VK_FORMAT_R32_SFLOAT},
                {UND_FLOAT64, VK_FORMAT_R64_SFLOAT},
                {UND_UINT8, VK_FORMAT_R8_UINT},
                {UND_UINT16, VK_FORMAT_R16_UINT},
                {UND_UINT32, VK_FORMAT_R32_UINT},
                {UND_INT8, VK_FORMAT_R8_SINT},
                {UND_INT16, VK_FORMAT_R16_SINT},
                {UND_INT32, VK_FORMAT_R32_SINT},
                {UND_INT64, VK_FORMAT_R64_SINT},
                {UND_R8, VK_FORMAT_R8_SRGB},

                // types with 2 components
                {UND_VEC2F, VK_FORMAT_R32G32_SFLOAT},
                {UND_VEC2F16, VK_FORMAT_R16G16_SFLOAT},
                {UND_VEC2UI8, VK_FORMAT_R8G8_UINT},
                {FixedType(Type::FLOAT, 8, 2), VK_FORMAT_R64G64_SFLOAT},
                {FixedType(Type::INT, 1, 2), VK_FORMAT_R8G8_SINT},
                {FixedType(Type::INT, 2, 2), VK_FORMAT_R16G16_SINT},
                {FixedType(Type::INT, 4, 2), VK_FORMAT_R32G32_SINT},
                {FixedType(Type::INT, 8, 2), VK_FORMAT_R64G64_SINT},
                {UND_R8G8, VK_FORMAT_R8G8_SRGB},

                // types with 3 components
                {UND_VEC3F, VK_FORMAT_R32G32B32_SFLOAT},
                {UND_VEC3F16, VK_FORMAT_R16G16B16_SFLOAT},
                {UND_VEC3UI8, VK_FORMAT_R8G8B8_UINT},
                {FixedType(Type::FLOAT, 8, 3), VK_FORMAT_R64G64B64_SFLOAT},
                {FixedType(Type::INT, 1, 3), VK_FORMAT_R8G8B8_SINT},
                {FixedType(Type::INT, 2, 3), VK_FORMAT_R16G16B16_SINT},
                {FixedType(Type::INT, 4, 3), VK_FORMAT_R32G32B32_SINT},
                {FixedType(Type::INT, 8, 3), VK_FORMAT_R64G64B64_SINT},
                {UND_R8G8B8, VK_FORMAT_R8G8B8_SRGB},
                {UND_B8G8R8, VK_FORMAT_B8G8R8_SRGB},

                // types with 4 components
                {UND_VEC4F, VK_FORMAT_R32G32B32A32_SFLOAT},
                {UND_VEC4F16, VK_FORMAT_R16G16B16A16_SFLOAT},
                {UND_VEC4UI8, VK_FORMAT_R8G8B8A8_UINT},
                {FixedType(Type::FLOAT, 8, 4),VK_FORMAT_R64G64B64A64_SFLOAT},
                {FixedType(Type::INT, 1, 4), VK_FORMAT_R8G8B8A8_SINT},
                {FixedType(Type::INT, 2, 4), VK_FORMAT_R16G16B16A16_SINT},
                {FixedType(Type::INT, 4, 4), VK_FORMAT_R32G32B32A32_SINT},
                {FixedType(Type::INT, 8, 4), VK_FORMAT_R64G64B64A64_SINT},
                {UND_R8G8B8A8, VK_FORMAT_R8G8B8A8_SRGB},
                {UND_B8G8R8A8, VK_FORMAT_B8G8R8A8_SRGB},

                // depth buffer formats
                {UND_DEPTH32F, VK_FORMAT_D32_SFLOAT},
                {UND_DEPTH32F_STENCIL8, VK_FORMAT_D32_SFLOAT_S8_UINT},
                {UND_DEPTH24F_STENCIL8, VK_FORMAT_D24_UNORM_S8_UINT},
        };

        VkFormat translate(const FixedType& type) {

            for(const std::pair<FixedType, VkFormat>& p : FORMAT_DICTIONARY) {

                if(p.first == type)
                    return p.second;
            }

            // printing an error message
            std::string type_str;
            switch (type.m_type) {
                case Type::UNDEFINED : type_str = "UNDEFINED";break;
                case Type::INT : type_str = "INT";break;
                case Type::UNSIGNED_INT : type_str = "UNSIGNED_INT";break;
                case Type::FLOAT : type_str = "FLOAT";break;
                case Type::COLOR_BGRA : type_str = "COLOR_BGRA";break;
                case Type::COLOR_RGBA : type_str = "COLOR_RGBA";break;
                case Type::DEPTH_BUFFER : type_str = "DEPTH_BUFFER";break;
                case Type::DEPTH_STENCIL_BUFFER : type_str = "DEPTH_STENCIL_BUFFER";break;
            }

            UND_ERROR << "failed to translate format: " << type.m_num_components << " component(s) of type " << type_str << ", size of components: " << type.m_size << "\n";

            return VK_FORMAT_UNDEFINED;
        }

        FixedType translate(const VkFormat& format) {

            for(const std::pair<FixedType, VkFormat>& p : FORMAT_DICTIONARY) {

                if(p.second == format)
                    return p.first;
            }

            UND_ERROR << "failed to translate format: " << format << "\n";
            return UND_UNDEFINED_TYPE;
        }

	} // vulkan

} // undicht