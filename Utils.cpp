#include "Utils.hpp"

namespace CWire3DUtilities {

    float3x3 get_3d_rot_x(float x) {
        float s_x = sin(x);
        float c_x = cos(x);

        return {
            {1.0f, 0.0f, 0.0f},
            {0.0f, c_x, s_x},
            {0.0f, -s_x, c_x}
        };
    }

    float3x3 get_3d_rot_y(float y) {
        float s_y = sin(y);
        float c_y = cos(y);

        return {
            {c_y, 0.0f, -s_y},
            {0.0f, 1.0f, 0.0f},
            {s_y, 0.0f, c_y}
        };
    }

    float3x3 get_3d_rot_z(float z) {
        float s_z = sin(z);
        float c_z = cos(z);

        return {
            {c_z, s_z, 0.0f},
            {-s_z, c_z, 0.0f},
            {0.0f, 0.0f, 1.0f}
        };
    }

    float3x3 get_3d_rot(float3 angle) {
        return mul(
            get_3d_rot_x(angle.x),
            get_3d_rot_y(angle.y),
            get_3d_rot_z(angle.z)
        );
    }

    float to_radians(float degrees) {
        return degrees * to_rad;
    }

    float to_degrees(float radians) {
        return radians * to_deg;
    }

    // Following function taken from https://blackpawn.com/texts/pointinpoly/
    bool in_triangle(float3 p, float3 t1, float3 t2, float3 t3) {
        // Compute vectors        
        float3 v1 = t3 - t1;
        float3 v2 = t2 - t1;
        float3 v3 = p - t1;

        // Compute dot products
        float d11 = dot(v1, v1);
        float d12 = dot(v1, v2);
        float d13 = dot(v1, v3);
        float d22 = dot(v2, v2);
        float d23 = dot(v2, v3);

        // Compute barycentric coordinates
        float invDenom = 1.0f / (d11 * d22 - d12 * d12);
        float u = (d22 * d13 - d12 * d23) * invDenom;
        float v = (d11 * d23 - d12 * d13) * invDenom;

        // Check if point is in triangle
        return (u >= 0) && (v >= 0) && (u + v < 1);
    }
}