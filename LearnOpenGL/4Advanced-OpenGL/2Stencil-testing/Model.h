#ifndef MODEL_H
#define MODEL_H

#include <SOIL\SOIL.h>
#include "Mesh.h"

GLint TextureFromFile(const char* path, std::string directory);

class Model
{
public:
	Model(GLchar* path);
	void Draw(Shader shader);

private:
	std::vector<Mesh> meshes;
	std::string directory;
	std::vector<Texture> textures_loaded;

	void loadModel(std::string path);
	void processNode(aiNode* node, const aiScene* scene);
	Mesh processMesh(aiMesh* mesh, const aiScene* scene);

	std::vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName);
};

#endif // !MODEL_H