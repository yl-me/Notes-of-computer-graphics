#include <windows.h>
#include <math.h>
#include <stdio.h>
#include <stdarg.h>
#include <gl/glew.h>
#include <GL\glut.h>
#include <time.h>
#include "Previous.h"

#pragma comment( lib, "winmm.lib" )                                // Search for winMM library while linking
#pragma comment(lib, "legacy_stdio_definitions.lib")

#ifndef CDS_FULLSCREEN 
#define CDS_FULLSCREEN 4
#endif

void DrawTargets();

GL_Window* g_window;
Keys* g_keys;

GLuint base;                                                 // Front display list
GLuint roll;                                                 // Rolling clouds
GLint level = 1;                                             // Current level
GLint miss;                                                  // Missed Targets
GLint kills;                                                 // Level kill counter
GLint score;                                                 // Current score
bool game;                                                   // Game over?

typedef int(*compfn)(const void*, const void*);              // Typedef for compare function

struct objects {
    GLuint rot;                                              // Rotation
    bool hit;                                                // Object hit
    GLuint frame;                                             // Current explosion fram
    GLuint dir;                                              // Object Direction
    GLuint texid;                                            // Texture ID
    GLfloat x;                                               // Object X position
    GLfloat y;                                               // Object Y position
    GLfloat spin;                                            // Object spin
    GLfloat distance;                                        // Object distance
};

typedef struct {
    GLubyte* imageData;
    GLuint bpp;                                              // Image color depth in bits per pixel
    GLuint width;
    GLuint height;
    GLuint texID;
} TextureImage;

TextureImage textures[10];

objects object[30];

struct dimension {
    GLfloat w;                                                  // Width
    GLfloat h;                                                  // Height
};

// Size of each object : Blueface,      Bucket,       Targets,       Coke,          Vase
dimension size[5] = { {1.0f, 1.0f},{ 1.0f, 1.0f },{ 1.0f, 1.0f },{0.5f, 1.0f}, {0.75f, 1.5f} };

bool LoadTGA(TextureImage* texture, char* filename)
{
    GLubyte TGAheader[12] = { 0,0,2,0,0,0,0,0,0,0,0,0 };// Uncompressed TGA header
    GLubyte TGAcompare[12];                             // Used to compare TGA header
    GLubyte header[6];                                  // First 6 useful bytes from the header
    GLuint bytesPerPixel;                               // Holds number of bytes per pixel used in the TGA file
    GLuint imageSize;                                    // Used to store the image size when setting aside ram
    GLuint temp;                                        // Temporary variable
    GLuint type = GL_RGBA;                              // Set the default GL mode to RGBA (32 bpp)

    FILE* file = fopen(filename, "rb");
    if (file == NULL || fread(TGAcompare, 1, sizeof(TGAcompare), file) != sizeof(TGAcompare) ||
        memcmp(TGAheader, TGAcompare, sizeof(TGAheader)) != 0 ||
        fread(header, 1, sizeof(header), file) != sizeof(header))
    {
        if (file == NULL) {
            return FALSE;
        }
        else {
            fclose(file);
            return FALSE;
        }
    }

    texture->width = header[1] * 256 + header[0];
    texture->height = header[3] * 256 + header[2];

    if (texture->width <= 0 || texture->height <= 0 || (header[4] != 24 && header[4] != 32)) {
        fclose(file);
        return FALSE;
    }
    texture->bpp = header[4];
    bytesPerPixel = texture->bpp / 8;
    imageSize = texture->width * texture->height * bytesPerPixel;
    texture->imageData = (GLubyte*)malloc(imageSize);

    if (texture->imageData == NULL || fread(texture->imageData, 1, imageSize, file) != imageSize) {
        if (texture->imageData != NULL) {
            free(texture->imageData);
        }
        fclose(file);
        return  FALSE;
    }
    for (GLuint i = 0; i < int(imageSize); i += bytesPerPixel) {
        temp = texture->imageData[i];
        texture->imageData[i] = texture->imageData[i + 2];
        texture->imageData[i + 2] = temp;
    }
    fclose(file);
    glGenTextures(1, &texture[0].texID);

    glBindTexture(GL_TEXTURE_2D, texture[0].texID);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    if (texture[0].bpp == 24) {
        type = GL_RGB;
    }
    glTexImage2D(GL_TEXTURE_2D, 0, type, texture[0].width, texture[0].height, 0, type, 
        GL_UNSIGNED_BYTE, texture[0].imageData);
    return true;
}

