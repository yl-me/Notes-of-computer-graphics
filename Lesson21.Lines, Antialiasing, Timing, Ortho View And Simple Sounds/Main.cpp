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

/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
bool vline[11][10];                 // Verticle lines
bool hline[10][11];                 // Horizontal lines
bool ap;                            // 'A' key press
bool filled;                        // Done filling in the grid
bool gameover;
bool anti = TRUE;                   // Antialiasing

int loop1;
int loop2;
int delay;                          // Enemy delay
int adjust = 3;                     // Speed adjustment for really slow video cards
int lives = 5;                      // Player lives
int level = 1;                      // Internal game level
int level2 = level;                 // Display game level
int stage = 1;                      // Game stage

struct object {                     // Player
    int fx, fy;                     // Fine Movement position
    int x, y;                       // Current player position
    float spin;                     // Spin direction
};
struct object player;
struct object enemy[9];
struct object hourglass;

struct {                                      // Timer information
    __int64 frequency;
    float resolution;
    unsigned long mm_timer_start;             // Multimedia timer start value
    unsigned long mm_timer_elapsed;           // Multimedia timer elspsed time
    bool performance_timer;                   // Using the performance timer ?
    __int64 performance_timer_start;          // Performance timer start value
    __int64 performance_timer_elapsed;        // Performance timer elspsed value
} timer;

int steps[6] = { 1, 2, 4, 5, 10, 20 };        // Stepping values for slow video adjustment

GLuint texture[2];                   // Font texture storage space
GLuint base;                         // Base display list for the font

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM); // Declaration for WndProc

void TimerInit(void)                                  // Initialize timer
{
    memset(&timer, 0, sizeof(timer));
    // Check to see if a performance counter is available
    // If one is available the timer frequency will be updated
    if (!QueryPerformanceFrequency((LARGE_INTEGER *)&timer.frequency)) {
        // No performance counter available
        timer.performance_timer = FALSE;
        timer.mm_timer_start = timeGetTime();            // To get current time
        timer.resolution = 1.0f / 1000.0f;
        timer.frequency = 1000;
        timer.mm_timer_elapsed = timer.mm_timer_start;
    }
    else {
        // Performance counter is available, use it instead of the multimedia timer
        // Get the current time and store it in performance_timer_start
        QueryPerformanceCounter((LARGE_INTEGER *)&timer.performance_timer_start);
        timer.performance_timer = TRUE;
        timer.resolution = (float)(((double)1.0f) / ((double)timer.frequency));
        timer.performance_timer_elapsed = timer.performance_timer_start;
    }
}

float TimerGetTime()                      // Get time in milliseconds
{
    __int64 time;

    if (timer.performance_timer) {
        QueryPerformanceCounter((LARGE_INTEGER *)&time);
        return ((float)(time - timer.performance_timer_start) * timer.resolution) * 1000.0f;
    }
    else {
        return ((float)(timeGetTime() - timer.mm_timer_start) * timer.resolution) * 1000.0f;
    }
}

void ResetObjects(void)                      // Reset player and enemies
{
    player.x = 0;
    player.y = 0;
    player.fx = 0;
    player.fy = 0;

    for (loop1 = 0; loop1 < (stage * level); ++loop1) {
        enemy[loop1].x = 5 + rand() % 6;
        enemy[loop1].y = rand() % 11;
        enemy[loop1].fx = enemy[loop1].x * 60;
        enemy[loop1].fy = enemy[loop1].y * 40;
    }
}

AUX_RGBImageRec* LoadBMP(char* Filename)
{
    FILE* File = NULL;
    
    if (!Filename) {
        return NULL;
    }

    File = fopen(Filename, "r");
    if (File) {
        fclose(File);
        return auxDIBImageLoad(Filename);
    }

    return NULL;
}

int LoadGLTextures()
{
    int Status = FALSE;

    AUX_RGBImageRec* TextureImage[2];
    
    memset(TextureImage, 0, sizeof(void*) * 2);

    if ((TextureImage[0] = LoadBMP("Font.bmp")) &&
        (TextureImage[1] = LoadBMP("Image.bmp")))
    {
        Status = TRUE;
        glGenTextures(2, &texture[0]);

        for (loop1 = 0; loop1 < 2; ++loop1) {
            glBindTexture(GL_TEXTURE_2D, texture[loop1]);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexImage2D(GL_TEXTURE_2D, 0, 3, TextureImage[loop1]->sizeX, TextureImage[loop1]->sizeY,
                0, GL_RGB, GL_UNSIGNED_BYTE, TextureImage[loop1]->data);
        }
    }

    for (loop1 = 0; loop1 < 2; ++loop1) {
        if (TextureImage[loop1]) {
            if (TextureImage[loop1]->data) {
                free(TextureImage[loop1]->data);
            }
            free(TextureImage[loop1]);
        }
    }
    return Status;
}

