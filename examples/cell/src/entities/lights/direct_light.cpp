#include "direct_light.h"
#include "glm/gtc/matrix_transform.hpp"

namespace cell {

    using namespace undicht;

    const BufferLayout DIRECT_LIGHT_LAYOUT({UND_VEC3F, UND_VEC3F});
    // vec3: position
    // vec3: color

    DirectLight::DirectLight() {

    }

    DirectLight::DirectLight(const glm::vec3& direction, const glm::vec3& color, const glm::vec3 shadow_origin) {

        setDirection(direction);
        setColor(color);
        setShadowOrigin(shadow_origin);
    }

    void DirectLight::setDirection(const glm::vec3& direction) {

        _direction = glm::normalize(direction);
        updateShadowMatrices();
    }

    void DirectLight::setColor(const glm::vec3& color) {

        _color = color;
    }

    void DirectLight::setShadowOrigin(const glm::vec3& shadow_origin) {

        _shadow_origin = shadow_origin;
        updateShadowMatrices();
    }

    void DirectLight::setShadowProjSize(float proj_width, float proj_height) {
        // the area the projection matrix should cover

        _shadow_proj_width = proj_width;
        _shadow_proj_height = proj_height;

        updateShadowMatrices();
    } 


    const glm::vec3& DirectLight::getDirection() const {

        return _direction;
    }

    const glm::vec3& DirectLight::getColor() const {

        return _color;
    }

    const glm::vec3& DirectLight::getShadowOrigin() const {

        return _shadow_origin;
    }

    const glm::mat4& DirectLight::getShadowView() const{
        
        return _shadow_view;
    }

    const glm::mat4& DirectLight::getShadowProj() const{
        
        return _shadow_proj;
    }

    ////////////////////////////////////// private functions //////////////////////////////////////

    void DirectLight::updateShadowMatrices() {

        float right = _shadow_proj_width / 2.0f;
        float top = _shadow_proj_height / 2.0f;

        _shadow_view = glm::lookAt(getShadowOrigin(), getShadowOrigin() + getDirection(), glm::vec3(0.0f, 1.0f, 0.0f));
        //_shadow_view = glm::lookAt(glm::vec3(10,30,0), glm::vec3(0,0,0), glm::vec3(0, 1, 0));
        //_shadow_view = glm::rotate(glm::mat4(1.0f), 180.0f, glm::vec3(0, 1, 0));
        _shadow_proj = glm::ortho(-right, right, -top, top, -100.0f, 100.0f);
    }

}