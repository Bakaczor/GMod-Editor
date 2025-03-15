#pragma once
#include "pch.h"
#include "vector3.h"

namespace gmod {
    template <floating_point T>
    struct quaternion {
    public:
        quaternion(T w = 1, T x = 0, T y = 0, T z = 0) : m_w(w), m_x(x), m_y(y), m_z(z) {}

        static quaternion from_angle_axis(T angle, T ax, T ay, T az) {
            const T half = angle / 2;
            const T sin = std::sin(half);

            return quaternion(std::cos(half), ax * sin, ay * sin, az * sin);
        }

        static quaternion from_euler(T pitch, T yaw, T roll) {
            const T half_pitch = pitch / 2;
            const T half_yaw = yaw / 2;
            const T half_roll = roll / 2;

            const T cos_pitch = std::cos(half_pitch);
            const T sin_pitch = std::sin(half_pitch);
            const T cos_yaw = std::cos(half_yaw);
            const T sin_yaw = std::sin(half_yaw);
            const T cos_roll = std::cos(half_roll);
            const T sin_roll = std::sin(half_roll);

            return quaternion(
                cos_pitch * cos_yaw * cos_roll + sin_pitch * sin_yaw * sin_roll,
                sin_pitch * cos_yaw * cos_roll - cos_pitch * sin_yaw * sin_roll,
                cos_pitch * sin_yaw * cos_roll + sin_pitch * cos_yaw * sin_roll,
                cos_pitch * cos_yaw * sin_roll - sin_pitch * sin_yaw * cos_roll
            );
        }

        T& w() { return m_w; }

        const T& w() const { return m_w; }

        T& x() { return m_x; }

        const T& x() const { return m_x; }

        T& y() { return m_y; }

        const T& y() const { return m_y; }

        T& z() { return m_z; }

        const T& z() const { return m_z; }

        vector3<T> v() { return vector3<T>(m_x, m_y, m_z); }

        quaternion operator+(const quaternion& other) const {
            return quaternion(m_w + other.m_w, m_x + other.m_x, m_y + other.m_y, m_z + other.m_z);
        }

        quaternion operator-(const quaternion& other) const {
            return quaternion(m_w - other.m_w, m_x - other.m_x, m_y - other.m_y, m_z - other.m_z);
        }

        quaternion operator*(T scalar) const {
            return quaternion(m_w * scalar, m_x * scalar, m_y * scalar, m_z * scalar);
        }

        friend quaternion operator*(T scalar, const quaternion& q) {
            return q * scalar;
        }

        quaternion operator*(const quaternion& other) const {
            return quaternion(
                m_w * other.m_w - m_x * other.m_x - m_y * other.m_y - m_z * other.m_z,
                m_w * other.m_x + m_x * other.m_w + m_y * other.m_z - m_z * other.m_y,
                m_w * other.m_y + m_y * other.m_w + m_z * other.m_x - m_x * other.m_z,
                m_w * other.m_z + m_z * other.m_w + m_x * other.m_y - m_y * other.m_x
            );
        }

        quaternion operator-() const {
            return quaternion(-m_w, -m_x, -m_y, -m_z);
        }

        T magnitude() const {
            return std::sqrt(m_w * m_w + m_x * m_x + m_y * m_y + m_z * m_z);
        }

        bool operator==(const quaternion& other) const {
            return m_w == other.m_w && m_x == other.m_x && m_y == other.m_y && m_z == other.m_z;
        }

        bool operator!=(const quaternion& other) const {
            return !(*this == other);
        }

        friend std::ostream& operator<<(std::ostream& os, const quaternion& q) {
            os << "(" << q.m_w << " [" << q.m_x << ", " << q.m_y << ", " << q.m_z << "])";
            return os;
        }

        void normalize() {
            const T mag = this->magnitude();
            m_w /= mag;
            m_x /= mag;
            m_y /= mag;
            m_z /= mag;
        }

        quaternion normalized() const {
            const T inv_mag = 1 / this->magnitude();
            return *this * inv_mag;
        }

        void conjugate() {
            m_x = -m_x;
            m_y = -m_y;
            m_z = -m_z;
        }

        quaternion conjugated() const {
            return quaternion(m_w, -m_x, -m_y, -m_z);
        }

        void invert() {
            this->conjugate();
            const T mag_2 = m_w * m_w + m_x * m_x + m_y * m_y + m_z * m_z;
            m_w /= mag_2;
            m_x /= mag_2;
            m_y /= mag_2;
            m_z /= mag_2;
        }

        quaternion inverted() const {
            const T mag_2 = m_w * m_w + m_x * m_x + m_y * m_y + m_z * m_z;
            return this->conjugated() * (1 / mag_2);
        }

    private:
        T m_w, m_x, m_y, m_z;
    };

    template<floating_point T>
    T dot(const quaternion<T>& q1, const quaternion<T>& q2) {
        return q1.m_w * q2.m_w + q1.m_x * q2.m_x + q1.m_y * q2.m_y + q1.m_z * q2.m_z;
    }

    template<floating_point T>
    quaternion<T> normalize(const quaternion<T>& q) {
        return q.normalized();
    }
}