GLvoid BuildFont(GLvoid)
{
    base = glGenLists(256);
    glBindTexture(GL_TEXTURE_2D, texture[0]);
    for (loop1 = 0; loop1 < 256; ++loop1) {
        float cx = float(loop1 % 16) / 16.0f;                        // X position of current character
        float cy = float(loop1 / 16) / 16.0f;                        // Y position of current character

        glNewList(base + loop1, GL_COMPILE);
            glBegin(GL_QUADS);
                glTexCoord2f(cx, 1.0f - cy - 0.0625f);               // Bottom left
                glVertex2d(0, 16);
                glTexCoord2f(cx + 0.0625f, 1.0f - cy - 0.0625f);     // Bottom right
                glVertex2d(16, 16);
                glTexCoord2f(cx + 0.0625f, 1.0f - cy);               // Top right
                glVertex2d(16, 0);
                glTexCoord2f(cx, 1.0f - cy);                         // Top left
                glVertex2d(0, 0);
            glEnd();
            glTranslated(15, 0, 0);
        glEndList();
    }
}

GLvoid KillFont(GLvoid)
{
    glDeleteLists(base, 256);
}

GLvoid glPrint(GLint x, GLint y, int set, const char* fmt, ...)
{
    char text[256];
    va_list ap;                             // Pointer to listof arguments

    if (fmt == NULL) {
        return;
    }

    va_start(ap, fmt);                      // Parses the string for variables
        vsprintf(text, fmt, ap);            // And converts symbols ot actual numbers
    va_end(ap);                             // Results are stoed in text

    if (set > 1) {                          // Did user choose an invalid character set
        set = 1;
    }
    glEnable(GL_TEXTURE_2D);
    glLoadIdentity();
    glTranslated(x, y, 0);                  // Position the text (0, 0, -Bottom left)
    glListBase(base - 32 + (128 * set));    // Choose the font set (0 or 1)

    if (set == 0) {
        glScalef(1.5f, 2.0f, 1.0f);         // Enlarge font width and height
    }
    glCallLists(strlen(text), GL_UNSIGNED_BYTE, text);    // Write the text to the screen
    glDisable(GL_TEXTURE_2D);
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
    //gluPerspective(45.0f, (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);
    glOrtho(0.0f, width, height, 0.0f, -1.0f, 1.0f);  // Create orhto 640X480 view (0, 0, at the top)

    glMatrixMode(GL_MODELVIEW);                       // Seclet the modelview matrix
    glLoadIdentity();                                 // Reset the modelview matrix
}

int InitGL(GLvoid)                                    // All setup for OpenGL goes here
{
    /*
     *  Smooth shading blends colors nicely across a polygon, and smoothes out lighting.
     */

    if (!LoadGLTextures()) {
        return FALSE;
    }
    BuildFont();

    glShadeModel(GL_SMOOTH);                          // Enables smooth shading
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);             // Black background

    glClearDepth(1.0f);                               // Depth buffer setup

    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);           // Set line antialiasing
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    return TRUE;
}

/*
 *  For now all we will do is clear the screen to the color we previously decided on,
 *  clear the depth buffer and reset the scene. We wont draw anything yet.
 */
