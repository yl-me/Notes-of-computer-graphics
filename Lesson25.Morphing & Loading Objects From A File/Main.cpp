#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <gl/glew.h>
#include <gl/glut.h>
#pragma comment(lib, "legacy_stdio_definitions.lib")

/*
*  Every OpenGL program is linked to a Rendering Context.
*  A Rendering Context is what links OpenGL calls to the Device Context.
*  In order for your program to draw to a Window you need to create a Device Context.
*  The DC connects the Window to the GDI (Graphics Device Interface).
*/

HGLRC     hRC = NULL;         // Permanent rendering context
HDC       hDC = NULL;         // Private GDI device context
HWND      hWnd = NULL;        // Holds our window handle
HINSTANCE hInstance;          // Holds the instance of the application

/*
*  It's important to make this global so that each procedure knows if
*  the program is running in fullscreen mode or not.
*/

bool keys[256];         // Array used for the keyboard routine
bool active = TRUE;     // Window active flag set to TRUE by default
bool fullscreen = TRUE; // Fullscreen flag set to fullscreen mode by default
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
GLfloat xrot, yrot, zrot;
GLfloat xspeed, yspeed, zspeed;
GLfloat cx, cy, cz = -15.0f;

int key = 1;                  // Used to make sure same morph key is not pressed
int step = 0;                 // Step counter
int steps = 200;              // Maximum number of steps
bool morph = false;

typedef struct {              // 3D points
    float x, y, z;
} VERTEX;

typedef struct {              // Object
    int vertices;
    VERTEX* points;
} OBJECT;

int maxver;                   // Will eventually hold the maximum number of vertices
OBJECT morph1, morph2, morph3, morph4;   // 4 morphable object
OBJECT helper;                // Helper objext
OBJECT* sour;                 // Sour object
OBJECT* dest;                 // Dest object

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM); // Declaration for WndProc

void objallocate(OBJECT* k, int n)
{
    k->points = (VERTEX*)malloc(sizeof(VERTEX) * n);
}

void objfree(OBJECT* k)
{
    free(k->points);
}

void readstr(FILE* f, char* string)
{
    do {
        fgets(string, 255, f);
    } while ((string[0] == '/') || (string[0] == '\n'));
}

void objload(char* name, OBJECT* k)
{
    int ver;                      // Will hold vertices count
    float rx, ry, rz;             // Hold vertex X, Y & Z position
    FILE* filein;
    char oneline[255];

    filein = fopen(name, "rt");
    readstr(filein, oneline);
    sscanf(oneline, "Vertices: %d\n", &ver);
    k->vertices = ver;
    objallocate(k, ver);
    for (int i = 0; i < ver; ++i) {
        readstr(filein, oneline);
        sscanf(oneline, "%f %f %f", &rx, &ry, &rz);
        k->points[i].x = rx;
        k->points[i].y = ry;
        k->points[i].z = rz;
    }
    fclose(filein);
    
    if (ver > maxver)
        maxver = ver;
}

VERTEX calculate(int i)          // Calculates movement of points during morphing
{
    VERTEX a;
    a.x = (sour->points[i].x - dest->points[i].x) / steps;
    a.y = (sour->points[i].y - dest->points[i].y) / steps;
    a.z = (sour->points[i].z - dest->points[i].z) / steps;
    return a;
}

