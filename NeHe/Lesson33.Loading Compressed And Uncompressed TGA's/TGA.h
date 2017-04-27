#ifndef TGA_H
#define TGA_H

#include <Windows.h>
#include <stdio.h>
#include <GL\glut.h>
#include "Texture.h"
#pragma comment(lib, "legacy_stdio_definitions.lib")

typedef struct {
    GLubyte Header[12];
} TGAHeader;

typedef struct {
    GLubyte header[6];
    GLuint bytesPerPixel;
    GLuint imageSize;
    GLuint type;                       // GL_RGB or GL_RGBA
    GLuint Height;
    GLuint Width;
    GLuint Bpp;                        // The number of bits per pixel 
} TGA;

TGAHeader tgaheader;
TGA tga;

// Uncompressed TGA header
GLubyte uTGAcompare[12] = { 0,0,2,0,0,0,0,0,0,0,0,0 };
// Compressed TGA header
GLubyte cTGAcompare[12] = { 0,0,10,0,0,0,0,0,0,0,0,0 };

// Load an uncompressed file
bool LoadUncompressedTGA(Texture*, char*, FILE*);
// Load an compressed file
bool LoadCompressedTGA(Texture*, char*, FILE*);

#endif