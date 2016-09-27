/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
#include <windows.h>
#include <stdio.h>
#include <Vfw.h>
#include <gl/glew.h>
#include <GL\glut.h>
#include <GL\GLUAX.H>
#include "Previous.h"
#pragma comment(lib, "legacy_stdio_definitions.lib")
#pragma comment(lib, "vfw32.lib")


#ifndef CDS_FULLSCREEN
#define CDA_FULLSCREEN
#endif

GL_Window* g_window;
Keys* g_keys;

/*
*  Every OpenGL program is linked to a Rendering Context.
*  A Rendering Context is what links OpenGL calls to the Device Context.
*  In order for your program to draw to a Window you need to create a Device Context.
*  The DC connects the Window to the GDI (Graphics Device Interface).
*/

HGLRC     hRC = NULL;                      // Permanent rendering context
HDC       hdc = CreateCompatibleDC(0);     // Private GDI device context
HWND      hWnd = NULL;                     // Holds our window handle
HINSTANCE hInstance;                       // Holds the instance of the application

/*
*  It's important to make this global so that each procedure knows if
*  the program is running in fullscreen mode or not.
*/

bool keys[256];         // Array used for the keyboard routine
bool active = TRUE;     // Window active flag set to TRUE by default
bool fullscreen = TRUE; // Fullscreen flag set to fullscreen mode by default

float angle;            // Rotation
int next;               // Animtion
int frame = 0;          // Frame counter
int effect;             // Current effect
bool sp;                // 'space' pressed ?
bool env = TRUE;        // Environment mapping
bool ep;                // 'E' pressed ?
bool bg = TRUE;         // Background
bool bp;                // 'B' pressed ?

AVISTREAMINFO psi;      // Pointer to a structure containing stream info
PAVISTREAM pavi;        // Handle to an open stream
PGETFRAME pgf;          // Pointer to a GetFrame object
BITMAPINFOHEADER bmih;  // Header information for DrawDibDraw decoding

long lastframe;         // Last frame of the stream
int width;              // Video
int height;
char* pdata;            // Texture data
int mpf;                // Will hold rough millisecond per frame

GLUquadricObj* quadratic;
HDRAWDIB hdd;           // Handle for dib
HBITMAP hBitmap;        // Handle to a device dependant bitmap
unsigned char* data = 0;// Resized image

void flipIt(void* buffer)                             // Flips the red and blue bytes
{
    void* b = buffer;
    __asm {                                           // Assembler
        mov ecx, 256 * 256                            // Set up a counter 
        mov ebx, b                                    // Points ebx to data
        label :                                       // Label used for looping
        mov al, [ebx + 0]                             // Loads value at ebx into al
        mov ah, [ebx + 2]                             // Loads value at ebx+2 into ah
        mov [ebx + 2], al                             // Stores value in al at ebx+2
        mov [ebx + 0], ah                             // Stores value in ah at ebx

        add ebx, 3                                    // Moves through the data by 3 bytes
        dec ecx                                       // Decreases loop counter
        jnz label                                     // If not zero jump back to label
    }
}

void OpenAVI(LPCSTR szFile)
{
    TCHAR title[100];
    AVIFileInit();                                    // Opens the AVIFile library
    if (AVIStreamOpenFromFile(&pavi, szFile, streamtypeVIDEO, 0, OF_READ, NULL) != 0) {
        MessageBox(HWND_DESKTOP, "Failed to open the AVI stream", "ERROR", MB_OK | MB_ICONEXCLAMATION);
    }
    AVIStreamInfo(pavi, &psi, sizeof(psi));           // Reads infomation about the stream into psi
    width = psi.rcFrame.right - psi.rcFrame.left;     // Width is right side of frame minus left
    height = psi.rcFrame.bottom - psi.rcFrame.top;    // Height

    lastframe = AVIStreamLength(pavi);                // The last frame of the stream

    mpf = AVIStreamSampleToTime(pavi, lastframe) / lastframe;  // Calculate rough milliseconds per frame

    bmih.biSize = sizeof(BITMAPINFOHEADER); //  The dimensions and color format of a device-independent bitmap 
    bmih.biPlanes = 1;
    bmih.biBitCount = 24;                             // Bits format (24 bit, 3 bytes)
    bmih.biWidth = 256;
    bmih.biHeight = 256;
    bmih.biCompression = BI_RGB;                      // Requested mode

    hBitmap = CreateDIBSection(hdc, (BITMAPINFO*)(&bmih), DIB_RGB_COLORS, (void**)(&data), NULL, NULL);
    SelectObject(hdc, hBitmap);                       // Select hBitmap    into device context

    pgf = AVIStreamGetFrameOpen(pavi, NULL);          // Create the PGETFRAME using request mode
    if (pgf == NULL) {
        MessageBox(HWND_DESKTOP, "Failed to open the AVI frame", "ERROR", MB_OK | MB_ICONEXCLAMATION);
    }
    // Information for the title bar
    wsprintf(title, "AVI player: width: %d, height: %d, frames: %d", width, height, lastframe);
    SetWindowText(g_window->hWnd, title);             // Modify the title bar
}

