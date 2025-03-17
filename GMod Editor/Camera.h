#pragma once
#include "../gmod/Transform.h"
#include "../gmod/vector3.h"
#include "../gmod/vector4.h"
#include <limits>

class Camera {
public:
	bool cameraChanged = false;
	float moveSensitivity = 0.005f;
	float rotateSensitivity = 0.01f;
	float zoomSensitivity = 0.01f;

	explicit Camera(gmod::Transform<float> target = gmod::Transform<float>(),
		float minDist = 0.0f, float maxDist = std::numeric_limits<float>::max(), float dist = 0.0f);

	explicit Camera(float minDist, float maxDist = std::numeric_limits<float>::max(), float dist = 0.0f);

	gmod::matrix4<float> viewMatrix() const ;
	gmod::vector4<float> cameraPosition() const;

	void Move(float dx, float dy);
	void Rotate(float dx, float dy);
	void Zoom(float dd);
	void SetDistanceRange(float minDist, float maxDist);

	float distance() const;
	gmod::Transform<float> target() const;

private:
	float m_dist, m_minDist, m_maxDist;
	gmod::Transform<float> m_target;

	void ClampDistance();
};