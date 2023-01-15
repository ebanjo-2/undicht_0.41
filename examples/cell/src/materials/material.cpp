#include "material.h"

namespace cell {

    Material::Material() {

    }

    Material::Material(const std::string& name, const std::string& diffuse_texture, const std::string& specular_texture) :
        _name(name), _diffuse_texture(diffuse_texture), _specular_texture(specular_texture) {

    }

    void Material::setName(const std::string& name) {
        
        _name = name;
    }

    const std::string& Material::getName() const {
        
        return _name;
    }

    void Material::setDiffuseTexture(const std::string& diffuse_texture) {
        
        _diffuse_texture = diffuse_texture;
    }

    void Material::setSpecularTexture(const std::string& specular_texture) {

        _specular_texture = specular_texture;
    }

    const std::string& Material::getDiffuseTexture() const {
        
        return _diffuse_texture;
    }

    const std::string& Material::getSpecularTexture() const {

        return _specular_texture;
    }

} // cell