#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "mesh.h"
#include <iostream>
#include <string>
#include <vector>
using std::string, std::vector, glm::vec3, glm::vec2;


int main()
{
	Vertex test_vertex = Vertex{ vec3(1,2,3), vec3(2,3,4), vec2(3,4)};
	Texture test_texture = Texture{0u, "Hello"};
	Mesh mesh = Mesh{ vector{test_vertex}, vector{2u,3u}, vector{test_texture} };

	std::cout << "PREPARE" << std::endl;
}