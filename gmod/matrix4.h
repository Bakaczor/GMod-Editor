#pragma once
#include "pch.h"
#include "vector4.h"
#include "quaternion.h"

namespace gmod {
    template<floating_point T>
    struct vector4;

    template<floating_point T>
    struct matrix4 {
    public:
        matrix4(const vector4<T>& vec) : m_A{ vec.x(), 0, 0, 0, 0, vec.y(), 0, 0, 0, 0, vec.z(), 0, 0, 0, 0, vec.w() } {}

        matrix4(
            T a00 = 0, T a01 = 0, T a02 = 0, T a03 = 0,
            T a10 = 0, T a11 = 0, T a12 = 0, T a13 = 0,
            T a20 = 0, T a21 = 0, T a22 = 0, T a23 = 0,
            T a30 = 0, T a31 = 0, T a32 = 0, T a33 = 0
        ) : m_A{ a00, a01, a02, a03, a10, a11, a12, a13,
            a20, a21, a22, a23, a30, a31, a32, a33 } {}

        matrix4(std::array<T, 16>&& A) : m_A(std::move(A)) {}

        static matrix4 identity() {
            return matrix4(
                1, 0, 0, 0,
                0, 1, 0, 0,
                0, 0, 1, 0,
                0, 0, 0, 1
            );
        }

        // Rodrigues
        static matrix4 from_angle_axis(T angle, T ax, T ay, T az) {
            const T sin = std::sin(angle);
            const T cos = std::cos(angle);
            const T cos_1 = 1 - cos;

            return matrix4(
                 cos + ax * ax * cos_1,      ax * ay * cos_1 - az * sin,   ay * sin + ax * az * cos_1, 0,
                 az * sin + ax * ay * cos_1, cos + ay * ay * cos_1,       -ax * sin + ay * az * cos_1, 0,
                -ay * sin + ax * az * cos_1, ax * sin + ay * az * cos_1,   cos + az * az * cos_1,      0,
                 0,                          0,                            0,                          1

            );
        }

        static matrix4 from_quaternion(const quaternion<T>& q) {
            return {
                1 - 2 * (q.y() * q.y() + q.z() * q.z()),     2 * (q.x() * q.y() - q.z() * q.w()),     2 * (q.x() * q.z() + q.y() * q.w()), 0,
                    2 * (q.x() * q.y() + q.z() * q.w()), 1 - 2 * (q.x() * q.x() + q.z() * q.z()),     2 * (q.y() * q.z() - q.x() * q.w()), 0,
                    2 * (q.x() * q.z() - q.y() * q.w()),     2 * (q.y() * q.z() + q.x() * q.w()), 1 - 2 * (q.x() * q.x() + q.y() * q.y()), 0,
                    0,                                       0,                                       0,                                   1
            };
        }

        T& operator[](int index) {
            if (index < 0 || index >= 16) {
                throw std::out_of_range("Index out of range for matrix4");
            }
            return m_A[index];
        }

        const T& operator[](int index) const {
            if (index < 0 || index >= 16) {
                throw std::out_of_range("Index out of range for matrix4");
            }
            return m_A.at(index);
        }

        T& operator()(int row, int col) {
            if (row < 0 || row >= 4 || col < 0 || col >= 4) {
                throw std::out_of_range("Indices out of range for matrix4");
            }
            return m_A[row * 4 + col];
        }

        const T& operator()(int row, int col) const {
            if (row < 0 || row >= 4 || col < 0 || col >= 4) {
                throw std::out_of_range("Indices out of range for matrix4");
            }
            return m_A.at(row * 4 + col);
        }

        matrix4 operator+(const matrix4& other) const {
            matrix4 result;
            for (int i = 0; i < 16; ++i) {
                result.m_A[i] = m_A[i] + other.m_A[i];
            }
            return result;
        }

        matrix4 operator-(const matrix4& other) const {
            matrix4 result;
            for (int i = 0; i < 16; ++i) {
                result.m_A[i] = m_A[i] - other.m_A[i];
            }
            return result;
        }

        matrix4 operator*(T scalar) const {
            matrix4 result;
            for (int i = 0; i < 16; ++i) {
                result.m_A[i] = m_A[i] * scalar;
            }
            return result;
        }

        friend matrix4 operator*(T scalar, const matrix4& mat) {
            return mat * scalar;
        }

        // matrix4 * vector4
        vector4<T> operator*(const vector4<T>& vec) const {
            return vector4<T>(
                m_A[0]  * vec.x() + m_A[1]  * vec.y() + m_A[2]  * vec.z() + m_A[3]  * vec.w(),
                m_A[4]  * vec.x() + m_A[5]  * vec.y() + m_A[6]  * vec.z() + m_A[7]  * vec.w(),
                m_A[8]  * vec.x() + m_A[9]  * vec.y() + m_A[10] * vec.z() + m_A[11] * vec.w(),
                m_A[12] * vec.x() + m_A[13] * vec.y() + m_A[14] * vec.z() + m_A[15] * vec.w()
            );
        }

