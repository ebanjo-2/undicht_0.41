#ifndef ORTHOGRAPHIC_PROJECTION_H
#define ORTHOGRAPHIC_PROJECTION_H

#include "glm/glm.hpp"

namespace undicht {

	namespace tools {

		class OrthographicProjection {

		protected:

			float _near_plane = 0.1f;
			float _far_plane = 1000.0f;

			float _width = 100.0f;
			float _height = 100.0f;

			// whether to update the projection matrix
			bool _update_proj = true;
			glm::mat4 _proj_mat;

		public:

			OrthographicProjection();
			virtual ~OrthographicProjection();

			void setViewRange(float near_plane, float far_plane);
			void setExtent(float width, float height);

			float getNearPlane() const;
			float getFarPlane() const;
			float getWidth() const;
			float getHeight() const;

			glm::mat4& getProjectionMatrix();

		};

	} // graphics

} // undicht

#endif // ORTHOGRAPHIC_PROJECTION_H
