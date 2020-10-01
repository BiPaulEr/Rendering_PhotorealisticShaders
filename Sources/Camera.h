#ifndef CAMERA_H
#define CAMERA_H

#include "Transform.h"

/// Basic camera model
class Camera : public Transform {
public:
	inline float getFov () const { return m_fov; }
	inline void setFoV (float f) { m_fov = f; }
	inline float getAspectRatio () const { return m_aspectRatio; }
	inline void setAspectRatio (float a) { m_aspectRatio = a; }
	inline float getNear () const { return m_near; }
	inline void setNear (float n) { m_near = n; }
	inline float getFar () const { return m_far; }
	inline void setFar (float n) { m_far = n; }
	
	/**
	 *  The view matrix is the inverse of the camera model matrix, 
	 *  so that we can express the entire world in the camera frame 
	 *  by transforming all its entities with this matrix. 
	 */
	inline glm::mat4 computeViewMatrix () const {
		glm::mat4 rotationMatrix = glm::mat4_cast (curQuat);
		return inverse (rotationMatrix * computeTransformMatrix ());
	}
	
	/// Returns the projection matrix stemming from the camera intrinsic parameter. 
	inline glm::mat4 computeProjectionMatrix () const {
		return glm::perspective (glm::radians (m_fov), m_aspectRatio, m_near, m_far);
	}

private:
	float m_fov = 45.f; // Field of view, in degrees
	float m_aspectRatio = 1.f; // Ratio between the width and the height of the image
	float m_near = 0.1f; // Distance before which geometry is excluded fromt he rasterization process
	float m_far = 10.f; // Distance after which the geometry is excluded fromt he rasterization process
	glm::quat curQuat;
	glm::quat lastQuat;
};

#endif // CAMERA_H