GLvoid BuildFont(GLvoid)
{
    base = glGenLists(95);
    glBindTexture(GL_TEXTURE_2D, textures[9].texID);
    for (int loop = 0; loop < 95; ++loop) {
        float cx = float(loop % 16) / 16.0f;
        float cy = float(loop / 16) / 8.0f;

        glNewList(base + loop, GL_COMPILE);
            glBegin(GL_QUADS);
                glTexCoord2f(cx, 1.0f - cy - 0.120f);
                glVertex2i(0, 0);
                glTexCoord2f(cx + 0.0625f, 1.0f - cy - 0.120f);
                glVertex2i(16, 0);
                glTexCoord2f(cx + 0.0625f, 1.0f - cy);
                glVertex2i(16, 16);
                glTexCoord2f(cx, 1.0f - cy); glVertex2i(0, 16);
            glEnd();
            glTranslated(10, 0, 0);
        glEndList();
    }
}

GLvoid glPrint(GLint x, GLint y, const char *string, ...)
{
    char text[256];
    va_list    ap;                                              // Pointer to list of arguments

    if (string == NULL) {
        return;
    }

    va_start(ap, string);                                    // Parses the string for variables
    vsprintf(text, string, ap);                              // And converts symbols to actual numbers
    va_end(ap);                                              // Results are stored in text

    glBindTexture(GL_TEXTURE_2D, textures[9].texID);
    glPushMatrix();
    glLoadIdentity();
    glTranslated(x, y, 0);
    glListBase(base - 32);
    glCallLists(strlen(text), GL_UNSIGNED_BYTE, text);       // Draws the display list text
    glPopMatrix();
}

int Compare(struct objects* elem1, struct objects* elem2)
{
    if (elem1->distance < elem2->distance) {
        return -1;
    }
    else if (elem1->distance > elem2->distance) {
        return 1;
    }
    else {
        return 0;
    }
}

GLvoid InitObject(int num)
{
    object[num].rot = 1;                                     // Clockwise rotation
    object[num].frame = 0;                                    // Reset the explosion fram
    object[num].hit = FALSE;                                 // Reset object hit status
    object[num].texid = rand() % 5;
    object[num].distance = -(float(rand() % 4001) / 100.0f);
    object[num].y = -1.5f + (float(rand() % 451) / 100.0f);
    object[num].x = ((object[num].distance - 15.0f) / 2.0f) - (5 * level) - float(rand() % (5 * level));
    object[num].dir = (rand() % 2);

    if (object[num].dir == 0) {
        object[num].rot = 2;
        object[num].x = -object[num].x;
    }
    if (object[num].texid == 0) {                            // Blue face
        object[num].y = -2.0f;
    }
    if (object[num].texid == 1) {                            // Bucket
        object[num].dir = 3;
        object[num].x = float(rand() % int(object[num].distance - 10.0f)) + 
            ((object[num].distance - 10.0f) / 2.0f);
        object[num].y = 4.5f;
    }
    if (object[num].texid == 2) {                            // Target
        object[num].dir = 2;
        object[num].x = float(rand() % int(object[num].distance - 10.0f)) + 
            ((object[num].distance - 10.0f) / 2.0f);
        object[num].y = -3.0f - float(rand() % (5 * level));
    }
    qsort((void*)&object, level, sizeof(struct objects), (compfn)Compare);
}

