#include "Camera.h"
#include <numbers>

using namespace app;

Camera::Camera(gmod::Transform<float> target, float dist, float minDist, float maxDist) : m_target(target), m_dist(dist) {
	SetDistanceRange(minDist, maxDist);
	m_target.SetRotation(-0.5f, 0.5f, 0.0f);
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

void app::Camera::SetTargetPosition(gmod::vector3<float> pos) {
	m_target.SetTranslation(pos.x(), pos.y(), pos.z());
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
	gmod::vector3<float> dt = moveSensitivity * (-dx * m_target.right() + dy * m_target.up());
	m_target.UpdateTranslation(dt.x(), dt.y(), dt.z());
}

void Camera::Rotate(float dx, float dy) {
	m_target.UpdateRotation_Euler(rotateSensitivity * -dy, rotateSensitivity * -dx, 0);
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
		ax.x(), ax.y(), ax.z(), -dot(ax, eye),
		ay.x(), ay.y(), ay.z(), -dot(ay, eye),
		az.x(), az.y(), az.z(), -dot(az, eye),
		0,		0,		0,		 1
	);
}

gmod::matrix4<float> Camera::viewMatrix_inv() const {
	const gmod::vector3<float> ax = m_target.right();
	const gmod::vector3<float> ay = m_target.up();
	const gmod::vector3<float> az = m_target.forward();
	const gmod::vector3<float> eye = cameraPosition();
	const float tx = dot(ax, eye);
	const float ty = dot(ay, eye);
	const float tz = dot(az, eye);
	const float ctx = tx * ax.x() + ty * ay.x() + tz * az.x();
	const float cty = tx * ax.y() + ty * ay.y() + tz * az.y();
	const float ctz = tx * ax.z() + ty * ay.z() + tz * az.z();

	return gmod::matrix4<float>(
		ax.x(), ay.x(), az.x(), ctx,
		ax.y(), ay.y(), az.y(), cty,
		ax.z(), ay.z(), az.z(), ctz,
		0,      0,      0,      1
	);
}

gmod::vector4<float> Camera::cameraPosition() const {
	if (m_dist == 0.0f) {
		return gmod::vector4<float>(m_target.position(), 1.0f);
	}
	gmod::vector3<float> t = -m_dist * m_target.forward();
	return gmod::vector4<float>(m_target.position() + t, 1.0f);
}


