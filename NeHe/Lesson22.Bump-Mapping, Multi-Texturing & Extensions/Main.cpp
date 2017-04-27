/*****************************************************************************************************************************************/
/*****************************************************************************************************************************************/
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

//The GLfloat MAX_EMBOSS specifies the "strength" of the Bump Mapping-Effect
// but reduce visual quality to the same extent by leaving so-called "artefacts" at the edges of the surfaces.
#define MAX_EMBOSS (GLfloat)0.01f      // Maximum emboss-translate. Increase to get higher immersion
#define __ARB_ENABLE true               // Used to disable ARB extensions entirely
// #define EXT_INFO                    // Uncomment to see your extensions at start_up?
#define MAX_EXTENSION_SPACE 10240      // Character for extension-strings
#define MAX_EXTENSION_LENGTH 256       // Maxiunm character in one extension-string
bool multitextureSupported = false;    // Flag indicating whether multitexture is supported
bool useMultitexture = true;           // Use it if it is supported
GLint maxTexelUnits = 1;                // Number of texel-pipelines.

// GDI-context handles
PFNGLMULTITEXCOORD1FARBPROC glMultiTexCoord1fARB = NULL;
PFNGLMULTITEXCOORD2FARBPROC    glMultiTexCoord2fARB = NULL;
PFNGLMULTITEXCOORD3FARBPROC glMultiTexCoord3fARB = NULL;
PFNGLMULTITEXCOORD4FARBPROC    glMultiTexCoord4fARB = NULL;
PFNGLACTIVETEXTUREARBPROC   glActiveTextureARB = NULL;
PFNGLCLIENTACTIVETEXTUREARBPROC glClientActiveTextureARB = NULL;

bool emboss = false;
bool bumps = true;

GLfloat xrot;
GLfloat yrot;
GLfloat xspeed;
GLfloat yspeed;
GLfloat z = -5.0f;

GLuint filter = 1;                                    // Which filter to use
GLuint texture[3];
GLuint bump[3];                                       // Bumpmappings
GLuint invbump[3];                                    // Inverted bumpmaps
GLuint glLogo;                                        // Handle for Opengl-Logo
GLuint multiLogo;                                     // Handle for multitexture-enabled-logo
GLfloat LightAmbient[] = { 0.2f, 0.2f, 0.2f };
GLfloat LightDiffuse[] = { 1.0f, 1.0f, 1.0f };
GLfloat LightPosition[] = { 0.0f, 0.0f, 2.0f };
GLfloat Gray[] = { 0.5f, 0.5f, 0.5f, 1.0f };

// Data contains the faces of the cube in format 2xTexCoord, 3xVertex.
// Note That the tesselation of the cube is only absolute minimum.
GLfloat data[] = {
    // Front face
    0.0f, 0.0f,     -1.0f, -1.0f, +1.0f,
    1.0f, 0.0f,     +1.0f, -1.0f, +1.0f,
    1.0f, 1.0f,     +1.0f, +1.0f, +1.0f,
    0.0f, 1.0f,     -1.0f, +1.0f, +1.0f,
    // Back face
    1.0f, 0.0f,     -1.0f, -1.0f, -1.0f,
    1.0f, 1.0f,     -1.0f, +1.0f, -1.0f,
    0.0f, 1.0f,     +1.0f, +1.0f, -1.0f,
    0.0f, 0.0f,     +1.0f, -1.0f, -1.0f,
    // Top Face
    0.0f, 1.0f,     -1.0f, +1.0f, -1.0f,
    0.0f, 0.0f,     -1.0f, +1.0f, +1.0f,
    1.0f, 0.0f,     +1.0f, +1.0f, +1.0f,
    1.0f, 1.0f,     +1.0f, +1.0f, -1.0f,
    // Bottom Face
    1.0f, 1.0f,     -1.0f, -1.0f, -1.0f,
    0.0f, 1.0f,     +1.0f, -1.0f, -1.0f,
    0.0f, 0.0f,     +1.0f, -1.0f, +1.0f,
    1.0f, 0.0f,     -1.0f, -1.0f, +1.0f,
    // Right Face
    1.0f, 0.0f,     +1.0f, -1.0f, -1.0f,
    1.0f, 1.0f,     +1.0f, +1.0f, -1.0f,
    0.0f, 1.0f,     +1.0f, +1.0f, +1.0f,
    0.0f, 0.0f,     +1.0f, -1.0f, +1.0f,
    // Left Face
    0.0f, 0.0f,     -1.0f, -1.0f, -1.0f,
    1.0f, 0.0f,     -1.0f, -1.0f, +1.0f,
    1.0f, 1.0f,     -1.0f, +1.0f, +1.0f,
    0.0f, 1.0f,     -1.0f, +1.0f, -1.0f
};

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM); // Declaration for WndProc

