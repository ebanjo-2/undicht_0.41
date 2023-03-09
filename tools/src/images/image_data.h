#ifndef IMAGE_DATA_H
#define IMAGE_DATA_H

#include "vector"
#include "cstdint"
#include "cstdlib"
#include "glm/glm.hpp"

namespace undicht {

    namespace tools {
        
        template<typename PIXEL_TYPE>
        class ImageData {
            // data for a 2D image
          protected:

            uint32_t _width;
            uint32_t _height;
            uint32_t _nr_channels = 4;

            std::vector<PIXEL_TYPE> _pixels;

          public:

            ImageData() = default;
            ImageData(uint32_t width, uint32_t height, uint32_t nr_channels);

            void setExtent(uint32_t width, uint32_t height);
            void setNrChannels(uint32_t nr_channels); // channels per pixel

            void setPixels(const PIXEL_TYPE* pixels, size_t num_pixels);
            void setPixel(const PIXEL_TYPE* pixel, uint32_t x, uint32_t y);

            uint32_t getWidth() const;
            uint32_t getHeight() const;
            uint32_t getNrChannels() const;

            /// @return nullptr if x or y are outside the images extent
            PIXEL_TYPE* getPixel(uint32_t x, uint32_t y) const;

            /// @brief sample the image for a given uv coordinate (range [0, 1])
            /// for values outside [0, 1] the texture will repeat itself
            PIXEL_TYPE* getPixel(float u, float v) const;

            /// @brief sample the image by linearly interpolating between the nearest pixels to the u v coordinates
            std::vector<PIXEL_TYPE> sampleLinear(float u, float v) const;

            /// @brief sample a equirectangular map in a given direction
            PIXEL_TYPE* getPixel(const glm::vec3& dir) const;

            /// @return a pointer to the internal pixel buffer
            const PIXEL_TYPE* getPixelData() const;

            /// @return the number of bytes used by the pixels in this image
            const uint32_t getPixelDataSize() const;

        };

    } // tools

} // undicht

#endif // IMAGE_DATA_H