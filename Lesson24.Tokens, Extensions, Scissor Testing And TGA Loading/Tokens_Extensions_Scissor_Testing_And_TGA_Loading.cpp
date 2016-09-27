#include <windows.h>
#include <stdarg.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <gl/glew.h>
#include <gl/glut.h>
#include <GL/GLUAX.H>
#include <GL/glext.h>
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
int scroll;             // Used for scrolling the screen
int maxtokens;          // Keeps track of the number of extensions supported
int swidth;             // Scissor width
int sheight;            // Scissor height

GLuint base;            // Base display list for the font

typedef struct {
    GLubyte* imageData;
    GLuint bpp;        // Image color depth in bits per pixel
    GLuint width;
    GLuint height;
    GLuint texID;      // Texture ID used to select a texture
} TextureImage;

TextureImage texture[1];

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM); // Declaration for WndProc

bool LoadTGA(TextureImage* texture, char* filename)                        // Loads a TGA filr into memory
{
    GLubyte TGAheader[12] = { 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0 };        // Uncompressed TGA
    GLubyte TGAcompare[12];       // Used to compare TGA header
    GLubyte header[6];            // First 6 useful bytes from the header
    GLuint bytesPerpixel;         // Holds number of byte per pixel used in the TGA file

    GLuint imageSize;             // Used to store the image size when setting aside ram
    GLuint temp;                  // Temporary variable
    GLuint type = GL_RGBA;        // Set the default GL mode to RGBA (32 bpp)

    FILE* file = fopen(filename, "rb");

    if ((file == NULL) ||         // Dose file even exits
        (fread(TGAcompare, 1, sizeof(TGAcompare), file) != sizeof(TGAcompare)) || // Are there 12 bytes to read
            (memcmp(TGAheader, TGAcompare, sizeof(TGAheader)) != 0) || // Dose the header match what we want
            (fread(header, 1, sizeof(header), file) != sizeof(header))) // If so read next 6 header bytes
    {
        if (file == NULL) {
            return false;
        }
        else {
            fclose(file);
            return false;
        }
    }
    texture->width = header[1] * 256 + header[0];   // Determine the TGA width (highbyte * 256 + lowbyte)
    texture->height = header[3] * 256 + header[2];  // Determine the TGA height (highbyte * 256 + lowbyte)

    if (texture->width <= 0 || texture->height <= 0 || (header[4] != 24 && header[4] != 32)) {
        fclose(file);
        return false;
    }

    texture->bpp = header[4];                       // Grab the TGA's bits per pixel (24 or 32)
    bytesPerpixel = texture->bpp / 8;               // Divide by 8 to get the bytes per pixel
    // Calculate the memory required for the TGA data
    imageSize = texture->width * texture->height * bytesPerpixel;

    texture->imageData = (GLubyte*)malloc(imageSize);     // Resever memory to hold the TGA data

    if (texture->imageData == NULL ||
        fread(texture->imageData, 1, imageSize, file) != imageSize)
    {
        if (texture->imageData == NULL) {
            free(texture->imageData);
        }
        fclose(file);
        return false;
    }

    for (GLuint i = 0; i < int(imageSize); i += bytesPerpixel) {
        temp = texture->imageData[i];        // Swap the 1st and 3rd bytes (R and B)
        texture->imageData[i] = texture->imageData[i + 2];
        texture->imageData[i + 2] = temp;
    }
    fclose(file);

    // Build a texture from the data
    glGenTextures(1, &texture[0].texID);

    glBindTexture(GL_TEXTURE_2D, texture[0].texID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    // The TGA was 24 bit, the type will be GL_RGB,  the TGA was 32 bit, the type would be GL_RGBA
    if (texture[0].bpp == 24) {
        type = GL_RGB;
    }

    glTexImage2D(GL_TEXTURE_2D, 0, type, texture[0].width, texture[0].height, 0, type,
        GL_UNSIGNED_BYTE, texture[0].imageData);
    return true;
}

GLvoid BuildFont(GLvoid)
{
    base = glGenLists(256);
    glBindTexture(GL_TEXTURE_2D, texture[0].texID);

    for (int loop = 0; loop < 256; ++loop) {
        float cx = float(loop % 16) / 16.0f;          // X position of current character
        float cy = float(loop / 16) / 16.0f;          // Y position of current character
        glNewList(base + loop, GL_COMPILE);
            glBegin(GL_QUADS);
                glTexCoord2f(cx, 1.0f - cy - 0.0625f);        // Texture coord
                glVertex2d(0, 16);                            // Vertex coord
                glTexCoord2f(cx + 0.0625f, 1.0f - cy - 0.0625f);
                glVertex2i(16, 16);
                glTexCoord2f(cx + 0.0625f, 1.0f - cy - 0.001f);
                glVertex2i(16, 0);
                glTexCoord2f(cx, 1.0f - cy - 0.001f);
                glVertex2i(0, 0);
            glEnd();
            glTranslated(14, 0, 0);
        glEndList();
    }
}

GLvoid KillFont(GLvoid)
{
    glDeleteLists(base, 256);
}

GLvoid glPrint(GLint x, GLint y, int set, const char* fmt, ...)
{
    char text[1024];
    va_list ap;                 // Pointer to list of arguments

    if (fmt == NULL) {
        return;
    }

    va_start(ap, fmt);          // Parses the string for variables
        vsprintf(text, fmt, ap);    // And converts symbols to actual
    va_end(ap);                 // Result are stored in text

    if (set > 1) {
        set = 1;
    }
    glEnable(GL_TEXTURE_2D);
    glLoadIdentity();
    glTranslated(x, y, 0);
    glListBase(base - 32 + (128 * set));              // Choose the font set (0 or 1)
    
    glScalef(1.0f, 2.0f, 1.0f);                       // Make the text 2X taller

    glCallLists(strlen(text), GL_UNSIGNED_BYTE, text);   // Write the text to the screen

    glDisable(GL_TEXTURE_2D);
}

GLvoid ReSizeGLScene(GLsizei width, GLsizei height)   // Resize and initialize the GL window
{
    swidth = width;                                   // Set scissor width to window width
    sheight = height;

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
//    gluPerspective(45.0f, (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);
    glOrtho(0.0f, width, height, 0.0f, -1.0f, 1.0f);  // Create orhto 640X480 view (0, 0, at the top)

    glMatrixMode(GL_MODELVIEW);                       // Seclet the modelview matrix
    glLoadIdentity();                                 // Reset the modelview matrix
}

int InitGL(GLvoid)                                    // All setup for OpenGL goes here
{
    if (!LoadTGA(&texture[0], "Font.tga")) {
        return FALSE;
    }

    BuildFont();

    glShadeModel(GL_SMOOTH);                          // Enables smooth shading
    glClearColor(0.0f, 0.0f, 0.0f, 0.5f);             // Black background

    glClearDepth(1.0f);                               // Depth buffer setup

    glBindTexture(GL_TEXTURE_2D, texture[0].texID);

    return TRUE;
}
/*
 *  For now all we will do is clear the screen to the color we previously decided on,
 *  clear the depth buffer and reset the scene. We wont draw anything yet.
 */
int DrawGLScene(GLvoid)                                  // Here's where we do all the drawing
{
    char* token;                                         // Storage for our token
    int cnt = 0;                                         // Loacl counter variable

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glColor3f(1.0f, 0.5f, 0.5f);
    glPrint(50, 16, 1, "Renderer");
    glPrint(80, 48, 1, "Vendor");
    glPrint(66, 80, 1, "Version");

    glColor3f(1.0f, 0.7f, 0.4f);
    glPrint(200, 16, 1, (char*)glGetString(GL_RENDERER));
    glPrint(200, 48, 1, (char*)glGetString(GL_VENDOR));
    glPrint(200, 80, 1, (char*)glGetString(GL_VERSION));

    glColor3f(0.5f, 0.5f, 1.0f);
    glPrint(250, 432, 1, "OpenGl");

    glLoadIdentity();
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_LINE_STRIP);
        glVertex2d(639, 417);
        glVertex2d(0, 417);
        glVertex2d(0, 480);
        glVertex2d(639, 480);
        glVertex2d(639, 128);
    glEnd();
    glBegin(GL_LINE_STRIP);
        glVertex2d(0, 128);
        glVertex2d(639, 128);
        glVertex2d(639, 1);
        glVertex2d(0, 1);
        glVertex2d(0, 417);
    glEnd();

    glScissor(1, int(0.135416f * sheight), swidth - 2, int(0.597916f * sheight)); // Define scissor region
    glEnable(GL_SCISSOR_TEST);
    // Allocate memory for our extension string
    char* text = (char*)malloc(strlen((char*)glGetString(GL_EXTENSIONS)) + 1);
    strcpy(text, (char*)glGetString(GL_EXTENSIONS));  // Grab the extension list, store in text

    token = strtok(text, " ");
    while (token != NULL) {
        cnt++;
        if (cnt > maxtokens) {
            maxtokens = cnt;
        }

        glColor3f(0.5f, 1.0f, 0.5f);
        glPrint(0, 96 + (cnt * 32) - scroll, 0, "%i", cnt);    // Print the current extension number

        glColor3f(1.0f, 1.0f, 0.5f);
        glPrint(50, 96 + (cnt * 32) - scroll, 0, token);       // Print the current token extension name

        token = strtok(NULL, " ");                // Serch for the next token 
    }

    glDisable(GL_SCISSOR_TEST);
    free(text);

    glFlush();
    return true;
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
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
    KillFont();
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
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

            if (keys[VK_UP] && (scroll > 0)) {
                scroll -= 2;
            }
            if (keys[VK_DOWN] && (scroll < 32 * (maxtokens - 9))) {
                scroll += 2;
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
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/