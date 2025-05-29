#pragma once
#include <vector>

struct float3 {
    float x, y, z;
};

struct uint2 {
    unsigned int u, v;
};

struct quaternion {
    float x, y, z, w;
};

struct pointRef {
    unsigned int id;
};

using controlPoints = std::vector<pointRef>;