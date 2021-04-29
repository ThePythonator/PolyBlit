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
            get_3d_rot_z(angle.z),
            get_3d_rot_y(angle.y),
            get_3d_rot_x(angle.x)
        );
    }

    float to_radians(float degrees) {
        return degrees * to_rad;
    }

    float to_degrees(float radians) {
        return radians * to_deg;
    }
}