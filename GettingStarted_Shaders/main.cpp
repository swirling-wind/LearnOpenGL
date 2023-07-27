#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <tuple>
#include <array>

void process_input(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, true);
	}
}

void framebuffer_size_callback(GLFWwindow*, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}

void check_load_glad(GLFWwindow* window)
{
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		std::exit(EXIT_FAILURE);
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	// glad: load all OpenGL function pointers
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		std::exit(EXIT_FAILURE);
	}
}

unsigned int check_then_link_shaders(unsigned int vertexShader, unsigned int fragmentShader)
{
	int success;
	char infoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
	}

	success = 1;
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		memset(infoLog, 0, sizeof(infoLog));
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
	}

	unsigned int shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);

	success = 1;
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success)
	{
		memset(infoLog, 0, sizeof(infoLog));
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
	}
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
	return shaderProgram;
}

unsigned int build_shader_program()
{
	unsigned int vertexShader, fragmentShader;
	// Vertex Shader
	const char* vertexShaderSource = R"(
		#version 330 core
		uniform vec3 xyOffset;
		uniform vec3 colors;
		layout (location = 0) in vec3 aPos;
		layout (location = 4) in vec3 aColor;
		out vec3 ourColor;

		void main() {
			gl_Position = vec4(aPos + xyOffset, 1.0);
			
			if (aColor.x > 0.99f) {
				ourColor = colors;
			} else if (aColor.y > 0.99f) {
				ourColor = vec3(colors.z, colors.xy);
			} else {
				ourColor = vec3(colors.yz, colors.x);
			}
			
		}
	)";  // "gl_Position = vec4(-aPos, 1.0);" to invert all vertex
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);

	// Fragment shader
	const char* fragmentShaderSource = R"(
		#version 330 core
		out vec4 FragColor;
		in vec3 ourColor;
		
		void main() {
			FragColor = vec4(ourColor, 1.0);
		}
	)";
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);

	return check_then_link_shaders(vertexShader, fragmentShader);
}

std::tuple<unsigned int, unsigned int, unsigned int> get_VAO_VBO_EBO()
{
	float vertices[] = {
		// Position				// Color
		-0.5f, -0.5f, 0.0f,		1.0f, 0.0f, 0.0f,
		0.5f, -0.5f, 0.0f,		0.0f, 1.0f, 0.0f,
		-0.45f, 0.5f, 0.0f,		0.0f, 0.0f, 1.0f,
		0.45f, 0.5f, 0.0f,		1.0f, 0.0f, 0.0f
	};

	unsigned int indices[] = {
		3, 2, 0,
		2, 3, 1
	};

	unsigned int VBO, VAO, EBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(4);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	return { VAO, VBO, EBO };
}

void render(unsigned int shaderProgram, unsigned int VAO)
{
	glClearColor(0.2f, 0.3f, 0.3f, 0.2f);
	glClear(GL_COLOR_BUFFER_BIT);
	glUseProgram(shaderProgram);

	float timeValue = static_cast<float>(glfwGetTime());
	int xyOffsetID = glGetUniformLocation(shaderProgram, "xyOffset");
	glUniform3f(xyOffsetID, cos(timeValue) / 2.0f, sin(timeValue) / 2.0f, 0.0f);

	int colorOffsetID = glGetUniformLocation(shaderProgram, "colors");
	float red, green, blue;
	float acceleration = timeValue * 4;
	int pos = static_cast<int>(acceleration) % 3;
	float intensity = acceleration - static_cast<int>(acceleration);
	if (pos == 0) {
		red = 0.0f;
		green = 1.0f - intensity;
		blue = intensity;
	}
	else if (pos == 1) {
		red = intensity;
		green = 0.0f;
		blue = 1.0f - intensity;
	}
	else {
		red = 1.0f - intensity;
		green = intensity;
		blue = 0.0f;
	}
	glUniform3f(colorOffsetID, red, green, blue);

	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

int main()
{

	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	//glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GL_TRUE);
	GLFWwindow* window = glfwCreateWindow(800, 600, "LearnOpenGL", NULL, NULL);
	check_load_glad(window);
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	//int nrAttributes;  glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &nrAttributes);
	//std::cout << "Maximum nr of vertex attributes supported: " << nrAttributes << std::endl;

	unsigned int shaderProgram = build_shader_program();
	auto [VAO, VBO, EBO] = get_VAO_VBO_EBO();

	while (!glfwWindowShouldClose(window))
	{
		process_input(window);

		render(shaderProgram, VAO);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
	glDeleteProgram(shaderProgram);

	glfwTerminate();
}