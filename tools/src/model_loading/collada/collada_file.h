#ifndef COLLADA_FILE_H
#define COLLADA_FILE_H

#include "model_loading/model_loader.h"
#include <xml/xml_file.h>


namespace undicht {

	namespace tools {

		class ColladaFile : public ModelLoader, public XmlFile {

			public:
				// ModelLoader api functions

				////////////////////////////////////////// functions to load single meshes / textures ///////////////////////////////////////////

				/** @return the number of meshes stored in the file */
				virtual int getMeshCount();

				/** @return the number of unique textures used by the meshes
				* if a mesh should use for example a color + normal texture, thats 2 */
				virtual int getTextureCount();

				/** loads the vertices of the mesh
				* @param id: to iterate through the meshes of the file */
				virtual void getMesh(MeshData& loadTo_mesh, unsigned int id = 0, bool load_positions = true, bool load_uvs = true, bool load_normals = true);

				/** @param id: to iterate through the texture of the file */
				virtual void getTexture(ImageData<char>& loadTo_texture, int id = 0);

				////////////////////////////////////////// functions to load all meshes / textures ///////////////////////////////////////////

				virtual void loadAllMeshes(std::vector<MeshData>& loadTo_meshes, bool load_positions = true, bool load_uvs = true, bool load_normals = true);

				virtual void loadAllTextures(std::vector<ImageData<char>>& loadTo_textures);

		    private:
		        // functions to bring more structure to the loading process

				/** loading the vertices from a geometry element */
		        virtual void loadGeometry(const XmlElement& geometry, std::vector<float>& vertices, BufferLayout& vertex_layout, bool load_positions = true, bool load_uvs = true, bool load_normals = true);

		        /** @param source_name: POSITION, NORMAL or TEXCOORD
		        * @return the element containing the actual data (float array) (0 if the mesh does not have a source with that name) */
		        virtual const XmlElement* getMeshSource(const XmlElement* mesh, const std::string& source_name);

		        // functions to load textures for a material

		        virtual void loadMaterialTextures(const XmlElement* material, ImageData<char>& loadTo_texture);

			public:

		        ColladaFile();
		        ColladaFile(const std::string& file_name);
		        virtual ~ColladaFile();

		};

	} // tools

} // undicht

#endif // COLLADA_FILE_H
