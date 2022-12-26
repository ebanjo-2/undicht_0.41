#include "mtl_file.h"
#include "fstream"
#include "debug.h"

namespace undicht {

    namespace tools {

        MTLFile::MTLFile() {

        }

        MTLFile::MTLFile(const std::string& file_name) {
            
            open(file_name);

        }

        MTLFile::~MTLFile() {

        }

        bool MTLFile::open(const std::string& file_name) {

            std::ifstream file(file_name);
            _file_name = file_name;

			if (!file.is_open()) {
                UND_ERROR << "failed to open file: " << file_name << "\n";
				return false;
			}

            loadFileContent(file);

            return true;
        }

        int MTLFile::getMaterialID(const std::string& mat_name) const {
            // returns -1 if the material doesnt exist in the file

            for(int i = 0; i < _materials.size(); i++) {
                if(!_materials.at(i)._name.compare(mat_name)) {
                    return i;
                }
            }

            return -1;
        }

        const MTLFile::Material* MTLFile::getMaterial(uint32_t mat_id) const {
            // returns nullptr if the material doesnt exist in the file

            if(mat_id >= _materials.size()) 
                return nullptr;

            return &_materials.at(mat_id);
        }

        const std::vector<MTLFile::Material>& MTLFile::getMaterials() const {

            return _materials;
        }


        ////////////////////////////////////// private functions for loading the material file /////////////////////////////////

        void MTLFile::loadFileContent(std::ifstream& file) {
            
            std::string line;

            // finding the first material
            while(line.compare(0, 7, "newmtl "))
                std::getline(file, line);

            while(!file.eof()) {
                // current line contains material name
                Material new_mat;
                new_mat._name = line.substr(7, line.size() - 7);

                // loading the materials contents
                do {
                    std::getline(file, line);
                    char* str = (char*)line.data();

                    if(!line.compare(0, 3, "Ns "))
                        new_mat._ns = std::strtof(str + 3, nullptr);

                    if(!line.compare(0, 3, "Ni "))
                        new_mat._ni = std::strtof(str + 3, nullptr);

                    if(!line.compare(0, 2, "d "))
                        new_mat._d = std::strtof(str + 2, nullptr);
                    
                    if(!line.compare(0, 6, "illum "))
                        new_mat._illum = std::strtoul(str + 6, nullptr, 10);

                    if(!line.compare(0, 3, "Ka "))
                        for(int i = 0; i < 3; i++)
                            new_mat._ka[i] = std::strtof(str, &str);

                    if(!line.compare(0, 3, "Kd "))
                        for(int i = 0; i < 3; i++)
                            new_mat._kd[i] = std::strtof(str, &str);

                    if(!line.compare(0, 3, "Ks "))
                        for(int i = 0; i < 3; i++)
                            new_mat._ks[i] = std::strtof(str, &str);

                    if(!line.compare(0, 7, "map_Kd "))
                        new_mat._map_kd = line.substr(7, line.size() - 7);
                    
                    if(!line.compare(0, 9, "map_disp "))
                        new_mat._map_disp = line.substr(9, line.size() - 9);

                    if(!line.compare(0, 7, "map_Ka "))
                        new_mat._map_ka = line.substr(7, line.size() - 7);

                    if(!line.compare(0, 9, "map_bump "))
                        new_mat._map_bump = line.substr(9, line.size() - 9);

                } while(line.compare(0, 7, "newmtl ") && !file.eof());

                _materials.push_back(new_mat);

            }

        }


    } // tools

} // undicht