GLvoid ReSizeGLScene(GLsizei width, GLsizei height)   // Resize and initialize the GL window
{
    if (height == 0) {                                // Prevent a divide by zero by
        height = 1;                                   // Making height equal one
    }
    
    glViewport(0, 0, width, height);                  // Reset the current viewport

    /*
     *  The following lines set the screen up for a perspective view. 
     *  Meaning things in the distance get smaller. This creates a realistic looking scene. 
     *  The perspective is calculated with a 45 degree viewing angle based on 
     *  the windows width and height. The 0.1f, 100.0f is the starting point and 
     *  ending point for how deep we can draw into the screen.
     *
     *  The projection matrix is responsible for adding perspective to our scene.
     *  glLoadIdentity() restores the selected matrix to it's original state.
     *  The modelview matrix is where our object information is stored.
     *   Lastly we reset the modelview matrix.
     */

    glMatrixMode(GL_PROJECTION);                      // Select the projection matrix
    glLoadIdentity();                                 // Reset the projection matrix
    
                                                      // Calculate the aspect ratio of the window
    gluPerspective(45.0f, (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);
//    glOrtho(0.0f, width, height, 0.0f, -1.0f, 1.0f);  // Create orhto 640X480 view (0, 0, at the top)

    glMatrixMode(GL_MODELVIEW);                       // Seclet the modelview matrix
    glLoadIdentity();                                 // Reset the modelview matrix
}

int InitGL(GLvoid)                                    // All setup for OpenGL goes here
{
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);                // Set the blending function for translucency
    glClearColor(0.0f, 0.0f, 0.0f, 0.5f);             // Black background
    glClearDepth(1.0f);                               // Depth buffer setup
    glDepthFunc(GL_LESS);                             // The type of depth test to do
    glEnable(GL_DEPTH_TEST);
    glShadeModel(GL_SMOOTH);                          // Enables smooth color shading
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NEAREST);

    maxver = 0;
    objload("sphere.txt", &morph1);                   // Sphere
    objload("torus.txt", &morph2);                    // Torus
    objload("tube.txt", &morph3);                     // Tube
    objallocate(&morph4, 486);                        // Manually reserver Ram for a 4th 468 vertices object

    for (int i = 0; i < 486; ++i) {
        morph4.points[i].x = ((float)(rand() % 14000) / 1000) - 7;
        morph4.points[i].y = ((float)(rand() % 14000) / 1000) - 7;
        morph4.points[i].z = ((float)(rand() % 14000) / 1000) - 7;
    }

    objload("sphere.txt", &helper);
    sour = dest = &morph1;

    return TRUE;
}
/*
 *  For now all we will do is clear the screen to the color we previously decided on,
 *  clear the depth buffer and reset the scene. We wont draw anything yet.
 */
void DrawGLScene(GLvoid)                                  // Here's where we do all the drawing
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    glTranslatef(cx, cy, cz);
    glRotatef(xrot, 1.0f, 0.0f, 0.0f);
    glRotatef(yrot, 0.0f, 1.0f, 0.0f);
    glRotatef(zrot, 0.0f, 0.0f, 1.0f);

    xrot += xspeed;
    yrot += yspeed;
    zrot += zspeed;

    GLfloat tx, ty, tz;         // Temp
    VERTEX q;                   //    Holds returned calculated values for one vertex 

    glBegin(GL_POINTS);
        for (int i = 0; i < morph1.vertices; ++i) {
            if (morph) {
                q = calculate(i);
            }
            else {
                q.x = q.y = q.z = 0;
            }
            helper.points[i].x -= q.x;
            helper.points[i].y -= q.y;
            helper.points[i].z -= q.z;
            tx = helper.points[i].x;
            ty = helper.points[i].y;
            tz = helper.points[i].z;

            glColor3f(0.0f, 1.0f, 1.0f);
            glVertex3f(tx, ty, tz);

            glColor3f(0.0f, 0.5f, 1.0f);
            tx -= 2 * q.x;
            ty -= 2 * q.y;
            tz -= 2 * q.z;
            glVertex3f(tx, ty, tz);

            glColor3f(0.0f, 0.0f, 1.0f);
            tx -= 2 * q.x;
            ty -= 2 * q.y;
            tz -= 2 * q.z;
            glVertex3f(tx, ty, tz);
        }
    glEnd();

    if (morph && step <= steps)
        step++;
    else {
        morph = false;
        sour = dest;
        step = 0;
    }
}
/*
 *  The job of KillGLWindow() is to release the Rendering Context, 
 *  the Device Context and finally the Window Handle. 
 */