bool isInString(char* string, const char* search)     // String search
{
    int pos = 0;
    int maxpos = strlen(search) - 1;
    int len = strlen(string);
    char* other;
    for (int i = 0; i < len; ++i) {
        if ((i == 0) || ((i > 1) && string[i - 1] == '\n')) {    // New extension begins here
            other = &string[i];
            pos = 0;                                   // Begin new search
            while (string[i] != '\n') {                // Search whole extension-string
                if (string[i] == search[pos]) pos++;   // Next position
                if ((pos > maxpos) && string[i + 1] == '\n')
                    return true;
                i++;
            }
        }
    }
    return false;
}

bool initMultitexture(void)
{
    char* extensions;
    extensions = _strdup((char*)glGetString(GL_EXTENSIONS));    // Fetch extension string
    int len = strlen(extensions);
    for (int i = 0; i < len; ++i) {
        if (extensions[i] == ' ') {
            extensions[i] = '\n';
        }
    }
#ifdef EXT_INFO
            MessageBox(hWnd, extensions, "supported GL extensions", MB_OK | MB_ICONINFORMATION);
#endif
    if (isInString(extensions, "GL_ARB_multitexture") &&     // Is multitexturing suported
        __ARB_ENABLE &&                                       // Override flag
        isInString(extensions, "GL_EXT_texture_env_combine"))// Texture-environment-combining supported
    {
        glGetIntegerv(GL_MAX_TEXTURE_UNITS_ARB, &maxTexelUnits);
        glMultiTexCoord1fARB = (PFNGLMULTITEXCOORD1FARBPROC)wglGetProcAddress("glMultiTexCoord1fARB");
        glMultiTexCoord2fARB = (PFNGLMULTITEXCOORD2FARBPROC)wglGetProcAddress("glMultiTexCoord2fARB");
        glMultiTexCoord3fARB = (PFNGLMULTITEXCOORD3FARBPROC)wglGetProcAddress("glMultiTexCoord3fARB");
        glMultiTexCoord4fARB = (PFNGLMULTITEXCOORD4FARBPROC)wglGetProcAddress("glMultiTexCoord4fARB");
        glActiveTextureARB = (PFNGLACTIVETEXTUREARBPROC)wglGetProcAddress("glActiveTextureARB");
        glClientActiveTextureARB = (PFNGLCLIENTACTIVETEXTUREARBPROC)
            wglGetProcAddress("glClientActiveTextureARB");
#ifdef EXT_INFO
        MessageBox(hWnd, "The GL_ARB_multitexture extension will be used.", "feature supported!",
            MB_OK | MB_ICONINFORMATION);
#endif
        return true;
    }
    useMultitexture = false;
    return false;
}

void initLights(void)
{
    glLightfv(GL_LIGHT1, GL_AMBIENT, LightAmbient);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, LightDiffuse);
    glLightfv(GL_LIGHT1, GL_POSITION, LightPosition);
    glEnable(GL_LIGHT1);
}

