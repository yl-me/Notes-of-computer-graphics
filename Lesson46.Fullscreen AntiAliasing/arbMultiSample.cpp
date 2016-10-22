#include <Windows.h>
#include <GL\glew.h>
#include <GL\glut.h>
#include "arbMultiSample.h"

#define WGL_SAMPLE_BUFFERS_ARB 0X2041
#define WGL_SAMPLES_ARB 0x2042

bool arbMultisampleSupported = false;
int arbMultisampleFormat = 0;

bool WGLisExtensionSupported(const char* extension)
{
	const size_t extlen = strlen(extension);
	const char* supported = NULL;

	// Try to use wglGetExtensionStringARB on cureent DC
	PROC wglGetExtString = wglGetProcAddress("wglGetExtensionsStringARB");

	if (wglGetExtString) {
		supported = ((char*(__stdcall*)(HDC))wglGetExtString)(wglGetCurrentDC());
	}

	// If that failed, try standard OpenGL extension string
	if (supported == NULL) {
		supported = (char*)glGetString(GL_EXTENSIONS);
	}

	// If that failed too, must be no extension supported
	if (supported == NULL) {
		return false;
	}
	// Begin examination at start of string, increment by 1 on false match
	for (const char* p = supported; ; ++p) {
		p = strstr(p, extension);
		if (p == NULL) {
			return false;
		}

		if ((p == supported || p[-1] == ' ') || (p[extlen] == '\0' || p[extlen] == ' ')) {
			return true;
		}
	}
}

bool InitMultisample(HINSTANCE hInstance, HWND hWnd, PIXELFORMATDESCRIPTOR pdf)
{
	// The string exists in WGL
	if (!WGLisExtensionSupported("WGL_ARB_multisample")) {
		arbMultisampleSupported = false;
		return false;
	}
	// Get pixel format
	PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB =
		(PFNWGLCHOOSEPIXELFORMATARBPROC)wglGetProcAddress("wglChoosePixelFormatARB");

	if (!wglChoosePixelFormatARB) {
		arbMultisampleSupported = false;
		return false;
	}
	// Get current device context
	HDC hDC = GetDC(hWnd);
	int pixelFormat;
	BOOL valid;
	UINT numFormats;
	float fAttributes[] = { 0, 0 };

	// These attributes are the bits we want to test for in our sample
	int iAttributes[] = { WGL_DRAW_TO_WINDOW_ARB, GL_TRUE, WGL_SUPPORT_OPENGL_ARB,GL_TRUE,
		WGL_ACCELERATION_ARB,WGL_FULL_ACCELERATION_ARB,
		WGL_COLOR_BITS_ARB,24,
		WGL_ALPHA_BITS_ARB,8,
		WGL_DEPTH_BITS_ARB,16,
		WGL_STENCIL_BITS_ARB,0,
		WGL_DOUBLE_BUFFER_ARB,GL_TRUE,
		WGL_SAMPLE_BUFFERS_ARB,GL_TRUE,
		WGL_SAMPLES_ARB, 4 ,
		0,0 };
	
	// Get a pixel format for 4 samples
	valid = wglChoosePixelFormatARB(hDC, iAttributes, fAttributes, 1, &pixelFormat, &numFormats);

	if (valid && numFormats >= 1) {
		arbMultisampleSupported = true;
		arbMultisampleFormat = pixelFormat;
		return arbMultisampleSupported;
	}

	iAttributes[19] = 2;
	valid = wglChoosePixelFormatARB(hDC, iAttributes, fAttributes, 1, &pixelFormat, &numFormats);
	if (valid && numFormats >= 1) {
		arbMultisampleSupported = true;
		arbMultisampleFormat = pixelFormat;
		return arbMultisampleSupported;
	}

	return arbMultisampleSupported;
}

