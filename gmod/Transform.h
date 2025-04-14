#pragma once
#include "pch.h"
#include "matrix4.h"
#include "quaternion.h"
#include "vector3.h"

namespace gmod {
	template <floating_point T>
	class Transform {
	public:
		Transform(const vector3<T>& position = { 0, 0, 0 }, const vector3<T>& eulerAngles = { 0, 0, 0 }, const vector3<T>& scale = { 1, 1, 1 }) {
			SetTranslation(position.x(), position.y(), position.z());
			SetRotation(eulerAngles.x(), eulerAngles.y(), eulerAngles.z());
			SetScaling(scale.x(), scale.y(), scale.z());
		}

		vector3<T> position() const {
			return vector3<T>(m_tx, m_ty, m_tz);
		}

		vector3<T> eulerAngles() const {
			return vector3<T>(m_rx, m_ry, m_rz);
		}

		vector3<T> scale() const {
			return vector3<T>(m_sx, m_sy, m_sz);
		}

		quaternion<T> rotation() const {
			return quaternion<T>::from_euler(m_rx, m_ry, m_rz);
		}

		vector3<T> right() const { return m_right; }

		vector3<T> up() const { return m_up; }

		vector3<T> forward() const { return m_forward; }

		matrix4<T> modelMatrix() const {
			matrix4<T> Mt = matrix4<T>::translation(m_tx, m_ty, m_tz);
			matrix4<T> Mr = matrix4<T>::from_quaternion(m_rot);
			matrix4<T> Ms = matrix4<T>::scaling(m_sx, m_sy, m_sz);
			return Mt * Mr * Ms;
		}

		void SetTranslation(T tx, T ty, T tz) {
			m_tx = tx;
			m_ty = ty;
			m_tz = tz;
		}

		void SetRotation(T rx, T ry, T rz) {
			m_rot = quaternion<T>::from_euler(rx, ry, rz).normalized();
			m_rx = ClampRotation(rx);
			m_ry = ClampRotation(ry);
			m_rz = ClampRotation(rz);
			ResetAxes();
			RotateAxes(m_rot);
		}

		void SetRotationAroundPoint(T rx, T ry, T rz, const vector3<T>& p) {
			auto offset = position() - p;
			offset.rotate(m_rot.inverted());

			SetRotation(rx, ry, rz);
			offset.rotate(m_rot);
			SetTranslation(p.x() + offset.x(), p.y() + offset.y(), p.z() + offset.z());
		}

		void SetScaling(T sx, T sy, T sz) {
			m_sx = sx;
			m_sy = sy;
			m_sz = sz;
			AssertScales();
		}

		void SetScalingAroundPoint(T sx, T sy, T sz, const vector3<T>& p) {
			auto scales = AssertedScales(sx, sy, sz);
			sx = scales.x();
			sy = scales.y();
			sz = scales.z();

			auto offset = position() - p;
			offset.x() *= 1 / m_sx;
			offset.y() *= 1 / m_sy;
			offset.z() *= 1 / m_sz;
 
			SetScaling(sx, sy, sz);
			offset.x() *= m_sx;
			offset.y() *= m_sy;
			offset.z() *= m_sz;
			SetTranslation(p.x() + offset.x(), p.y() + offset.y(), p.z() + offset.z());
		}

		void UpdateTranslation(T dtx, T dty, T dtz) {
			m_tx += dtx;
			m_ty += dty;
			m_tz += dtz;
		}

		// used for updating camera
		void UpdateRotation_Euler(T drx, T dry, T drz) {
			m_rx = ClampRotation(m_rx + drx);
			const float PIdiv2 = std::numbers::pi_v<float> / 2;
			if (std::fabs(m_rx) > PIdiv2) {
				dry = -dry;
			}
			m_ry = ClampRotation(m_ry + dry);
			m_rz = ClampRotation(m_rz + drz);
			ResetAxes();
			RotateAxes(quaternion<T>::from_euler(m_rx, m_ry, m_rz).normalized());
		}

