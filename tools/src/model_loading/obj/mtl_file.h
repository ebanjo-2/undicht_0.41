#ifndef MTL_FILE_H
#define MTL_FILE_H

#include "stdint.h"
#include <string>
#include <vector>

namespace undicht {

    namespace tools {


        class MTLFile {
          public:
            // internal data structures

            struct Material {
                std::string _name;
                float _ns = 0.0f; // specular exponent
                float _ka[3] = {0.0f,0.0f,0.0f}; // ambient color
                float _kd[3] = {0.0f,0.0f,0.0f}; // diffuse color
                float _ks[3] = {0.0f,0.0f,0.0f}; // specular color
                float _ni = 1.0f; // index of refraction
                float _d = 0.0f; // transparency
                uint8_t _illum = 0; // illumination mode
                std::string _map_kd; // diffuse color texture
                std::string _map_disp; // displacement map
                std::string _map_ka; // ambient color texture
                std::string _map_bump; // bump map
            };

          protected:

            std::string _file_name;

            std::vector<Material> _materials;

          public:

            MTLFile();
            MTLFile(const std::string& file_name);
            virtual ~MTLFile();

            bool open(const std::string& file_name);

            // returns -1 if the material doesnt exist in the file
            int getMaterialID(const std::string& mat_name) const;
            // returns nullptr if the material doesnt exist in the file
            const Material* getMaterial(uint32_t mat_id) const;
            const std::vector<Material>& getMaterials() const;

          protected:
            // private functions for loading the material file

            void loadFileContent(std::ifstream& file);

        };

    } // tools

} // namespace undicht

#endif // MTL_FILE_H