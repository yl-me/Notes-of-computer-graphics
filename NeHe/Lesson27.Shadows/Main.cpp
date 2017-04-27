#include <windows.h>
#include <gl/glew.h>
#include <GL/GLUAX.H>
#include "C:\Users\lenovo\Desktop\OpenGL\OpenGL\3Dobject.h"

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
typedef float GLvector4f[4];
typedef float GLmatrix16f[16];

ShadowedObject object;
GLfloat xrot = 0;
GLfloat yrot = 0;
GLfloat xspeed = 0;
GLfloat yspeed = 0;

// Light parameters
GLfloat LightPos[] = { 0.0f, 5.0f, -4.0f, 1.0f };
GLfloat LightAmb[] = { 0.2f, 0.2f, 0.2f, 1.0f };
GLfloat LightDif[] = { 0.6f, 0.6f, 0.6f, 1.0f };
GLfloat LightSpc[] = { -0.2f, -0.2f, -0.2f, 1.0f };  // Specular light

GLfloat MatAmb[] = { 0.4f, 0.4f, 0.4f, 1.0f };
GLfloat MatDif[] = { 0.2f, 0.6f, 0.9f, 1.0f };
GLfloat MatSpc[] = { 0.0f, 0.0f, 0.0f, 1.0f };
GLfloat MatShn[] = { 0.0f };              // Material - shininess

float ObjectPosition[] = { -2.0f, -2.0f, -5.0f };

GLUquadricObj * q;                // Quadratic for drawing a sphere

float SpherePosition[] = { -4.0f, -5.0f, -6.0f };

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM); // Declaration for WndProc
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
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
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
void VMatMult(GLmatrix16f M, GLvector4f v)
{
    GLfloat res[4];
    res[0] = M[0] * v[0] + M[4] * v[1] + M[8] * v[2] + M[12] * v[3];
    res[1] = M[1] * v[0] + M[5] * v[1] + M[9] * v[2] + M[13] * v[3];
    res[2] = M[2] * v[0] + M[6] * v[1] + M[10] * v[2] + M[14] * v[3];
    res[3] = M[3] * v[0] + M[7] * v[1] + M[11] * v[2] + M[15] * v[3];
    v[0] = res[0];
    v[1] = res[1];
    v[2] = res[2];
    v[3] = res[3];
}

int InitGLObject()
{
    if (!readObject("Data/Object2.txt", object)) {
        return false;
    }
    setConnectivity(object);        // Set face to face connectivity
    
    for (int i = 0; i < object.nFaces; ++i) {
        calculatePlane(object, object.pFaces[i]);     // Compute plane equations for all faces
    }
    return true;
}

int InitGL(GLvoid)                                    // All setup for OpenGL goes here
{
    if (!InitGLObject()) {
        return false;
    }

    glShadeModel(GL_SMOOTH);
    glClearColor(0.0f, 0.0f, 0.0f, 0.5);              // Background
    glClearDepth(1.0f);                               // Depth buffer setup
    glClearStencil(0);                                // Clear the stencil buffer to 0
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    
    
    glLightfv(GL_LIGHT1, GL_AMBIENT, LightAmb);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, LightDif);
    glLightfv(GL_LIGHT1, GL_POSITION, LightPos);
    glLightfv(GL_LIGHT1, GL_SPECULAR, LightSpc);

    glEnable(GL_LIGHT1);
    glEnable(GL_LIGHTING);

    glMaterialfv(GL_FRONT, GL_AMBIENT, MatAmb);   // Set material ambience
    glMaterialfv(GL_FRONT, GL_DIFFUSE, MatDif);   // Set material diffuse
    glMaterialfv(GL_FRONT, GL_SPECULAR, MatSpc);  // Set material specular
    glMaterialfv(GL_FRONT, GL_SHININESS, MatShn); // Set material shininess

    glCullFace(GL_BACK);                        // Set culling face to back face
    glEnable(GL_CULL_FACE);
    glClearColor(0.1f, 1.0f, 0.5f, 1.0f);       // Greenish color

    q = gluNewQuadric();
    gluQuadricNormals(q, GL_SMOOTH);            // Generate smooth Normals for the quad
    gluQuadricTexture(q, GL_FALSE);              // Disable auto texture coords for the quad

    return TRUE;
}

void DrawGLRoom()
{
    glBegin(GL_QUADS);
        // Floor
        glNormal3f(0.0f, 1.0f, 0.0f);
        glVertex3f(-10.0f, -10.0f, -20.0f);
        glVertex3f(-10.0f, -10.0f, 20.0f);
        glVertex3f(10.0f, -10.0f, 20.0f);
        glVertex3f(10.0f, -10.0f, -20.0f);
        // Ceiling
        glNormal3f(0.0f, -1.0f, 0.0f);
        glVertex3f(-10.0f, 10.0f, 20.0f);
        glVertex3f(-10.0f, 10.0f, -20.0f);
        glVertex3f(10.0f, 10.0f, -20.0f);
        glVertex3f(10.0f, 10.0f, 20.0f);
        // Front Wall
        glNormal3f(0.0f, 0.0f, 1.0f);
        glVertex3f(-10.0f, 10.0f, -20.0f);
        glVertex3f(-10.0f, -10.0f, -20.0f);
        glVertex3f(10.0f, -10.0f, -20.0f);
        glVertex3f(10.0f, 10.0f, -20.0f);
        // Back Wall
        glNormal3f(0.0f, 0.0f, -1.0f);
        glVertex3f(10.0f, 10.0f, 20.0f);
        glVertex3f(10.0f, -10.0f, 20.0f);
        glVertex3f(-10.0f, -10.0f, 20.0f);
        glVertex3f(-10.0f, 10.0f, 20.0f);
        // Left Wall
        glNormal3f(1.0f, 0.0f, 0.0f);
        glVertex3f(-10.0f, 10.0f, 20.0f);
        glVertex3f(-10.0f, -10.0f, 20.0f);
        glVertex3f(-10.0f, -10.0f, -20.0f);
        glVertex3f(-10.0f, 10.0f, -20.0f);
        // Right Wall
        glNormal3f(-1.0f, 0.0f, 0.0f);
        glVertex3f(10.0f, 10.0f, -20.0f);
        glVertex3f(10.0f, -10.0f, -20.0f);
        glVertex3f(10.0f, -10.0f, 20.0f);
        glVertex3f(10.0f, 10.0f, 20.0f);
    glEnd();
}

