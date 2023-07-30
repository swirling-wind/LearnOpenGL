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
#include <Lighting_Colors/init_camera.h>
#include <Lighting_Colors/init_shaders.h>

unsigned int BuildObjShader()
{
	unsigned int vertex_shader = glCreateShader(GL_VERTEX_SHADER), fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	const char* vertex_shader_script = R"(
		#version 330 core
		layout (location = 0) in vec3 orig_pos;

		uniform mat4 model;
		uniform mat4 view;
		uniform mat4 projection;

		void main() {
			gl_Position = projection * view * model * vec4(orig_pos, 1.0);
		})";
	glShaderSource(vertex_shader, 1, &vertex_shader_script, NULL);	glCompileShader(vertex_shader);
	const char* fragment_shader_script = R"(
		#version 330 core
		out vec4 frag_color;

		uniform vec3 object_color;
		uniform vec3 light_color;
		void main() {
			frag_color = vec4(light_color * object_color, 1.0);
		})";
	glShaderSource(fragment_shader, 1, &fragment_shader_script, NULL);	glCompileShader(fragment_shader);
	return LinkToShaderProgram(vertex_shader, fragment_shader);
}

unsigned int BuildLightShader()
{
	unsigned int light_frag_shader = glCreateShader(GL_FRAGMENT_SHADER);
	const char* light_frag_shader_script = R"(
		#version 330 core
		out vec4 frag_color;
		void main()
		{
			frag_color = vec4(1.0); 
		}
	)";
	glShaderSource(light_frag_shader, 1, &light_frag_shader_script, NULL);	glCompileShader(light_frag_shader);


	return 0;
}

int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	GLFWwindow* window = glfwCreateWindow(800, 600, "LearnOpenGL", NULL, NULL);
	SetCallbackAndLoadGlad(window);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

	glm::mat4 model_matrix = glm::mat4(1.0f), view_matrix = glm::mat4(1.0f), projection_matrix;
	const unsigned int shader_program = BuildObjShader();
	const auto [vao, vbo, light_vao] = GetVaoVboAndLightVao();
	float delta_time_between_frames = 0.0f, last_frame_time = 0.0f;

	const auto obj_color = glm::vec3(1.0f, 0.5, 0.31f);
	const auto light_color = glm::vec3(1.0f, 1.0f, 1.0f);

	while (!glfwWindowShouldClose(window))
	{
		glClearColor(0.2f, 0.3f, 0.3f, 0.2f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glUseProgram(shader_program);
		glBindVertexArray(vao);

		float current_frame_time = static_cast<float>(glfwGetTime());
		delta_time_between_frames = current_frame_time - last_frame_time;
		last_frame_time = current_frame_time;

		glUniform3fv(glGetUniformLocation(shader_program, "object_color"), 1, &obj_color[0]);
		glUniform3fv(glGetUniformLocation(shader_program, "light_color"), 1, &light_color[0]);

		view_matrix = camera.GetViewMatrix();
		projection_matrix = glm::perspective(glm::radians(camera.zoom), 800.0f / 600.0f, 0.1f, 100.0f);
		glUniformMatrix4fv(glGetUniformLocation(shader_program, "view"), 1, GL_FALSE, glm::value_ptr(view_matrix));
		glUniformMatrix4fv(glGetUniformLocation(shader_program, "projection"), 1, GL_FALSE, glm::value_ptr(projection_matrix));
		int model_location = glGetUniformLocation(shader_program, "model");
		glUniformMatrix4fv(model_location, 1, GL_FALSE, glm::value_ptr(model_matrix));
		glDrawArrays(GL_TRIANGLES, 0, 36);

		glBindVertexArray(0);
		KeyboardCallback(window, delta_time_between_frames);
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	glDeleteVertexArrays(1, &vao);
	glDeleteVertexArrays(1, &light_vao);
	glDeleteBuffers(1, &vbo);
	glDeleteProgram(shader_program);
	glfwTerminate();
	return 0;
}