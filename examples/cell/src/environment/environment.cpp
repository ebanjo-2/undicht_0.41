#include "environment.h"
#include "vector"
#include "array"
#include "IBL/ibl.h"
#include "core/vulkan/formats.h"
#include "debug.h"
#include "images/image_file.h"

namespace cell {

    using namespace undicht;
    using namespace vulkan;
    using namespace tools;

    void Environment::init(const undicht::vulkan::LogicalDevice& gpu) {

        _env_cube_map.setExtent(_env_cube_map_size, _env_cube_map_size, 1, 6);
        _env_cube_map.setFormat(translate(UND_VEC4F)); // hdr (could be VEC4F16)
        _env_cube_map.setCubeMap(true);
        _env_cube_map.init(gpu);

        _irradiance_map.setExtent(_irradiance_map_size, _irradiance_map_size, 1, 6);
        _irradiance_map.setFormat(translate(UND_VEC4F)); // hdr (could be VEC4F16)
        _irradiance_map.setCubeMap(true);
        _irradiance_map.init(gpu);

        _specular_prefilter_map.setExtent(_specular_prefilter_map_size, _specular_prefilter_map_size, 1, 6);
        _specular_prefilter_map.setFormat(translate(UND_VEC4F)); // hdr (could be VEC4F16)
        _specular_prefilter_map.setMipMaps(true, false, _specular_prefilter_mip_levels);
        _specular_prefilter_map.setCubeMap(true); // reflections for rougher surfaces will be stored in higher mip levels
        _specular_prefilter_map.init(gpu);

    }

    void Environment::cleanUp() {

        _env_cube_map.cleanUp();
        _irradiance_map.cleanUp();
        _specular_prefilter_map.cleanUp();
    }

    void Environment::load(const std::string& file_name) {
        /// loads the environment map from the hdr spherical map
        /// and calculates the lighting maps based on it

        ImageData<float> hdr_sphere;
        ImageFile(file_name, hdr_sphere);

        UND_LOG << "loaded the spherical map from the file\n";
        
        // generate cubemap faces from the loaded environment map
        CubeMapData<float> env_map;
        env_map.setExtent(_env_cube_map_size);
        env_map.setPixels(hdr_sphere);

        // store the faces in the cubemap
        for(int i = 0; i < 6; i++) {
            const ImageData<float>& face = env_map.getFace((CubeMapData<float>::Face)i);
            _env_cube_map.setData((const char*)face.getPixelData(), face.getPixelDataSize(), i);
        }

        UND_LOG << "finished loading the skybox\n";

        // generate convoluted environment map (irradiance map)
        CubeMapData<float> irradiance_map;
        convoluteCubeMap(env_map, irradiance_map, _irradiance_map_size);

        // store the faces in the irradiance cubemap
        for(int i = 0; i < 6; i++) {
            const ImageData<float>& face = irradiance_map.getFace((CubeMapData<float>::Face)i);
            _irradiance_map.setData((const char*)face.getPixelData(), face.getPixelDataSize(), i);
        }

        UND_LOG << "finished calculating the diffuse lighting from the environment\n";

        // pre filter the environment for specular reflections
        std::vector<CubeMapData<float>> prefilter_mip_maps;
        prefilterSpecularReflections(env_map, prefilter_mip_maps, _specular_prefilter_map_size, _specular_prefilter_mip_levels);

        for(int mip_level = 0; mip_level < _specular_prefilter_mip_levels; mip_level++) {
            for(int i = 0; i < 6; i++) {
                const ImageData<float>& face = prefilter_mip_maps.at(mip_level).getFace((CubeMapData<float>::Face)i);
                _specular_prefilter_map.setData((const char*)face.getPixelData(), face.getPixelDataSize(), i, mip_level);
            }
        }

        UND_LOG << "finished calculating the specular reflection maps\n";

    }

    const undicht::vulkan::Texture& Environment::getSkyBox() const {

        return _env_cube_map;
    }

    const undicht::vulkan::Texture& Environment::getIrradiance() const {

        return _irradiance_map;
    }

    const undicht::vulkan::Texture& Environment::getSpecular() const {

        return _specular_prefilter_map;
    }

} // cell