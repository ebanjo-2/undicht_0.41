#include "cube_map_data.h"

namespace undicht {

    namespace tools {

        template class CubeMapData<char>;
        template class CubeMapData<float>; // hdr images

        // defining the directions on every cube map face
        using glm::vec3; // +x, -x, +y (down), -y(up), +z, -z
        const std::array<vec3, 6> CUBE_MAP_DIRS   =   {vec3(1,0,0), vec3(-1,0,0), vec3(0,1,0), vec3(0,-1,0), vec3(0,0,1),  vec3(0,0,-1)};
        const std::array<vec3, 6> CUBE_MAP_UPS    =   {vec3(0,-1,0), vec3(0,-1,0),  vec3(0,0,1), vec3(0,0,-1), vec3(0,-1,0),  vec3(0,-1,0)};
        const std::array<vec3, 6> CUBE_MAP_RIGHTS =   {vec3(0,0,-1), vec3(0,0,1), vec3(1,0,0), vec3(1,0,0),  vec3(1,0,0), vec3(-1,0,0)};

        template<typename PIXEL_TYPE>
        CubeMapData<PIXEL_TYPE>::CubeMapData(uint32_t extent, uint32_t nr_channels) {

            setExtent(extent);
            setNrChannels(nr_channels);
        }

        template<typename PIXEL_TYPE>
        void CubeMapData<PIXEL_TYPE>::setExtent(uint32_t extent) {
            
            _extent = extent;

            for(ImageData<PIXEL_TYPE>& face : _faces)
                face.setExtent(extent, extent);

        }

        template<typename PIXEL_TYPE>
        void CubeMapData<PIXEL_TYPE>::setNrChannels(uint32_t nr_channels) {

            _nr_channels = nr_channels;

            for(ImageData<PIXEL_TYPE>& face : _faces)
                face.setNrChannels(nr_channels);

        }

        template<typename PIXEL_TYPE>
        void CubeMapData<PIXEL_TYPE>::setPixels(const ImageData<PIXEL_TYPE>& equirectangular_map) {
            /// @brief convert an equirectangular map to a cubemap (both can describe 360 degrees of an environment)

            // resize the cube maps faces
            setNrChannels(equirectangular_map.getNrChannels());

            for(int face = 0; face < 6; face++) { // six faces of the cubemap

                for(int x = 0; x < _extent; x++) { // going through every pixel on the cube map face
                    for(int y = 0; y < _extent; y++) {
                        
                        // calculate the direction in which the source map should be sampled
                        glm::vec3 sample_dir = calcDir(x, y, (Face)face);

                        // store the pixel in the cubemap face
                        const PIXEL_TYPE* pixel = equirectangular_map.getPixel(sample_dir);
                        getFace((Face)face).setPixel(pixel, x, y);
        
                    }
                }
            }

        }

        template<typename PIXEL_TYPE>
        void CubeMapData<PIXEL_TYPE>::setPixel(const PIXEL_TYPE* pixel, const glm::vec3& dir) {

            PIXEL_TYPE* _pixel = getPixel(dir);

            for(int i = 0; i < _nr_channels; i++) {
                _pixel[i] = pixel[i];
            }

        }

        template<typename PIXEL_TYPE>
        void CubeMapData<PIXEL_TYPE>::setFace(const PIXEL_TYPE* pixels, size_t num_pixels, Face face) {
            
            _faces.at(face).setPixels(pixels, num_pixels);
        }

        template<typename PIXEL_TYPE>
        uint32_t CubeMapData<PIXEL_TYPE>::getExtent() const {
            
            return _extent;
        }

        template<typename PIXEL_TYPE>
        uint32_t CubeMapData<PIXEL_TYPE>::getNrChannels() const {

            return _nr_channels;        
        }

