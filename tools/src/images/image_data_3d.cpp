#include "image_data_3d.h"

namespace undicht {
    
    namespace tools {

        template class ImageData3D<char>;
        template class ImageData3D<float>; // hdr images

        template<typename PIXEL_TYPE>
        ImageData3D<PIXEL_TYPE>::ImageData3D(uint32_t width, uint32_t height, uint32_t depth) {
            
            setExtent(width, height, depth);
        }

        template<typename PIXEL_TYPE>
        void ImageData3D<PIXEL_TYPE>::setExtent(uint32_t width, uint32_t height, uint32_t depth) {
            
            _width = width;
            _height = height;
            _depth = depth;

            _image_layers.resize(depth);

            for(ImageData<PIXEL_TYPE>& image : _image_layers) {
                image.setExtent(width, height);
                image.setNrChannels(_nr_channels);
            }

        }
        
        template<typename PIXEL_TYPE>
        void ImageData3D<PIXEL_TYPE>::setNrChannels(uint32_t nr_channels) {
            // channels per pixel

            _nr_channels = nr_channels;

            for(ImageData<PIXEL_TYPE>& image : _image_layers) 
                image.setNrChannels(_nr_channels);
            
        }; 
        
        template<typename PIXEL_TYPE>
        void ImageData3D<PIXEL_TYPE>::setPixel(const PIXEL_TYPE* pixel, uint32_t x, uint32_t y, uint32_t z) {

            if((x < _width) && (y < _height) && (z < _depth)) {

                _image_layers.at(z).setPixel(pixel, x, y);

            }

        }
        
        template<typename PIXEL_TYPE>
        uint32_t ImageData3D<PIXEL_TYPE>::getWidth() const {
            
            return _width;
        }
        
        template<typename PIXEL_TYPE>
        uint32_t ImageData3D<PIXEL_TYPE>::getHeight() const {
            
            return _height;
        }
        
        template<typename PIXEL_TYPE>
        uint32_t ImageData3D<PIXEL_TYPE>::getDepth() const {
            
            return _depth;
        }
        
        template<typename PIXEL_TYPE>
        uint32_t ImageData3D<PIXEL_TYPE>::getNrChannels() const {
            // channels per pixel

            return _nr_channels;
        }
        
        template<typename PIXEL_TYPE>
        ImageData<PIXEL_TYPE>& ImageData3D<PIXEL_TYPE>::getImageLayer(uint32_t depth) const {
            
            return (ImageData<PIXEL_TYPE>&)_image_layers.at(depth);
        }
        
        template<typename PIXEL_TYPE>
        PIXEL_TYPE* ImageData3D<PIXEL_TYPE>::getPixel(uint32_t x, uint32_t y, uint32_t z) const {

            if((x < _width) && (y < _height) && (z < _depth)) 
                return _image_layers.at(z).getPixel(x, y);
            else
                return nullptr;

        }

        template<typename PIXEL_TYPE>
        PIXEL_TYPE* ImageData3D<PIXEL_TYPE>::getPixel(float u, float v, float w) const {
            /// @brief sample the image for a given uvw coordinate (range [0, 1])
            /// for values outside [0, 1] the texture will repeat itself   

            uint32_t x = uint32_t(u * _width + 0.5f) % _width;
            uint32_t y = uint32_t(v * _height + 0.5f) % _height;
            uint32_t z = uint32_t(w * _depth + 0.5f) % _depth;

            return getPixel(x, y, z);
        }

        template<typename PIXEL_TYPE>
        std::vector<PIXEL_TYPE> ImageData3D<PIXEL_TYPE>::sampleLinear(float u, float v, float w) const {
        // linear interpolation will only happen in uv direction

            uint32_t z = uint32_t(w * _depth + 0.5f) % _depth;

            return _image_layers.at(z).sampleLinear(u, v);
        }

    } // tools

} // namespace undicht 