int DrawGLScene(GLvoid)                                  // Here's where we do all the drawing
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);  // Clear the screen and the depth buffer
    glBindTexture(GL_TEXTURE_2D, texture[0]);

    glColor3f(1.0f, 0.5f, 1.0f);                         // Set color to purple
    glPrint(207, 24, 0, "GRID CRAZY");
    glColor3f(1.0f, 1.0f, 0.0f);
    glPrint(20, 20, 1, "Level:%2i", level2);             // Write actual level stats
    glPrint(20, 40, 1, "Stage:2%i", stage);              // Write stage stats

    if (gameover) {
        glColor3ub(rand() % 255, rand() % 255, rand() % 255);    // Pick A random color
        glPrint(456, 20, 1, "GAME  OVER");
        glPrint(456, 35, 1, "PRESS SPACE");
    }

    for (loop1 = 0; loop1 < lives - 1; ++loop1) {                 // Draw liver counter
        glLoadIdentity();
        glTranslatef(490 + (loop1 * 40.0f), 40.0f, 0.0f);         // Move to the right of our title text
        glRotatef(-player.spin, 0.0f, 0.0f, 1.0f);                // Rotate counter clockwise
        glColor3f(0.0f, 1.0f, 0.0f);                              // Light green
        glBegin(GL_LINES);                                        // Start drawing our player using lines
            glVertex2d(-5, -5);
            glVertex2d(5, 5);
            glVertex2d(5, -5);
            glVertex2d(-5, 5);
        glEnd();

        glRotatef(-player.spin * 0.5f, 0.0f, 0.0f, 1.0f);
        glColor3f(0.0f, 0.75f, 0.0f);                             // Dark green
        glBegin(GL_LINES);                                        // Start drawing our player using lines
            glVertex2d(-7, 0);
            glVertex2d(7, 0);
            glVertex2d(0, -7);
            glVertex2d(0, 7);
        glEnd();
    }

    filled = TRUE;                                       // Set filled to true before testing
    glLineWidth(1.0f);                                   // Set line width for celld to 2.0f
    glDisable(GL_LINE_SMOOTH);                           // Disable antialiasing
    glLoadIdentity();
    for (loop1 = 0; loop1 < 11; ++loop1) {               // Left to right
        for (loop2 = 0; loop2 < 11; ++loop2) {           // Top to bottom
            glColor3f(0.0f, 0.5f, 1.0f);
            if (hline[loop1][loop2]) {                   // Has the horizontal line been traced
                glColor3f(1.0f, 1.0f, 1.0f);
            }
            if (loop1 < 10) {                            // Don't draw to far right
                if (!hline[loop1][loop2]) {              // If a horizontal line isn't filled
                    filled = FALSE;
                }
                glBegin(GL_LINES);                       // Start drwaing horizontal cell borders
                    glVertex2d(20 + (loop1 * 60), 70 + (loop2 * 40));
                    glVertex2d(80 + (loop1 * 60), 70 + (loop2 * 40));
                glEnd();
            }
            glColor3f(0.0f, 0.5f, 1.0f);
            if (vline[loop1][loop2]) {                   // Has the horizontal line been trace
                glColor3f(1.0f, 1.0f, 1.0f);
            }
            if (loop2 < 10) {
                if (!vline[loop1][loop2]) {              // If a verticle line isn't filled
                    filled = FALSE;
                }
                glBegin(GL_LINES);                       // Start drwaing verticle cell borders
                    glVertex2d(20 + (loop1 * 60), 70 + (loop2 * 40));
                    glVertex2d(20 + (loop1 * 60), 110 + (loop2 * 40));
                glEnd();
            }
            glEnable(GL_TEXTURE_2D);
            glColor3f(1.0f, 1.0f, 1.0f);
            glBindTexture(GL_TEXTURE_2D, texture[1]);
            if ((loop1 < 10) && (loop2 < 10)) {           // If in bounds, fill in traced boxes
                if (hline[loop1][loop2] && hline[loop1][loop2 + 1] && 
                    vline[loop1][loop2] && vline[loop1 + 1][loop2])
                {
                    glBegin(GL_QUADS);
                        glTexCoord2f(float(loop1 / 10.0f) + 0.1f, 1.0f - (float(loop2 / 10.0f)));
                        glVertex2d(20 + (loop1 * 60) + 59, (70 + loop2 * 40 + 1));   // Top right
                        glTexCoord2f(float(loop1 / 10.0f), 1.0f - (float(loop2 / 10.0f)));
                        glVertex2d(20 + (loop1 * 60) + 1, (70 + loop2 * 40 + 1));   // Top left
                        glTexCoord2f(float(loop1 / 10.0f), 1.0f - (float(loop2 / 10.0f) + 0.1f));
                        glVertex2d(20 + (loop1 * 60) + 1, (70 + loop2 * 40 + 39));   // Botttom left
                        glTexCoord2f(float(loop1 / 10.0f) + 0.1f, 1.0f - (float(loop2 / 10.0f) + 0.1f));
                        glVertex2d(20 + (loop1 * 60) + 59, (70 + loop2 * 40 + 39));   // Bottom right
                    glEnd();
                }
            }
            glDisable(GL_TEXTURE_2D);
        }    
    }
    glLineWidth(1.0f);

    if (anti) {
        glEnable(GL_LINE_SMOOTH);                         // Enable antialiasing
    }

    if (hourglass.fx == 1) {
        glLoadIdentity();
        glTranslatef(20.0f + (hourglass.x * 60), 70.0f + (hourglass.y * 40), 0.0f);
        glRotatef(hourglass.spin, 0.0f, 0.0f, 1.0f);
        glColor3ub(rand() % 255, rand() % 255, rand() % 255);
        glBegin(GL_LINES);                                 // Hourglass
            glVertex2d(-5, -5);
            glVertex2d(5, 5);
            glVertex2d(5, -5);
            glVertex2d(-5, 5);
            glVertex2d(-5, 5);
            glVertex2d(5, 5);
            glVertex2d(-5, -5);
            glVertex2d(5, -5);
        glEnd();
    }

    glLoadIdentity();
    glTranslatef(player.fx + 20.0f, player.fy + 70.0f, 0.0f);     // Player
    glRotatef(player.spin, 0.0f, 0.0f, 1.0f);
    glColor3f(0.0f, 1.0f, 0.0f);
    glBegin(GL_LINES);                           // Player using lines
        glVertex2d(-5, -5);
        glVertex2d(5, 5);
        glVertex2d(5, -5);
        glVertex2d(-5, 5);
    glEnd();

    glRotatef(player.spin * 0.5f, 0.0f, 0.0f, 1.0f);
    glColor3f(0.0f, 0.75f, 0.0f);
    glBegin(GL_LINES);                           // Player using lines
        glVertex2d(-7, 0);
        glVertex2d(7, 0);
        glVertex2d(0, -7);
        glVertex2d(0, 7);
    glEnd();

    for (loop1 = 0; loop1 < (stage * level); ++loop1) {             // Enemies
        glLoadIdentity();
        glTranslatef(enemy[loop1].fx + 20.0f, enemy[loop1].fy + 70.0f, 0.0f);
        glColor3f(1.0f, 0.5f, 0.5f);                     // Pink
        glBegin(GL_LINES);
            glVertex2d(0, -7);
            glVertex2d(-7, 0);
            glVertex2d(-7, 0);
            glVertex2d(0, 7);
            glVertex2d(0, 7);
            glVertex2d(7, 0);
            glVertex2d(7, 0);
            glVertex2d(0, -7);
        glEnd();

        glRotatef(enemy[loop1].spin, 0.0f, 0.0f, 1.0f);
        glColor3f(1.0f, 0.0f, 0.0f);
        glBegin(GL_LINES);
            glVertex2d(-7, -7);
            glVertex2d(7, 7);
            glVertex2d(-7, 7);
            glVertex2d(7, -7);
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
    ResetObjects();                                       // Set player / enemy starting position
    TimerInit();                                          // Initialize the timer

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

            float start = TimerGetTime();                 // Grab timer value before we draw

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
            // Waste Cycles On Fast Systems
            while (TimerGetTime() < start + float(steps[adjust] * 2.0f)) {}

            if (keys['A'] && !ap) {
                ap = TRUE;
                anti = !anti;
            }
            if (!keys['A']) {
                ap = FALSE;
            }
            if (!gameover && active) {                    // If game isn't over and active move objects
                for (loop1 = 0; loop1 < (stage * level); ++loop1) {
                    if ((enemy[loop1].x < player.x) && (enemy[loop1].fy == enemy[loop1].y * 40)) {
                        enemy[loop1].x++;                 // Move the enemy right
                    }
                    if ((enemy[loop1].x > player.x) && (enemy[loop1].fy == enemy[loop1].y * 40)) {
                        enemy[loop1].x--;                 // Move the enemy left
                    }
                    if ((enemy[loop1].y < player.y) && (enemy[loop1].fx == enemy[loop1].x * 60)) {
                        enemy[loop1].y++;                 // Move the enemy down
                    }
                    if ((enemy[loop1].y > player.y) && (enemy[loop1].fx == enemy[loop1].x * 60)) {
                        enemy[loop1].y--;                 // Move the enemy up
                    }

                    if (delay > (3 - level) && (hourglass.fx != 2)) {
                        delay = 0;
                        for (loop2 = 0; loop2 < (stage * level); ++loop2) {
                            if (enemy[loop2].fx < enemy[loop2].x * 60) {
                                enemy[loop2].fx += steps[adjust];
                                enemy[loop2].spin += steps[adjust];
                            }
                            if (enemy[loop2].fx > enemy[loop2].x * 60) {
                                enemy[loop2].fx -= steps[adjust];
                                enemy[loop2].spin -= steps[adjust];
                            }
                            if (enemy[loop2].fy < enemy[loop2].y * 40) {
                                enemy[loop2].fy += steps[adjust];
                                enemy[loop2].spin += steps[adjust];
                            }
                            if (enemy[loop2].fy > enemy[loop2].y * 40) {
                                enemy[loop2].fy -= steps[adjust];
                                enemy[loop2].spin -= steps[adjust];
                            }
                        }
                    }

                    if ((enemy[loop1].fx == player.fx) && (enemy[loop1].fy == player.fy)) {
                        lives--;
                        if (lives == 0) {
                            gameover = TRUE;
                        }
                        ResetObjects();
                        PlaySound("Die.wav", NULL, SND_SYNC);
                    }
                }

                if (keys[VK_RIGHT] && (player.x < 10) && (player.fx == player.x * 60) &&
                    (player.fy == player.y * 40))
                {
                    // Make current horizontal border as filled
                    hline[player.x][player.y] = TRUE;
                    player.x++;                                 // Move the player right
                }
                if (keys[VK_LEFT] && (player.x > 0) && (player.fx == player.x * 60) &&
                    (player.fy == player.y * 40))
                {
                    player.x--;
                    hline[player.x][player.y] = TRUE;
                }
                if (keys[VK_DOWN] && (player.y < 10) && (player.fx == player.x * 60) &&
                    (player.fy == player.y * 40))
                {
                    vline[player.x][player.y] = TRUE;
                    player.y++;
                }
                if (keys[VK_UP] && (player.y > 0) && (player.fx == player.x * 60) &&
                    (player.fy == player.y * 40))
                {
                    player.y--;
                    vline[player.x][player.y] = TRUE;
                }

                if (player.fx < player.x * 60) {
                    player.fx += steps[adjust];
                }
                if (player.fx > player.x * 60) {
                    player.fx -= steps[adjust];
                }
                if (player.fy < player.y * 40) {
                    player.fy += steps[adjust];
                }
                if (player.fy > player.y * 40) {
                    player.fy -= steps[adjust];
                }
            }
            else {
                if (keys[' ']) {
                    gameover = FALSE;
                    filled = TRUE;
                    level = 1;
                    level2 = 1;
                    stage = 0;
                    lives = 5;
                }
            }

            if (filled) {
                PlaySound("Complete.wav", NULL, SND_SYNC);
                stage++;
                if (stage > 3) {
                    stage = 1;
                    level++;
                    level2++;
                    if (level > 3) {
                        level = 3;
                        lives++;
                        if (lives > 5) {
                            lives = 5;
                        }
                    }
                }
                ResetObjects();
                for (loop1 = 0; loop1 < 11; ++loop1) {
                    for (loop2 = 0; loop2 < 11; ++loop2) {
                        if (loop1 < 10) {
                            hline[loop1][loop2] = FALSE;
                        }
                        if (loop2 < 10) {
                            vline[loop1][loop2] = FALSE;
                        }
                    }
                }
            }

            if ((player.fx == hourglass.x * 60) && (player.fy == hourglass.y * 40) &&
                (hourglass.fx == 1))
            {
                PlaySound("freeze.wav", NULL, SND_ASYNC | SND_LOOP);
                hourglass.fx = 2;
                hourglass.fy = 0;
            }

            player.spin += 0.5f * steps[adjust];
            if (player.spin > 360.0f) {
                player.spin -= 360.0f;
            }
            hourglass.spin -= 0.25f * steps[adjust];
            if (hourglass.spin < 0.0f) {
                hourglass.spin += 360.0f;
            }
            hourglass.fy += steps[adjust];
            if ((hourglass.fx == 0) && (hourglass.fy > 6000 / level)) {
                PlaySound("hourglass.wav", NULL, SND_ASYNC);
                hourglass.x = rand() % 10 + 1;
                hourglass.y = rand() % 11;
                hourglass.fx = 1;
                hourglass.fy = 0;
            }
            if ((hourglass.fx == 1) && (hourglass.fy > 6000 / level)) {
                hourglass.fx = 0;
                hourglass.fy = 0;
            }
            if ((hourglass.fx == 2) && (hourglass.fy > 500 + (500 * level))) {
                PlaySound(NULL, NULL, 0);
                hourglass.fx = 0;
                hourglass.fy = 0;
            }
            delay++;
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