        template<typename PIXEL_TYPE>
        PIXEL_TYPE* CubeMapData<PIXEL_TYPE>::getPixel(const glm::vec3& dir) const {

            // determine the face the dir is pointing at
            Face face;
            glm::vec3 abs_dir = glm::abs(dir); // absolute values of the components in the dir vector
            if((abs_dir.x > abs_dir.y) && (abs_dir.x >= abs_dir.z) && (dir.x > 0)) face = X_POSITIVE; // +x
            else if((abs_dir.x >= abs_dir.y) && (abs_dir.x >= abs_dir.z) && (dir.x < 0)) face = X_NEGATIVE; // -x
            else if((abs_dir.y >= abs_dir.x) && (abs_dir.y >= abs_dir.z) && (dir.y > 0)) face = Y_POSITIVE; // +y
            else if((abs_dir.y >= abs_dir.x) && (abs_dir.y >= abs_dir.z) && (dir.y < 0)) face = Y_NEGATIVE; // -y
            else if((abs_dir.z >= abs_dir.x) && (abs_dir.z >= abs_dir.y) && (dir.z > 0)) face = Z_POSITIVE; // +z
            else if((abs_dir.z >= abs_dir.x) && (abs_dir.z >= abs_dir.y) && (dir.z < 0)) face = Z_NEGATIVE; // -z
            // i know that there are quite a few boolean expressions here that are unnessecary, but its more readable this way imo

            // making sure the dir vector "touches" the cube map face
            glm::vec3 point_on_face = dir / glm::dot(dir, CUBE_MAP_DIRS.at(face));

            // calculating the uv coords for the point_on_face
            float u = glm::dot(point_on_face, CUBE_MAP_RIGHTS.at(face)) + 1.0f; // should also be devided by 2.0f
            float v = glm::dot(point_on_face, CUBE_MAP_UPS.at(face)) + 1.0f; // but doing >> 1 on the uint32_t should be faster
            u = glm::clamp(u, 0.0f, 2.0f); // clamp because im scared of float inaccuracies
            v = glm::clamp(v, 0.0f, 2.0f);

            // calculating the pixel coords
            uint32_t x = uint32_t(u * (_extent - 1)) >> 1; 
            uint32_t y = uint32_t(v * (_extent - 1)) >> 1;

            return _faces.at(face).getPixel(x, y);
        }

        template<typename PIXEL_TYPE>
        ImageData<PIXEL_TYPE>& CubeMapData<PIXEL_TYPE>::getFace(Face face) const {
            
            return (ImageData<PIXEL_TYPE>&)_faces.at(face);
        }

        template<typename PIXEL_TYPE>
        const PIXEL_TYPE* CubeMapData<PIXEL_TYPE>::getFaceData(Face face) const {
            
            return _faces.at((int)face).getPixelData();
        }

        template<typename PIXEL_TYPE>
        uint32_t CubeMapData<PIXEL_TYPE>::getFaceDataSize() const {
            
            return _extent * _extent * _nr_channels * sizeof(PIXEL_TYPE);
        }

        template<typename PIXEL_TYPE>
        glm::vec3 CubeMapData<PIXEL_TYPE>::calcDir(int x, int y, Face face) const {
            /// @brief calculate the direction that points at the pixel at pixel coords (x,y) on the specified face

            const float texel_size = 1.0f / _extent; // size of one "pixel" on the cubemap (in uv coords)

            glm::vec3 dir = CUBE_MAP_DIRS.at(face);
            glm::vec3 right = CUBE_MAP_RIGHTS.at(face);
            glm::vec3 up = CUBE_MAP_UPS.at(face);
            glm::vec3 sample_dir = dir + ((x * texel_size) - 0.5f) * 2.0f * right + ((y * texel_size) - 0.5f) * 2.0f * up;
            sample_dir = glm::normalize(sample_dir);

            return sample_dir;
        }

        template<typename PIXEL_TYPE>
        glm::vec3 CubeMapData<PIXEL_TYPE>::calcDir(float u, float v, Face face) const {
            /// @brief calculate the direction that points at the pixel at uv coords (u,v) on the specified face

            glm::vec3 dir = CUBE_MAP_DIRS.at(face);
            glm::vec3 right = CUBE_MAP_RIGHTS.at(face);
            glm::vec3 up = CUBE_MAP_UPS.at(face);
            glm::vec3 sample_dir = dir + (u - 0.5f) * 2.0f * right + (v - 0.5f) * 2.0f * up;
            sample_dir = glm::normalize(sample_dir);

            return sample_dir;
        }

    } // tools

} // undicht