#include "material.h"

namespace cell {

    Material::Material() {

    }

    Material::Material(const std::string& name, const std::string& albedo_texture, const std::string& normal_texture) :
        _name(name), _albedo_texture(albedo_texture), _normal_texture(normal_texture) {

    }

    void Material::setName(const std::string& name) {
        
        _name = name;
    }

    const std::string& Material::getName() const {
        
        return _name;
    }

    void Material::setAlbedoTexture(const std::string& albedo_texture) {
        
        _albedo_texture = albedo_texture;
    }

    void Material::setNormalTexture(const std::string& normal_texture) {

        _normal_texture = normal_texture;
    }

    const std::string& Material::getAlbedoTexture() const {
        
        return _albedo_texture;
    }

    const std::string& Material::getNormalTexture() const {

        return _normal_texture;
    }

} // cell