        // matrix4 * matrix4
        matrix4 operator*(const matrix4& other) const {
            matrix4 result;

            result.m_A[0]  = m_A[0]  * other.m_A[0] + m_A[1]  * other.m_A[4] + m_A[2]  * other.m_A[8]  + m_A[3]  * other.m_A[12];
            result.m_A[1]  = m_A[0]  * other.m_A[1] + m_A[1]  * other.m_A[5] + m_A[2]  * other.m_A[9]  + m_A[3]  * other.m_A[13];
            result.m_A[2]  = m_A[0]  * other.m_A[2] + m_A[1]  * other.m_A[6] + m_A[2]  * other.m_A[10] + m_A[3]  * other.m_A[14];
            result.m_A[3]  = m_A[0]  * other.m_A[3] + m_A[1]  * other.m_A[7] + m_A[2]  * other.m_A[11] + m_A[3]  * other.m_A[15];
                                                                                                                 
            result.m_A[4]  = m_A[4]  * other.m_A[0] + m_A[5]  * other.m_A[4] + m_A[6]  * other.m_A[8]  + m_A[7]  * other.m_A[12];
            result.m_A[5]  = m_A[4]  * other.m_A[1] + m_A[5]  * other.m_A[5] + m_A[6]  * other.m_A[9]  + m_A[7]  * other.m_A[13];
            result.m_A[6]  = m_A[4]  * other.m_A[2] + m_A[5]  * other.m_A[6] + m_A[6]  * other.m_A[10] + m_A[7]  * other.m_A[14];
            result.m_A[7]  = m_A[4]  * other.m_A[3] + m_A[5]  * other.m_A[7] + m_A[6]  * other.m_A[11] + m_A[7]  * other.m_A[15];
                                                              
            result.m_A[8]  = m_A[8]  * other.m_A[0] + m_A[9]  * other.m_A[4] + m_A[10] * other.m_A[8]  + m_A[11] * other.m_A[12];
            result.m_A[9]  = m_A[8]  * other.m_A[1] + m_A[9]  * other.m_A[5] + m_A[10] * other.m_A[9]  + m_A[11] * other.m_A[13];
            result.m_A[10] = m_A[8]  * other.m_A[2] + m_A[9]  * other.m_A[6] + m_A[10] * other.m_A[10] + m_A[11] * other.m_A[14];
            result.m_A[11] = m_A[8]  * other.m_A[3] + m_A[9]  * other.m_A[7] + m_A[10] * other.m_A[11] + m_A[11] * other.m_A[15];

            result.m_A[12] = m_A[12] * other.m_A[0] + m_A[13] * other.m_A[4] + m_A[14] * other.m_A[8]  + m_A[15] * other.m_A[12];
            result.m_A[13] = m_A[12] * other.m_A[1] + m_A[13] * other.m_A[5] + m_A[14] * other.m_A[9]  + m_A[15] * other.m_A[13];
            result.m_A[14] = m_A[12] * other.m_A[2] + m_A[13] * other.m_A[6] + m_A[14] * other.m_A[10] + m_A[15] * other.m_A[14];
            result.m_A[15] = m_A[12] * other.m_A[3] + m_A[13] * other.m_A[7] + m_A[14] * other.m_A[11] + m_A[15] * other.m_A[15];

            return result;
        }

        void transpose() {
            for (int i = 0; i < 4; i++) {
                for (int j = i + 1; j < 4; j++) {
                    const T tmp = (*this)(i, j);
                    (*this)(i, j) = (*this)(j, i);
                    (*this)(j, i) = tmp;
                }
            }
        }

        matrix4 transposed() const {
            matrix4 result;
            for (int i = 0; i < 4; i++) {
                for (int j = 0; j < 4; j++) {
                    result(i, j) = (*this)(j, i);
                }
            }
            return result;
        }

        matrix4 operator-() const {
            matrix4 result;
            for (int i = 0; i < 16; i++) {
                result.m_A[i] = -m_A[i];
            }
            return result;
        }

        bool operator==(const matrix4& other) const {
            return m_A == other.m_A;
        }

        bool operator!=(const matrix4& other) const {
            return !(*this == other);
        }

        friend std::ostream& operator<<(std::ostream& os, const matrix4& mat) {
            os << "[" << mat.m_A[0]  << ", " << mat.m_A[1]  << ", " << mat.m_A[2]  << ", " << mat.m_A[3]  << "]" << std::endl
               << "[" << mat.m_A[4]  << ", " << mat.m_A[5]  << ", " << mat.m_A[6]  << ", " << mat.m_A[7]  << "]" << std::endl
               << "[" << mat.m_A[8]  << ", " << mat.m_A[9]  << ", " << mat.m_A[10] << ", " << mat.m_A[11] << "]" << std::endl
               << "[" << mat.m_A[12] << ", " << mat.m_A[13] << ", " << mat.m_A[14] << ", " << mat.m_A[15] << "]";
            return os;
        }

