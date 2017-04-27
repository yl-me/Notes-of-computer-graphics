#ifndef TEXTURE_H
#define TEXTURE_H

#include <windows.h>
#include <stdio.h>
#include <GL\glut.h>

#pragma comment(lib, "legacy_stdio_definitions.lib")

typedef struct {
    GLubyte* imageData;
    GLuint bpp;                        // The number of bits per pixel 
    GLuint width;
    GLuint height;
    GLuint texID;                     // Texture ID
    GLuint type;                       // GL_RGB or GL_RGBA
} Texture;

#endif