int LoadGLTextures(void)
{
    bool status = true;
    AUX_RGBImageRec* Image = NULL;                  // Create storage space for the texture
    char* alpha = NULL;

    // Load the title-bitmap for base-texture
    if (Image = auxDIBImageLoad("Data/Base.bmp")) {
        glGenTextures(3, texture);

        // Create nearest filtered texture
        glBindTexture(GL_TEXTURE_2D, texture[0]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, Image->sizeX, Image->sizeY, 0, GL_RGB, 
            GL_UNSIGNED_BYTE, Image->data);
        // Create linear filtered texture
        glBindTexture(GL_TEXTURE_2D, texture[1]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, Image->sizeX, Image->sizeY, 0, GL_RGB, 
            GL_UNSIGNED_BYTE, Image->data);
        // Create mipmapped texture
        glBindTexture(GL_TEXTURE_2D, texture[2]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
        gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB8, Image->sizeX, Image->sizeY, GL_RGB, 
            GL_UNSIGNED_BYTE, Image->data);
    }
    else {
        status = false;
    }

    if (Image) {
        if (Image->data) {
            delete Image->data;
        }
        delete Image;
        Image = NULL;
    }

    // Load the bumpmaps
    if (Image = auxDIBImageLoad("Data/Bump.bmp")) {
        glPixelTransferf(GL_RED_SCALE, 0.5f);             // Scale RGB by 50%
        glPixelTransferf(GL_GREEN_SCALE, 0.5f);
        glPixelTransferf(GL_BLUE_SCALE, 0.5f);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, Gray);
        glGenTextures(3, bump);

        // Create nearest filtered texture
        glBindTexture(GL_TEXTURE_2D, bump[0]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, Image->sizeX, Image->sizeY, 0, GL_RGB,
            GL_UNSIGNED_BYTE, Image->data);
        // Create linear filtered texture
        glBindTexture(GL_TEXTURE_2D, bump[1]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, Image->sizeX, Image->sizeY, 0, GL_RGB,
            GL_UNSIGNED_BYTE, Image->data);
        // Create mipmapped texture
        glBindTexture(GL_TEXTURE_2D, bump[2]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
        gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB8, Image->sizeX, Image->sizeY, GL_RGB,
            GL_UNSIGNED_BYTE, Image->data);

        for (int i = 0; i < 3 * Image->sizeX * Image->sizeY; ++i) {
            Image->data[i] = 255 - Image->data[i];
        }
        glGenTextures(3, invbump);
        // Create nearest filtered texture
        glBindTexture(GL_TEXTURE_2D, invbump[0]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, Image->sizeX, Image->sizeY, 0, GL_RGB,
            GL_UNSIGNED_BYTE, Image->data);
        // Create linear filtered texture
        glBindTexture(GL_TEXTURE_2D, invbump[1]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, Image->sizeX, Image->sizeY, 0, GL_RGB,
            GL_UNSIGNED_BYTE, Image->data);
        // Create mipmapped texture
        glBindTexture(GL_TEXTURE_2D, invbump[2]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
        gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB8, Image->sizeX, Image->sizeY, GL_RGB,
            GL_UNSIGNED_BYTE, Image->data);

        glPixelTransferf(GL_RED_SCALE, 1.0f);                // Scale RGB back to 100% again        
        glPixelTransferf(GL_GREEN_SCALE, 1.0f);
        glPixelTransferf(GL_BLUE_SCALE, 1.0f);
    }
    else {
        status = false;
    }
    if (Image) {
        if (Image->data) {
            delete Image->data;
        }
        delete Image;
        Image = NULL;
    }

    if (Image = auxDIBImageLoad("Data/OpenGL_ALPHA.bmp")) {
        alpha = new char[4 * Image->sizeX * Image->sizeY];
        // Create memory for RGBAB8-Texture
        for (int a = 0; a < Image->sizeX * Image->sizeY; ++a) {
            alpha[4 * a + 3] = Image->data[a * 3];               // Pick only red value as alpha
        }
        if (!(Image = auxDIBImageLoad("Data/OpenGL.bmp"))) {
            status = false;
        }
        for (int a = 0; a < Image->sizeX * Image->sizeY; ++a) {
            alpha[4 * a] = Image->data[a * 3];                   // R
            alpha[4 * a + 1] = Image->data[a * 3 + 1];           // G
            alpha[4 * a + 2] = Image->data[a * 3 + 2];           // B
        }
        glGenTextures(1, &glLogo);
        // RGBA8-Texture
        glBindTexture(GL_TEXTURE_2D, glLogo);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, Image->sizeX, Image->sizeY, 0, GL_RGBA,
            GL_UNSIGNED_BYTE, alpha);
        delete alpha;
    }
    else {
        status = false;
    }
    if (Image) {
        if (Image->data)
            delete Image->data;
        delete Image;
        Image = NULL;
    }

    if (Image = auxDIBImageLoad("Data/multi_on_alpha.bmp")) {
        // Create memory for RGBAB8-Texture
        alpha = new char[4 * Image->sizeX *Image->sizeY];
        for (int a = 0; a < Image->sizeX * Image->sizeY; ++a) {
            alpha[4 * a + 3] = Image->data[a * 3];               // Pick only red value as alpha
        }
        if (!(Image = auxDIBImageLoad("Data/multi_on.bmp"))) {
            status = false;
        }
        for (int a = 0; a < Image->sizeX * Image->sizeY; ++a) {
            alpha[4 * a] = Image->data[a * 3];                   // R
            alpha[4 * a + 1] = Image->data[a * 3 + 1];           // G
            alpha[4 * a + 2] = Image->data[a * 3 + 2];           // B
        }
        glGenTextures(1, &multiLogo);

        // RGBA8-Texture
        glBindTexture(GL_TEXTURE_2D, multiLogo);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, Image->sizeX, Image->sizeY, 0, GL_RGBA,
            GL_UNSIGNED_BYTE, alpha);
        delete alpha;
    }
    else {
        status = false;
    }
    if (Image) {
        if (Image->data)
            delete Image->data;
        delete Image;
        Image = NULL;
    }
    return status;
}

