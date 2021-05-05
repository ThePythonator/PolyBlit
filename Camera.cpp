#include "Camera.hpp"

namespace CWire3DEntities {
	Camera::Camera(uint2 display_size, float3 position, float3 angle, uint8_t fov, bool flip_y) {
		this->display_size = display_size;
		display_size_half = display_size / 2;

		this->position = position;
		this->angle = angle;

		this->fov = CWire3DUtilities::to_radians(fov);
		f = display_size.x / (2 * tan(this->fov));

		this->flip_y = flip_y;
	}

	void Camera::update_rotation() {
		rotation_matrix = CWire3DUtilities::get_3d_rot(angle);
	}

	void Camera::rotate(float3 angle) {
		this->angle = this->angle + angle;
	}

	void Camera::translate(float3 amount) {
		position = position + amount;
	}

	void Camera::move(float3 amount) {
		translate(mul(CWire3DUtilities::get_3d_rot_y(-angle.y), amount));
	}

	float3 Camera::project_point(float3 point, bool offset_to_center, bool clip_sides) {
		float3 rotated = mul(rotation_matrix, (point - position));

		if (rotated.z <= 0.0f) {
			return float3{ 0.0f, 0.0f, 0.0f };
		}

		float x = (rotated.x * f) / rotated.z;
		float y = (rotated.y * f) / rotated.z;

		if (flip_y) {
			y = -y;
		}

		if (offset_to_center) {
			x += display_size_half.x;
			y += display_size_half.y;
		}

		if (clip_sides) {
			if (x < -display_size.x || x > display_size.x * 2 || y < -display_size.y || y > display_size.y * 2) {
				//printf("%f %f %f\n", x, y, rotated.z);
				return float3{ 0.0f, 0.0f, 0.0f };
			}
		}

		return float3{ x, y, rotated.z };
	}

	float3* Camera::get_position() {
		return &position;
	}
}