/*
 *  For now all we will do is clear the screen to the color we previously decided on,
 *  clear the depth buffer and reset the scene. We wont draw anything yet.
 */
bool DrawGLScene(GLvoid)                                  // Here's where we do all the drawing
{
    GLmatrix16f Minv;
    GLvector4f wlp, lp;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    
    glLoadIdentity();
    glTranslatef(0.0f, 0.0f, -20.0f);
    glLightfv(GL_LIGHT1, GL_POSITION, LightPos);
    glTranslatef(SpherePosition[0], SpherePosition[1], SpherePosition[2]);
    gluSphere(q, 1.5f, 32, 16);

    glLoadIdentity();
    glRotatef(-yrot, 0.0f, 1.0f, 0.0f);
    glRotatef(-xrot, 1.0f, 0.0f, 0.0f);
    glGetFloatv(GL_MODELVIEW_MATRIX, Minv);
    
    lp[0] = LightPos[0];
    lp[1] = LightPos[1];
    lp[2] = LightPos[2];
    lp[3] = LightPos[3];
    VMatMult(Minv, lp);
    glTranslatef(-ObjectPosition[0], -ObjectPosition[1], -ObjectPosition[2]);
    glGetFloatv(GL_MODELVIEW_MATRIX, Minv);
    
    wlp[0] = 0.0f;
    wlp[1] = 0.0f;
    wlp[2] = 0.0f;
    wlp[3] = 1.0f;
    VMatMult(Minv, wlp);

    lp[0] += wlp[0];
    lp[1] += wlp[1];
    lp[2] += wlp[2];

    glColor4f(0.7f, 0.4f, 0.0f, 1.0f);
    glLoadIdentity();
    glTranslatef(0.0f, 0.0f, -20.0f);
    DrawGLRoom();
    glTranslatef(ObjectPosition[0], ObjectPosition[1], ObjectPosition[2]);
    glRotatef(xrot, 1.0f, 0.0f, 0.0f);
    glRotatef(yrot, 0.0f, 1.0f, 0.0f);
    drawObject(object);
    castShadow(object, lp);

    glColor4f(0.7f, 0.4f, 0.0f, 1.0f);
    glDisable(GL_LIGHTING);
    glDepthMask(GL_FALSE);
    glTranslatef(lp[0], lp[1], lp[2]);

    gluSphere(q, 0.2f, 16, 8);
    glEnable(GL_LIGHTING);
    glDepthMask(GL_TRUE);
    xrot += xspeed;
    yrot += yspeed;
    glFlush();
    return true;
}

void ProcessKeyboard()
{
    // Spin object
    if (keys[VK_LEFT]) yspeed -= 0.001f;
    if (keys[VK_RIGHT]) yspeed += 0.001f;
    if (keys[VK_UP]) xspeed -= 0.001f;
    if (keys[VK_DOWN]) xspeed += 0.001f;

    // Adjust light's position
    if (keys['L']) LightPos[0] += 0.01f;
    if (keys['J']) LightPos[0] -= 0.01f;
    if (keys['I']) LightPos[1] += 0.01f;
    if (keys['K']) LightPos[1] -= 0.01f;
    if (keys['O']) LightPos[2] += 0.01f;
    if (keys['U']) LightPos[2] -= 0.01f;

    // Adjust object's position
    if (keys[VK_NUMPAD6]) ObjectPosition[0] += 0.01f;
    if (keys[VK_NUMPAD4]) ObjectPosition[0] -= 0.01f;
    if (keys[VK_NUMPAD8]) ObjectPosition[1] += 0.01f;
    if (keys[VK_NUMPAD5]) ObjectPosition[1] -= 0.01f;
    if (keys[VK_NUMPAD9]) ObjectPosition[2] += 0.01f;
    if (keys[VK_NUMPAD7]) ObjectPosition[2] -= 0.01f;

    // Adjust ball's position
    if (keys['D']) SpherePosition[0] += 0.01f;
    if (keys['A']) SpherePosition[0] -= 0.01f;
    if (keys['W']) SpherePosition[1] += 0.01f;
    if (keys['S']) SpherePosition[1] -= 0.01f;
    if (keys['E']) SpherePosition[2] += 0.01f;
    if (keys['Q']) SpherePosition[2] -= 0.01f;
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
//******************************************************************************************************************************************/
//******************************************************************************************************************************************/
    killObject(object);
//******************************************************************************************************************************************/
//******************************************************************************************************************************************/
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
        1,                                                // stencil buffer
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
    if (!CreateGLWindow("3D Shapes", 800, 600, 32, fullscreen)) {  // (Modified)
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
                    ProcessKeyboard();
                }
            }
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

work.cpp