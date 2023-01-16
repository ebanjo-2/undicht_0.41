#ifndef MATERIAL_H
#define MATERIAL_H

#include "string"

namespace cell {

    class Material {

      protected:

        std::string _name;

        // properties
        std::string _albedo_texture; // contains albedo + roughness (as alpha)
        std::string _normal_texture; // contains normal map + metalness (as alpha)

      public:

        Material();
        Material(const std::string& name, const std::string& albedo_texture = "", const std::string& normal_texture = "");

        void setName(const std::string& name);
        const std::string& getName() const;

        void setAlbedoTexture(const std::string& albedo_texture);
        void setNormalTexture(const std::string& normal_texture);
        const std::string& getAlbedoTexture() const;
        const std::string& getNormalTexture() const;
    };

} // cell

#endif // MATERIAL_H