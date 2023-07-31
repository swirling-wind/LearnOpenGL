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
	const char* vertex_shader_script = R"(#version 330 core

		layout (location = 0) in vec3 orig_pos;
		layout (location = 1) in vec3 orig_normal;

		out vec3 inter_normal;
		out vec3 frag_pos;

		
		uniform mat4 model;
		uniform mat4 view;
		uniform mat4 projection;

		void main() {
			frag_pos = vec3(model * vec4(orig_pos, 1.0));
			inter_normal = mat3(transpose(inverse(model))) * orig_normal;
			gl_Position = projection * view * vec4(frag_pos, 1.0);

			
		})";
	glShaderSource(vertex_shader, 1, &vertex_shader_script, NULL);
	const char* fragment_shader_script = R"(#version 330 core

		in vec3 inter_normal;
		in vec3 frag_pos;

		out vec4 frag_color;

		uniform vec3 view_pos;
		uniform vec3 light_pos;
		uniform vec3 object_color;
		uniform vec3 light_color;

		void main() {
			// ambient
			float ambient_strength = 0.1;
			vec3 ambient = ambient_strength * light_color;

			// diffuse
			vec3 norm = normalize(inter_normal);
			vec3 light_dir = normalize(light_pos - frag_pos);
			float diff_magnitude = max(dot(inter_normal, light_dir), 0.0);
			vec3 diffuse = diff_magnitude * light_color;

			// specular
			float specular_strength = 0.9f;
			vec3 view_dir = normalize(view_pos - frag_pos);
			vec3 reflect_dir = reflect(-light_dir, norm);
			float spec = pow(max(dot(view_dir, reflect_dir), 0.0), 32);
			vec3 specular = specular_strength * spec * light_color;

			vec3 result = (ambient + diffuse + specular) * object_color;
			frag_color = vec4(result, 1.0);
		})";
	glShaderSource(fragment_shader, 1, &fragment_shader_script, NULL);
	return LinkToShaderProgram(vertex_shader, fragment_shader);
}

unsigned int BuildLightShader()
{
	unsigned int light_vertex_shader = glCreateShader(GL_VERTEX_SHADER), light_frag_shader = glCreateShader(GL_FRAGMENT_SHADER);
	const char* light_vertext_shader_script = R"(
		#version 330 core
		layout (location = 0) in vec3 aPos;

		uniform mat4 model;
		uniform mat4 view;
		uniform mat4 projection;

		void main() {
			gl_Position = projection * view * model * vec4(aPos, 1.0);
		}

	)";
	glShaderSource(light_vertex_shader, 1, &light_vertext_shader_script, NULL);
	const char* light_frag_shader_script = R"(
		#version 330 core
		out vec4 frag_color;
		void main() {
			frag_color = vec4(1.0); 
		}
	)";
	glShaderSource(light_frag_shader, 1, &light_frag_shader_script, NULL);
	return LinkToShaderProgram(light_vertex_shader, light_frag_shader);
}

int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	GLFWwindow* window = glfwCreateWindow(800, 600, "LearnOpenGL", NULL, NULL);
	SetCallbackAndLoadGlad(window);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

	const unsigned int obj_shader_program = BuildObjShader();
	const unsigned int light_shader_program = BuildLightShader();
	const auto [obj_vao, light_vao, vbo] = GetObjVaoLightVaoAndVbo();
	float delta_time_between_frames = 0.0f, last_frame_time = 0.0f;

	const glm::vec3 obj_color(1.0f, 0.5f, 0.31f);
	const glm::vec3 light_color(1.0f, 1.0f, 1.0f);
	const glm::vec3 light_pos(-0.5f, 0.1f, -0.5f);

	while (!glfwWindowShouldClose(window))
	{
		glClearColor(0.2f, 0.3f, 0.3f, 0.2f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		float current_frame_time = static_cast<float>(glfwGetTime());
		delta_time_between_frames = current_frame_time - last_frame_time;
		last_frame_time = current_frame_time;

		glm::mat4 model_matrix = glm::mat4(1.0f);
		const glm::mat4 view_matrix = camera.GetViewMatrix();
		const glm::mat4 projection_matrix = glm::perspective(glm::radians(camera.zoom), 800.0f / 600.0f, 0.1f, 100.0f);

		// Obj Shader
		model_matrix = glm::translate(model_matrix, glm::vec3(-0.8f, -0.8f, -0.5f));
		glUseProgram(obj_shader_program);
		glBindVertexArray(obj_vao);
		glUniform3fv(glGetUniformLocation(obj_shader_program, "object_color"), 1, &obj_color[0]);
		glUniform3fv(glGetUniformLocation(obj_shader_program, "light_color"), 1, &light_color[0]);
		glUniform3fv(glGetUniformLocation(obj_shader_program, "light_pos"), 1, &light_pos[0]);
		glUniformMatrix4fv(glGetUniformLocation(obj_shader_program, "model"), 1, GL_FALSE, glm::value_ptr(model_matrix));
		glUniformMatrix4fv(glGetUniformLocation(obj_shader_program, "view"), 1, GL_FALSE, glm::value_ptr(view_matrix));
		glUniformMatrix4fv(glGetUniformLocation(obj_shader_program, "projection"), 1, GL_FALSE, glm::value_ptr(projection_matrix));
		glDrawArrays(GL_TRIANGLES, 0, 36);

		// Light Shader
		model_matrix = glm::translate(glm::mat4(1.0f), light_pos);
		model_matrix = glm::scale(model_matrix, glm::vec3(0.2f));
		glUseProgram(light_shader_program);
		glBindVertexArray(light_vao);
		glUniformMatrix4fv(glGetUniformLocation(light_shader_program, "model"), 1, GL_FALSE, glm::value_ptr(model_matrix));
		glUniformMatrix4fv(glGetUniformLocation(light_shader_program, "view"), 1, GL_FALSE, glm::value_ptr(view_matrix));
		glUniformMatrix4fv(glGetUniformLocation(light_shader_program, "projection"), 1, GL_FALSE, glm::value_ptr(projection_matrix));
		glDrawArrays(GL_TRIANGLES, 0, 36);

		glBindVertexArray(0);
		KeyboardCallback(window, delta_time_between_frames);
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	glDeleteVertexArrays(1, &obj_vao);
	glDeleteVertexArrays(1, &light_vao);
	glDeleteBuffers(1, &vbo);
	glDeleteProgram(obj_shader_program);
	glfwTerminate();
	return 0;
}