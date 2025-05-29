#pragma once
#include "../gmod/Transform.h"
#include "../gmod/vector3.h"
#include "../gmod/vector4.h"
#include <limits>

namespace app {
	class Camera {
	public:
		bool cameraChanged = false;
		float moveSensitivity = 0.005f;
		float rotateSensitivity = 0.005f;
		float zoomSensitivity = 0.0005f;

		explicit Camera(gmod::Transform<float> target = gmod::Transform<float>(),
			float dist = 0.0f, float minDist = -std::numeric_limits<float>::max(), float maxDist = std::numeric_limits<float>::max());

		explicit Camera(float dist = 0.0f, float minDist = -std::numeric_limits<float>::max(), float maxDist = std::numeric_limits<float>::max());

		gmod::matrix4<float> viewMatrix() const;
		gmod::matrix4<float> viewMatrix_inv() const;
		gmod::matrix4<float> stereoViewMatrix(int sign, float D) const;
		gmod::vector4<float> cameraPosition() const;

		void Move(float dx, float dy);
		void Rotate(float dx, float dy);
		void Zoom(float dd);
		void SetDistanceRange(float minDist, float maxDist);
		void SetTargetPosition(gmod::vector3<float> pos);

		float distance() const;
		gmod::Transform<float> target() const;

	private:
		float m_dist, m_minDist, m_maxDist;
		gmod::Transform<float> m_target;

		void ClampDistance();
	};
}