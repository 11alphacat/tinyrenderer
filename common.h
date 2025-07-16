#pragma once
/*
    common used math and graphic structual definition and function declarations;
*/
#include "tgaimage.h"
#include <cmath>
#include <cassert>
#include <iostream>
#include <array>

namespace comGL {  

template<size_t N> struct vec;
using vec2 = vec<2>;  // Alias for 2D vector
using vec3 = vec<3>;  // Alias for 3D vector

// ====================================== VECTOR =================================
template<size_t N>
struct vec {
    std::array<double, N> data {};
    
    // Default constructor (zero-initialized)
    vec() = default;
    
    // Variadic constructor for element-wise initialization
    template<typename... Args, typename = std::enable_if_t<sizeof...(Args) == N>>
    vec(Args... args) : data{static_cast<double>(args)...} {}

    // Element access with bounds checking
    double& operator[](size_t i) { 
        assert(i < N);
        return data[i];
    }
    
    // Const element access with bounds checking
    double operator[](size_t i) const { 
        assert(i < N);
        return data[i];
    }

    // Dot product (vector multiplication)
    double dot(const vec& rhs) const {
        double sum = 0.0;
        for (size_t i = 0; i < N; ++i) {
            sum += data[i] * rhs.data[i];
        }
        return sum;
    }
    
    // Vector magnitude (Euclidean norm)
    double norm() const {
        return std::sqrt(dot(*this));
    }

    // Unit vector in same direction
    vec normalized() const {
        const double len = norm();
        assert(len > 0.0);
        return *this * (1.0 / len);
    }
    
    // Compound addition assignment
    vec& operator+=(const vec& rhs) {
        for (size_t i = 0; i < N; ++i) 
            data[i] += rhs.data[i];
        return *this;
    }
    
    // Compound subtraction assignment
    vec& operator-=(const vec& rhs) {
        for (size_t i = 0; i < N; ++i) 
            data[i] -= rhs.data[i];
        return *this;
    }
    
    // Compound scalar multiplication
    vec& operator*=(double scalar) {
        for (size_t i = 0; i < N; ++i) 
            data[i] *= scalar;
        return *this;
    }
    
    // Compound element-wise multiplication (Hadamard product)
    vec& operator*=(const vec& rhs) {
        for (size_t i = 0; i < N; ++i) 
            data[i] *= rhs.data[i];
        return *this;
    }
    
    // Compound scalar division
    vec& operator/=(double scalar) {
        assert(scalar != 0.0);
        return *this *= 1.0 / scalar;
    }
};

// ==================================== VECTOR OPERATORS (NON-MEMBER FUNCTION) ===============================
template<size_t N>
vec<N> operator+(vec<N> a, const vec<N>& b) {
    return a += b;
}

template<size_t N>
vec<N> operator-(vec<N> a, const vec<N>& b) {
    return a -= b;
}

template<size_t N>
vec<N> operator*(vec<N> a, double scalar) {
    return a *= scalar;
}

template<size_t N>
vec<N> operator*(double scalar, vec<N> a) {
    return a *= scalar;
}

template<size_t N>
vec<N> operator/(vec<N> a, double scalar) {
    return a /= scalar;
}

// Element-wise multiplication (Hadamard product)
template<size_t N>
vec<N> operator*(vec<N> a, const vec<N>& b) {
    return a *= b;
}

// Stream output operator
template<size_t N>
std::ostream& operator<<(std::ostream& os, const vec<N>& v) {
    os << "[";
    for (size_t i = 0; i < N; ++i) {
        if (i > 0) os << ", ";
        os << v[i];
    }
    os << "]";
    return os;
}
// ========================================== 2D VECTOR ====================================
template<>
struct vec<2> {
    // Union for dual access patterns
    union {
        std::array<double, 2> data {};
        struct { double x, y; };
    };
    
    // Default constructor
    vec() : x(0), y(0) {}
    
    // Component-wise constructor
    vec(double x, double y) : x(x), y(y) {}
    
