#include "obj_file.h"
#include "fstream"
#include "debug.h"
#include "file_tools.h"
#include "images/image_file.h"

namespace undicht {

    namespace tools {

		OBJFile::OBJFile() {

        }

		OBJFile::OBJFile(const std::string& file_name) {

            open(file_name);
        }

		OBJFile::~OBJFile() {

        }

        bool OBJFile::open(const std::string& file_name) {

            std::ifstream file(file_name);
            _file_name = file_name;

			if (!file.is_open()) {
                UND_ERROR << "failed to open file: " << file_name << "\n";
				return false;
			}

            loadFileContent(file);

            return true;
        }

		////////////////////////////////////////// functions to load single meshes / textures ///////////////////////////////////////////

		int OBJFile::getMeshCount() {
		    /** @return the number of meshes stored in the file */

            return _objects.size();
        }


		int OBJFile::getTextureCount() {
		    /** @return the number of unique textures used by the meshes
		    * if a mesh should use for example a color + normal texture, thats 2 */

            return _mtl_file.getMaterials().size();
        }

		void OBJFile::getMesh(MeshData& loadTo_mesh, unsigned int id, bool load_positions, bool load_uvs, bool load_normals) {
		    /** loads the vertices of the mesh
		    * @param id: to iterate through the meshes of the file */

            std::vector<MeshData> all_meshes;
            loadAllMeshes(all_meshes, load_positions, load_uvs, load_normals);

            if(all_meshes.size() > id) {
                loadTo_mesh = all_meshes.at(id);
            } else {
                UND_ERROR << "failed to load mesh " << id << " from file " << _file_name << " : mesh doesnt exist\n";
            }

        }

		void OBJFile::getTexture(ImageData<char>& loadTo_texture, int id) {
		    /** @param id: to iterate through the texture of the file */

            std::vector<ImageData<char>> all_textures;

            loadAllTextures(all_textures);

            if(all_textures.size() > id) {
                loadTo_texture = all_textures.at(id);
            } else {
                UND_ERROR << "failed to load texture " << id << " for obj file: " << _file_name << " : texture doesnt exist\n";
            }

        }

		////////////////////////////////////////// functions to load all meshes / textures ///////////////////////////////////////////

		void OBJFile::loadAllMeshes(std::vector<MeshData>& loadTo_meshes, bool load_positions, bool load_uvs, bool load_normals) {
            
            load_positions = load_positions && _positions.size();
            load_uvs = load_uvs && _tex_coords.size();
            load_normals = load_normals && _normals.size();
    
            for(const Object& o: _objects) {

                MeshData data;

                if(load_positions) data.vertex_layout.m_types.push_back(UND_VEC3F);
                if(load_uvs) data.vertex_layout.m_types.push_back(UND_VEC2F);
                if(load_normals) data.vertex_layout.m_types.push_back(UND_VEC3F);

                for(const Group& g : o._groups) { // should only be one, might be 0?

                    data.color_texture = _mtl_file.getMaterialID(g._mtl); // do be done

                    for(const Face& f : g._faces) {

                        for(int vertex = 0; vertex < 3; vertex++) {
                            
                            if(f._vertex_ids.size() && load_positions) {
                                data.vertices.push_back(_positions.at(f._vertex_ids.at(vertex) - 1).x);
                                data.vertices.push_back(_positions.at(f._vertex_ids.at(vertex) - 1).y);
                                data.vertices.push_back(_positions.at(f._vertex_ids.at(vertex) - 1).z);
                            } else if (load_positions) {
                                data.vertices.push_back(0.0f);
                                data.vertices.push_back(0.0f);
                                data.vertices.push_back(0.0f);
                            }

                            if(f._tex_coord_ids.size() && load_uvs) {
                                data.vertices.push_back(_tex_coords.at(f._tex_coord_ids.at(vertex) - 1).u);
                                data.vertices.push_back(_tex_coords.at(f._tex_coord_ids.at(vertex) - 1).v);
                            } else if (load_uvs) {
                                data.vertices.push_back(0.0f);
                                data.vertices.push_back(0.0f);
                            }

                            if(f._normal_ids.size() && load_normals) {
                                data.vertices.push_back(_normals.at(f._normal_ids.at(vertex) - 1).x);
                                data.vertices.push_back(_normals.at(f._normal_ids.at(vertex) - 1).y);
                                data.vertices.push_back(_normals.at(f._normal_ids.at(vertex) - 1).z);
                            } else if (load_normals) {
                                data.vertices.push_back(0.0f);
                                data.vertices.push_back(0.0f);
                                data.vertices.push_back(0.0f);
                            }

                            
                        }

                    }

                }

                loadTo_meshes.push_back(data);
            }

        }

