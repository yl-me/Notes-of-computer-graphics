#include <windows.h>
#include <stdio.h>
#include <math.h>
#include <gl/glew.h>
#include <gl/glut.h>
#include <GL/GLUAX.H>
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

BOOL light;             // Lighting ON/OFF
BOOL blend;             // Blend ON/OFF
BOOL twinkle;           // Twinkling stars
BOOL tp;                // T pressed
BOOL lp;                // L pressed
BOOL fp;                // F pressed
BOOL bp;                // B pressed

GLfloat LightAmbient[] = {0.5f, 0.5f, 0.5f, 1.0f};    // Ambient light values

GLfloat LightDiffuse[] = {1.0f, 1.0f, 1.0f, 1.0f};    // Diffuse light values

GLfloat LightPosition[] = {0.0f, 0.0f, 2.0f, 1.0f};   // Light position
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
GLuint filter;
GLuint texture[3];
// Piover180 is simply a conversion factor for converting between degrees and radians
const float piover180 = 0.0174532925f;
float heading;
float xpos;
float zpos;
float ytemp1;
float ytemp2;

GLfloat yrot;
GLfloat walkbias = 0.0f;
GLfloat walkbiasangle = 0.0f;
GLfloat lookupdown = 0.0f;
GLfloat z = 0.0f;

typedef struct tagVERTEX VERTEX;
struct tagVERTEX {
    float x, y, z;                                    // 3D Coordinates
    float u, v;                                       // Texture Coordinates
};

typedef struct tagTRIANGLE TRIANGLE;
struct tagTRIANGLE {
    VERTEX vertex[3];                                 // Array Of Three Vertices
};

typedef struct tagSECTOR SECTOR;
struct tagSECTOR {                                    // Build Our Sector Structure
    int numtriangles;                                 // Number Of Triangles In Sector
    TRIANGLE* triangle;                               // Pointer To Array Of Triangles
};

SECTOR sector1;

/*
 *  CreateGLWindow() has a reference to WndProc() but WndProc() comes after CreateGLWindow().
 */

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM); // Declaration for WndProc

/*
 *  The job of the next section of code is to resize the OpenGL scene 
 *  whenever the window (assuming you are using a Window rather than fullscreen mode) has been resized.
 */
void readstr(FILE* f, char* string)
{
    do {
        fgets(string, 255, f);
    } while ((string[0] == '/') || (string[0] == '\n'));
    return ;
}

void SetupWorld()
{
    float x, y, z, u, v;
    int numtriangles;
    FILE* filein;
    char oneline[255];
    filein = fopen("World.txt", "rt");

    readstr(filein, oneline);
    sscanf(oneline, "NUMPOLLIES %d\n", &numtriangles);

    sector1.triangle = new TRIANGLE[numtriangles];
    sector1.numtriangles = numtriangles;

    for (int triloop = 0; triloop < numtriangles; ++triloop) {
        for (int vertloop = 0; vertloop < 3; ++vertloop) {
            readstr(filein, oneline);
            sscanf(oneline, "%f %f %f %f %f", &x, &y, &z, &u, &v);
            sector1.triangle[triloop].vertex[vertloop].x = x;
            sector1.triangle[triloop].vertex[vertloop].y = y;
            sector1.triangle[triloop].vertex[vertloop].z = z;
            sector1.triangle[triloop].vertex[vertloop].u = u;
            sector1.triangle[triloop].vertex[vertloop].v = v;
        }
    }
    fclose(filein);
    return ;
}
 /******************************************************************************************************************************************/
 /******************************************************************************************************************************************/
AUX_RGBImageRec* LoadBMP(char* Filename)              // Loads a bitmap image
{
    FILE* File = NULL;                                // File handle

    if (!Filename) {                                  // Make sure a filename was given
        return NULL;                                  // If not return NULL
    }

    File = fopen(Filename, "r");                      // Check to see of the file exists
    if (File) {
        fclose(File);
        return auxDIBImageLoad(Filename);             // Load the bitmap and return a pointer
    }

    return NULL;
}