    // Element access with bounds checking
    double& operator[](size_t i) {
        assert(i < 2);
        return (&x)[i];
    }
    
    // Const element access with bounds checking
    double operator[](size_t i) const {
        assert(i < 2);
        return (&x)[i];
    }
    
    // Optimized dot product
    double dot(const vec2& rhs) const {
        return x*rhs.x + y*rhs.y;
    }
    
    // 2D cross product (returns scalar)
    double cross(const vec2& rhs) const {
        return x*rhs.y - y*rhs.x;
    }
    
    // Angle from positive x-axis
    double angle() const {
        return std::atan2(y, x);
    }
    
    // Rotation transformation
    vec2 rotated(double angle) const {
        const double cos_ang = std::cos(angle);
        const double sin_ang = std::sin(angle);
        return {
            x*cos_ang - y*sin_ang,  // New x component
            x*sin_ang + y*cos_ang   // New y component
        };
    }
    
    // Compound addition assignment
    vec2& operator+=(const vec2& rhs) {
        x += rhs.x;
        y += rhs.y;
        return *this;
    }
};

// =========================================== 3D VECTOR =====================================
template<>
struct vec<3> {
    // Union allows both array-style and named member access
    union {
        std::array<double, 3> data {};
        struct { double x, y, z; };
    };
    
    // Default constructor
    vec() = default;
    
    // Component-wise constructor
    vec(double x, double y, double z) : x(x), y(y), z(z) {}
    
    // Element access with bounds checking
    double& operator[](size_t i) {
        assert(i < 3);
        return (&x)[i];
    }
    
    // Const element access with bounds checking
    double operator[](size_t i) const {
        assert(i < 3);
        return (&x)[i];
    }
    
    // Optimized dot product
    double dot(const vec3& rhs) const {
        return x*rhs.x + y*rhs.y + z*rhs.z;
    }
    
    // Cross product (3D-specific operation)
    vec3 cross(const vec3& rhs) const {
        return {
            y*rhs.z - z*rhs.y,   // X component
            z*rhs.x - x*rhs.z,   // Y component
            x*rhs.y - y*rhs.x    // Z component
        };
    }
    
    // Compound addition assignment
    vec3& operator+=(const vec3& rhs) {
        x += rhs.x;
        y += rhs.y;
        z += rhs.z;
        return *this;
    }

    vec3& operator+=(double scalar) {
        x += scalar;
        y += scalar;
        z += scalar;
        return *this;
    }
    
    // Compound subtraction assignment
    vec3& operator-=(const vec3& rhs) {
        x -= rhs.x;
        y -= rhs.y;
        z -= rhs.z;
        return *this;
    }
    
    vec3& operator-=(double scalar) {
        operator+=(-scalar);
        return *this;
    }


    // Compound scalar multiplication
    vec3& operator*=(double scalar) {
        x *= scalar;
        y *= scalar;
        z *= scalar;
        return *this;
    }
    
    // Compound element-wise multiplication
    vec3& operator*=(const vec3& rhs) {
        x *= rhs.x;
        y *= rhs.y;
        z *= rhs.z;
        return *this;
    }
    
    // Compound scalar division
    vec3& operator/=(double scalar) {
        assert(scalar != 0.0);
        return *this *= 1.0 / scalar;
    }
    
    // Plane projections
    vec2 xy() const { return {x, y}; }
    vec2 xz() const { return {x, z}; }
    vec2 yz() const { return {y, z}; }
};


// ===================================== VERTEX =====================================
struct Vertex {
    vec3 position {};    
    vec3 texCoord {};    
    vec3 normal {};      
};
  

// ===================================== DRAW =====================================
void line(vec2 from, vec2 to, TGAImage &framebuffer, const TGAColor& color);
void triangle(vec3 a, vec3 b, vec3 c, TGAImage &zbuffer, TGAImage &framebuffer, TGAColor color);



// ===================================== MATH =====================================

}; // namespace comGL