void doCube(void)
{
    int i;
    glBegin(GL_QUADS);
        // Front face
        glNormal3f(0.0f, 0.0f, 1.0f);
        for (i = 0; i < 4; ++i) {
            glTexCoord2f(data[5 * i], data[5 * i + 1]);
            glVertex3f(data[5 * i + 2], data[5 * i + 3], data[5 * i + 4]);
        }
        // Back face
        glNormal3f(0.0f, 0.0f, -1.0f);
        for (i = 4; i < 8; ++i) {
            glTexCoord2f(data[5 * i], data[5 * i + 1]);
            glVertex3f(data[5 * i + 2], data[5 * i + 3], data[5 * i + 4]);
        }
        // Top face
        glNormal3f(0.0f, 1.0f, 0.0f);
        for (i = 8; i < 12; ++i) {
            glTexCoord2f(data[5 * i], data[5 * i + 1]);
            glVertex3f(data[5 * i + 2], data[5 * i + 3], data[5 * i + 4]);
        }
        // Bottom face
        glNormal3f(0.0f,-1.0f, 0.0f);
        for (i = 12; i < 16; ++i) {
            glTexCoord2f(data[5 * i], data[5 * i + 1]);
            glVertex3f(data[5 * i + 2], data[5 * i + 3], data[5 * i + 4]);
        }
        // Right face
        glNormal3f(1.0f, 0.0f, 0.0f);
        for (i = 16; i < 20; ++i) {
            glTexCoord2f(data[5 * i], data[5 * i + 1]);
            glVertex3f(data[5 * i + 2], data[5 * i + 3], data[5 * i + 4]);
        }
        // Left face
        glNormal3f(-1.0f, 0.0f, 0.0f);
        for (i = 20; i < 24; ++i) {
            glTexCoord2f(data[5 * i], data[5 * i + 1]);
            glVertex3f(data[5 * i + 2], data[5 * i + 3], data[5 * i + 4]);
        }
    glEnd();
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
    /*
     *  Smooth shading blends colors nicely across a polygon, and smoothes out lighting.
     */
    multitextureSupported = initMultitexture();
    if (!LoadGLTextures()) {
        return FALSE;
    }

    glEnable(GL_TEXTURE_2D);
    glShadeModel(GL_SMOOTH);                          // Enables smooth shading
    glClearColor(0.0f, 0.0f, 0.0f, 0.5f);             // Black background

    glClearDepth(1.0f);                               // Depth buffer setup

    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);                           // Type of depth testing
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);           // Set line antialiasing

    initLights();                                      // Initialize OpenGL light
    return TRUE;
}
// Calculates V = VM, M    is 4X4 in column-major, V is 4dim. Row (i.e. "Transposed")
void VMatMult(GLfloat* M, GLfloat* V)
{
    GLfloat res[3];
    res[0] = M[0] * V[0] + M[1] * V[1] + M[2] * V[2] + M[3] * V[3];
    res[1] = M[4] * V[0] + M[5] * V[1] + M[6] * V[2] + M[7] * V[3];
    res[2] = M[8] * V[0] + M[9] * V[1] + M[10] * V[2] + M[11] * V[3];
    V[0] = res[0];
    V[1] = res[1];
    V[2] = res[2];
    V[3] = M[15];                                      // Homogenous coordinate
}
// Sets up the texture-offsets
// n: Normal on surface. Must be of length 1
// c: Current vertex on surface
// l: Lightposition
// s: Direction of s-texture-coordinate in object space (Must be normalized)
// t: Direction of t-texture-coordinate in object space (Must be normalized)
void SetUpBumps(GLfloat* n, GLfloat* c, GLfloat* l, GLfloat* s, GLfloat* t) 
{
    GLfloat v[3];                                        // Vector form current position to light
    GLfloat lenQ;                                        // Used to normalize
    // Calculate v from current vertex c to lightposition and normalized v
    v[0] = l[0] - c[0];
    v[1] = l[1] - c[1];
    v[2] = l[2] - c[2];
    lenQ = (GLfloat)sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
    v[0] /= lenQ;
    v[1] /= lenQ;
    v[2] /= lenQ;
    // Project v such that we get two values along each texture-coordinate axis
    c[0] = (s[0] * v[0] + s[1] * v[1] + s[2] * v[2]) * MAX_EMBOSS;
    c[1] = (t[0] * v[0] + t[1] * v[1] + t[2] * v[2]) * MAX_EMBOSS;
}

