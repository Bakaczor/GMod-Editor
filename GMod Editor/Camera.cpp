#include "Camera.h"

Camera::Camera(gmod::Transform<float> target, float minDist, float maxDist, float dist) : m_target(target), m_dist(dist) {
	SetDistanceRange(minDist, maxDist);
}

Camera::Camera(float minDist, float maxDist, float distance) : Camera(gmod::Transform<float>(), minDist, maxDist, distance) {}

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
	gmod::vector3<float> dt = moveSensitivity * (dx * m_target.right() + dy * m_target.up());
	m_target.UpdateTranslation(dt.x(), dt.y(), dt.z());
}

void Camera::Rotate(float dx, float dy) {
	m_target.UpdateRotation(rotateSensitivity * dx, rotateSensitivity * dy, 0);
}

void Camera::Zoom(float dd) {
	m_dist += zoomSensitivity * dd;
	ClampDistance();
}

gmod::matrix4<float> Camera::viewMatrix() const {
	const auto& pos = m_target.position();
	const auto& angles = m_target.eulerAngles();
	return gmod::matrix4<float>::translation(pos.x(), pos.y(), pos.z()) * gmod::matrix4<float>::rotationY(angles.y()) *
		gmod::matrix4<float>::rotationX(angles.x()) * gmod::matrix4<float>::translation(0, 0, m_dist);
}

gmod::vector4<float> Camera::cameraPosition() const {
	if (m_dist == 0.0f) {
		return gmod::vector4<float>(m_target.position(), 1.0f);
	}
	gmod::vector3<float> t = -m_dist * m_target.forward();
	return gmod::vector4<float>(m_target.position() + t, 1.0f);
}