int LoadGLTextures()                                  // Load bitmap and convert to texture
{
    int Status = FALSE;                               // Status indicator

    AUX_RGBImageRec* TextureImage[1];                 // Create    storage space for the texture

    memset(TextureImage, 0, sizeof(void*)*1);         // Set the pointer to NULL

    // Load the bitmap, check for error, if bitmap's not found quit
    if (TextureImage[0] = LoadBMP("1.bmp")) {
        Status = TRUE;

        glGenTextures(1, &texture[0]);                // Create the texture
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
        glBindTexture(GL_TEXTURE_2D, texture[0]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); // Linear filtering
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); // Linear filtering
        glTexImage2D(GL_TEXTURE_2D, 0, 3, TextureImage[0]->sizeX, TextureImage[0]->sizeY, 0, GL_RGB,
            GL_UNSIGNED_BYTE, TextureImage[0]->data);
        
        glBindTexture(GL_TEXTURE_2D, texture[1]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // Linear filtering
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // Linear filtering
        glTexImage2D(GL_TEXTURE_2D, 0, 3, TextureImage[0]->sizeX, TextureImage[0]->sizeY, 0, GL_RGB,
            GL_UNSIGNED_BYTE, TextureImage[0]->data);

        glBindTexture(GL_TEXTURE_2D, texture[2]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
        gluBuild2DMipmaps(GL_TEXTURE_2D, 3, TextureImage[0]->sizeX, TextureImage[0]->sizeY, GL_RGB,
            GL_UNSIGNED_BYTE, TextureImage[0]->data);
    }
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
    if (TextureImage[0]) {
        if (TextureImage[0]->data) {
            free(TextureImage[0]->data);
        }
        free(TextureImage[0]);
    }
    return Status;
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

    glMatrixMode(GL_MODELVIEW);                       // Seclet the modelview matrix
    glLoadIdentity();                                 // Reset the modelview matrix
}

int InitGL(GLvoid)                                    // All setup for OpenGL goes here
{
    if (!LoadGLTextures()) {                          // Jump to texture loading routine
        return FALSE;                                 // If texture didn't load return false
    }
    glEnable(GL_TEXTURE_2D);                          // Enable texture mapping
    /*
     *  Smooth shading blends colors nicely across a polygon, and smoothes out lighting.
     */

/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);             // Black background

    /*
     *  Think of the depth buffer as layers into the screen. 
     *  The depth buffer keeps track of how deep objects are into the screen.
     */

    glClearDepth(1.0f);                               // Depth buffer setup
    /*
     *  Next we tell OpenGL we want the best perspective correction to be done. 
     *  This causes a very tiny performance hit, but makes the perspective view look a bit better.
     */
    glDepthFunc(GL_LESS);
    glEnable(GL_DEPTH_TEST);
    glShadeModel(GL_SMOOTH);                          // Enables smooth shading
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);   // Really nice perspective calculations

    SetupWorld();
    return TRUE;
}

/*
 *  For now all we will do is clear the screen to the color we previously decided on, 
 *  clear the depth buffer and reset the scene. We wont draw anything yet.
 */
int DrawGLScene(GLvoid)                                  // Here's where we do all the drawing
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);  // Clear the screen and the depth buffer
    glLoadIdentity();

    GLfloat x_m, y_m, z_m, u_m, v_m;
    GLfloat xtrans = -xpos;
    GLfloat ztrans = -zpos;
    GLfloat ytrans = -walkbias - 0.25f - ytemp1 - ytemp2;   // Used for bouncing motion up and down
    GLfloat sceneroty = 360.0f - yrot;                   // 360 degree angle for player direction
    
    int numtriangles;

    glRotatef(lookupdown, 1.0f, 0.0f, 0.0f);
    glRotatef(sceneroty, 0.0f, 1.0f, 0.0f);
    
    glTranslatef(xtrans, ytrans-0.2, ztrans);
    glBindTexture(GL_TEXTURE_2D, texture[filter]);

    numtriangles = sector1.numtriangles;

    for (int loop_m = 0; loop_m < numtriangles; ++loop_m) {
        glBegin(GL_TRIANGLES);
            glNormal3f(0.0f, 0.0f, 1.0f);
            x_m = sector1.triangle[loop_m].vertex[0].x;
            y_m = sector1.triangle[loop_m].vertex[0].y;
            z_m = sector1.triangle[loop_m].vertex[0].z;
            u_m = sector1.triangle[loop_m].vertex[0].u;
            v_m = sector1.triangle[loop_m].vertex[0].v;
            glTexCoord2f(u_m, v_m); glVertex3f(x_m, y_m, z_m);

            x_m = sector1.triangle[loop_m].vertex[1].x;
            y_m = sector1.triangle[loop_m].vertex[1].y;
            z_m = sector1.triangle[loop_m].vertex[1].z;
            u_m = sector1.triangle[loop_m].vertex[1].u;
            v_m = sector1.triangle[loop_m].vertex[1].v;
            glTexCoord2f(u_m, v_m); glVertex3f(x_m, y_m, z_m);

            x_m = sector1.triangle[loop_m].vertex[2].x;
            y_m = sector1.triangle[loop_m].vertex[2].y;
            z_m = sector1.triangle[loop_m].vertex[2].z;
            u_m = sector1.triangle[loop_m].vertex[2].u;
            v_m = sector1.triangle[loop_m].vertex[2].v;
            glTexCoord2f(u_m, v_m); glVertex3f(x_m, y_m, z_m);
        glEnd();
    }
    return TRUE;                                         // everthing went OK
}
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/*
 *  The job of KillGLWindow() is to release the Rendering Context, 
 *  the Device Context and finally the Window Handle. 
 */