void doLogo(void)
{
    // Must call this list. Billboards the two logos.
    glDepthFunc(GL_ALWAYS);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    glDisable(GL_LIGHTING);
    glLoadIdentity();
    glBindTexture(GL_TEXTURE_2D, glLogo);
    glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(0.23f, -0.4f, -1.0f);
        glTexCoord2f(1.0f, 0.0f); glVertex3f(0.53f, -0.4f, -1.0f);
        glTexCoord2f(1.0f, 1.0f); glVertex3f(0.53f, -0.25f, -1.0f);
        glTexCoord2f(0.0f, 1.0f); glVertex3f(0.23f, -0.25f, -1.0f);
    glEnd();
    if (useMultitexture) {
        glBindTexture(GL_TEXTURE_2D, multiLogo);
        glBegin(GL_QUADS);
            glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.53f, -0.25f, -1.0f);
            glTexCoord2f(1.0f, 0.0f); glVertex3f(-0.33f, -0.25f, -1.0f);
            glTexCoord2f(1.0f, 1.0f); glVertex3f(-0.33f, -0.15f, -1.0f);
            glTexCoord2f(0.0f, 1.0f); glVertex3f(-0.53f, -0.15f, -1.0f);
        glEnd();
    }
    glDepthFunc(GL_LEQUAL);
}

bool doMesh1TexelUnits(void)
{
    GLfloat c[4] = { 0.0f, 0.0f, 0.0f, 1.0f };           // Holds current vertex
    GLfloat n[4] = { 0.0f, 0.0f, 0.0f, 1.0f };           // Normalized normal of current surface
    GLfloat s[4] = { 0.0f, 0.0f, 0.0f, 1.0f };           // s-Texture coordinate direction
    GLfloat t[4] = { 0.0f, 0.0f, 0.0f, 1.0f };           // t-Texture coordinate direction
    GLfloat l[4];                 // Holds our lightposition to be transformed into object space
    GLfloat Minv[16];             // Holds the inverted modeview matrix to do so
    int i;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // Build inverse modeview matrix first. This substitutes one push/pop with one glLoadIdentity()
    // Simply build it by doing all transformation negated and in reverse order
    glLoadIdentity();
    glRotatef(-yrot, 0.0f, 1.0f, 0.0f);
    glRotatef(-xrot, 1.0f, 0.0f, 0.0f);
    glTranslatef(0.0f, 0.0f, -z);
    glGetFloatv(GL_MODELVIEW_MATRIX, Minv);
    glLoadIdentity();
    glTranslatef(0.0f, 0.0f, z);
    glRotatef(xrot, 1.0f, 0.0f, 0.0f);
    glRotatef(yrot, 0.0f, 1.0f, 0.0f);

    // Transform the lightposition into object coordinate
    l[0] = LightPosition[0];
    l[1] = LightPosition[1];
    l[2] = LightPosition[2];
    l[3] = 1.0f;
    VMatMult(Minv, l);
    // First pass. This will render a cube only consisting out of bump map.
    glBindTexture(GL_TEXTURE_2D, bump[filter]);
    glDisable(GL_BLEND);
    glDisable(GL_LIGHTING);
    doCube();
    // Second pass. This will render a cube with the correct emboss bump mapping, but without colors.
    glBindTexture(GL_TEXTURE_2D, invbump[filter]);
    glBlendFunc(GL_ONE, GL_ONE);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_BLEND);

    glBegin(GL_QUADS);
    // Front face
    n[0] = 0.0f; n[1] = 0.0f; n[2] = 1.0f;
    s[0] = 1.0f; s[1] = 0.0f; s[2] = 0.0f;
    t[0] = 0.0f; t[1] = 1.0f; t[3] = 0.0f;
    for (i = 0; i < 4; ++i) {
        c[0] = data[5 * i + 2];
        c[1] = data[5 * i + 3];
        c[2] = data[5 * i + 4];
        SetUpBumps(n, c, l, s, t);
        glTexCoord2f(data[5 * i] + c[0], data[5 * i + 1] + c[1]);
        glVertex3f(data[5 * i + 2], data[5 * i + 3], data[5 * i + 4]);
    }
    // back face
    n[0] = 0.0f; n[1] = 0.0f; n[2] = -1.0f;
    s[0] = -1.0f; s[1] = 0.0f; s[2] = 0.0f;
    t[0] = 0.0f; t[1] = 1.0f; t[2] = 0.0f;
    for (i = 4; i<8; i++) {
        c[0] = data[5 * i + 2];
        c[1] = data[5 * i + 3];
        c[2] = data[5 * i + 4];
        SetUpBumps(n, c, l, s, t);
        glTexCoord2f(data[5 * i] + c[0], data[5 * i + 1] + c[1]);
        glVertex3f(data[5 * i + 2], data[5 * i + 3], data[5 * i + 4]);
    }
    // Top face
    n[0] = 0.0f; n[1] = 1.0f; n[2] = 0.0f;
    s[0] = 1.0f; s[1] = 0.0f; s[2] = 0.0f;
    t[0] = 0.0f; t[1] = 0.0f; t[2] = -1.0f;
    for (i = 8; i<12; i++) {
        c[0] = data[5 * i + 2];
        c[1] = data[5 * i + 3];
        c[2] = data[5 * i + 4];
        SetUpBumps(n, c, l, s, t);
        glTexCoord2f(data[5 * i] + c[0], data[5 * i + 1] + c[1]);
        glVertex3f(data[5 * i + 2], data[5 * i + 3], data[5 * i + 4]);
    }
    // Bottom face
    n[0] = 0.0f; n[1] = -1.0f; n[2] = 0.0f;
    s[0] = -1.0f; s[1] = 0.0f; s[2] = 0.0f;
    t[0] = 0.0f; t[1] = 0.0f; t[2] = -1.0f;
    for (i = 12; i<16; i++) {
        c[0] = data[5 * i + 2];
        c[1] = data[5 * i + 3];
        c[2] = data[5 * i + 4];
        SetUpBumps(n, c, l, s, t);
        glTexCoord2f(data[5 * i] + c[0], data[5 * i + 1] + c[1]);
        glVertex3f(data[5 * i + 2], data[5 * i + 3], data[5 * i + 4]);
    }
    // Right face
    n[0] = 1.0f; n[1] = 0.0f; n[2] = 0.0f;
    s[0] = 0.0f; s[1] = 0.0f; s[2] = -1.0f;
    t[0] = 0.0f; t[1] = 1.0f; t[2] = 0.0f;
    for (i = 16; i<20; i++) {
        c[0] = data[5 * i + 2];
        c[1] = data[5 * i + 3];
        c[2] = data[5 * i + 4];
        SetUpBumps(n, c, l, s, t);
        glTexCoord2f(data[5 * i] + c[0], data[5 * i + 1] + c[1]);
        glVertex3f(data[5 * i + 2], data[5 * i + 3], data[5 * i + 4]);
    }
    // Left face
    n[0] = -1.0f; n[1] = 0.0f; n[2] = 0.0f;
    s[0] = 0.0f; s[1] = 0.0f; s[2] = 1.0f;
    t[0] = 0.0f; t[1] = 1.0f; t[2] = 0.0f;
    for (i = 20; i<24; i++) {
        c[0] = data[5 * i + 2];
        c[1] = data[5 * i + 3];
        c[2] = data[5 * i + 4];
        SetUpBumps(n, c, l, s, t);
        glTexCoord2f(data[5 * i] + c[0], data[5 * i + 1] + c[1]);
        glVertex3f(data[5 * i + 2], data[5 * i + 3], data[5 * i + 4]);
    }
    glEnd();
    // Third pass. This will finish cube-rendering, complete with lighting.
    if (!emboss) {
        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        glBindTexture(GL_TEXTURE_2D, texture[filter]);
        glBlendFunc(GL_DST_COLOR, GL_SRC_COLOR);
        glEnable(GL_LIGHTING);
        doCube();
    }
    // Last pass
    xrot += xspeed;
    yrot += yspeed;
    if (xrot > 360.0f) xrot -= 360.0f;
    if (xrot < 0.0f) xrot += 360.0f;
    if (yrot > 360.0f) xrot -= 360.0f;
    if (yrot < 0.0f) xrot += 360.0f;

    doLogo();
    return true;
}

