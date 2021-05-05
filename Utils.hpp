#pragma once

#include "linalg.h"
#include <math.h>

using namespace linalg::aliases;

namespace CWire3DUtilities {

	const float pi = 3.14159265f;
	const float half_pi = pi / 2.0f;

	const float to_rad = pi / 180;
	const float to_deg = 180 / pi;

	float3x3 get_3d_rot_x(float x);
	float3x3 get_3d_rot_y(float y);
	float3x3 get_3d_rot_z(float z);

	float3x3 get_3d_rot(float3 angle);

	float to_radians(float degrees);
	float to_degrees(float radians);
}