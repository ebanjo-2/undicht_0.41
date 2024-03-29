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
            uint32_t y = uint32_t(v * _height ) % _height;

            return getPixel(x, y);
        }

        template<typename PIXEL_TYPE>
        std::vector<PIXEL_TYPE> ImageData<PIXEL_TYPE>::sampleLinear(float u, float v) const {
            /// @brief sample the image by linearly interpolating between the nearest pixels to the u v coordinates

            // translate the uv coords to the [0, 1] range
            u = u - floor(u);
            v = v - floor(v);

            float pos_on_pixel_x = u * _width - floor(u * _width); // 0 to 1
            float pos_on_pixel_y = v * _height - floor(v * _height);

            uint32_t x0 = uint32_t(u * _width) % _width; // pixel closest to the uv coord
            uint32_t y0 = uint32_t(v * _height) % _height;
            uint32_t x1;
            uint32_t y1;

            float x0_weight;
            float y0_weight;
            float x1_weight;
            float y1_weight;

            if(pos_on_pixel_x < 0.5f) {
                x1 = (x0 == 0 ? _width - 1 : x0 - 1); // x coord of the other pixel that influenzes the mixed color
                x0_weight = pos_on_pixel_x + 0.5f;
            } else {
                x1 = (x0 == _width - 1 ? 0 : x0 + 1);
                x0_weight = (1.0f - pos_on_pixel_x) + 0.5f;
            }

            if(pos_on_pixel_y < 0.5f) {
                y1 = (y0 == 0 ? _height - 1 : y0 - 1); // y coord of the other pixel that influenzes the mixed color
                y0_weight = pos_on_pixel_y + 0.5f;
            } else {
                y1 = (y0 == _height - 1 ? 0 : y0 + 1);
                y0_weight = (1.0f - pos_on_pixel_y) + 0.5f;
            }

            x1_weight = 1.0f - x0_weight;
            y1_weight = 1.0f - y0_weight;

            std::vector<PIXEL_TYPE> final_color;

            for(int i = 0; i < _nr_channels; i++) {

                PIXEL_TYPE channel = 0;
                channel += getPixel(x0, y0)[i] * x0_weight * y0_weight;
                channel += getPixel(x0, y1)[i] * x0_weight * y1_weight;
                channel += getPixel(x1, y0)[i] * x1_weight * y0_weight;
                channel += getPixel(x1, y1)[i] * x1_weight * y1_weight;
                final_color.push_back(channel);
            }

            return final_color;
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