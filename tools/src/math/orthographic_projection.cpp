#include "orthographic_projection.h"

#include "glm/glm.hpp"
#include <glm/gtc/matrix_transform.hpp>


namespace undicht {

	namespace tools {

		OrthographicProjection::OrthographicProjection() {
			//ctor

		}

		OrthographicProjection::~OrthographicProjection(){
			//dtor
		}

		void OrthographicProjection::setViewRange(float near_plane, float far_plane) {

			_update_proj = true;

			_near_plane = near_plane;
			_far_plane = far_plane;
		}

		void OrthographicProjection::setExtent(float width, float height) {

			_update_proj = true;

			_width = width;
			_height = height;
		}

		float OrthographicProjection::getNearPlane() const {

			return _near_plane;
		}

		float OrthographicProjection::getFarPlane() const {

			return _far_plane;
		}

		float OrthographicProjection::getWidth() const {

			return _width;
		}

		float OrthographicProjection::getHeight() const {

			return _height;
		}

		glm::mat4& OrthographicProjection::getProjectionMatrix() {

			if(_update_proj) {
				_proj_mat = glm::ortho(-_width/2, _width/2, -_height/2, _height/2, _near_plane, _far_plane);
				_update_proj = false;
			}

			return _proj_mat;
		}

	} // tools

} // undicht