void GrabAVIFrame(int frame)                          // Grabs a frame from the stream
{
    LPBITMAPINFOHEADER lpbi;                          // Holds the bitmap header information
    lpbi = (LPBITMAPINFOHEADER)AVIStreamGetFrame(pgf, frame); // Grabs data from the AVI stream
    // Pointer to data returned by AVIStreamGetFrame
    pdata = (char*)lpbi + lpbi->biSize + lpbi->biClrUsed * sizeof(RGBQUAD);
    // Convert data to requested bitmap format
    DrawDibDraw(hdd, hdc, 0, 0, 256, 256, lpbi, pdata, 0, 0, width, height, 0);

    flipIt(data);                                     // Swap the red and blue bytes (GL compatability
    // Update the texture
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 256, 256, GL_RGB, GL_UNSIGNED_BYTE, data);
}

void closeAVI(void)                                   // Properly closes the AVI file
{
    DeleteObject(hBitmap);                            // Delete the device dependant bitmap object
    DrawDibClose(hdd);                                // Closes the    DrawDib device context
    AVIStreamGetFrameClose(pgf);                      // Deallocates the GetFrame resource
    AVIStreamRelease(pavi);                           // Release the stream
    AVIFileExit();                                    // Release the file
}

BOOL Initialize(GL_Window* window, Keys* keys)
{
    g_window = window;
    g_keys = keys;

    angle = 0.0f;
    hdd = DrawDibOpen();                              // Grabs a device context for Dib
    glClearColor(0.0f, 0.0f, 0.0f, 0.5f);
    glClearDepth(1.0f);
    glDepthFunc(GL_LEQUAL);                           // The type of depth testing (less or equal)
    glEnable(GL_DEPTH_TEST);
    glShadeModel(GL_SMOOTH);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST); // Set perspective calculations to most accurate
    
    quadratic = gluNewQuadric();                      // Create a pointer to the quadric object
    gluQuadricNormals(quadratic, GLU_SMOOTH);         // Create smooth noraml
    gluQuadricTexture(quadratic, GL_TRUE);            // Create texture coords

    glEnable(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);// Set the texture generation mode for S to sphere mapping
    glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);// Set the texture generation mode for T to sphere mapping
    OpenAVI("Data/face2.avi");
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 256, 256, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    return TRUE;
}

void Deinitialize(void)
{
    closeAVI();
}

void Update(DWORD milliseconds)
{
    ShowCursor(TRUE);
    if (g_keys->keyDown[VK_ESCAPE] == TRUE) {
        TerminateApplication(g_window);
    }
    if (g_keys->keyDown[VK_F1] == TRUE) {
        ToggleFullscreen(g_window);
    }
    if ((g_keys->keyDown[' ']) && !sp) {
        sp = TRUE;
        effect++;
        if (effect >= 3)
            effect = 0;
    }
    if (!g_keys->keyDown[' '])
        sp = FALSE;
    
    if ((g_keys->keyDown['B']) && !bp) {
        bp = TRUE;
        bg = !bg;
    }
    if (!g_keys->keyDown['B'])
        bp = FALSE;

    if ((g_keys->keyDown['E']) && !ep) {
        ep = TRUE;
        env = !env;
    }
    if (!g_keys->keyDown['E'])
        ep = FALSE;

    angle += (float)(milliseconds) / 60.0f;           // Update angle based on the timer
    next += milliseconds;                             // Increase next based on the timer
    frame = next / mpf;                               // Calculate the current frame

    if (frame >= lastframe) {
        frame = 0;
        next = 0;                                     // Reset the animation timer
    }
}