bool doMesh2TexelUnits(void)
{
    GLfloat c[4] = { 0.0f, 0.0f, 0.0f, 1.0f };           // Holds current vertex
    GLfloat n[4] = { 0.0f, 0.0f, 0.0f, 1.0f };           // Normalized normal of current surface
    GLfloat s[4] = { 0.0f, 0.0f, 0.0f, 1.0f };           // s-Texture coordinate direction
    GLfloat t[4] = { 0.0f, 0.0f, 0.0f, 1.0f };           // t-Texture coordinate direction
    GLfloat l[4];                 // Holds our lightposition to be transformed into object space
    GLfloat Minv[16];             // Holds the inverted modeview matrix to do so
    int i;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // Build inverse modeview matrix first. This substitutes one push/pop with one
    // Simply build it by doing all transformation negated and in reverse order
    glLoadIdentity();
    glRotatef(-yrot, 0.0f, 1.0f, 0.0f);
    glRotatef(-xrot, 1.0f, 0.0f, 0.0f);
    glTranslatef(0.0f, 0.0f, -z);
    glGetFloatv(GL_MODELVIEW_MATRIX, Minv);
    glLoadIdentity();
    glTranslatef(0.0f, 0.0f, z);
    glRotatef(xrot, 1.0f, 0.0f, 0.0f);
    glRotatef(yrot, 0.0f, 1.0f, 0.0f);

    // Transform the lightposition into object coordinate
    l[0] = LightPosition[0];
    l[1] = LightPosition[1];
    l[2] = LightPosition[2];
    l[3] = 1.0f;
    VMatMult(Minv, l);
    // First pass. This will render a cube consisting out of the grey-scale erode map.
    // Texture-units #0
    glActiveTextureARB(GL_TEXTURE0_ARB);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, bump[filter]);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_EXT);
    glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_REPLACE);

    // Texture-units #1
    glActiveTextureARB(GL_TEXTURE1_ARB);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, invbump[filter]);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_EXT);
    glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_ADD);

    //General switches
    glDisable(GL_BLEND);
    glDisable(GL_LIGHTING);

    glBegin(GL_QUADS);
    // Front face
    n[0] = 0.0f;
    n[1] = 0.0f;
    n[2] = 1.0f;
    s[0] = 1.0f;
    s[1] = 0.0f;
    s[2] = 0.0f;
    t[0] = 0.0f;
    t[1] = 1.0f;
    t[2] = 0.0f;
    for (i = 0; i < 4; i++) {
        c[0] = data[5 * i + 2];
        c[1] = data[5 * i + 3];
        c[2] = data[5 * i + 4];
        SetUpBumps(n, c, l, s, t);
        glMultiTexCoord2fARB(GL_TEXTURE0_ARB, data[5 * i], data[5 * i + 1]);
        glMultiTexCoord2fARB(GL_TEXTURE1_ARB, data[5 * i] + c[0], data[5 * i + 1] + c[1]);
        glVertex3f(data[5 * i + 2], data[5 * i + 3], data[5 * i + 4]);
    }
    // Back face
    n[0] = 0.0f;
    n[1] = 0.0f;
    n[2] = -1.0f;
    s[0] = -1.0f;
    s[1] = 0.0f;
    s[2] = 0.0f;
    t[0] = 0.0f;
    t[1] = 1.0f;
    t[2] = 0.0f;
    for (i = 4; i<8; i++) {
        c[0] = data[5 * i + 2];
        c[1] = data[5 * i + 3];
        c[2] = data[5 * i + 4];
        SetUpBumps(n, c, l, s, t);
        glMultiTexCoord2fARB(GL_TEXTURE0_ARB, data[5 * i], data[5 * i + 1]);
        glMultiTexCoord2fARB(GL_TEXTURE1_ARB, data[5 * i] + c[0], data[5 * i + 1] + c[1]);
        glVertex3f(data[5 * i + 2], data[5 * i + 3], data[5 * i + 4]);
    }
    // Top face
    n[0] = 0.0f;
    n[1] = 1.0f;
    n[2] = 0.0f;
    s[0] = 1.0f;
    s[1] = 0.0f;
    s[2] = 0.0f;
    t[0] = 0.0f;
    t[1] = 0.0f;
    t[2] = -1.0f;
    for (i = 8; i<12; i++) {
        c[0] = data[5 * i + 2];
        c[1] = data[5 * i + 3];
        c[2] = data[5 * i + 4];
        SetUpBumps(n, c, l, s, t);
        glMultiTexCoord2fARB(GL_TEXTURE0_ARB, data[5 * i], data[5 * i + 1]);
        glMultiTexCoord2fARB(GL_TEXTURE1_ARB, data[5 * i] + c[0], data[5 * i + 1] + c[1]);
        glVertex3f(data[5 * i + 2], data[5 * i + 3], data[5 * i + 4]);
    }
    // Bottom face
    n[0] = 0.0f;
    n[1] = -1.0f;
    n[2] = 0.0f;
    s[0] = -1.0f;
    s[1] = 0.0f;
    s[2] = 0.0f;
    t[0] = 0.0f;
    t[1] = 0.0f;
    t[2] = -1.0f;
    for (i = 12; i<16; i++) {
        c[0] = data[5 * i + 2];
        c[1] = data[5 * i + 3];
        c[2] = data[5 * i + 4];
        SetUpBumps(n, c, l, s, t);
        glMultiTexCoord2fARB(GL_TEXTURE0_ARB, data[5 * i], data[5 * i + 1]);
        glMultiTexCoord2fARB(GL_TEXTURE1_ARB, data[5 * i] + c[0], data[5 * i + 1] + c[1]);
        glVertex3f(data[5 * i + 2], data[5 * i + 3], data[5 * i + 4]);
    }
    // Right face
    n[0] = 1.0f;
    n[1] = 0.0f;
    n[2] = 0.0f;
    s[0] = 0.0f;
    s[1] = 0.0f;
    s[2] = -1.0f;
    t[0] = 0.0f;
    t[1] = 1.0f;
    t[2] = 0.0f;
    for (i = 16; i<20; i++) {
        c[0] = data[5 * i + 2];
        c[1] = data[5 * i + 3];
        c[2] = data[5 * i + 4];
        SetUpBumps(n, c, l, s, t);
        glMultiTexCoord2fARB(GL_TEXTURE0_ARB, data[5 * i], data[5 * i + 1]);
        glMultiTexCoord2fARB(GL_TEXTURE1_ARB, data[5 * i] + c[0], data[5 * i + 1] + c[1]);
        glVertex3f(data[5 * i + 2], data[5 * i + 3], data[5 * i + 4]);
    }
    // Left face
    n[0] = -1.0f;
    n[1] = 0.0f;
    n[2] = 0.0f;
    s[0] = 0.0f;
    s[1] = 0.0f;
    s[2] = 1.0f;
    t[0] = 0.0f;
    t[1] = 1.0f;
    t[2] = 0.0f;
    for (i = 20; i<24; i++) {
        c[0] = data[5 * i + 2];
        c[1] = data[5 * i + 3];
        c[2] = data[5 * i + 4];
        SetUpBumps(n, c, l, s, t);
        glMultiTexCoord2fARB(GL_TEXTURE0_ARB, data[5 * i], data[5 * i + 1]);
        glMultiTexCoord2fARB(GL_TEXTURE1_ARB, data[5 * i] + c[0], data[5 * i + 1] + c[1]);
        glVertex3f(data[5 * i + 2], data[5 * i + 3], data[5 * i + 4]);
    }
    glEnd();

    // Second pass. This will render our complete bump-mapped cube.
    glActiveTextureARB(GL_TEXTURE1_ARB);
    glDisable(GL_TEXTURE_2D);
    glActiveTextureARB(GL_TEXTURE0_ARB);
    if (!emboss) {
        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        glBindTexture(GL_TEXTURE_2D, texture[filter]);
        glBlendFunc(GL_DST_COLOR, GL_SRC_COLOR);
        glEnable(GL_BLEND);
        glEnable(GL_LIGHTING);
        doCube();
    }

    // Last pass.
    xrot += xspeed;
    yrot += yspeed;
    if (xrot>360.0f) xrot -= 360.0f;
    if (xrot<0.0f) xrot += 360.0f;
    if (yrot>360.0f) yrot -= 360.0f;
    if (yrot<0.0f) yrot += 360.0f;

    doLogo();
    return true;
}

