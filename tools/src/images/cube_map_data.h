#ifndef CUBE_MAP_DATA_H
#define CUBE_MAP_DATA_H

#include "image_data.h"
#include "array"
#include "glm/glm.hpp"

namespace undicht {

    namespace tools {

        const extern std::array<glm::vec3, 6> CUBE_MAP_DIRS;
        const extern std::array<glm::vec3, 6> CUBE_MAP_UPS;
        const extern std::array<glm::vec3, 6> CUBE_MAP_RIGHTS;

        template<typename PIXEL_TYPE>
        class CubeMapData {

          public:

            enum Face : int {
                X_POSITIVE = 0,
                X_NEGATIVE = 1,
                Y_POSITIVE = 2,
                Y_NEGATIVE = 3,
                Z_POSITIVE = 4,
                Z_NEGATIVE = 5,
            };

          protected:

            uint32_t _extent; // same in all 3 directions (aka "a cube")
            uint32_t _nr_channels = 4;
            std::array<ImageData<PIXEL_TYPE>, 6> _faces;

          public:

            CubeMapData() = default;
            CubeMapData(uint32_t extent, uint32_t nr_channels);

            void setExtent(uint32_t extent);
            void setNrChannels(uint32_t nr_channels);
            void setPixels(const ImageData<PIXEL_TYPE>& equirectangular_map);
            void setPixel(const PIXEL_TYPE* pixel, const glm::vec3& dir);
            void setFace(const PIXEL_TYPE* pixels, size_t num_pixels, Face face);

            uint32_t getExtent() const;
            uint32_t getNrChannels() const;
            PIXEL_TYPE* getPixel(const glm::vec3& dir) const;
            ImageData<PIXEL_TYPE>& getFace(Face face) const;
            const PIXEL_TYPE* getFaceData(Face face) const;
            uint32_t getFaceDataSize() const; // size of the pixel data per face in bytes

            /// @brief calculate the direction that points at the pixel at pixel coords (x,y) on the specified face
            glm::vec3 calcDir(int x, int y, Face face) const;
            glm::vec3 calcDir(float u, float v, Face face) const;

        };

    } // tools

} // undicht

#endif // CUBE_MAP_DATA_H