GLvoid KillGLWindow(GLvoid)                              // Properly kill the window
{
    objfree(&morph1);
    objfree(&morph2);
    objfree(&morph3);
    objfree(&morph4);
    objfree(&helper);
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/

    if (fullscreen) {                                    // Are we in fullscreen mode
        
        /*
         *  We use ChangeDisplaySettings(NULL,0) to return us to our original desktop.
         *  After we've switched back to the desktop we make the cursor visible again.
         */

        ChangeDisplaySettings(NULL, 0);                  // if so switch back to the desktop
        ShowCursor(TRUE);                                // Show mouse pointer
    }

    if (hRC) {                                           // Do we have a rendering context
        if (!wglMakeCurrent(NULL, NULL)) {                // Are we able to release the DC and RC contexts
            MessageBox(NULL, "Release of DC and RC failed.", "SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
        }

        if (!wglDeleteContext(hRC)) {                     // Are we able to delete the RC
            MessageBox(NULL, "Release rendering context failed.", "SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
            hRC = NULL;                                  // Set RC to NULL
        }

        if (hDC && !ReleaseDC(hWnd, hDC)) {              // Are we able to release the DC
            MessageBox(NULL, "Release device context failed.", "SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
            hDC = NULL;                                  // Set DC to NULL
        }
        if (hWnd && !DestroyWindow(hWnd)) {              // Are we able to destroy the window
            MessageBox(NULL, "Could not release hWnd.", "SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
            hWnd = NULL;                                 // Set hWnd to NULL
        }

        if (!UnregisterClass("OpenGL", hInstance)) {     // Are we able to unregister class
            MessageBox(NULL, "Could not register class.", "SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
            hInstance = NULL;                            // Set hInstance to NULL
        }
    }
}

/*
 * The next section of code creates our OpenGL Window.
 */

BOOL CreateGLWindow(char* title, int width, int height, int bits, bool fullscreenflag)
{
    /*
     * Find  a pixel format that matches the one we want
     */
    GLuint PixelFormat;                                  // Holds the result after serching for a match
    
    /*
     * Before you create a window, you MUST register a Class for the window
     */
    WNDCLASS wc;                                         // Windows class structure

    /*
     *  dwExStyle and dwStyle will store the Extended and normal Window Style Information.
    */
    DWORD dwExStyle;                                     // Window extend style
    DWORD dwStyle;                                       // Window style

    RECT WindowRect;                                     // Grabs rectangle upper left/lower right values
    WindowRect.left = (long)0;                           // Set left value to 0
    WindowRect.right = (long)width;                      // Set right value to requested width
    WindowRect.top = (long)0;                            // Set top value to 0
    WindowRect.bottom = (long)height;                    // Set bottom value to requested height

    fullscreen = fullscreenflag;                         // Set the global fullscreen flag

    /*
     *  The style CS_HREDRAW and CS_VREDRAW force the Window to redraw whenever it is resized. 
     *  CS_OWNDC creates a private DC for the Window. Meaning the DC is not shared across applications. 
     *  WndProc is the procedure that watches for messages in our program. 
     *  No extra Window data is used so we zero the two fields. Then we set the instance. 
     *  Next we set hIcon to NULL meaning we don't want an ICON in the Window, 
     *  and for a mouse pointer we use the standard arrow. The background color doesn't matter 
     *  (we set that in GL). We don't want a menu in this Window so we set it to NULL, 
     *  and the class name can be any name you want. I'll use "OpenGL" for simplicity.
     */
    hInstance = GetModuleHandle(NULL);                   // Grab an instance for our window
    wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;       // Redraw on move, and own DC for window
    wc.lpfnWndProc = (WNDPROC)WndProc;                   // WndProc handles message
    wc.cbClsExtra = 0;                                   // No extra window date
    wc.cbWndExtra = 0;                                   // No extra window date
    wc.hInstance = hInstance;                            // set the instance
    wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);              // Load the default icon
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);            // Load the arrow pointer
    wc.hbrBackground = NULL;                             // No background requried for GL
    wc.lpszMenuName = NULL;                              // We don't want a menu
    wc.lpszClassName = "OpenGL";                         // set the class name

    if (!RegisterClass(&wc)) {                           // Attempt to register the window class
        MessageBox(NULL, "Failed to register the window class.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
        return FALSE;                                    // Exit and return false
    }

    if (fullscreen) {                                    // attempt fullsreen model
        
        /*
        T*  here are a few very important things you should keep in mind when switching to full screen mode.
         *  Make sure the width and height that you use in fullscreen mode is the same as 
         *  the width and height you plan to use for your window, and most importantly,
         *  set fullscreen mode BEFORE you create your window.
         */
        DEVMODE dmScreenSettings;                        // Device mode
        memset(&dmScreenSettings, 0, sizeof(dmScreenSettings)); // Make sure memory's cleared
        dmScreenSettings.dmSize = sizeof(dmScreenSettings);     // Size of devmode structure
        dmScreenSettings.dmPelsWidth = width;            // Select window width
        dmScreenSettings.dmPelsHeight = height;          // Select window height
        dmScreenSettings.dmBitsPerPel = bits;            // Select bits per pixel
        dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
        
        /*
         *  In the line below ChangeDisplaySettings tries to switch to a mode that matches 
         *  what we stored in dmScreenSettings. I use the parameter CDS_FULLSCREEN when switching modes, 
         *  because it's supposed to remove the start bar at the bottom of the screen, 
         *  plus it doesn't move or resize the windows on your desktop when you switch to 
         *  fullscreen mode and back.
         */
        //Try to set selected mode and get results. Note: CDS_FULLSCREEN gets rid of start bar
        if (ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL) {
            //If the mode fails, offer two options. Quit or run in a window
            if (MessageBox(NULL, "The requested fullscreen mode is not supported by\n your video card. Use"
                "windowed mode instead?", "GL", MB_YESNO | MB_ICONEXCLAMATION) == IDYES)
            {
                fullscreen = FALSE;                       // Select windowed mode (fullscreen=FLASE)
            }
            else {
                // Pop up a message box letting user know the programe is closing.
                MessageBox(NULL, "Program will now close.", "ERROR", MB_OK | MB_ICONSTOP);
                return FALSE;                             // Exit and return FALSE
            }
        }
    }

    if (fullscreen) {                                     // Are we still in fullscreen mode
        
        /*
         *  If we are still in fullscreen mode we'll set the extended style to WS_EX_APPWINDOW, 
         *  which force a top level window down to the taskbar once our window is visible. 
         *  For the window style we'll create a WS_POPUP window. 
         *  This type of window has no border around it, making it perfect for fullscreen mode.

         *  Finally, we disable the mouse pointer. If your program is not interactive, 
         *  it's usually nice to disable the mouse pointer when in fullscreen mode. It's up to you though.
         */
        dwExStyle = WS_EX_APPWINDOW;                      // Window extended style
        dwStyle = WS_POPUP;                               // Window style
        ShowCursor(FALSE);                                // Hide mosue pointer 
    }
    else {

        /*
         *  If we're using a window instead of fullscreen mode, 
         *  we'll add WS_EX_WINDOWEDGE to the extended style. This gives the window a more 3D look. 
         *  For style we'll use WS_OVERLAPPEDWINDOW instead of WS_POPUP. 
         *  WS_OVERLAPPEDWINDOW creates a window with a title bar, sizing border, 
         *  window menu, and minimize / maximize buttons.
         */
        dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;   // Window extended style
        dwStyle = WS_OVERLAPPEDWINDOW;                    // Window style
    }

    /*
     *  By using the AdjustWindowRectEx command none of our OpenGL scene will be covered up by the borders, 
     *  instead, the window will be made larger to account for the pixels needed to draw the window border. 
     *  In fullscreen mode, this command has no effect.
     */
    AdjustWindowRectEx(&WindowRect, dwStyle, FALSE, dwExStyle);  // Adjust window to true resqusted
    
    /*
     *  WS_CLIPSIBLINGS and WS_CLIPCHILDREN are both REQUIRED for OpenGL to work properly. 
     *  These styles prevent other windows from drawing over or into our OpenGL Window.
     */
    if (!(hWnd = CreateWindowEx(dwExStyle,                // Extended style for the window
        "OpenGL",                                         // Class name
        title,                                            // Window title
        WS_CLIPSIBLINGS |                                 // Requried window style
        WS_CLIPCHILDREN |                                 // Requried window style
        dwStyle,                                          // Select window style
        0, 0,                                             // Window position
        WindowRect.right - WindowRect.left,               // Calculate adjusted window width
        WindowRect.bottom - WindowRect.top,               // Calculate adjusted window height
        NULL,                                             // No parent window
        NULL,                                             // No menu
        hInstance,                                        // Instance
        NULL)))                                           // Don't pass anything to WM_CREATE
    {
        KillGLWindow();                                   //Reset the display
        MessageBox(NULL, "Window creation error.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
        return FALSE;                                     // Retrurn FALSE;
    }

    /*
     *  aside from the stencil buffer and the (slow) accumulation buffer
     */
    static PIXELFORMATDESCRIPTOR pfd =                    // pfd tells windows how we want things to be 
    {
        sizeof(PIXELFORMATDESCRIPTOR),                    // Size of this pixel format descriptor
        1,                                                // Version number
        PFD_DRAW_TO_WINDOW |                              // Format must support window
        PFD_SUPPORT_OPENGL |                              // Format must support OpenGL
        PFD_DOUBLEBUFFER,                                 // Must support double buffer
        PFD_TYPE_RGBA,                                    // Request an RGBA format
        bits,                                             // Select our color depth
        0, 0, 0, 0, 0, 0,                                 // Color bits ignored
        0,                                                // No alpha buffer
        0,                                                // shift bit ignored
        0,                                                // No accumulation buffer
        0, 0, 0, 0,                                       // Accumulation bits ignored
        16,                                               // 16Bits Z_Buffer (depth buffer)
        0,                                                // No stencil buffer
        0,                                                // No auxiliary buffer
        PFD_MAIN_PLANE,                                   // Main drawing layer
        0,                                                // Reserved
        0, 0, 0                                           // Layer makes ignored
    };

    if (!(hDC = GetDC(hWnd))) {                           // Did we get a device context
        KillGLWindow();                                   // Reset the display
        MessageBox(NULL, "Can't create a GL device context.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
        return FALSE;                                     // Return FALSE
    }

    if (!(PixelFormat = ChoosePixelFormat(hDC, &pfd))) {  // Did window find a matching pixel format
        KillGLWindow();                                   // Reset the display
        MessageBox(NULL, "Can't find a suitable pixelformat.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
        return FALSE;                                     // Return FALSE;
    }

    if (!SetPixelFormat(hDC, PixelFormat, &pfd)) {        // Are we able to set the pixel format
        KillGLWindow();                                   // Reset the display
        MessageBox(NULL, "Can't set the pixelformat.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
        return FALSE;                                     // Return FALSE;
    }

    if (!(hRC = wglCreateContext(hDC))) {                 // Are we able to rendering context
        KillGLWindow();                                   // Reset the display
        MessageBox(NULL, "Can't create a GL rendering context.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
        return FALSE;                                     // Return FASLE;
    }

    if (!wglMakeCurrent(hDC, hRC)) {                      // Try to activate the rendering context
        KillGLWindow();                                   // Reset the display
        MessageBox(NULL, "Can't activate the GL rendering context.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
        return FALSE;                                     // Return FALSE    
    }

    /*
     *  ReSizeGLScene passing the screen width and height to set up our perspective OpenGL screen.
     */
    ShowWindow(hWnd, SW_SHOW);                            // Show the window
    SetForegroundWindow(hWnd);                            // slightly higher priority
    SetFocus(hWnd);                                       // Sets keyboard focus to the window
    ReSizeGLScene(width, height);                         // Set up our perspective GL screen

/*
 *  we can set up lighting, textures, and anything else that needs to be setup in InitGL().
 */
    if (!InitGL()) {                                      // Initialize our newly created GL window
        KillGLWindow();                                   // Reset the display
        MessageBox(NULL, "Initialize Failed.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
        return FALSE;                                     // Return FALSE
    }
    return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd,                       // Handle for this window
    UINT uMsg,                                            // Message for this window
    WPARAM wParam,                                        // Additional message information
    LPARAM lParam)                                        // Additional message information
{
    switch (uMsg) {                                       // Check for window message
    case WM_ACTIVATE: {                               // Check minimization state
        if (!HIWORD(wParam)) {
            active = TRUE;                            // Program is active
        }
        else {
            active = FALSE;                           // Program is no longer active
        }
        return 0;                                     // Return to the message loop
    }
    case WM_SYSCOMMAND: {                             // Intercept system commands
        switch (wParam) {                             // Check system calls
        case SC_SCREENSAVE:                       // Screensaver trying to start
        case SC_MONITORPOWER:                     // Monitor trying to enter powersave
            return 0;                                 // Prevent form happening
        }
        break;                                        // Exit
    }
    case WM_CLOSE: {                                  // Did we receive a close message
        PostQuitMessage(0);                           // Send a quit message
        return 0;
    }
    case WM_KEYDOWN: {                                // Is a key being held down
        keys[wParam] = TRUE;                          // if so, mark it as TRUE
        return 0;                                     // Jump back
    }
    case WM_KEYUP: {                                  // Has a key been released
        keys[wParam] = FALSE;                         // if so, mark it as FALSE
        return 0;                                     // Jump back
    }
    case WM_SIZE: {                                   // Resize the OpenGL window
        ReSizeGLScene(LOWORD(lParam), HIWORD(lParam));   // LoWord = width HiWord = height
        return 0;                                     // Jump back
    }
    }
    return DefWindowProc(hWnd, uMsg, wParam, lParam);     // Pass all unhandled message to DefWindwProc
}

int WINAPI WinMain(HINSTANCE hInstance,                   // Instance
    HINSTANCE hPrevInstance,                              // Previous instance
    LPSTR lpCmdLine,                                      // Command line parameters
    int nCmdShow)                                         // Window show state
{
    MSG msg;                                              // Window message structure
    BOOL done = FALSE;                                    // Bool variable to exit loop
                                                          // Ask the user which screen mode they prefer
    if (MessageBox(NULL, "Would you like to run in fullscreen mode?",
        "Start fullscreen?", MB_YESNO | MB_ICONQUESTION) == IDNO)
    {
        fullscreen = FALSE;                               // Window mode
    }
    // Create our OpenGL window
    if (!CreateGLWindow("3D Shapes", 640, 480, 16, fullscreen)) {  // (Modified)
        return 0;                                         // Quit if window was not create
    }
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/

    while (!done) {                                       // Loop that runs until donw = TRUE
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {   // Is there a message wating
            if (msg.message == WM_QUIT) {                 // Havw we received a quit message
                done = TRUE;                              // if so done  = TRUE
            }
            else {                                        // If not, deal with window message
                TranslateMessage(&msg);                   // Translate message
                DispatchMessage(&msg);                    // Dispatch message
            }
        }
        else {
            // Draw the scene. Watch for ESC key and quit message from DrawGLScene()
            if (active) {                                 // Program active
                if (keys[VK_ESCAPE]) {                    // Was ESC pressed
                    done = TRUE;                          // ESC signalled a quit
                }
                else {                                    // Not time to quit, update screen
                    DrawGLScene();                        // Draw scene
                    SwapBuffers(hDC);                     // Swap buffers (double buffering)
                }
            }

            if (keys[VK_PRIOR]) zspeed += 0.0001f;
            if (keys[VK_NEXT]) zspeed -= 0.0001f;
            if (keys[VK_DOWN]) xspeed += 0.0001f;
            if (keys[VK_UP]) xspeed -= 0.0001f;
            if (keys[VK_RIGHT]) yspeed += 0.0001f;
            if (keys[VK_LEFT]) yspeed -= 0.0001f;
            if (keys['Q']) cz -= 0.001f;
            if (keys['Z']) cz += 0.001f;
            if (keys['W']) cy += 0.001f;
            if (keys['S']) cy -= 0.001f;
            if (keys['D']) cx += 0.001f;
            if (keys['A']) cx -= 0.001f;
            if (keys['1'] && (key != 1) && !morph) { key = 1; morph = true; dest = &morph1; }
            if (keys['2'] && (key != 2) && !morph) { key = 2; morph = true; dest = &morph2; }
            if (keys['3'] && (key != 3) && !morph) { key = 3; morph = true; dest = &morph3; }
            if (keys['4'] && (key != 4) && !morph) { key = 4; morph = true; dest = &morph4; }
            
            /*
            *  It allows us to press the F1 key to switch from fullscreen mode to
            *  windowed mode or windowed mode to fullscreen mode.
            */
            if (keys[VK_F1]) {                            // Is F1 being pressed
                keys[VK_F1] = FALSE;                      // If so make key FASLE
                KillGLWindow();                           // Kill our current window
                fullscreen = !fullscreen;                 // Toggle fullscreen / window mode
                //Recreate our OpenGL window(modified)
                if (!CreateGLWindow("3D Shapes", 640, 480, 16, fullscreen)) {
                    return 0;                             // Quit if window was not create
                }
            }
        }
    }
    // Shutdown
    KillGLWindow();                                       // Kill the window
    return (msg.wParam);                                  // Exit the program
}
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/