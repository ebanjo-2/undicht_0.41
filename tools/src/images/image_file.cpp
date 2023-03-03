#include "image_file.h"
#include "stb_image.h"
#include "debug.h"

namespace undicht {

    namespace tools {

        ImageFile::ImageFile(const std::string &file_name, ImageData<char> &data) {

            loadImage(file_name, data);
        }

        ImageFile::ImageFile(const std::string &file_name, ImageData<float> &data) {

            loadImage(file_name, data);
        }

        bool ImageFile::loadImage(const std::string &file_name, ImageData<char> &data) {

            int width, height, nr_channels;

            stbi_set_flip_vertically_on_load(false);
            unsigned char* tmp = stbi_load(file_name.data(), &width, &height, &nr_channels, STBI_rgb_alpha);
            nr_channels = 4; // forced by STBI_rgb_alpha

            if(!tmp) {
                UND_ERROR << "failed to read image file: " << file_name << "\n";
                return false;
            }

            data.setExtent(width, height);
            data.setNrChannels(nr_channels);
            uint32_t image_size = width * height * nr_channels;
            data.setPixels((char*)tmp, image_size);

            stbi_image_free(tmp);

            return true;
        }

        bool ImageFile::loadImage(const std::string &file_name, ImageData<float> &data) {

            int width, height, nr_channels;

            stbi_set_flip_vertically_on_load(false);
            float* tmp = stbi_loadf(file_name.data(), &width, &height, &nr_channels, STBI_rgb_alpha);
            nr_channels = 4; // forced by STBI_rgb_alpha

            if(!tmp) {
                UND_ERROR << "failed to read image file: " << file_name << "\n";
                return false;
            }

            data.setExtent(width, height);
            data.setNrChannels(nr_channels);
            uint32_t image_size = width * height * nr_channels;
            data.setPixels((float*)tmp, image_size);

            stbi_image_free(tmp);

            return true;
        }

    } // tools

} // undicht