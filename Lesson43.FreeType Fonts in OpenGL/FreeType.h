#ifndef FREE_TYPE_H
#define FREE_TYPE_H

#include <ft2build.h>
#include <freetype\freetype.h>
#include <freetype\ftglyph.h>
#include <freetype\ftoutln.h>
#include <freetype\fttrigon.h>

#include <windows.h>
#include <GL\glew.h>
#include <GL\glut.h>

#include <vector>
#include <string>

#include <stdexcept>
/*
 *	MSVC will spit out all sorts of useless waring if you create vectors of strings, 
 *	this pragma gets rid of them.
 */
#pragma warning(disable: 4786)

namespace freetype{
using std::vector;
using std::string;

class font_data{
public:
	float h;                     // The height of the font
	GLuint* textures;
	GLuint list_base;            // The first display list Id

public:
	void init(const char* fname, unsigned int h);      // Create a font
	void clean();                // Free all the resources associated with the font
};

void print(const font_data& ft_font, float x, float y, const char* fmt, ...);
}

#endif