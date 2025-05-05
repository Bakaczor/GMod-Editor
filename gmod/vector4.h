#pragma once
#include "pch.h"
#include "vector3.h"
#include "matrix4.h"

namespace gmod {
    template<floating_point T>
    struct vector3;

    template<floating_point T>
    struct matrix4;

    template<floating_point T>
    struct vector4 {
    public:
        vector4(T x = 0, T y = 0, T z = 0, T w = 1) : m_x(x), m_y(y), m_z(z), m_w(w) {}
        vector4(const vector3<T>& v, T w = 1) : m_x(v.x()), m_y(v.y()), m_z(v.z()), m_w(w) {}

        T& x() { return m_x; }

        const T& x() const { return m_x; }

        T& y() { return m_y; }

        const T& y() const { return m_y; }

        T& z() { return m_z; }

        const T& z() const { return m_z; }

        T& w() { return m_w; }

        const T& w() const { return m_w; }

        T& operator[](int index) {
            switch (index) {
                case 0: return m_x;
                case 1: return m_y;
                case 2: return m_z;
                case 3: return m_w;
                default: throw std::out_of_range("Index out of range for vector4");
            }
        }

        const T& operator[](int index) const {
            switch (index) {
                case 0: return m_x;
                case 1: return m_y;
                case 2: return m_z;
                case 3: return m_w;
                default: throw std::out_of_range("Index out of range for vector4");
            }
        }

        vector4 operator+(const vector4& other) const {
            return vector4(m_x + other.m_x, m_y + other.m_y, m_z + other.m_z, m_w + other.m_w);
        }

        vector4 operator-(const vector4& other) const {
            return vector4(m_x - other.m_x, m_y - other.m_y, m_z - other.m_z, m_w - other.m_w);
        }

        vector4 operator*(T scalar) const {
            return vector4(m_x * scalar, m_y * scalar, m_z * scalar, m_w * scalar);
        }

        friend vector4 operator*(T scalar, const vector4& vec) {
            return vec * scalar;
        }

        // dot product
        T operator*(const vector4& other) const {
            return m_x * other.m_x + m_y * other.m_y + m_z * other.m_z + m_w * other.m_w;
        }

        // vector4 * matrix4
        vector4 operator*(const matrix4<T>& other) const {
            return vector4(
                other[0] * m_x + other[4] * m_y + other[8]  * m_z + other[12] * m_w,
                other[1] * m_x + other[5] * m_y + other[9]  * m_z + other[13] * m_w,
                other[2] * m_x + other[6] * m_y + other[10] * m_z + other[14] * m_w,
                other[3] * m_x + other[7] * m_y + other[11] * m_z + other[15] * m_w
            );
        }

        vector4 operator-() const {
            return vector4(-m_x, -m_y, -m_z, -m_w);
        }

        T length() const {
            return std::sqrt(m_x * m_x + m_y * m_y + m_z * m_z + m_w * m_w);
        }

        bool operator==(const vector4& other) const {
            return m_x == other.m_x && m_y == other.m_y && m_z == other.m_z && m_w == other.m_w;
        }

        bool operator!=(const vector4& other) const {
            return !(*this == other);
        }

        friend std::ostream& operator<<(std::ostream& os, const vector4& vec) {
            os << "(" << vec.m_x << ", " << vec.m_y << ", " << vec.m_z << ", " << vec.m_w << ")";
            return os;
        }

        void normalize() {
            const T len = this->length();
            if (len == 0) {
                throw std::logic_error("Length was equal to zero for vector4");
            }
            m_x /= len;
            m_y /= len;
            m_z /= len;
            m_w /= len;
        }

        vector4 normalized() const {
            const T inv_len = 1 / this->length();
            return *this * inv_len;
        }

    private:
        T m_x, m_y, m_z, m_w;
    };

    template<floating_point T>
    T dot(const vector4<T>& vec1, const vector4<T>& vec2) {
        return vec1 * vec2;
    }

    template<floating_point T>
    vector4<T> normalize(const vector4<T>& vec) {
        return vec.normalized();
    }

    template<floating_point T>
    vector4<T> project(const vector4<T>& u, const vector4<T>& v) {
        return ((v * u) / (u * u)) * u;
    }

    template<floating_point T>
    T distance(const vector4<T>& p, const vector4<T>& q) {
        const T diffX = p.x() - q.x();
        const T diffY = p.y() - q.y();
        const T diffZ = p.z() - q.z();
        const T diffW = p.w() - q.w();
        return std::sqrt(diffX * diffX + diffY * diffY + diffZ * diffZ + diffW * diffW);
    }
}