#include <Windows.h>
#include <GL/glew.h>
#include <GL/glut.h>
#include <GL/GLUAX.H>
#include <math.h>
#include <stdio.h>
#include <olectl.h>
#include "Previous.h"

#pragma comment(lib, "legacy_stdio_definitions.lib")

#ifndef CDS_FULLSCREEN
#define CDS_FULLSCREEN 4
#endif

GL_Window* g_window;
Keys* g_keys;

GLfloat fogColor[4] = { 0.6f, 0.3f, 0.0f, 1.0f };     // Fog colour
GLfloat camz;                                // Camera Z depth

#define GL_FOG_COORDINATE_SOURCE_EXT 0x8450
#define GL_FOG_COORDINATE_EXT 0x8451

typedef void(APIENTRY* PFNGLFOGCOORDFEXTPROC) (GLfloat coord);   // Declare function prototype

PFNGLFOGCOORDFEXTPROC glFogCoordfEXT = NULL;      // glFogCoordfEXT function
GLuint texture[1];

int BuildTexture(char* szPathName, GLuint& texid)
{
	HDC hdcTemp;
	HBITMAP hbmpTemp;                      // Holds the bitmap temporarily
	IPicture* pPicture;                    // IPicture interface
	OLECHAR wszPath[MAX_PATH + 1];         // Full path to picture (wchar)
	char szPath[MAX_PATH + 1];             // Full path to picture
	long lWidth;
	long lHeight;
	long lWidthPixels;
	long lHeightPixels;
	GLint glMaxTexDim;                     // Maximum texture size

	if (strstr(szPathName, "https://")) {
		strcpy(szPath, szPathName);
	}
	else {
		GetCurrentDirectory(MAX_PATH, szPath);
		strcat(szPath, "\\");
		strcat(szPath, szPathName);
	}
	// Convert from ASCII to Unicode
	MultiByteToWideChar(
		CP_ACP,                // ANSI Codepage
		0,                     // Specifies the handling of unmapped characters
		szPath,                // The wide-character string to be converted
		-1,                    // The width of the wide-character string
		wszPath,               // The translated string
		MAX_PATH               // The maximum size of our file path 256 characters 
		);

	// Load the image
	HRESULT hr = OleLoadPicturePath(wszPath, 0, 0, 0, IID_IPicture, (void**)&pPicture);

	if (FAILED(hr)) {
		return FALSE;
	}

	hdcTemp = CreateCompatibleDC(GetDC(0));    // Create the windows compatible device context
	if (!hdcTemp) {
		pPicture->Release();
		return FALSE;
	}
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &glMaxTexDim);    // Get maximum texture size supported

	pPicture->get_Width(&lWidth);          // Get IPicture width
	lWidthPixels = MulDiv(lWidth, GetDeviceCaps(hdcTemp, LOGPIXELSX), 2540);  // Convert to pixels
	pPicture->get_Height(&lHeight);          // Get IPicture height
	lHeightPixels = MulDiv(lHeight, GetDeviceCaps(hdcTemp, LOGPIXELSX), 2540);  // Convert to pixels

	if (lWidthPixels <= glMaxTexDim) {
		lWidthPixels = 1 << (int)floor((log((double)lWidthPixels) / log(2.0f)) + 0.5f);
	}
	else {
		lWidthPixels = glMaxTexDim;
	}

	if (lHeightPixels <= glMaxTexDim) {
		lHeightPixels = 1 << (int)floor((log((double)lHeightPixels) / log(2.0f)) + 0.5f);
	}
	else {
		lHeightPixels = glMaxTexDim;
	}

	BITMAPINFO bi = {0};            // The type of bit map
	DWORD* pBits = 0;

	bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);   // Structure size
	bi.bmiHeader.biBitCount = 32;
	bi.bmiHeader.biWidth = lWidthPixels;              // Power of two width
	bi.bmiHeader.biHeight = lHeightPixels;
	bi.bmiHeader.biCompression = BI_RGB;              // RGB encoding
	bi.bmiHeader.biPlanes = 1;                        // 1 bitplane

	hbmpTemp = CreateDIBSection(hdcTemp, &bi, DIB_RGB_COLORS, (void**)&pBits, 0, 0);
	if (!hbmpTemp) {
		DeleteDC(hdcTemp);
		pPicture->Release();
		return FALSE;
	}
	SelectObject(hdcTemp, hbmpTemp);
	// Render the IPicture on to the bitmap
	pPicture->Render(hdcTemp, 0, 0, lWidthPixels, lHeightPixels, 0, lHeight, lWidth, -lHeight, 0);

	// BGR
	for (long i = 0; i < lWidthPixels * lHeightPixels; ++i) {
		BYTE* pPixel = (BYTE*)(&pBits[i]);              // Grab the current pixel
		BYTE temp = pPixel[0];
		pPixel[0] = pPixel[2];
		pPixel[2] = temp;
		pPixel[3] = 255;
	}

	glGenTextures(1, &texid);
	glBindTexture(GL_TEXTURE_2D, texid);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, lWidthPixels, lHeightPixels, 0, GL_RGBA, GL_UNSIGNED_BYTE, pBits);

	DeleteObject(hbmpTemp);          // Delete the object
	DeleteDC(hdcTemp);

	pPicture->Release();             // Decrements IPicture reference count

	return TRUE;
}

int Extension_Init()
{
	char Extension_Name[] = "EXT_fog_coord";

	// Allocate memory for our extension string
	char* glextstring = (char*)malloc(strlen((char*)glGetString(GL_EXTENSIONS)) + 1);
	strcpy(glextstring, (char*)glGetString(GL_EXTENSIONS));        // Store

	if (!strstr(glextstring, Extension_Name)) {
		return FALSE;
	}
	free(glextstring);

	// Setup and enable glFogCoordfEXT
	glFogCoordfEXT = (PFNGLFOGCOORDFEXTPROC)wglGetProcAddress("glFogCoordfEXT");
	return TRUE;
}

