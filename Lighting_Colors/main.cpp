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
#include <init_shaders.h>

unsigned int BuildShaderProgram()
{
	unsigned int vertex_shader = glCreateShader(GL_VERTEX_SHADER), fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
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
	glShaderSource(vertex_shader, 1, &vertex_shader_script, NULL);	glCompileShader(vertex_shader);

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
	glShaderSource(fragment_shader, 1, &fragment_shader_script, NULL);	glCompileShader(fragment_shader);
	return LinkToShaderProgram(vertex_shader, fragment_shader);
}

int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
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
	const auto [vao, vbo, ebo] = GetVaoVboEbo();
	const auto& [container_tex, face_tex] = LoadTexture(shader_program);
	float delta_time_between_frames = 0.0f, last_frame_time = 0.0f;
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, container_tex);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, face_tex);	
	while (!glfwWindowShouldClose(window))
	{
		glClearColor(0.2f, 0.3f, 0.3f, 0.2f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glUseProgram(shader_program);		
		glBindVertexArray(vao);

		float current_frame_time = static_cast<float>(glfwGetTime());
		delta_time_between_frames = current_frame_time - last_frame_time;
		last_frame_time = current_frame_time;

		view_matrix = camera.GetViewMatrix();
		projection_matrix = glm::perspective(glm::radians(camera.zoom), 800.0f / 600.0f, 0.1f, 100.0f);
		glUniformMatrix4fv(glGetUniformLocation(shader_program, "view"), 1, GL_FALSE, glm::value_ptr(view_matrix));
		glUniformMatrix4fv(glGetUniformLocation(shader_program, "projection"), 1, GL_FALSE, glm::value_ptr(projection_matrix));
		int model_location = glGetUniformLocation(shader_program, "model");
		for (int i = 0; i < 10; ++i)
		{
			model_matrix = glm::translate(glm::mat4(1.0f), cube_positions[i]);
			model_matrix = glm::rotate(model_matrix, glm::radians(20.0f * i), glm::vec3(1.0f, 0.3f, 0.5f));
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