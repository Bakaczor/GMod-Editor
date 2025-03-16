#pragma once
#include "pch.h"
#include "vector3.h"
#include "quaternion.h"
#include "matrix4.h"

namespace gmod {
	template <floating_point T>
	class Transform {
	public:
		Transform(const vector3<T>& position = { 0, 0, 0 }, const vector3<T>& eulerAngles = { 0, 0, 0 }, const vector3<T>& scale = { 1, 1, 1 }) :
			m_tx(position.x()), m_ty(position.y()), m_tz(position.z()),
			m_rx(eulerAngles.x()), m_ry(eulerAngles.y()), m_rz(eulerAngles.z()),
			m_sx(scale.x()), m_sy(scale.y()), m_sz(scale.z()) {
			RotateAxes(quaternion<T>::from_euler(m_rx, m_ry, m_rz));
			if (m_sx == 0 || m_sy == 0 || m_sz == 0) {
				throw std::logic_error("Transform's scale cannot be equal to zero");
			}
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
			matrix4<T> Mr = matrix4<T>::rotation(m_rx, m_ry, m_rz);
			matrix4<T> Ms = matrix4<T>::scaling(m_sx, m_sy, m_sz);
			return Mt * Mr * Ms;
		}

		void SetTranslation(T tx, T ty, T tz) {
			m_tx = tx;
			m_ty = ty;
			m_tz = tz;
		}

		void SetRotation(T rx, T ry, T rz) {
			m_rx = rx;
			m_ry = ry;
			m_rz = rz;
			RotateAxes(quaternion<T>::from_euler(m_rx, m_ry, m_rz));
		}

		void SetScaling(T sx, T sy, T sz) {
			m_sx = sx;
			m_sy = sy;
			m_sz = sz;
			if (m_sx == 0 || m_sy == 0 || m_sz == 0) {
				throw std::logic_error("Transform's scale cannot be equal to zero");
			}
		}

		void UpdateTranslation(T dtx, T dty, T dtz) {
			m_tx += dtx;
			m_ty += dty;
			m_tz += dtz;
		}

		void UpdateRotation(T drx, T dry, T drz) {
			m_rx += drx;
			m_ry += dry;
			m_rz += drz;
			RotateAxes(quaternion<T>::from_euler(m_rx, m_ry, m_rz));
		}

		void UpdateScaling(T dsx, T dsy, T dsz) {
			m_sx += dsx;
			m_sy += dsy;
			m_sz += dsz;
			if (m_sx == 0 || m_sy == 0 || m_sz == 0) {
				throw std::logic_error("Transform's scale cannot be equal to zero");
			}
		}
	private:
		T m_tx, m_ty, m_tz;
		T m_rx, m_ry, m_rz;
		T m_sx, m_sy, m_sz;

		vector3<T> m_right;
		vector3<T> m_up;
		vector3<T> m_forward;

		void ResetAxes() {
			m_right = vector3<T>(1, 0, 0);
			m_up = vector3<T>(0, 1, 0);
			m_forward = vector3<T>(0, 0, 1);
		}

		void RotateAxes(const quaternion<T>& q) {
			ResetAxes();
			m_right.rotate(q);
			m_right.normalize();
			m_up.rotate(q);
			m_up.normalize();
			m_forward.rotate(q);
			m_forward.normalize();
		}
	};
}