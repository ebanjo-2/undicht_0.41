#include "material.h"

namespace cell {

    Material::Material() {

    }

    Material::Material(const std::string& name, const std::string& diffuse_texture) :
        _name(name), _diffuse_texture(diffuse_texture) {

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

    const std::string& Material::getDiffuseTexture() const {
        
        return _diffuse_texture;
    }

} // cell