void Draw(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    GrabAVIFrame(frame);

    if (bg) {                                        // Is background visiable?
        glLoadIdentity();
        glBegin(GL_QUADS);
            glTexCoord2f(1.0f, 1.0f); glVertex3f(11.0f, 8.3f, -20.0f);
            glTexCoord2f(0.0f, 1.0f); glVertex3f(-11.0f, 8.3f, -20.0f);
            glTexCoord2f(0.0f, 0.0f); glVertex3f(-11.0f, -8.3f, -20.0f);
            glTexCoord2f(1.0f, 0.0f); glVertex3f(11.0f, -8.3f, -20.0f);
        glEnd();
    }
    glLoadIdentity();
    glTranslatef(0.0f, 0.0f, -10.0f);
    if (env) {                                        // Is environment mapping on?
        glEnable(GL_TEXTURE_GEN_S);
        glEnable(GL_TEXTURE_GEN_T);
    }
    glRotatef(angle * 2.3f, 1.0f, 0.0f, 0.0f);
    glRotatef(angle * 1.8f, 0.0f, 1.0f, 0.0f);
    glTranslatef(0.0f, 0.0f, 2.0f);

    switch (effect) {
    case 0:
        glRotatef(angle * 1.3f, 1.0f, 0.0f, 0.0f);
        glRotatef(angle * 1.1f, 0.0f, 1.0f, 0.0f);
        glRotatef(angle * 1.2f, 0.0f, 0.0f, 1.0f);
        glBegin(GL_QUADS);
            // Front Face
            glNormal3f(0.0f, 0.0f, 0.5f);
            glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f, 1.0f);
            glTexCoord2f(1.0f, 0.0f); glVertex3f(1.0f, -1.0f, 1.0f);
            glTexCoord2f(1.0f, 1.0f); glVertex3f(1.0f, 1.0f, 1.0f);
            glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f, 1.0f, 1.0f);
            // Back Face
            glNormal3f(0.0f, 0.0f, -0.5f);
            glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f, -1.0f);
            glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f, 1.0f, -1.0f);
            glTexCoord2f(0.0f, 1.0f); glVertex3f(1.0f, 1.0f, -1.0f);
            glTexCoord2f(0.0f, 0.0f); glVertex3f(1.0f, -1.0f, -1.0f);
            // Top Face
            glNormal3f(0.0f, 0.5f, 0.0f);
            glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f, 1.0f, -1.0f);
            glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, 1.0f, 1.0f);
            glTexCoord2f(1.0f, 0.0f); glVertex3f(1.0f, 1.0f, 1.0f);
            glTexCoord2f(1.0f, 1.0f); glVertex3f(1.0f, 1.0f, -1.0f);
            // Bottom Face
            glNormal3f(0.0f, -0.5f, 0.0f);
            glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f, -1.0f, -1.0f);
            glTexCoord2f(0.0f, 1.0f); glVertex3f(1.0f, -1.0f, -1.0f);
            glTexCoord2f(0.0f, 0.0f); glVertex3f(1.0f, -1.0f, 1.0f);
            glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f, 1.0f);
            // Right Face
            glNormal3f(0.5f, 0.0f, 0.0f);
            glTexCoord2f(1.0f, 0.0f); glVertex3f(1.0f, -1.0f, -1.0f);
            glTexCoord2f(1.0f, 1.0f); glVertex3f(1.0f, 1.0f, -1.0f);
            glTexCoord2f(0.0f, 1.0f); glVertex3f(1.0f, 1.0f, 1.0f);
            glTexCoord2f(0.0f, 0.0f); glVertex3f(1.0f, -1.0f, 1.0f);
            // Left Face
            glNormal3f(-0.5f, 0.0f, 0.0f);
            glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f, -1.0f);
            glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f, 1.0f);
            glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f, 1.0f, 1.0f);
            glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f, 1.0f, -1.0f);
        glEnd();
        break;
    case 1:
        glRotatef(angle * 1.3f, 1.0f, 0.0f, 0.0f);
        glRotatef(angle * 1.1f, 0.0f, 1.0f, 0.0f);
        glRotatef(angle * 1.2f, 0.0f, 0.0f, 1.0f);
        gluSphere(quadratic, 1.3f, 20, 20);                   // Sphere
        break;

    case 2:
        glRotatef(angle * 1.3f, 1.0f, 0.0f, 0.0f);
        glRotatef(angle * 1.1f, 0.0f, 1.0f, 0.0f);
        glRotatef(angle * 1.2f, 0.0f, 0.0f, 1.0f);
        gluCylinder(quadratic, 1.0f, 1.0f, 3.0f, 32, 32);
        break;
    }
    if (env) {
        glDisable(GL_TEXTURE_GEN_S);
        glDisable(GL_TEXTURE_GEN_T);
    }
    glFlush();
}
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/