		void OBJFile::loadAllTextures(std::vector<ImageData<char>>& loadTo_textures) {

            for(const MTLFile::Material& mat : _mtl_file.getMaterials()) {
                
                // only loading the diffuse texture for now
                std::string image_file_name = mat._map_kd;
                ImageData<char> image_data;

                if(image_file_name.size()) {
                    ImageFile image_file(getFilePath(_file_name) + image_file_name, image_data);
                } else {
                    image_data.setExtent(1,1);
                    image_data.setNrChannels(4);
                    char pixel[] = {
                        mat._kd[0] * 255,
                        mat._kd[1] * 255,
                        mat._kd[2] * 255,
                        0
                    };
                    image_data.setPixel(pixel, 0, 0);
                }

                loadTo_textures.push_back(image_data);
            }

        }

        /////////////////////////////////////////////// private obj loading functions ///////////////////////////////////////////////

        void OBJFile::loadFileContent(std::ifstream& file) {
            // loads the content of the file into the internal data structures

            std::string line;
            std::string data;
            readNextLine(file, line);
            while(!file.eof()) {
                
                // mtllib
                while(readMtlLib(line, data)) {
                    _current_mtl_lib = data;
                    _mtl_file.open(getFilePath(_file_name) + data);
                    readNextLine(file, line);
                }

                // object
                while(readObject(line, data)) {
                    Object new_object;
                    new_object._name = data;
                    new_object._mtl_lib = _current_mtl_lib;
                    _objects.push_back(new_object);
                    readNextLine(file, line);
                }

                // creating a default object if no object was specified in the file
                if(!_objects.size()) {
                    Object new_object;
                    new_object._name = "default";
                    new_object._mtl_lib = _current_mtl_lib;
                    _objects.push_back(new_object);
                }

                // vertex positions
                while(readVertexPos(line, _positions))
                    readNextLine(file, line);
                        
                // tex coords
                while(readTexCoord(line, _tex_coords))
                    readNextLine(file, line);

                // vertex normals
                while(readVertexNorm(line, _normals))
                    readNextLine(file, line);

                // group
                while(readGroup(line, data)) {
                    Group new_group;
                    new_group._name = data;
                    _objects.back()._groups.push_back(new_group);
                    readNextLine(file, line);
                }

                // creating a default group if no group was specified in the file
                if(!_objects.back()._groups.size()) {
                    Group new_group;
                    new_group._name = "default";
                    _objects.back()._groups.push_back(new_group);
                }

                // usemtl
                while(readUseMtl(line, data)) {
                    _objects.back()._groups.back()._mtl = data;
                    readNextLine(file, line);
                }

                // faces
                while(readFace(line, _objects.back()._groups.back())) {
                    readNextLine(file, line);
                }
            }

        }

        bool OBJFile::readNextLine(std::ifstream& file, std::string& line) {
            // reads the next line that is a valid line (i.e. not a comment line or other lines that currently cant be processed)
            // returns false if the end of the file is reached

            while(!file.eof()) {
                std::getline(file, line);
                if(!line.compare(0, 7, "mtllib ")) break;
                if(!line.compare(0, 2, "o ")) break;
                if(!line.compare(0, 2, "g ")) break;
                if(!line.compare(0, 7, "usemtl ")) break;
                if(!line.compare(0, 2, "v ")) break;
                if(!line.compare(0, 3, "vt ")) break;
                if(!line.compare(0, 3, "vn ")) break;
                if(!line.compare(0, 2, "f ")) break;
            }
            
            if(file.eof())
                return false;
            else
                return true;
        }

