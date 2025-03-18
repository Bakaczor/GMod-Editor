#include "Camera.h"

Camera::Camera(gmod::Transform<float> target, float dist, float minDist, float maxDist) : m_target(target), m_dist(dist) {
	SetDistanceRange(minDist, maxDist);
}

Camera::Camera(float dist, float minDist, float maxDist) : Camera(gmod::Transform<float>(), dist, minDist, maxDist) {}

float Camera::distance() const {
	return m_dist;
}

gmod::Transform<float> Camera::target() const {
	return m_target;
}

void Camera::SetDistanceRange(float minDist, float maxDist) {
	if (maxDist < minDist) {
		maxDist = minDist;
	}
	m_minDist = minDist;
	m_maxDist = maxDist;
	ClampDistance();
}

void Camera::ClampDistance() {
	if (m_dist < m_minDist) {
		m_dist = m_minDist;
	}
	else if (m_dist > m_maxDist) {
		m_dist = m_maxDist;
	}
}

void Camera::Move(float dx, float dy) {
	gmod::vector3<float> dt = moveSensitivity * (-dx * m_target.right() + -dy * m_target.up());
	m_target.UpdateTranslation(dt.x(), dt.y(), dt.z());
}

void Camera::Rotate(float dx, float dy) {
	m_target.UpdateRotation(rotateSensitivity * -dy, rotateSensitivity * dx, 0);
}

void Camera::Zoom(float dd) {
	m_dist += zoomSensitivity * dd;
	ClampDistance();
}

gmod::matrix4<float> Camera::viewMatrix() const {
	const gmod::vector3<float> ax = m_target.right();
	const gmod::vector3<float> ay = m_target.up();
	const gmod::vector3<float> az = m_target.forward();
	const gmod::vector3<float> eye = cameraPosition();

	return gmod::matrix4<float>(
		 ax.x(),		ay.x(),		   az.x(),		 0,
		 ax.y(),		ay.y(),		   az.y(),		 0,
		 ax.z(),		ay.z(),	   	   az.z(),		 0,
		-dot(ax, eye), -dot(ay, eye), -dot(az, eye), 1

	);
}

gmod::vector4<float> Camera::cameraPosition() const {
	if (m_dist == 0.0f) {
		return gmod::vector4<float>(m_target.position(), 1.0f);
	}
	gmod::vector3<float> t = -m_dist * m_target.forward();
	return gmod::vector4<float>(m_target.position() + t, 1.0f);
}


