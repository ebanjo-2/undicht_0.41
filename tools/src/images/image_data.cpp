#include "image_data.h"
#include "cstdint"

namespace undicht {

    namespace tools {

        template class ImageData<char>;
        template class ImageData<float>; // hdr images

        template<typename PIXEL_TYPE>
        ImageData<PIXEL_TYPE>::ImageData(uint32_t width, uint32_t height, uint32_t nr_channels) {

            setExtent(width, height);
            setNrChannels(nr_channels);
        }

        template<typename PIXEL_TYPE>
        void ImageData<PIXEL_TYPE>::setExtent(uint32_t width, uint32_t height) {

            _width = width;
            _height = height;
            _pixels.resize(width * height * _nr_channels);
        }

        template<typename PIXEL_TYPE>
        void ImageData<PIXEL_TYPE>::setNrChannels(uint32_t nr_channels) {
            // channels per pixel

            _nr_channels = nr_channels;
            _pixels.resize(_width * _height * _nr_channels);
        } 

        template<typename PIXEL_TYPE>
        void ImageData<PIXEL_TYPE>::setPixels(const PIXEL_TYPE* pixels, size_t num_pixels) {
            
            _pixels.clear();
            _pixels.insert(_pixels.begin(), pixels, pixels + num_pixels);
        }

        template<typename PIXEL_TYPE>
        void ImageData<PIXEL_TYPE>::setPixel(const PIXEL_TYPE* pixel, uint32_t x, uint32_t y) {
            
            for(int channel = 0; channel < _nr_channels; channel++) {

                _pixels.at((y * _width + x) * _nr_channels + channel) = pixel[channel];
            }
        }

        template<typename PIXEL_TYPE>
        uint32_t ImageData<PIXEL_TYPE>::getWidth() const {
            
            return _width;
        }

        template<typename PIXEL_TYPE>
        uint32_t ImageData<PIXEL_TYPE>::getHeight() const {
            
            return _height;
        }

        template<typename PIXEL_TYPE>
        uint32_t ImageData<PIXEL_TYPE>::getNrChannels() const {
            
            return _nr_channels;
        }
        
        template<typename PIXEL_TYPE>
        PIXEL_TYPE* ImageData<PIXEL_TYPE>::getPixel(uint32_t x, uint32_t y) const {
            /// @return nullptr if x or y are outside the images extent

            return (PIXEL_TYPE*)&_pixels.at((y * _width + x) * _nr_channels);
        }

        template<typename PIXEL_TYPE>
        PIXEL_TYPE* ImageData<PIXEL_TYPE>::getPixel(float u, float v) const{
            /// @brief sample the image for a given uv coordinate (range [0, 1])
            /// for values outside [0, 1] the texture will repeat itself

            uint32_t x = uint32_t(u * _width) % _width;
            uint32_t y = uint32_t(v * _height) % _height;

            return getPixel(x, y);
        }

        template<typename PIXEL_TYPE>
        PIXEL_TYPE* ImageData<PIXEL_TYPE>::getPixel(const glm::vec3& dir) const{
            /// @brief sample a equirectangular map in a given direction
            /// @param dir the direction
            /// @return a pointer to the pixel for the given direction

            // math taken from https://learnopengl.com/PBR/IBL/Diffuse-irradiance
            const glm::vec2 invAtan = glm::vec2(0.1591, 0.3183);
            glm::vec2 uv = glm::vec2(glm::atan(dir.z, dir.x), glm::asin(dir.y));
            uv *= invAtan;
            uv += 0.5;

            return getPixel(uv.x, uv.y);
        }

        template<typename PIXEL_TYPE>
        const PIXEL_TYPE* ImageData<PIXEL_TYPE>::getPixelData() const {
            /// @return a pointer to the internal pixel buffer

            return _pixels.data();
        }

        template<typename PIXEL_TYPE>
        const uint32_t ImageData<PIXEL_TYPE>::getPixelDataSize() const {
            /// @return the number of bytes used by the pixels in this image

            return _pixels.size() * sizeof(PIXEL_TYPE);
        }

    } // tools

} // undicht