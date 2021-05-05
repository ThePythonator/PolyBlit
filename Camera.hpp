#pragma once

#include "linalg.h"
#include "Utils.hpp"

using namespace linalg::aliases;

namespace CWire3DEntities {
	class Camera {
	public:
		Camera() : Camera(uint2{ 320, 240 }) { };
		Camera(uint2 display_size, float3 position = float3{ 0.0f, 0.0f, 0.0f }, float3 angle = float3{ 0.0f, 0.0f, 0.0f }, uint8_t fov = 90, bool flip_y = true);

		void set_clip(float2 clip);

		void update_rotation();

		void rotate(float3 angle);
		void translate(float3 amount);
		void move(float3 amount);

		float3 project_point(float3 point, bool offset_to_center = true, bool clip_sides = false);//true

		float3 get_position();
		float3 get_angle();

		void set_position(float3 position);
		void set_angle(float3 angle);

		bool should_clip(float z_depth);

	protected:
		uint8_t fov;
		float f;

		float2 clip;

		float3 position, angle;
		uint2 display_size, display_size_half;

		float3x3 rotation_matrix;

		bool flip_y;
	};
}