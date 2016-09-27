#include <Windows.h>
#include <GL/glew.h>
#include <GL/glut.h>
#include <GL/GLUAX.H>
#include <math.h>
#include <stdio.h>
#include "Previous.h"
#include "Resource.h"

#pragma comment(lib, "legacy_stdio_definitions.lib")

#ifndef CDS_FULLSCREEN
#define CDS_FULLSCREEN 4
#endif

GL_Window* g_window;
Keys* g_keys;

GLuint texture[3];
struct object {
    int tex;                  // Integer used to select texture
    float x, y, z;            // Position
    float yi;                 // Y axis increase speed
    float spinz;              // Z axis spin
    float spinzi;             // Z axis spin speed
    float flap;               // Flapping triangle
    float fi;                 // Flap direction (increase value)
};

object obj[50];

void SetObject(int loop)            // Initialize
{
    obj[loop].tex = rand() % 3;
    obj[loop].x = rand() % 34 - 17.0f;
    obj[loop].y = 18.0f;
    obj[loop].z = -((rand() % 30000 / 1000.0f) + 10.0f);
    obj[loop].spinzi = (rand() % 10000) / 5000.0f - 1.0f;
    obj[loop].flap = 0.0f;
    obj[loop].fi = 0.05f + (rand() % 100) / 1000.0f;
    obj[loop].yi = 0.001f + (rand() % 1000) / 10000.0f;
}

void LoadGLTexture()
{
    HBITMAP hBMP;                   // Handle of the bitmap
    BITMAP BMP;                     // Bitmap structure
    // The ID of the 3 bitmap images
    byte Texture[] = { IDB_BUTTERFLY1,  IDB_BUTTERFLY2, IDB_BUTTERFLY3 };

    glGenTextures(sizeof(Texture), &texture[0]);
    for (int loop = 0; loop < sizeof(Texture); ++loop) {
        //  GetModuleHandle(NULL) - A handle to an instance
        /* MAKEINTRESOURCE(Texture[loop]) - Converts an Integer Value (Texture[loop]) 
         * to a resource value (this is the image to load). 
         */
        // IMAGE_BITMAP - Tells our program that the resource to load is a bitmap image
        // 0, 0 are the desired height and width of the image in pixels
        // LR_CREATEDIBSECTION returns a DIB section bitmap,
        hBMP = (HBITMAP)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(Texture[loop]),
            IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
        
        if (hBMP) {
            GetObject(hBMP, sizeof(BMP), &BMP);
            glPixelStorei(GL_UNPACK_ALIGNMENT, 4);         // Pixel storage mode (word alignment / 4 bytes)
            glBindTexture(GL_TEXTURE_2D, texture[loop]);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            // Generate mipmapped texture
            gluBuild2DMipmaps(GL_TEXTURE_2D, 3, BMP.bmWidth, BMP.bmHeight, GL_BGR_EXT,
                GL_UNSIGNED_BYTE, BMP.bmBits);
            DeleteObject(hBMP);
        }
    }
}

BOOL Initialize(GL_Window* window, Keys* keys)
{
    g_window = window;
    g_keys = keys;

    LoadGLTexture();

    glClearColor(0.0f, 0.0f, 0.0f, 0.5f);
    glClearDepth(1.0f);
    glDepthFunc(GL_LEQUAL);                 // Less or equal
    glDisable(GL_DEPTH_TEST);
    glShadeModel(GL_SMOOTH);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    glEnable(GL_TEXTURE_2D);
    glBlendFunc(GL_ONE, GL_SRC_ALPHA);
    glEnable(GL_BLEND);

    for (int loop = 0; loop < 50; ++loop) {
        SetObject(loop);
    }
    return TRUE;
}

void Deinitialize(void) {}

void Update(DWORD milliseconds)
{
    if (g_keys->keyDown[VK_ESCAPE] == TRUE)
    {
        TerminateApplication(g_window);
    }

    if (g_keys->keyDown[VK_F1] == TRUE)
    {
        ToggleFullscreen(g_window);
    }
}

void Draw(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    for (int loop = 0; loop < 50; ++loop) {
        glLoadIdentity();
            glBindTexture(GL_TEXTURE_2D, texture[obj[loop].tex]);
            glTranslatef(obj[loop].x, obj[loop].y, obj[loop].z);
            glRotatef(45.0f, 1.0f, 0.0f, 0.0f);
            glRotatef(obj[loop].spinz, 0.0f, 0.0f, 1.0f);

            glBegin(GL_TRIANGLES);
            glTexCoord2f(1.0f, 1.0f); glVertex3f(1.0f, 1.0f, 0.0f);
            glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f, 1.0f, obj[loop].flap);
            glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f, 0.0f);

            glTexCoord2f(1.0f, 1.0f); glVertex3f(1.0f, 1.0f, 0.0f);
            glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f, 0.0f);
            glTexCoord2f(1.0f, 0.0f); glVertex3f(1.0f, -1.0f, obj[loop].flap);
        glEnd();

        obj[loop].y -= obj[loop].yi;
        obj[loop].spinz += obj[loop].spinzi;
        obj[loop].flap += obj[loop].fi;

        if (obj[loop].y < -18.0f) {
            SetObject(loop);
        }
        if ((obj[loop].flap > 1.0f) || (obj[loop].flap < -1.0f)) {
            obj[loop].fi = -obj[loop].fi;
        }
    }
    Sleep(15);
    glFlush();
}