        ///////////////////////// reading lines (return false if the line does not contain the right data) /////////////////////////

        bool OBJFile::readMtlLib(const std::string& line, std::string& mtl_lib) {

            if(line.compare(0, 7, "mtllib "))
                return false; // not a line with a material lib

            mtl_lib = line.substr(7, line.size() - 7);

            return true;
        }

        bool OBJFile::readObject(const std::string& line, std::string& obj_name) {

            if(line.compare(0, 2, "o "))
                return false; // not a line with a object name

            obj_name = line.substr(2, line.size() - 2);

            return true;
        }

        bool OBJFile::readGroup(const std::string& line, std::string& group_name) {

            if(line.compare(0, 2, "g "))
                return false; // not a line with a group name

            group_name = line.substr(2, line.size() - 2);

            return true;
        }

        bool OBJFile::readUseMtl(const std::string& line, std::string& mtl_name) {
            if(line.compare(0, 7, "usemtl "))
                return false; // not a line with a usemtl command

            mtl_name = line.substr(7, line.size() - 7);

            return true;
        }

        bool OBJFile::readVertexPos(const std::string& line, std::vector<VertexPos>& loadTo) {
            // reading vertex data (return false if the line does not contain the right data)

            if(line.compare(0, 2, "v "))
                return false; // not a line with vertex position data
            
            // extracting the vertex positions from the line
            float pos[3];
            char* str = (char*)line.data() + 2;
            for(int i = 0; i < 3; i++) {
                pos[i] = std::strtof(str, &str); // strtof sets the second param to point to the char after the float that was extracted
            }            

            // storing the vertex positions
            loadTo.push_back({pos[0], pos[1], pos[2]});

            return true;
        }

        bool OBJFile::readTexCoord(const std::string& line, std::vector<TexCoord>& loadTo) {

            if(line.compare(0, 3, "vt "))
                return false; // not a line with tex coord data

            // extracting the tex coords from the line
            float uv[2];
            char* str = (char*)line.data() + 3;
            for(int i = 0; i < 2; i++) {
                uv[i] = std::strtof(str, &str); // strtof sets the second param to point to the char after the float that was extracted
            }

            // storing the tex coord
            loadTo.push_back({uv[0], uv[1]});

            return true;
        }

        bool OBJFile::readVertexNorm(const std::string& line, std::vector<VertexNorm>& loadTo) {

            if(line.compare(0, 3, "vn "))
                return false; // not a line with normal data

            // extracting the normal from the line
            float normal[3];
            char* str = (char*)line.data() + 3;
            for(int i = 0; i < 3; i++) {
                normal[i] = std::strtof(str, &str); // strtof sets the second param to point to the char after the float that was extracted
            }

            // storing the normal
            loadTo.push_back({normal[0], normal[1], normal[2]});

            return true;
        }

        bool OBJFile::readFace(const std::string& line, Group& loadTo) {

            if(line.compare(0, 2, "f "))
                return false; // not a line with face data

            Face new_face;
            char* str = (char*)line.data() + 1;
            for(int i = 0; i < 3; i++) {
                for(int j = 0; j < 3; j++) {
                    
                    str++; // moving past the first delimiter
                    if(str[0] == '/') continue; // attribute was not specified

                    int index = std::strtoul(str, &str, 10);

                    if(j == 0) new_face._vertex_ids.push_back(index);
                    if(j == 1) new_face._tex_coord_ids.push_back(index);
                    if(j == 2) new_face._normal_ids.push_back(index);

                    if(str[0] == ' ') break; // no further attributes were specified
                
                }
            }

            loadTo._faces.push_back(new_face);

            return true;
        }


    } // tools

} // undicht