#ifndef INIT_CAMERA_H
#define INIT_CAMERA_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <iostream>

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
		: front_(glm::vec3(0.0f, 0.0f, -1.0f)), zoom(60.0f)
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

extern Camera camera = Camera{ glm::vec3(0.0f, 0.0f, 3.0f) };
extern float x_last_time = 800.0f / 2.0f;
extern float y_last_time = 600.0f / 2.0f;
extern bool is_front_locked = true;

std::ostream& operator<<(std::ostream& out, const glm::vec3& g)
{
	return out << glm::to_string(g);
}

std::ostream& operator<<(std::ostream& out, const glm::mat4& g)
{
	return out << glm::to_string(g);
}

void ScrollCallback(GLFWwindow* window, double x_offset, double y_offset)
{
	camera.ProcessMouseScroll(y_offset);
}

void KeyboardCallback(GLFWwindow* window, const float delta_time_between_frames)
{
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
	{
		is_front_locked = true;
	}
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
	{
		is_front_locked = false;
	}
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, true);
	}
	float camera_speed = 2.0f * delta_time_between_frames;
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
	{
		camera.ProcessKeyboard(CameraMovement::kForward, camera_speed);
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
	{
		camera.ProcessKeyboard(CameraMovement::kBackward, camera_speed);
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
	{
		camera.ProcessKeyboard(CameraMovement::kLeft, camera_speed);
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
	{
		camera.ProcessKeyboard(CameraMovement::kRight, camera_speed);
	}
}

void FramebufferSizeCallback(GLFWwindow*, int width, int height)
{
	glViewport(0, 0, width, height);
}

void MouseCallback(GLFWwindow* window, double in_x_pos, double in_y_pos)
{
	float x_pos = static_cast<float>(in_x_pos);
	float y_pos = static_cast<float>(in_y_pos);

	float x_offset = x_pos - x_last_time;
	float y_offset = y_last_time - y_pos; // reversed since y-coordinates go from bottom to top
	x_last_time = x_pos;
	y_last_time = y_pos;
	if (!is_front_locked)
	{
		camera.ProcessMouseMovement(x_offset, y_offset);
	}
}

void SetCallbackAndLoadGlad(GLFWwindow* window)
{

	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		std::exit(EXIT_FAILURE);
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, FramebufferSizeCallback);
	glfwSetCursorPosCallback(window, MouseCallback);
	glfwSetScrollCallback(window, ScrollCallback);
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		std::exit(EXIT_FAILURE);
	}
	glEnable(GL_DEPTH_TEST);
}

#endif // !INIT_CAMERA_H