bool doMeshNoBumps(void) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);         // Clear The Screen And The Depth Buffer
    glLoadIdentity();                           // Reset The View
    glTranslatef(0.0f, 0.0f, z);

    glRotatef(xrot, 1.0f, 0.0f, 0.0f);
    glRotatef(yrot, 0.0f, 1.0f, 0.0f);

    if (useMultitexture) {
        glActiveTextureARB(GL_TEXTURE1_ARB);
        glDisable(GL_TEXTURE_2D);
        glActiveTextureARB(GL_TEXTURE0_ARB);
    }

    glDisable(GL_BLEND);
    glBindTexture(GL_TEXTURE_2D, texture[filter]);
    glBlendFunc(GL_DST_COLOR, GL_SRC_COLOR);
    glEnable(GL_LIGHTING);
    doCube();

    xrot += xspeed;
    yrot += yspeed;
    if (xrot>360.0f) xrot -= 360.0f;
    if (xrot<0.0f) xrot += 360.0f;
    if (yrot>360.0f) yrot -= 360.0f;
    if (yrot<0.0f) yrot += 360.0f;

    doLogo();
    return true;
}

/*
 *  For now all we will do is clear the screen to the color we previously decided on,
 *  clear the depth buffer and reset the scene. We wont draw anything yet.
 */
