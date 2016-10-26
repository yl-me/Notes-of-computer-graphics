#ifndef GL_FRAMEWORK_INCLUDED
#define GL_FRAMEWORK_INCLUDED

#ifndef CDS_FULLSCREEN
#define CDS_FULLSCREEN 4
#endif

#include <windows.h>

typedef struct {                                   // Structure for keyboard stuff
	BOOL keyDown[256];
} Keys;

typedef struct {                                   // Contains information vital to applications 
	HMODULE hInstance;                             // Application Instance
	const char* className;
} Application;

typedef struct {                                   // Window creation info
	Application* application;
	char* title;
	int width;
	int height;
	int bitsPerPixel;
	BOOL isFullScreen;
} GL_WindowInit;

typedef struct {                                   // Contains information vital to a window
	Keys* keys;
	HWND hWnd;                                     // Windows handle
	HDC hDC;                                       // Device context
	HGLRC hRC;                                     // Rendering context
	GL_WindowInit init;
	BOOL isVisible;                                // Window visiable?
	BOOL hrTimer;                                  // High resolution timer available
	DWORD lastTickCount;                           // Tick counter
	float timerResolution;
} GL_Window;

void TerminateApplication(GL_Window* window);      // Terminate the application

void ToggleFullscreen(GL_Window* window);          // Toggle fullscreen / Windowed mode

BOOL Initialize(GL_Window* window, Keys* keys);

void Deinitialize(void);

void Update(DWORD milliseconds);

void Draw(void);

#endif