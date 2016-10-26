#include <Windows.h>
#include <GL\glew.h>
#include <GL\glut.h>
#include "Previous.h"

#define WM_TOGGLEFULLSCREEN (WM_USER+1)                   // Application define message for toggling 
// between fulscreen / windowed mode
static BOOL g_isProgramLooping;                           // Window creation loop, for fullscreen / windowed mode
static BOOL g_createFullScreen;                           // If true, then create window

void TerminateApplication(GL_Window* window)              // Terminate the application
{
	PostMessage(window->hWnd, WM_QUIT, 0, 0);             // Send a WM_QUIT message
	g_isProgramLooping = FALSE;                           // Stop looping of the program
}

void ToggleFullscreen(GL_Window* window)                  // Toggle fullscreen /windowed mode
{
	PostMessage(window->hWnd, WM_TOGGLEFULLSCREEN, 0, 0); // Send a WM_TOGGLEFULLSCREEN message
}

void ReshapeGL(int width, int height)                     // Reshape the window  when it's moved or resized
{
	glViewport(0, 0, (GLsizei)(width), (GLsizei)(height)); // Reset the current viewport
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	// Calcutate the aspect ratio of the window
	gluPerspective(45.0f, (GLfloat)(width) / (GLfloat)(height), 1.0, 1000.0f);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

BOOL ChangeScreenResolution(int width, int height, int bitsPerPixel)     // Change the screen resolution
{
	DEVMODE dmScreenSettings;                              // Device mode
	ZeroMemory(&dmScreenSettings, sizeof(DEVMODE));        // Make sure memory is cleared
	dmScreenSettings.dmSize = sizeof(DEVMODE);             // Size of the devmode structure
	dmScreenSettings.dmPelsWidth = width;
	dmScreenSettings.dmPelsHeight = height;
	dmScreenSettings.dmBitsPerPel = bitsPerPixel;
	dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
	if (ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL) {
		return FALSE;                                      // Display change failed
	}
	return TRUE;
}

BOOL CreateWindowGL(GL_Window* window)
{
	__int64 timer;
	DWORD windowStyle = WS_OVERLAPPEDWINDOW;                // Define window style
	DWORD windowExtendedStyle = WS_EX_APPWINDOW;            // Define the window's extended style

	PIXELFORMATDESCRIPTOR pfd = {
		sizeof(PIXELFORMATDESCRIPTOR),                      // Size of this pixel format descriptor
		1,                                                  // Version Number
		PFD_DRAW_TO_WINDOW |                                // Format must support window
		PFD_SUPPORT_OPENGL |                                // Format must support openGL
		PFD_DOUBLEBUFFER,                                   // Must support double buffering
		PFD_TYPE_RGBA,                                      // Request an RGBA format
		window->init.bitsPerPixel,                          // Select color depth
		0, 0, 0, 0, 0, 0,                                   // Color bits ignored
		0,                                                  // No alpha buffer
		0,                                                  // Shift bit ignored
		0,                                                  // No accumulation buffer
		0, 0, 0, 0,                                         // Accumulation bits ignored
		16,                                                 // 16bits Z-buffer (depth buffer)
		0,                                                  // No stencil buffer
		0,                                                  // No auxiliary buffer
		PFD_MAIN_PLANE,                                     // Main drawing layer
		0,                                                  // Reserved
		0, 0, 0                                             // Layer masks ignored
	};
	RECT windowRect = { 0, 0, window->init.width, window->init.height };   // Window coordiantes

	GLuint PixelFormat;

	if (window->init.isFullScreen == TRUE) {
		if (ChangeScreenResolution(window->init.width, window->init.height, window->init.bitsPerPixel) == FALSE)
		{
			// Fullscreen mode failed, run in windowed mode instead
			MessageBox(HWND_DESKTOP, "Mode Switch Failed.\nRuning In Windowed Mode.",
				"Error", MB_OK | MB_ICONEXCLAMATION);
			window->init.isFullScreen = FALSE;
		}
		else {
			ShowCursor(FALSE);
			windowStyle = WS_POPUP;                         // Popup window
			windowExtendedStyle |= WS_EX_TOPMOST;
		}
	}
	else {
		// Adjust window, account for window borders
		AdjustWindowRectEx(&windowRect, windowStyle, 0, windowExtendedStyle);
	}
	// Create Opengl window
	window->hWnd = CreateWindowEx(windowExtendedStyle,      // Extended style
		window->init.application->className,                // Class name
		window->init.title,                                 // Window title
		windowStyle,                                        // Window style
		0, 0,                                               // Window X,Y position
		windowRect.right - windowRect.left,                 // Window width
		windowRect.bottom - windowRect.top,                 // Window height
		HWND_DESKTOP,                                       // Desktop is window's parent
		0,                                                  // No menu
		window->init.application->hInstance,                // Pass the window instance
		window);

	if (window->hWnd == 0) {                                // Was window creation a success?
		return FALSE;
	}
	window->hDC = GetDC(window->hWnd);
	if (window->hDC == 0) {
		DestroyWindow(window->hWnd);
		window->hWnd = 0;
		return FALSE;
	}
	PixelFormat = ChoosePixelFormat(window->hDC, &pfd);     // Find a compatible pixel format
	if (PixelFormat == 0) {
		ReleaseDC(window->hWnd, window->hDC);               // Release device context
		window->hDC = 0;
		DestroyWindow(window->hWnd);
		window->hWnd = 0;
		return FALSE;
	}
	if (SetPixelFormat(window->hDC, PixelFormat, &pfd) == FALSE) {   // Try to set the pixel format
		ReleaseDC(window->hWnd, window->hDC);
		window->hDC = 0;
		DestroyWindow(window->hWnd);
		window->hWnd = 0;
		return FALSE;
	}
	window->hRC = wglCreateContext(window->hDC);            // Try to get a rendering context
	if (window->hRC == 0) {
		ReleaseDC(window->hWnd, window->hDC);
		window->hDC = 0;
		DestroyWindow(window->hWnd);
		window->hWnd = 0;
		return FALSE;
	}
	// Make the rendering context our current rendering context
	if (wglMakeCurrent(window->hDC, window->hRC) == FALSE) {
		wglDeleteContext(window->hRC);                      //  Delete the rendering context
		window->hRC = 0;
		ReleaseDC(window->hWnd, window->hDC);
		window->hDC = 0;
		DestroyWindow(window->hWnd);
		window->hWnd = 0;
		return FALSE;
	}

	ShowWindow(window->hWnd, SW_NORMAL);                    // Make the window visiable
	window->isVisible = TRUE;
	ReshapeGL(window->init.width, window->init.height);     // Reshape our GL window
	ZeroMemory(window->keys, sizeof(Keys));                 // Clear all keys
/**************************************************************************************************************/
/**************************************************************************************************************/
	if (QueryPerformanceFrequency((LARGE_INTEGER *)&timer)) {
		// High resolution is available
		window->hrTimer = TRUE;
		// Grab the starting tick value
		window->lastTickCount = QueryPerformanceCounter((LARGE_INTEGER *)&timer);
		// Grab the counter frequency
		QueryPerformanceFrequency((LARGE_INTEGER *)&timer);
		// Set the timer resolution 1.0f / timer frequency
		window->timerResolution = (float)(((double)1.0f) / ((double)timer));
	}
	else {
		window->lastTickCount = GetTickCount();
		window->timerResolution = 1.0f / 1000.0f;
	}
/**************************************************************************************************************/
/**************************************************************************************************************/
	return TRUE;
}

BOOL DestroyWindowGL(GL_Window* window)
{
	if (window->hWnd != 0) {
		if (window->hDC != 0) {
			wglMakeCurrent(window->hDC, 0);                 // Setting current active rendering context to zero
			if (window->hRC != 0) {
				wglDeleteContext(window->hRC);
				window->hRC = 0;
			}
			ReleaseDC(window->hWnd, window->hDC);
			window->hDC = 0;
		}
		DestroyWindow(window->hWnd);
		window->hWnd = 0;
	}
	if (window->init.isFullScreen) {
		ChangeDisplaySettings(NULL, 0);                     // Switch back to desktop resolution
		ShowCursor(TRUE);
	}
	return TRUE;
}

// Process window message callback
LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	// Get the window context
	GL_Window* window = (GL_Window*)(GetWindowLong(hWnd, GWL_USERDATA));
	switch (uMsg) {                                         // Evaluate window message
	case WM_SYSCOMMAND:                                     // Intercept system commands
	{
		switch (wParam) {                                   // Check system calls
		case SC_SCREENSAVE:                                 // Screensaver trying to start?
		case SC_MONITORPOWER:                               // Mointer trying to enter powersave?
			return 0;                                           // Prevent form happening
		}
		break;
	}
	return 0;
	case WM_CREATE:
	{
		CREATESTRUCT* creation = (CREATESTRUCT*)(lParam);   // Store window structure pointer
		window = (GL_Window*)(creation->lpCreateParams);
		SetWindowLong(hWnd, GWL_USERDATA, (LONG)(window));
	}
	return 0;

	case WM_CLOSE:
		TerminateApplication(window);
		return 0;

	case WM_SIZE:
		switch (wParam) {
		case SIZE_MINIMIZED:                                 // Was window minimized?
			window->isVisible = FALSE;
			return 0;
		case SIZE_MAXIMIZED:
			window->isVisible = TRUE;
			ReshapeGL(LOWORD(lParam), HIWORD(lParam));
			return 0;
		case SIZE_RESTORED:
			window->isVisible = TRUE;
			ReshapeGL(LOWORD(lParam), HIWORD(lParam));
			return 0;
		}
		break;

	case WM_KEYDOWN:
		if ((wParam >= 0) && (wParam <= 255)) {
			window->keys->keyDown[wParam] = TRUE;            // Set the selected key(wParam) to true
			return 0;
		}
		break;

	case WM_KEYUP:
		if ((wParam >= 0) && (wParam <= 255)) {
			window->keys->keyDown[wParam] = FALSE;
			return 0;
		}
		break;

	case WM_TOGGLEFULLSCREEN:
		g_createFullScreen = (g_createFullScreen == TRUE) ? FALSE : TRUE;
		PostMessage(hWnd, WM_QUIT, 0, 0);
		break;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);        // Pass unhandle message to DefWindowProc
}

