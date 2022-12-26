#ifndef OBJ_FILE_H
#define OBJ_FILE_H

#include "model_loading/model_loader.h"
#include "string.h"
#include "vector"
#include "mtl_file.h"

namespace undicht {

    namespace tools {

        class OBJFile : public ModelLoader{
          
          protected:
            // internal data structures

            struct Face {
                // obj starts counting at 1!!!!!
                std::vector<uint32_t> _vertex_ids;
                std::vector<uint32_t> _tex_coord_ids; // (optional)
                std::vector<uint32_t> _normal_ids; // (optional)
            };

            struct Group {
                std::string _name; // group name
                std::string _mtl; // material
                // contains data about how to construct faces
                std::vector<Face> _faces;
            };

            struct VertexPos {
                float x;
                float y;
                float z;
                // there could be another one (w)
            };

            struct TexCoord {
                float u;
                float v;
            };

            struct VertexNorm {
                float x;
                float y;
                float z;
            };

            struct Object {
                std::string _name; // object name
                std::string _mtl_lib; // material lib
                // contains the face data
                std::vector<Group> _groups;
            };

          protected:
            
            std::string _file_name;
            std::vector<Object> _objects;

            // contains the raw vertex data
            std::vector<VertexPos> _positions;
            std::vector<TexCoord> _tex_coords;
            std::vector<VertexNorm> _normals;

            std::string _current_mtl_lib;
            MTLFile _mtl_file; // currently only one mtl file per obj file is supported

          public:
			// ModelLoader api functions

		    OBJFile();
		    OBJFile(const std::string& file_name);
		    virtual ~OBJFile();

            bool open(const std::string& file_name);

			////////////////////////////////////////// functions to load single meshes / textures ///////////////////////////////////////////

			/** @return the number of meshes stored in the file */
			virtual int getMeshCount();

			/** @return the number of unique textures used by the meshes
			* if a mesh should use for example a color + normal texture, thats 2 */
			virtual int getTextureCount();

			/** loads the vertices of the mesh
			* @param id: to iterate through the meshes of the file */
			virtual void getMesh(MeshData& loadTo_mesh, unsigned int id = 0);

			/** @param id: to iterate through the texture of the file */
			virtual void getTexture(ImageData& loadTo_texture, int id = 0);

			////////////////////////////////////////// functions to load all meshes / textures ///////////////////////////////////////////

			virtual void loadAllMeshes(std::vector<MeshData>& loadTo_meshes);

			virtual void loadAllTextures(std::vector<ImageData>& loadTo_textures);

          protected:
            // private obj loading functions

            // loads the content of the file into the internal data structures
            void loadFileContent(std::ifstream& file);

            // reads the next line that is a valid line (i.e. not a comment line or other lines that currently cant be processed)
            // returns false if the end of the file is reached
            bool readNextLine(std::ifstream& file, std::string& line);

            // reading lines (return false if the line does not contain the right data)
            bool readMtlLib(const std::string& line, std::string& mtl_lib);
            bool readObject(const std::string& line, std::string& obj_name);
            bool readGroup(const std::string& line, std::string& group_name);
            bool readUseMtl(const std::string& line, std::string& mtl_name);
            bool readVertexPos(const std::string& line, std::vector<VertexPos>& loadTo);
            bool readTexCoord(const std::string& line, std::vector<TexCoord>& loadTo);
            bool readVertexNorm(const std::string& line, std::vector<VertexNorm>& loadTo);
            bool readFace(const std::string& line, Group& loadTo);

        };

    } // namespace tools

} // namespace undicht

#endif // OBJ_FILE_H