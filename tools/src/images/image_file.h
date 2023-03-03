#ifndef IMAGE_FILE_H
#define IMAGE_FILE_H

#include "string"
#include "vector"
#include "images/image_data.h"

namespace undicht {

    namespace tools {

        class ImageFile {
        public:

            ImageFile() = default;
            ImageFile(const std::string& file_name, ImageData<char>& data);
            ImageFile(const std::string& file_name, ImageData<float>& data);

            bool loadImage(const std::string& file_name, ImageData<char>& data);
            bool loadImage(const std::string& file_name, ImageData<float>& data);

        };

    } // tools

} // undicht

#endif // IMAGE_FILE_H