#ifndef MATERIAL_H
#define MATERIAL_H

#include "string"

namespace cell {

    class Material {

      protected:

        std::string _name;

        // properties
        std::string _diffuse_texture; // contains diffuse color + roughness (as alpha)
        std::string _specular_texture; // contains specular color + metalness (as alpha)

      public:

        Material();
        Material(const std::string& name, const std::string& diffuse_texture = "", const std::string& specular_texture = "");

        void setName(const std::string& name);
        const std::string& getName() const;

        void setDiffuseTexture(const std::string& diffuse_texture);
        void setSpecularTexture(const std::string& specular_texture);
        const std::string& getDiffuseTexture() const;
        const std::string& getSpecularTexture() const;
    };

} // cell

#endif // MATERIAL_H