BOOL Initialize(GL_Window* window, Keys* keys)
{
	g_window = window;
	g_keys = keys;

	if (!Extension_Init()) {          // Fog extension
		return FALSE;
	}

	if (!BuildTexture("data/wall.bmp", texture[0])) {
		return FALSE;
	}

	glEnable(GL_TEXTURE_2D);
	glClearColor(0.0f, 0.0f, 0.0f, 0.5f);
	glClearDepth(1.0f);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_DEPTH_TEST);
	glShadeModel(GL_SMOOTH);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	glEnable(GL_FOG);
	glFogi(GL_FOG_MODE, GL_LINEAR);            // Fog fade is linear
	glFogfv(GL_FOG_COLOR, fogColor);           // Set the color
	glFogf(GL_FOG_START, 0.0f);                // Set the fog start (least dense)
	glFogf(GL_FOG_END, 1.0f);                  // Set the fog end (most dense)
	glHint(GL_FOG_HINT, GL_NICEST);            // Per-pixel fog calculation
	glFogi(GL_FOG_COORDINATE_SOURCE_EXT, GL_FOG_COORDINATE_EXT);   // Set fog based on vertice coordinate
	camz = -19.0f;                             // Set camera Z position to -19
	return TRUE;
}

void Deinitialize(void) {}

void Update(DWORD milliseconds)
{
	if (g_keys->keyDown[VK_ESCAPE] == TRUE)
		TerminateApplication(g_window);
	if (g_keys->keyDown[VK_F1] == TRUE)
		ToggleFullscreen(g_window);

	if (g_keys->keyDown[VK_UP] == TRUE && camz < 14.0f)
		camz += (float)(milliseconds) / 100.0f;
	if (g_keys->keyDown[VK_DOWN] == TRUE && camz > -19.0f)
		camz -= (float)(milliseconds) / 100.0f;
}

void Draw(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	glTranslatef(0.0f, 0.0f, camz);

	glBegin(GL_QUADS);                           // Back wall
		glFogCoordfEXT(1.0f); glTexCoord2f(0.0f, 0.0f);
		glVertex3f(-2.5f, -2.5f, -15.0f);
		glFogCoordfEXT(1.0f); glTexCoord2f(1.0f, 0.0f);
		glVertex3f(2.5f, -2.5f, -15.0f);
		glFogCoordfEXT(1.0f); glTexCoord2f(1.0f, 1.0f);
		glVertex3f(2.5f, 2.5f, -15.0f);
		glFogCoordfEXT(1.0f); glTexCoord2f(0.0f, 1.0f);
		glVertex3f(-2.5f, 2.5f, -15.0f);
	glEnd();
	
	glBegin(GL_QUADS);                            // Floor
		glFogCoordfEXT(1.0f); glTexCoord2f(0.0f, 0.0f);
		glVertex3f(-2.5f, -2.5f, -15.0f);
		glFogCoordfEXT(1.0f); glTexCoord2f(1.0f, 0.0f);
		glVertex3f(2.5f, -2.5f, -15.0f);
		glFogCoordfEXT(0.0f); glTexCoord2f(1.0f, 1.0f);
		glVertex3f(2.5f, -2.5f, 15.0f);
		glFogCoordfEXT(0.0f); glTexCoord2f(0.0f, 1.0f);
		glVertex3f(-2.5f, -2.5f, 15.0f);
	glEnd();
	
	glBegin(GL_QUADS);                            // Roof
		glFogCoordfEXT(1.0f); glTexCoord2f(0.0f, 0.0f);
		glVertex3f(-2.5f, 2.5f, -15.0f);
		glFogCoordfEXT(1.0f); glTexCoord2f(1.0f, 0.0f);
		glVertex3f(2.5f, 2.5f, -15.0f);
		glFogCoordfEXT(0.0f); glTexCoord2f(1.0f, 1.0f);
		glVertex3f(2.5f, 2.5f, 15.0f);
		glFogCoordfEXT(0.0f); glTexCoord2f(0.0f, 1.0f);
		glVertex3f(-2.5f, 2.5f, 15.0f);
	glEnd();

	glBegin(GL_QUADS);                            // Right Wall
		glFogCoordfEXT(0.0f); glTexCoord2f(0.0f, 0.0f);
		glVertex3f(2.5f, -2.5f, 15.0f);
		glFogCoordfEXT(0.0f); glTexCoord2f(0.0f, 1.0f);
		glVertex3f(2.5f, 2.5f, 15.0f);
		glFogCoordfEXT(1.0f); glTexCoord2f(1.0f, 1.0f);
		glVertex3f(2.5f, 2.5f, -15.0f);
		glFogCoordfEXT(1.0f); glTexCoord2f(1.0f, 0.0f);
		glVertex3f(2.5f, -2.5f, -15.0f);
	glEnd();

	glBegin(GL_QUADS);                            // Left Wall
		glFogCoordfEXT(0.0f); glTexCoord2f(0.0f, 0.0f);
		glVertex3f(-2.5f, -2.5f, 15.0f);
		glFogCoordfEXT(0.0f); glTexCoord2f(0.0f, 1.0f);
		glVertex3f(-2.5f, 2.5f, 15.0f);
		glFogCoordfEXT(1.0f); glTexCoord2f(1.0f, 1.0f);
		glVertex3f(-2.5f, 2.5f, -15.0f);
		glFogCoordfEXT(1.0f); glTexCoord2f(1.0f, 0.0f);
		glVertex3f(-2.5f, -2.5f, -15.0f);
	glEnd();

	glFlush();
}