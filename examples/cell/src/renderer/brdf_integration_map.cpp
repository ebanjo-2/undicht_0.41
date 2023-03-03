#include "brdf_integration_map.h"
#include "core/vulkan/formats.h"

#include "IBL/ibl.h"
#include "debug.h"

#include "file_tools.h"

#include "binary_data/binary_data_file.h"

namespace cell {

    using namespace undicht;
    using namespace vulkan;
    using namespace tools;

    // the file in which the calculated brdf integration map is stored for faster loading in the future
    const std::string BRDF_INTEGRATION_FILE = getFilePath(UND_CODE_SRC_FILE) + "brdf_int_map.und"; // not sure about the file ending

    void BRDFIntegrationMap::init(const undicht::vulkan::LogicalDevice& gpu) {

        _brdf_integration_map.setExtent(_brdf_integration_map_size, _brdf_integration_map_size);
        _brdf_integration_map.setFormat(translate(UND_VEC2F)); // could be VEC2F16
        _brdf_integration_map.init(gpu);

        // load the data
        BinaryDataFile file;
        std::vector<char> buffer;
        size_t bytes_to_load = _brdf_integration_map_size * _brdf_integration_map_size * 2 * sizeof(float);

        ImageData<float> brdf_map;

        if(file.open(BRDF_INTEGRATION_FILE) && file.read(0, buffer) && (buffer.size() == bytes_to_load)) {
            // load the data for the brdf map from the file (should be a lot faster than calculating the brdf integrals every time)
            brdf_map.setPixels((float*)&buffer.at(0), bytes_to_load / sizeof(float));
            UND_LOG << "finished loading the brdf data \n";
        } else {
            // calculate the data for the brdf map
            createBRDFIntegrationMap(brdf_map, _brdf_integration_map_size);
            file.newBinaryFile();
            size_t location = file.store((char*)brdf_map.getPixelData(), sizeof(float) * brdf_map.getPixelDataSize());
            UND_LOG << "finished calculating the brdf data, stored it at: " << location << "\n";
        }
        
        // load the data to the texture
        _brdf_integration_map.setData((const char*)brdf_map.getPixelData(), brdf_map.getPixelDataSize());

    }

    void BRDFIntegrationMap::cleanUp() {

        _brdf_integration_map.cleanUp();
    }

    const undicht::vulkan::Texture& BRDFIntegrationMap::getMap() const {

        return _brdf_integration_map;
    }

}