        // Gram-Schmidt
        void orthonormalize() {
            vector4<T> X(m_A[0], m_A[4], m_A[8], m_A[12]);
            vector4<T> Y(m_A[1], m_A[5], m_A[9], m_A[13]);
            vector4<T> Z(m_A[2], m_A[6], m_A[10], m_A[14]);
            vector4<T> W(m_A[3], m_A[7], m_A[11], m_A[15]);

            Y = Y - project(X, Y);

            auto Z_1 = Z - project(X, Z);
            Z = Z_1 - project(Y, Z_1);

            auto W_1 = W - project(X, W);
            auto W_2 = W_1 - project(Y, W_1);
            W = W_2 - project(Z, W_2);

            X.normalize();
            Y.normalize();
            Z.normalize();
            W.normalize();

            m_A[0] = X.x();  m_A[1] = Y.x();  m_A[2] = Z.x();  m_A[3] = W.x();
            m_A[4] = X.y();  m_A[5] = Y.y();  m_A[6] = Z.y();  m_A[7] = W.y();
            m_A[8] = X.z();  m_A[9] = Y.z();  m_A[10] = Z.z(); m_A[11] = W.z();
            m_A[12] = X.w(); m_A[13] = Y.w(); m_A[14] = Z.w(); m_A[15] = W.w();
        }

        matrix4 orthonormalized() const {
            matrix4 result(*this);
            result.orthonormalize();
            return result;
        }

#pragma region SPECIAL_MATRIX

        static matrix4 translation(T tx, T ty, T tz) {
            return matrix4(
                1, 0, 0, tx,
                0, 1, 0, ty,
                0, 0, 1, tz,
                0, 0, 0, 1
            );
        }

        static matrix4 inv_translation(T tx, T ty, T tz) {
            return matrix4(
                1, 0, 0, -tx,
                0, 1, 0, -ty,
                0, 0, 1, -tz,
                0, 0, 0, 1
            );
        }

        static matrix4 rotation(T pitch, T yaw, T roll) {
            const T cp = std::cos(pitch);
            const T sp = std::sin(pitch);
            const T cy = std::cos(yaw);
            const T sy = std::sin(yaw);
            const T cr = std::cos(roll);
            const T sr = std::sin(roll);

            return matrix4(
                 cy * cr, -cp * sr + sp * sy * cr,  sp * sr + cp * sy * cr, 0,
                 cy * sr,  cp * cr + sp * sy * sr, -sp * cr + cp * sy * sr, 0,
                -sy,       sp * cy,                 cp * cy,                0,
                 0,        0,                       0,                      1
            );
        }

        static matrix4 rotationX(T rad) {
            const T cos = std::cos(rad);
            const T sin = std::sin(rad);
            return matrix4(
                1, 0,    0,   0,
                0, cos, -sin, 0,
                0, sin,  cos, 0,
                0, 0,    0,   1
            );
        }

        static matrix4 inv_rotationX(T rad) {
            const T cos = std::cos(rad);
            const T sin = std::sin(rad);
            return matrix4(
                1,  0,   0,   0,
                0,  cos, sin, 0,
                0, -sin, cos, 0,
                0,  0,   0,   1
            );
        }

        static matrix4 rotationY(T rad) {
            const T cos = std::cos(rad);
            const T sin = std::sin(rad);
            return matrix4(
                 cos, 0, sin, 0,
                 0,   1, 0,   0,
                -sin, 0, cos, 0,
                 0,   0, 0,   1
            );
        }

        static matrix4 inv_rotationY(T rad) {
            const T cos = std::cos(rad);
            const T sin = std::sin(rad);
            return matrix4(
                cos, 0, -sin, 0,
                0,   1,  0,   0,
                sin, 0,  cos, 0,
                0,   0,  0,   1
            );
        }

        static matrix4 rotationZ(T rad) {
            const T cos = std::cos(rad);
            const T sin = std::sin(rad);
            return matrix4(
                cos, -sin, 0, 0,
                sin,  cos, 0, 0,
                0,    0, 1, 0,
                0,    0, 0, 1
            );
        }

        static matrix4 inv_rotationZ(T rad) {
            const T cos = std::cos(rad);
            const T sin = std::sin(rad);
            return matrix4(
                 cos, sin, 0, 0,
                -sin, cos, 0, 0,
                 0,   0,   1, 0,
                 0,   0,   0, 1
            );
        }

        static matrix4 scaling(T sx, T sy, T sz) {
            return matrix4(
                sx, 0,  0,  0,
                0,  sy, 0,  0,
                0,  0,  sz, 0,
                0,  0,  0,  1
            );
        }

        static matrix4 inv_scaling(T sx, T sy, T sz) {
            return matrix4(
                1 / sx, 0,      0,      0,
                0,      1 / sy, 0,      0,
                0,      0,      1 / sz, 0,
                0,      0,      0,      1
            );
        }
#pragma endregion

    private:
        std::array<T, 16> m_A;
    };
}

