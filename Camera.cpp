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

		clip = float2{ 0.0f, 0.0f };
	}

	void Camera::set_clip(float2 clip) {
		this->clip = clip;
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

	float3 Camera::project_point(float3 point, bool offset_to_center) {
		float3 rotated = mul(rotation_matrix, (point - position));

		if (rotated.z <= 0.0f || should_clip(rotated.z)) {
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

		return float3{ x, y, rotated.z };
	}

	float3 Camera::get_position() {
		return position;
	}

	float3 Camera::get_angle() {
		return angle;
	}

	void Camera::set_position(float3 position) {
		this->position = position;
	}

	void Camera::set_angle(float3 angle) {
		this->angle = angle;
	}

	bool Camera::should_clip(float z_depth) {
		return !(clip.x < z_depth && z_depth < clip.y);
	}

	bool Camera::should_clip_sides(float3 p1, float3 p2, float3 p3) {
		return should_clip_sides(p1.x, p1.y) && should_clip_sides(p2.x, p2.y) && should_clip_sides(p3.x, p3.y);
	}

	bool Camera::should_clip_sides(float x, float y) {
		return (x < -32.0f || x > display_size.x + 32.0f || y < -32.0f || y > display_size.y + 32.0f);
		//return (x < 0.0f || x > display_size.x || y < 0.0f || y > display_size.y);
	}
}