#ifndef IMAGE_DATA_3D_H
#define IMAGE_DATA_3D_H

#include "cstdint"
#include "images/image_data.h"
#include "vector"

namespace undicht {

    namespace tools {

        template<typename PIXEL_TYPE>
        class ImageData3D {

          protected:

            uint32_t _width;
            uint32_t _height;
            uint32_t _depth;
            uint32_t _nr_channels = 4;

            std::vector<ImageData<PIXEL_TYPE>> _image_layers;

          public:

            ImageData3D() = default;
            ImageData3D(uint32_t width, uint32_t height, uint32_t depth);

            void setExtent(uint32_t width, uint32_t height, uint32_t depth);
            void setNrChannels(uint32_t nr_channels); // channels per pixel

            void setPixel(const PIXEL_TYPE* pixel, uint32_t x, uint32_t y, uint32_t z);

            uint32_t getWidth() const;
            uint32_t getHeight() const;
            uint32_t getDepth() const;
            uint32_t getNrChannels() const; // channels per pixel

            ImageData<PIXEL_TYPE>& getImageLayer(uint32_t depth) const;
            PIXEL_TYPE* getPixel(uint32_t x, uint32_t y, uint32_t z) const;

        };

    } // tools

} // undicht

#endif // IMAGE_DATA_3D_H