		// used for updating models
		void UpdateRotation_Quaternion(T drx, T dry, T drz) {
			auto newRot = quaternion<T>::from_euler(drx, dry, drz).normalized();
			m_rot = normalize(newRot * m_rot);
			gmod::vector3<T> angles = quaternion<T>::to_euler(m_rot);
			m_rx = ClampRotation(angles.x());
			m_ry = ClampRotation(angles.y());
			m_rz = ClampRotation(angles.z());
			RotateAxes(newRot);
		}

		void UpdateRotationAroundPoint_Quaternion(T drx, T dry, T drz, const vector3<T>& p) {
			auto offset = position() - p;
			offset.rotate(m_rot.inverted());

			UpdateRotation_Quaternion(drx, dry, drz);
			offset.rotate(m_rot);
			SetTranslation(p.x() + offset.x(), p.y() + offset.y(), p.z() + offset.z());
		}

		void UpdateScaling(T dsx, T dsy, T dsz) {
			m_sx += dsx;
			m_sy += dsy;
			m_sz += dsz;
			AssertScales();
		}

		void UpdateScalingAroundPoint(T dsx, T dsy, T dsz, const vector3<T>& p) {
			auto scales = AssertedScales(dsx, dsy, dsz);
			dsx = scales.x();
			dsy = scales.y();
			dsz = scales.z();

			auto pos = position() - p;
			pos.x() *= 1 / m_sx;
			pos.y() *= 1 / m_sy;
			pos.z() *= 1 / m_sz;
			pos = pos + p;

			UpdateScaling(dsx, dsy, dsz);
			vector3<T> offset = pos - p;
			offset.x() *= m_sx;
			offset.y() *= m_sy;
			offset.z() *= m_sz;
			SetTranslation(p.x() + offset.x(), p.y() + offset.y(), p.z() + offset.z());
		}
	private:
		const T m_minScale = 10 * std::numeric_limits<T>::epsilon();

		T m_tx, m_ty, m_tz;
		T m_rx, m_ry, m_rz;
		T m_sx, m_sy, m_sz;
		quaternion<T> m_rot;

		vector3<T> m_right;
		vector3<T> m_up;
		vector3<T> m_forward;

		void AssertScales() {
			if (std::fabs(m_sx) < m_minScale) {
				m_sx = std::copysign(m_minScale, m_sx);
			}
			if (std::fabs(m_sy) < m_minScale) {
				m_sx = std::copysign(m_minScale, m_sy);
			}
			if (std::fabs(m_sz) < m_minScale) {
				m_sx = std::copysign(m_minScale, m_sz);
			}
		}

		vector3<T> AssertedScales(T sx, T sy, T sz) const {
			if (std::fabs(sx) < m_minScale) {
				sx = std::copysign(m_minScale, sx);
			}
			if (std::fabs(sy) < m_minScale) {
				sy = std::copysign(m_minScale, sy);
			}
			if (std::fabs(sz) < m_minScale) {
				sz = std::copysign(m_minScale, sz);
			}
			return vector3<T>(sx, sy, sz);
		}

		void ResetAxes() {
			m_right = vector3<T>(1, 0, 0);
			m_up = vector3<T>(0, 1, 0);
			m_forward = vector3<T>(0, 0, 1);
		}

		T ClampRotation(T rad) {
			const auto PImul2 = 2 * std::numbers::pi_v<T>;
			if (std::fabs(rad) > PImul2) {
				const int abs = static_cast<int>(rad / PImul2);
				rad -= abs * PImul2;
			}
			return rad;
		}

		void RotateAxes(const quaternion<T>& q) {
			m_right.rotate(q);
			m_right.normalize();
			m_up.rotate(q);
			m_up.normalize();
			m_forward.rotate(q);
			m_forward.normalize();
		}
	};
}