BOOL Initialize(GL_Window* window, Keys* keys)
{
    g_window = window;
    g_keys = keys;
    srand((unsigned)time(NULL));
    if ((!LoadTGA(&textures[0], "Data/BlueFace.tga")) ||     // Load the blueface texture
        (!LoadTGA(&textures[1], "Data/Bucket.tga")) ||       // Load the bucket texture
        (!LoadTGA(&textures[2], "Data/Target.tga")) ||       // Load the target Texture
        (!LoadTGA(&textures[3], "Data/Coke.tga")) ||         // Load the coke texture
        (!LoadTGA(&textures[4], "Data/Vase.tga")) ||         // Load the vase texture
        (!LoadTGA(&textures[5], "Data/Explode.tga")) ||      // Load the explosion texture
        (!LoadTGA(&textures[6], "Data/Ground.tga")) ||       // Load the ground texture
        (!LoadTGA(&textures[7], "Data/Sky.tga")) ||          // Load the sky texture
        (!LoadTGA(&textures[8], "Data/Crosshair.tga")) ||    // Load the crosshair texture
        (!LoadTGA(&textures[9], "Data/Font.tga")))           // Load the crosshair texture
    {
        return FALSE;
    }
    BuildFont();
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClearDepth(1.0f);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);       // Enable alpha blending
    glEnable(GL_BLEND);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_CULL_FACE);

    for (int loop = 0; loop < 30; ++loop) {
        InitObject(loop);
    }
    return TRUE;
}

void Deinitialize(void)
{
    glDeleteLists(base, 95);
}

void Selection(void)
{
    GLuint buffer[512];                                      // Set up a selection buffer
    GLint hits;                                              // The number of objects that we selected

    if (game) {
        return;
    }
    PlaySound("data/shot.wav", NULL, SND_ASYNC);
    GLint viewport[4];                                       // x, y, length, width
    
    // This sets the array <viewport> to the size and location of the screen relative to the window
    glGetIntegerv(GL_VIEWPORT, viewport);
    glSelectBuffer(512, buffer);                             // Tell opengl to use our array for selection
    
    // Puts opengl in selection mode. Nothing will be draw. Object ID's and extents are stored in the buffer
    (void)glRenderMode(GL_SELECT);
    glInitNames();                                           // Name stack
    glPushName(0);

    glMatrixMode(GL_PROJECTION);                             // Selects the projection matrix
    glPushMatrix();
    glLoadIdentity();

    // This create a matrix that will zoom up to a small portion of the screen, where the mouse is.
    gluPickMatrix((GLdouble)mouse_x, (GLdouble)(viewport[3] - mouse_y), 1.0f, 1.0f, viewport);

    // Apply the perspective matrix
    gluPerspective(45.0f, (GLfloat)(viewport[2] - viewport[0]) / (GLfloat)(viewport[3] - viewport[1]), 
        0.1f, 100.0f);
    glMatrixMode(GL_MODELVIEW);
    DrawTargets();                                           // Render the targets to the selection buffer
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    hits = glRenderMode(GL_RENDER);

    if (hits > 0) {
        int choose = buffer[3];
        int depth = buffer[1];
        for (int loop = 1; loop < hits; ++loop) {
            if (buffer[loop * 4 + 1] < GLuint(depth)) {
                choose = buffer[loop * 4 + 3];
                depth = buffer[loop * 4 + 1];
            }
        }
        if (!object[choose].hit) {
            object[choose].hit = TRUE;
            score += 1;
            kills += 1;
            if (kills > level * 5) {
                miss = 0;
                kills = 0;
                level += 1;
                if (level > 30) {
                    level = 30;
                }
            }
        }
    }
}

