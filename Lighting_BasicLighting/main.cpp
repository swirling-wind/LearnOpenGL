#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#define STB_IMAGE_IMPLEMENTATION
#pragma warning(push, 0)
#include <stb_image.h>
#pragma warning(pop)
#include <iostream>
#include <array>
#include "init_camera.h"
#include "init_shaders.h"

unsigned int BuildObjShader()
{
	unsigned int vertex_shader = glCreateShader(GL_VERTEX_SHADER), fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	const char* vertex_shader_script = R"(#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

out vec3 FragPos;
out vec3 Normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal;  
    
    gl_Position = projection * view * vec4(FragPos, 1.0);
}
	)";
	glShaderSource(vertex_shader, 1, &vertex_shader_script, NULL);
	const char* fragment_shader_script = R"(#version 330 core

out vec4 FragColor;

in vec3 Normal;  
in vec3 FragPos;  
  
uniform vec3 lightPos; 
uniform vec3 viewPos; 
uniform vec3 lightColor;
uniform vec3 objectColor;

void main()
{
    // ambient
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;
  	
    // diffuse 
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;
    
    // specular
    float specularStrength = 0.9;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 256);
    vec3 specular = specularStrength * spec * lightColor;  
        
    vec3 result = (ambient + diffuse + specular) * objectColor;
    FragColor = vec4(result, 1.0);
} 
)";
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

	unsigned int obj_shader_program = BuildObjShader();
	unsigned int light_shader_program = BuildLightShader();
	auto [obj_vao, light_vao, vbo] = GetObjVaoLightVaoAndVbo();
	float delta_time_between_frames = 0.0f, last_frame_time = 0.0f;

	glm::vec3 obj_color(1.0f, 0.5f, 0.31f);
	glm::vec3 light_color(1.0f, 1.0f, 1.0f);
	glm::vec3 light_pos(1.2f, 1.0f, 2.0f);

	while (!glfwWindowShouldClose(window))
	{
		glClearColor(0.2f, 0.3f, 0.3f, 0.2f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		float current_frame_time = static_cast<float>(glfwGetTime());
		delta_time_between_frames = current_frame_time - last_frame_time;
		last_frame_time = current_frame_time;

		glm::mat4 model_matrix = glm::mat4(1.0f);
		glm::mat4 view_matrix = camera.GetViewMatrix();
		glm::mat4 projection_matrix = glm::perspective(glm::radians(camera.zoom), 800.0f / 600.0f, 0.1f, 100.0f);
		glm::vec3 camera_pos = camera.GetPosition();

		// Obj Shader
		model_matrix = glm::translate(model_matrix, glm::vec3(-0.8f, -0.8f, -0.5f));
		glUseProgram(obj_shader_program);
		glBindVertexArray(obj_vao);
		glUniform3fv(glGetUniformLocation(obj_shader_program, "objectColor"), 1, &obj_color[0]);
		glUniform3fv(glGetUniformLocation(obj_shader_program, "lightColor"), 1, &light_color[0]);
		glUniform3fv(glGetUniformLocation(obj_shader_program, "lightPos"), 1, &light_pos[0]);
		glUniform3fv(glGetUniformLocation(obj_shader_program, "viewPos"), 1, &camera_pos[0]);
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