#ifndef MESH_H
#define MESH_H

#include <gl\glew.h>
#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>

#include <assimp/Importer.hpp>         // assimp
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <string>
#include <vector>
#include <sstream>
#include <iostream>

#include "Shader.h"

struct Vertex {
	glm::vec3 Position;
	glm::vec3 Normal;
	glm::vec2 TexCoords;
};

struct Texture {
	GLuint id;
	std::string type;
	aiString path;            // 储存纹理路径，每次加载纹理前检查(优化)
};

class Mesh {
public :
	GLuint VAO;
	std::vector<Vertex> vertices;
	std::vector<GLuint> indices;
	std::vector<Texture> textures;
	Mesh() = default;
	Mesh(std::vector<Vertex> vertices, std::vector<GLuint> indices, std::vector<Texture> textures);
	~Mesh() = default;
	void Draw(Shader shader);

private:
	GLuint VBO, EBO;
	void setupMesh();
};

#endif // !MESH_H