int DrawGLScene(GLvoid)                                  // Here's where we do all the drawing
{
    if (bumps) {
        if (useMultitexture && maxTexelUnits > 1)
            return  doMesh2TexelUnits();
        else return doMesh1TexelUnits();
    }
    else return doMeshNoBumps();
}
/*****************************************************************************************************************************************/
/*****************************************************************************************************************************************/
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
/*****************************************************************************************************************************************/
/*****************************************************************************************************************************************/

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
            if (keys['E'])
            {
                keys['E'] = false;
                emboss = !emboss;
            }

            if (keys['M'])
            {
                keys['M'] = false;
                useMultitexture = ((!useMultitexture) && multitextureSupported);
            }

            if (keys['B'])
            {
                keys['B'] = false;
                bumps = !bumps;
            }

            if (keys['F'])
            {
                keys['F'] = false;
                filter++;
                filter %= 3;
            }

            if (keys[VK_PRIOR])
            {
                z -= 0.02f;
            }

            if (keys[VK_NEXT])
            {
                z += 0.02f;
            }

            if (keys[VK_UP])
            {
                xspeed -= 0.0001f;
            }

            if (keys[VK_DOWN])
            {
                xspeed += 0.0001f;
            }

            if (keys[VK_RIGHT])
            {
                yspeed += 0.0001f;
            }

            if (keys[VK_LEFT])
            {
                yspeed -= 0.0001f;
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
/*****************************************************************************************************************************************/
/*****************************************************************************************************************************************/