#ifndef  SHADER_H
#define SHADER_H

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>

#include <GL\glew.h>

class Shader {
public:
	GLuint Program;   // ID
	Shader(const GLchar * vertexPath, const GLchar * fragmnetPath);
	void Use();
};

#endif //  SHADER_H