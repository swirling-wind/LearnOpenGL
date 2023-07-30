#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <iostream>
#include <array>
#include <init_camera.h>

unsigned int LinkToShaderProgram(unsigned int vertex_shader, unsigned int fragment_shader)
{
	int success = 1;
	char info_log[512];
	glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
	if (success == 0)
	{
		glGetShaderInfoLog(vertex_shader, 512, NULL, info_log);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << info_log << std::endl;
		success = 1;
		memset(info_log, 0, sizeof(info_log));
	}
	glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
	if (success == 0)
	{
		glGetShaderInfoLog(fragment_shader, 512, NULL, info_log);
		std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << info_log << std::endl;
		success = 1;
		memset(info_log, 0, sizeof(info_log));
	}
	unsigned int shader_program = glCreateProgram();
	glAttachShader(shader_program, vertex_shader);
	glAttachShader(shader_program, fragment_shader);
	glLinkProgram(shader_program);
	glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
	if (success == 0)
	{
		glGetProgramInfoLog(shader_program, 512, NULL, info_log);
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << info_log << std::endl;
	}
	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);
	return shader_program;
}

unsigned int BuildShaderProgram()
{
	unsigned int vertex_shader, fragment_shader;
	const char* vertex_shader_script = R"(
		#version 330 core
		layout (location = 0) in vec3 orig_pos;
		layout (location = 4) in vec2 orig_tex_coord;
		out vec2 inter_tex_coord;

		uniform mat4 model;
		uniform mat4 view;
		uniform mat4 projection;

		void main() {
			gl_Position = projection * view * model * vec4(orig_pos, 1.0);
			inter_tex_coord = orig_tex_coord;
		})";
	vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex_shader, 1, &vertex_shader_script, NULL);
	glCompileShader(vertex_shader);

	const char* fragment_shader_script = R"(
		#version 330 core
		out vec4 frag_color;
		in vec2 inter_tex_coord;

		uniform sampler2D face_tex;
		uniform sampler2D container_tex;
		void main() {
			frag_color =mix(texture(face_tex, inter_tex_coord), 
							texture(container_tex, inter_tex_coord), 0.5);
		})";
	fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment_shader, 1, &fragment_shader_script, NULL);
	glCompileShader(fragment_shader);
	return LinkToShaderProgram(vertex_shader, fragment_shader);
}

std::tuple<unsigned int, unsigned int, unsigned int> GetVaoVboEbo()
{
	float vertices[] = {
		// Position				// Texture Coordinates
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
		 0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

		-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f
	};
	unsigned int indices[] = {
		0, 1, 2,
		1, 2, 3,
	};
	unsigned int vbo, vao, ebo;
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);
	glGenBuffers(1, &ebo);
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
		(void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
		(void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(4);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	return { vao, vbo, ebo };
}

std::tuple<unsigned int, unsigned int> LoadTexture(unsigned int shader_program)
{
	unsigned int container_tex, face_tex;
	stbi_set_flip_vertically_on_load(true);
	int width, height, channel_num_per_pixel;
	glGenTextures(1, &container_tex);
	glBindTexture(GL_TEXTURE_2D, container_tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	unsigned char* container_data = stbi_load("container.jpg",
		&width, &height, &channel_num_per_pixel, 0);
	if (container_data != nullptr)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0,
			GL_RGB, GL_UNSIGNED_BYTE, container_data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture container" << std::endl;
	}

	width = 0; height = 0; channel_num_per_pixel = 0;
	glGenTextures(1, &face_tex);
	glBindTexture(GL_TEXTURE_2D, face_tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	unsigned char* face_data = stbi_load("awesomeface.png",
		&width, &height, &channel_num_per_pixel, 0);
	if (face_data != nullptr)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0,
			GL_RGBA, GL_UNSIGNED_BYTE, face_data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture face" << std::endl;
	}
	stbi_image_free(container_data);
	stbi_image_free(face_data);
	glUseProgram(shader_program);
	glUniform1i(glGetUniformLocation(shader_program, "face_tex"), 0);
	glUniform1i(glGetUniformLocation(shader_program, "container_tex"), 1);
	return { container_tex, face_tex };
}

int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	GLFWwindow* window = glfwCreateWindow(800, 600, "LearnOpenGL", NULL, NULL);
	SetCallbackAndLoadGlad(window);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

	glm::vec3 cube_positions[] = {
		glm::vec3(0.0f,  0.0f,  0.0f),
		glm::vec3(2.0f,  5.0f, -15.0f),
		glm::vec3(-1.5f, -2.2f, -2.5f),
		glm::vec3(-3.8f, -2.0f, -12.3f),
		glm::vec3(2.4f, -0.4f, -3.5f),
		glm::vec3(-1.7f,  3.0f, -7.5f),
		glm::vec3(1.3f, -2.0f, -2.5f),
		glm::vec3(1.5f,  2.0f, -2.5f),
		glm::vec3(1.5f,  0.2f, -1.5f),
		glm::vec3(-1.3f,  1.0f, -1.5f)
	};
	glm::mat4 model_matrix = glm::mat4(1.0f), view_matrix = glm::mat4(1.0f), projection_matrix = glm::mat4(1.0f);
	const unsigned int shader_program = BuildShaderProgram();
	auto [vao, vbo, ebo] = GetVaoVboEbo();
	auto textures_id = LoadTexture(shader_program);
	float delta_time_between_frames = 0.0f, last_frame_time = 0.0f;
	while (!glfwWindowShouldClose(window))
	{
		const auto& [container_tex, face_tex] = textures_id;
		glClearColor(0.2f, 0.3f, 0.3f, 0.2f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glUseProgram(shader_program);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, container_tex);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, face_tex);
		glBindVertexArray(vao);

		float current_frame_time = static_cast<float>(glfwGetTime());
		delta_time_between_frames = current_frame_time - last_frame_time;
		last_frame_time = current_frame_time;

		view_matrix = camera.GetViewMatrix();
		projection_matrix = glm::perspective(glm::radians(camera.zoom), 800.0f / 600.0f, 0.1f, 100.0f);
		int view_location = glGetUniformLocation(shader_program, "view");
		glUniformMatrix4fv(view_location, 1, GL_FALSE, glm::value_ptr(view_matrix));
		int projection_location = glGetUniformLocation(shader_program, "projection");
		glUniformMatrix4fv(projection_location, 1, GL_FALSE, glm::value_ptr(projection_matrix));
		int model_location = glGetUniformLocation(shader_program, "model");
		for (int i = 0; i < 10; ++i)
		{
			float angle = 20.0f * i;
			model_matrix = glm::translate(glm::mat4(1.0f), cube_positions[i]);
			model_matrix = glm::rotate(model_matrix, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
			if ((i + 1) % 3 == 0)
			{
				model_matrix = glm::rotate(model_matrix, current_frame_time * 0.2f, glm::vec3(1.0f, 0.3f, 0.5f));
			}
			glUniformMatrix4fv(model_location, 1, GL_FALSE, glm::value_ptr(model_matrix));
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}
		glBindVertexArray(0);
		KeyboardCallback(window, delta_time_between_frames);
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &vbo);
	glDeleteBuffers(1, &ebo);
	glDeleteProgram(shader_program);
	glfwTerminate();
	return 0;
}