void Update(DWORD milliseconds)
{
    if (g_keys->keyDown[VK_ESCAPE]) {
        TerminateApplication(g_window);
    }
    if (g_keys->keyDown[' '] && game) {
        for (int loop = 0; loop < 30; ++loop) {
            InitObject(loop);
        }
        game = FALSE;
        score = 0; 
        level = 1;
        kills = 0;
        miss = 0;
    }
    if (g_keys->keyDown[VK_F1]) {
        ToggleFullscreen(g_window);
    }
    roll -= milliseconds * 0.00005f;                         // Roll the clouds
    for (int loop = 0; loop < level; ++loop)
    {
        if (object[loop].rot == 1)                           // If Rotation is clockwise
            object[loop].spin -= 0.2f * (float(loop + milliseconds));    // Spin clockwise

        if (object[loop].rot == 2)                           // If rotation is counter clockwise
            object[loop].spin += 0.2f * (float(loop + milliseconds));    // Spin counter clockwise

        if (object[loop].dir == 1)                           // If direction is right
            object[loop].x += 0.012f * float(milliseconds);    // Move right

        if (object[loop].dir == 0)                           // If direction is left
            object[loop].x -= 0.012f * float(milliseconds);    // Move left

        if (object[loop].dir == 2)                           // If direction is up
            object[loop].y += 0.012f * float(milliseconds);    // Move up

        if (object[loop].dir == 3)                           // If direction isdown
            object[loop].y -= 0.0025f * float(milliseconds);   // Move down
        // If we are to far left, direction is left and the object was not hit
        if ((object[loop].x < (object[loop].distance - 15.0f) / 2.0f) && (object[loop].dir == 0) && 
            !object[loop].hit)
        {
            miss += 1;                                       // Increase miss
            object[loop].hit = TRUE;
        }

        if ((object[loop].x > -(object[loop].distance - 15.0f) / 2.0f) && (object[loop].dir == 1) && 
            !object[loop].hit)
        {
            miss += 1;
            object[loop].hit = TRUE;
        }

        if ((object[loop].y < -2.0f) && (object[loop].dir == 3) && !object[loop].hit) {
            miss += 1;
            object[loop].hit = TRUE;
        }

        if ((object[loop].y > 4.5f) && (object[loop].dir == 2)) {
            object[loop].dir = 3;
        }
    }
}

void Object(float width, float height, GLuint texid)
{
    glBindTexture(GL_TEXTURE_2D, textures[texid].texID);
    glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-width, -height, 0.0f);
        glTexCoord2f(1.0f, 0.0f); glVertex3f(width, -height, 0.0f);
        glTexCoord2f(1.0f, 1.0f); glVertex3f(width, height, 0.0f);
        glTexCoord2f(0.0f, 1.0f); glVertex3f(-width, height, 0.0f);
    glEnd();
}

void Explosion(int num)
{
    float ex = (float)((object[num].frame / 4) % 4) / 4.0f;
    float ey = (float)((object[num].frame / 4) / 4) / 4.0f;

    glBindTexture(GL_TEXTURE_2D, textures[5].texID);
    glBegin(GL_QUADS);
        glTexCoord2f(ex, 1.0f - (ey));
        glVertex3f(-1.0f, -1.0f, 0.0f);
        glTexCoord2f(ex + 0.25f, 1.0f - (ey));
        glVertex3f(1.0f, -1.0f, 0.0f);
        glTexCoord2f(ex + 0.25f, 1.0f - (ey + 0.25f));
        glVertex3f(1.0f, 1.0f, 0.0f);
        glTexCoord2f(ex, 1.0f - (ey + 0.25f));
        glVertex3f(-1.0f, 1.0f, 0.0f);
    glEnd();

    object[num].frame += 1;
    if (object[num].frame > 63)
    {
        InitObject(num);
    }
}

void DrawTargets(void)
{
    glLoadIdentity();
    glTranslatef(0.0f, 0.0f, -10.0f);
    for (int loop = 0; loop < level; ++loop) {
        glLoadName(loop);                                    // Assign object a name (ID)
        glPushMatrix();
        glTranslatef(object[loop].x, object[loop].y, object[loop].distance);
        if (object[loop].hit) {
            Explosion(loop);
        }
        else {
            glRotatef(object[loop].spin, 0.0f, 0.0f, 1.0f);
            Object(size[object[loop].texid].w, size[object[loop].texid].h, object[loop].texid);
        }
        glPopMatrix();
    }
}