BOOL RegisterWindowClass(Application* application)
{
	WNDCLASSEX windowClass;
	ZeroMemory(&windowClass, sizeof(WNDCLASSEX));            // Make sure memory is cleared
	windowClass.cbSize = sizeof(WNDCLASSEX);                 // Size of the windowClass structure
	windowClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;  // Redraws the window for any movement / resizing
	windowClass.lpfnWndProc = (WNDPROC)(WindowProc);         // WindowProc handles message
	windowClass.hInstance = application->hInstance;          // Set the instance
	windowClass.hbrBackground = (HBRUSH)(COLOR_APPWORKSPACE);// Class background brush color
	windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);       // Load the arrow pointer
	windowClass.lpszClassName = application->className;      // Sets the application className
	if (RegisterClassEx(&windowClass) == 0) {
		MessageBox(HWND_DESKTOP, "RegisterClassEx Failed!", "Error", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;
	}
	return TRUE;
}

int WINAPI WinMain(HINSTANCE hIstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	Application application;
	GL_Window window;
	Keys keys;
	BOOL isMessagePumpActive;
	MSG msg;
	DWORD tickCount;

	application.className = "OpenGL";
	application.hInstance = hIstance;

	ZeroMemory(&window, sizeof(GL_Window));
	window.keys = &keys;                                     // Window key structure
	window.init.application = &application;                  // Window application
	window.init.title = "Resource File";                     // Window title
	window.init.width = 640;                                 // Window width
	window.init.height = 480;                                // Window height
	window.init.bitsPerPixel = 16;                           // Bits per pixel
	window.init.isFullScreen = TRUE;                         // Fullscreen? (set to TRUE)

	ZeroMemory(&keys, sizeof(Keys));
	if (MessageBox(HWND_DESKTOP, "Would You Like To Run In Fullscreen Mode?", "Start FullScreen?",
		MB_YESNO | MB_ICONQUESTION) == IDNO)
	{
		window.init.isFullScreen = FALSE;
	}
	if (RegisterWindowClass(&application) == FALSE)
	{
		MessageBox(HWND_DESKTOP, "Error Registering Window Class!", "Error", MB_OK | MB_ICONEXCLAMATION);
		return -1;
	}
	g_isProgramLooping = TRUE;
	g_createFullScreen = window.init.isFullScreen;
	while (g_isProgramLooping) {                             // Loop until WM_QUIT is received
		window.init.isFullScreen = g_createFullScreen;       // Set init param of window creation to fullscreen?
		if (CreateWindowGL(&window) == TRUE) {               // Was window creation successful?
															 // At this point we should have a window that is setup to render OpenGL
			if (Initialize(&window, &keys) == FALSE) {
				TerminateApplication(&window);               // Close window, this will handle the shutdown
			}
			else {
				isMessagePumpActive = TRUE;
				while (isMessagePumpActive == TRUE) {
					// Success creating window. Check for window messages
					if (PeekMessage(&msg, window.hWnd, 0, 0, PM_REMOVE) != 0) {
						if (msg.message != WM_QUIT) {
							DispatchMessage(&msg);
						}
						else {
							isMessagePumpActive = FALSE;     // Terminate the message pump
						}
					}
					else {
						if (window.isVisible == FALSE) {
							WaitMessage();                   // Application is minimized wait for a message
						}
						else {
							// Process application loop
							tickCount = GetTickCount();      // Get the tick count
							Update(tickCount - window.lastTickCount); // Update the counter
							window.lastTickCount = tickCount;// Set last count to current count
							Draw();                          // Draw screen
							SwapBuffers(window.hDC);
						}
					}
				}
			}
			// Application is finished
			Deinitialize();
			DestroyWindowGL(&window);
		}
		else {
			MessageBox(HWND_DESKTOP, "Error Creating OpenGL Window", "Error", MB_OK | MB_ICONEXCLAMATION);
			g_isProgramLooping = FALSE;
		}
	}
	UnregisterClass(application.className, application.hInstance);    // UnRegister window class
	return 0;
}