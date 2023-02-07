#ifndef IMAGE_FILE_H
#define IMAGE_FILE_H

#include "string"
#include "vector"

namespace undicht {

    namespace tools {

        struct ImageData {
            std::vector<char> _pixels;
            uint32_t _width = 0;
            uint32_t _height = 0;
            uint32_t _nr_channels = 0;
        };

        struct HDRImageData {
            std::vector<float> _pixels;
            uint32_t _width = 0;
            uint32_t _height = 0;
            uint32_t _nr_channels = 0;
        };

        class ImageFile {
        public:

            ImageFile() = default;
            ImageFile(const std::string& file_name, ImageData& data);
            ImageFile(const std::string& file_name, HDRImageData& data);

            bool loadImage(const std::string& file_name, ImageData& data);
            bool loadImage(const std::string& file_name, HDRImageData& data);

        };

    } // tools

} // undicht

#endif // IMAGE_FILE_H