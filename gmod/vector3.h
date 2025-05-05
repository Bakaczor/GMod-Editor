#pragma once
#include "pch.h"
#include "vector4.h"
#include "matrix3.h"
#include "quaternion.h"

namespace gmod {
    template<floating_point T>
    struct vector4;

    template<floating_point T>
    struct quaternion;

    template<floating_point T>
    struct matrix3;

    template<floating_point T>
    struct vector3 {
    public:
        vector3(T x = 0, T y = 0, T z = 0) : m_x(x), m_y(y), m_z(z) {}

        vector3(const vector4<T>& v) : m_x(v.x()), m_y(v.y()), m_z(v.z()) {}

        T& x() { return m_x; }

        const T& x() const { return m_x; }

        T& y() { return m_y; }

        const T& y() const { return m_y; }

        T& z() { return m_z; }

        const T& z() const { return m_z; }

        T& operator[](int index) {
            switch (index) {
                case 0: return m_x;
                case 1: return m_y;
                case 2: return m_z;
                default: throw std::out_of_range("Index out of range for vector3");
            }
        }

        const T& operator[](int index) const {
            switch (index) {
                case 0: return m_x;
                case 1: return m_y;
                case 2: return m_z;
                default: throw std::out_of_range("Index out of range for vector3");
            }
        }

        vector3 operator+(const vector3& other) const {
            return vector3(m_x + other.m_x, m_y + other.m_y, m_z + other.m_z);
        }

        vector3 operator-(const vector3& other) const {
            return vector3(m_x - other.m_x, m_y - other.m_y, m_z - other.m_z);
        }

        vector3 operator*(T scalar) const {
            return vector3(m_x * scalar, m_y * scalar, m_z * scalar);
        }

        friend vector3 operator*(T scalar, const vector3& vec) {
            return vec * scalar;
        }

        // dot product
        T operator*(const vector3& other) const {
            return m_x * other.m_x + m_y * other.m_y + m_z * other.m_z;
        }

        // cross product
        vector3 operator^(const vector3& other) const {
            return vector3(
                m_y * other.m_z - m_z * other.m_y,
                m_z * other.m_x - m_x * other.m_z,
                m_x * other.m_y - m_y * other.m_x
            );
        }

        // vector3 * matrix3
        vector3 operator*(const matrix3<T>& other) const {
            return vector3(
                other[0] * m_x + other[3] * m_y + other[6] * m_z,
                other[1] * m_x + other[4] * m_y + other[7] * m_z,
                other[2] * m_x + other[5] * m_y + other[8] * m_z
            );
        }

        vector3 operator-() const {
            return vector3(-m_x, -m_y, -m_z);
        }

        T length() const {
            return std::sqrt(m_x * m_x + m_y * m_y + m_z * m_z);
        }

        bool operator==(const vector3& other) const {
            return m_x == other.m_x && m_y == other.m_y && m_z == other.m_z;
        }

        bool operator!=(const vector3& other) const {
            return !(*this == other);
        }

        friend std::ostream& operator<<(std::ostream& os, const vector3& vec) {
            os << "(" << vec.m_x << ", " << vec.m_y << ", " << vec.m_z << ")";
            return os;
        }

        void normalize() {
            const T len = this->length();
            if (len == 0) {
                throw std::logic_error("Length was equal to zero for vector3");
            }
            m_x /= len;
            m_y /= len;
            m_z /= len;
        }

        vector3 normalized() const {
            const T inv_len = 1 / this->length();
            return *this * inv_len;
        }

        void rotate(const quaternion<T>& q) {
            quaternion<T> v(0, m_x, m_y, m_z);
            quaternion<T> result = q * v * q.inverted();
            m_x = result.x();
            m_y = result.y();
            m_z = result.z();
        }

        vector3 rotated(const quaternion<T>& q) const {
            quaternion<T> v(0, m_x, m_y, m_z);
            quaternion<T> result = q * v * q.inverted();
            return vector3(result.x(), result.y(), result.z());
        }

    private:
        T m_x, m_y, m_z;
    };

    template<floating_point T>
    T dot(const vector3<T>& vec1, const vector3<T>& vec2) {
        return vec1 * vec2;
    }

    template<floating_point T>
    vector3<T> cross(const vector3<T>& vec1, const vector3<T>& vec2) {
        return vec1 ^ vec2;
    }

    template<floating_point T>
    vector3<T> normalize(const vector3<T>& vec) {
        return vec.normalized();
    }

    template<floating_point T>
    vector3<T> project(const vector3<T>& u, const vector3<T>& v) {
        return ((v * u) / (u * u)) * u;
    }

    template<floating_point T>
    T distance(const vector3<T>& p, const vector3<T>& q) {
        const T diffX = p.x() - q.x();
        const T diffY = p.y() - q.y();
        const T diffZ = p.z() - q.z();
        return std::sqrt(diffX * diffX + diffY * diffY + diffZ  * diffZ)
    }
}