GLvoid KillGLWindow(GLvoid)                              // Properly kill the window
{
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

            /*
             *  It allows us to press the F1 key to switch from fullscreen mode to
             *  windowed mode or windowed mode to fullscreen mode.
             */

            if (keys['L'] && !lp) {                       // L key being pressed not held
                lp = TRUE;                                // lp become TRUE
                light = !light;                           // Toggle light TRUE/FALSE

                if (!light) {
                    glDisable(GL_LIGHTING);               // Disable light
                }
                else {
                    glEnable(GL_LIGHTING);                // Enable light
                }
            }
            if (!keys['L']) {
                lp = FALSE;
            }

            if (keys['F'] && !fp) {
                fp = TRUE;
                filter += 1;
                if (filter > 2) {
                    filter = 0;
                }
            }
            if (!keys['F']) {
                fp = FALSE;
            }
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
            if (keys['W']) {                      //VK_PRIOR
                xpos -= (float)sin(heading*piover180) * 0.0005f;
                zpos -= (float)cos(heading*piover180) * 0.0005f;
                if (walkbiasangle >= 359.0f) {
                    walkbiasangle = 0.0f;
                }
                else {
                    walkbiasangle += 0.02f;
                }
                walkbias = (float)sin(walkbiasangle*piover180) / 20.0f;
            }
            if (keys['S']) {                           // VK_NEXT
                xpos += (float)sin(heading*piover180) * 0.0005f;
                zpos += (float)cos(heading*piover180) * 0.0005f;
                if (walkbiasangle <= 1.0f) {
                    walkbiasangle = 359.0f;
                }
                else {
                    walkbiasangle -= 0.02f;
                }
                walkbias = (float)sin(walkbiasangle*piover180) / 20.0f;
            }
            
            if (keys['A']) {
                zpos += (float)sin(heading * piover180) * 0.0005f;
                xpos -= (float)cos(heading * piover180) * 0.0005f;
                if (walkbiasangle >= 359.0f) {
                    walkbiasangle = 0.0f;
                }
                else {
                    walkbiasangle += 0.02f;
                }
                walkbias = (float)sin(walkbiasangle*piover180) / 20.0f;
            }
            if (keys['D']) {
                zpos -= (float)sin(heading * piover180) * 0.0005f;
                xpos += (float)cos(heading * piover180) * 0.0005f;
                if (walkbiasangle <= 1.0f) {
                    walkbiasangle = 359.0f;
                }
                else {
                    walkbiasangle -= 0.02f;
                }
                walkbias = (float)sin(walkbiasangle*piover180) / 20.0f;
            }

            if (keys[VK_SPACE]) {
                if (ytemp2 < 0.3)
                    ytemp2 += 0.0008;
            }
            else {
                if (ytemp2 >= 0.08)
                ytemp2 -= 0.0008;
            }

            if (keys[VK_CONTROL]) {
                if (ytemp1 > -0.3)
                    ytemp1 -= 0.0008;
            }
            else {
                if (ytemp1 <= 0.01)
                    ytemp1 += 0.0008;
            }

            if (keys['T'] && !tp) {
                tp = TRUE;
                twinkle = !twinkle;
            }
            if (!keys['T']) {
                tp = FALSE;
            }
            if (keys[VK_UP]) {
                z -= 0.05f;
                lookupdown -= 0.05f;
            }
            if (keys[VK_DOWN]) {
                z += 0.05f;
                lookupdown += 0.05f;
            }

            if (keys[VK_RIGHT]) {
                heading -= 0.05f;
                yrot = heading;
            }

            if (keys[VK_LEFT]) {
                heading += 0.05f;
                yrot = heading;
            }

/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
            if (keys['B'] && !bp) {
                bp = TRUE;
                blend = !blend;
                if (blend) {
                    glEnable(GL_BLEND);
                    glDisable(GL_DEPTH_TEST);
                }
                else {
                    glDisable(GL_BLEND);
                    glEnable(GL_DEPTH_TEST);
                }
            }
            if (keys['B']) {
                bp = FALSE;
            }
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