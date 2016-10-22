#ifndef __ARB_MULTISAMPLE_H__
#define __ARB_MULTISAMPLE_H__

#include <GL\wglext.h>     // WGL extensions
#include <GL\glext.h>      // GL extwnsions

// Globals
extern bool arbMultisampleSupported;
extern int arbMultisampleFormat;

// If you don't want multisampling, set this to 0
#define CHECK_FOR_MULTISAMPLE 1

// To heck for our sampling
bool InitMultisample(HINSTANCE hInstance, HWND hWnd, PIXELFORMATDESCRIPTOR pdf);

#endif // !__ARB_MULTISAMPLE_H__