void Draw(void)                                              // Draw scene
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    glPushMatrix();
    glBindTexture(GL_TEXTURE_2D, textures[7].texID);
    glBegin(GL_QUADS);
    glTexCoord2f(1.0f, roll / 1.5f + 1.0f); glVertex3f(28.0f, +7.0f, -50.0f);
    glTexCoord2f(0.0f, roll / 1.5f + 1.0f); glVertex3f(-28.0f, +7.0f, -50.0f);
    glTexCoord2f(0.0f, roll / 1.5f + 0.0f); glVertex3f(-28.0f, -3.0f, -50.0f);
    glTexCoord2f(1.0f, roll / 1.5f + 0.0f); glVertex3f(28.0f, -3.0f, -50.0f);

    glTexCoord2f(1.5f, roll + 1.0f); glVertex3f(28.0f, +7.0f, -50.0f);
    glTexCoord2f(0.5f, roll + 1.0f); glVertex3f(-28.0f, +7.0f, -50.0f);
    glTexCoord2f(0.5f, roll + 0.0f); glVertex3f(-28.0f, -3.0f, -50.0f);
    glTexCoord2f(1.5f, roll + 0.0f); glVertex3f(28.0f, -3.0f, -50.0f);

    glTexCoord2f(1.0f, roll / 1.5f + 1.0f); glVertex3f(28.0f, +7.0f, 0.0f);
    glTexCoord2f(0.0f, roll / 1.5f + 1.0f); glVertex3f(-28.0f, +7.0f, 0.0f);
    glTexCoord2f(0.0f, roll / 1.5f + 0.0f); glVertex3f(-28.0f, +7.0f, -50.0f);
    glTexCoord2f(1.0f, roll / 1.5f + 0.0f); glVertex3f(28.0f, +7.0f, -50.0f);

    glTexCoord2f(1.5f, roll + 1.0f); glVertex3f(28.0f, +7.0f, 0.0f);
    glTexCoord2f(0.5f, roll + 1.0f); glVertex3f(-28.0f, +7.0f, 0.0f);
    glTexCoord2f(0.5f, roll + 0.0f); glVertex3f(-28.0f, +7.0f, -50.0f);
    glTexCoord2f(1.5f, roll + 0.0f); glVertex3f(28.0f, +7.0f, -50.0f);
    glEnd();

    glBindTexture(GL_TEXTURE_2D, textures[6].texID);
    glBegin(GL_QUADS);
    glTexCoord2f(7.0f, 4.0f - roll); glVertex3f(27.0f, -3.0f, -50.0f);
    glTexCoord2f(0.0f, 4.0f - roll); glVertex3f(-27.0f, -3.0f, -50.0f);
    glTexCoord2f(0.0f, 0.0f - roll); glVertex3f(-27.0f, -3.0f, 0.0f);
    glTexCoord2f(7.0f, 0.0f - roll); glVertex3f(27.0f, -3.0f, 0.0f);
    glEnd();

    DrawTargets();
    glPopMatrix();

    // Crosshair (In Ortho View)
    RECT window;                                             // Storage For Window Dimensions
    GetClientRect(g_window->hWnd, &window);                  // Get Window Dimensions
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, window.right, 0, window.bottom, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glTranslated(mouse_x, window.bottom - mouse_y, 0.0f);
    Object(16, 16, 8);                                       // Crosshair

    glPrint(240, 450, "Productions");
    glPrint(10, 10, "Level: %i", level);
    glPrint(250, 10, "Score: %i", score);

    if (miss > 9) {
        miss = 9;
        game = TRUE;
    }

    if (game) {
        glPrint(490, 10, "GAME OVER");
    }
    else {
        glPrint(490, 10, "Morale: %i/10", 10 - miss);
    }

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

    glFlush();                                               // Flush the GL rendering pipeline
}