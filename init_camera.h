#ifndef INIT_CAMERA_H
#define INIT_CAMERA_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

enum struct CameraMovement {
	kForward,
	kBackward,
	kLeft,
	kRight
};

class Camera
{
public:
	// camera options
	float zoom;

	// constructor with vectors
	explicit Camera(
		glm::vec3 position,
		float yaw = -90.0f, float pitch = 0.0f)
		: front_(glm::vec3(0.0f, 0.0f, -1.0f)), zoom(45.0f)
	{
		position_ = position;
		yaw_ = yaw;
		pitch_ = pitch;
		UpdateCameraFront();
	}

	// returns the view matrix calculated using Euler Angles and the LookAt Matrix
	glm::mat4 GetViewMatrix()
	{
		return glm::lookAt(position_, position_ + front_, up_);
	}

	// processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
	void ProcessKeyboard(const CameraMovement direction, const float delta_time)
	{
		const float velocity = kMoveSpeed * delta_time;
		if (direction == CameraMovement::kForward)
			position_ += front_ * velocity;
		else if (direction == CameraMovement::kBackward)
			position_ -= front_ * velocity;
		else if (direction == CameraMovement::kLeft)
			position_ -= right_ * velocity;
		else if (direction == CameraMovement::kRight)
			position_ += right_ * velocity;
	}

	// processes input received from a mouse input system. Expects the offset value in both the x and y direction.
	void ProcessMouseMovement(double x_offset, double y_offset)
	{
		x_offset *= kMouseSensitivity;
		y_offset *= kMouseSensitivity;
		yaw_ += x_offset;
		pitch_ += y_offset;

		if (pitch_ > 89.0f)
			pitch_ = 89.0f;
		else if (pitch_ < -89.0f)
			pitch_ = -89.0f;
		UpdateCameraFront();
	}

	void ProcessMouseScroll(float yoffset)
	{
		this->zoom -= (float)yoffset;
		if (this->zoom < 1.0f)
			this->zoom = 1.0f;
		else if (this->zoom > 90.0f)
			this->zoom = 90.0f;
	}

private:
	const float kMoveSpeed = 2.5f;
	const float kMouseSensitivity = 0.1f;
	const glm::vec3 kWorldUp = glm::vec3(0.0f, 1.0f, 0.0f);

	glm::vec3 position_;
	glm::vec3 front_;
	glm::vec3 up_;
	glm::vec3 right_;
	float yaw_;
	float pitch_;

	void UpdateCameraFront()
	{
		glm::vec3 current_front;
		current_front.x = cos(glm::radians(this->yaw_)) * cos(glm::radians(this->pitch_));
		current_front.y = sin(glm::radians(this->pitch_));
		current_front.z = sin(glm::radians(this->yaw_)) * cos(glm::radians(this->pitch_));
		front_ = glm::normalize(current_front);
		// also re-calculate the Right and Up vector
		// normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
		right_ = glm::normalize(glm::cross(current_front, kWorldUp));
		up_ = glm::normalize(glm::cross(right_, current_front));
	}
};
#endif