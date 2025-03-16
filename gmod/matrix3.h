#pragma once
#include "pch.h"
#include "vector3.h"

namespace gmod {
    template<floating_point T>
    struct vector3;

    template<floating_point T>
    struct matrix3 {
    public:
        matrix3(const vector3<T>& vec) : m_A{ vec.x(), 0, 0, 0, vec.y(), 0, 0, 0, vec.z() } {}

        matrix3(
            T a00 = 0, T a01 = 0, T a02 = 0,
            T a10 = 0, T a11 = 0, T a12 = 0,
            T a20 = 0, T a21 = 0, T a22 = 0
        ) : m_A{ a00, a01, a02, a10, a11, a12, a20, a21, a22 } {}

        matrix3(std::array<T, 9>&& A) : m_A(std::move(A)) {}

        static matrix3 identity() {
            return matrix3(
                1, 0, 0,
                0, 1, 0,
                0, 0, 1
            );
        }

        T& operator[](int index) {
            if (index < 0 || index >= 9) {
                throw std::out_of_range("Index out of range for matrix3");
            }
            return m_A[index];
        }

        const T& operator[](int index) const {
            if (index < 0 || index >= 9) {
                throw std::out_of_range("Index out of range for matrix3");
            }
            return m_A.at(index);
        }

        T& operator()(int row, int col) {
            if (row < 0 || row >= 3 || col < 0 || col >= 3) {
                throw std::out_of_range("Indcies out of range for matrix3");
            }
            return m_A[row * 3 + col];
        }

        const T& operator()(int row, int col) const {
            if (row < 0 || row >= 3 || col < 0 || col >= 3) {
                throw std::out_of_range("Indcies out of range for matrix3");
            }
            return m_A.at(row * 3 + col);
        }

        matrix3 operator+(const matrix3& other) const {
            matrix3 result;
            for (int i = 0; i < 9; ++i) {
                result.m_A[i] = m_A[i] + other.m_A[i];
            }
            return result;
        }

        matrix3 operator-(const matrix3& other) const {
            matrix3 result;
            for (int i = 0; i < 9; ++i) {
                result.m_A[i] = m_A[i] - other.m_A[i];
            }
            return result;
        }

        matrix3 operator*(T scalar) const {
            matrix3 result;
            for (int i = 0; i < 9; ++i) {
                result.m_A[i] = m_A[i] * scalar;
            }
            return result;
        }

        friend matrix3 operator*(T scalar, const matrix3& mat) {
            return mat * scalar;
        }

        // matrix3 * vector3
        vector3<T> operator*(const vector3<T>& vec) const {
            return vector3<T>(
                m_A[0] * vec.x() + m_A[1] * vec.y() + m_A[2] * vec.z(),
                m_A[3] * vec.x() + m_A[4] * vec.y() + m_A[5] * vec.z(), 
                m_A[6] * vec.x() + m_A[7] * vec.y() + m_A[8] * vec.z() 
            );
        }

        // matrix3 * matrix3
        matrix3 operator*(const matrix3& other) const {
            matrix3 result;

            result.m_A[0] = m_A[0] * other.m_A[0] + m_A[1] * other.m_A[3] + m_A[2] * other.m_A[6];
            result.m_A[1] = m_A[0] * other.m_A[1] + m_A[1] * other.m_A[4] + m_A[2] * other.m_A[7];
            result.m_A[2] = m_A[0] * other.m_A[2] + m_A[1] * other.m_A[5] + m_A[2] * other.m_A[8];
                   
            result.m_A[3] = m_A[3] * other.m_A[0] + m_A[4] * other.m_A[3] + m_A[5] * other.m_A[6];
            result.m_A[4] = m_A[3] * other.m_A[1] + m_A[4] * other.m_A[4] + m_A[5] * other.m_A[7];
            result.m_A[5] = m_A[3] * other.m_A[2] + m_A[4] * other.m_A[5] + m_A[5] * other.m_A[8];
           
            result.m_A[6] = m_A[6] * other.m_A[0] + m_A[7] * other.m_A[3] + m_A[8] * other.m_A[6];
            result.m_A[7] = m_A[6] * other.m_A[1] + m_A[7] * other.m_A[4] + m_A[8] * other.m_A[7];
            result.m_A[8] = m_A[6] * other.m_A[2] + m_A[7] * other.m_A[5] + m_A[8] * other.m_A[8];

            return result;
        }

        void transpose() {
            for (int i = 0; i < 3; i++) {
                for (int j = i + 1; j < 3; j++) {
                    const T tmp = (*this)(i, j);
                    (*this)(i, j) = (*this)(j, i);
                    (*this)(j, i) = tmp;
                }
            }
        }

        matrix3 transposed() const {
            matrix3 result;
            for (int i = 0; i < 3; i++) {
                for (int j = 0; j < 3; j++) {
                    result(i, j) = (*this)(j, i);
                }
            }
            return result;
        }

        matrix3 operator-() const {
            matrix3 result;
            for (int i = 0; i < 9; i++) {
                result.m_A[i] = -m_A[i];
            }
            return result;
        }

        bool operator==(const matrix3& other) const {
            return m_A == other.m_A;
        }

        bool operator!=(const matrix3& other) const {
            return !(*this == other);
        }

        friend std::ostream& operator<<(std::ostream& os, const matrix3& mat) {
            os << "[" << m_A[0] << ", " << m_A[1] << ", " << m_A[2] << "]" << std::endl
               << "[" << m_A[3] << ", " << m_A[4] << ", " << m_A[5] << "]" << std::endl
               << "[" << m_A[6] << ", " << m_A[7] << ", " << m_A[8] << "]";
            return os;
        }

        // Gram-Schmidt
        void orthonormalize() {
            vector3<T> X(m_A[0], m_A[3], m_A[6]);
            vector3<T> Y(m_A[1], m_A[4], m_A[7]);
            vector3<T> Z(m_A[2], m_A[5], m_A[8]);

            Y = Y - project(X, Y);
            auto Z_1 = Z - project(X, Z);
            Z = Z_1 - project(Y, Z_1);;

            X.normalize();
            Y.normalize();
            Z.normalize();

            m_A[0] = X.x();  m_A[1] = Y.x();  m_A[2] = Z.x(); 
            m_A[3] = X.y();  m_A[4] = Y.y();  m_A[5] = Z.y(); 
            m_A[6] = X.z();  m_A[7] = Y.z();  m_A[8] = Z.z();
        }

        matrix3 orthonormalized() const {
            matrix3 result(*this);
            result.orthonormalize();
            return result;
        }

    private:
        std::array<T, 9> m_A;
    };
}