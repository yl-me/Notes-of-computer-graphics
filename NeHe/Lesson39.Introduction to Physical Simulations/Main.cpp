#include <Windows.h>
#include <GL/glew.h>
#include <GL/glut.h>
#include <GL/GLUAX.H>
#include <math.h>
#include <stdio.h>
#include "Previous.h"
#include "Physics.h"

// Introduction to Physical Simulations
#pragma comment(lib, "legacy_stdio_definitions.lib")

#ifndef CDS_FULLSCREEN
#define CDS_FULLSCREEN 4
#endif

GL_Window* g_window;
Keys* g_keys;

ConstantVelocity* constantVelocity = new ConstantVelocity();

MotionUnderGravitation* motionUnderGravitation = new MotionUnderGravitation(Vector3D(0.0f, -9.81f, 0.0f));

MassConnectedWithSpring* massConnectedWithSpring = new MassConnectedWithSpring(2.0f);

float slowMotionRatio = 10.0f;               // Slow down the simulation
float timeElapsed = 0;                       // Not equal to real world's time unless slowMotionRatio Is 1
GLuint base;                 // Display

GLYPHMETRICSFLOAT gmf[256];         // Storage font characters

GLvoid BuildFont(GL_Window* window)           // Build bitmap font
{
    HFONT font;
    base = glGenLists(256);
    font = CreateFont(-12, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_TT_PRECIS,
        CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, FF_DONTCARE | DEFAULT_PITCH, NULL);
    
    HDC hDC = window->hDC;
    SelectObject(hDC, font);

    wglUseFontOutlines(hDC,                    // Select the current DC
        0,                                       // Starting character
        255,                                   // Number of display lists to build
        base,                                  // Starting display lists
        0.0f,                                  // Deviation from the true outlines
        0.0f,                                  // Font thickness in the Z direction
        WGL_FONT_POLYGONS,                     // Use polygons, not lines
        gmf);                                  // Address of buffer to recieve data
}

GLvoid KillFont(GLvoid)
{
    glDeleteLists(base, 256);
}

GLvoid glPrint(float x, float y, float z, const char* fmt, ...)
{
    float length = 0;                         // The length of text
    char text[256];                           // String
    va_list ap;                               // Pointer to list of arguments

    if (fmt == NULL) {
        return;
    }

    va_start(ap, fmt);
        vsprintf(text, fmt, ap);
    va_end(ap);

    for (unsigned int loop = 0; loop < (strlen(text)); ++loop) {
        length += gmf[text[loop]].gmfCellIncX;             // Increase length by each characters width
    }
    glTranslatef(x - length, y, z);
    glPushAttrib(GL_LIST_BIT);
    glListBase(base);
    glCallLists(strlen(text), GL_UNSIGNED_BYTE, text);
    glPopAttrib();

    glTranslatef(-x, -y, -z);
}

BOOL Initialize(GL_Window* window, Keys* keys)
{
    g_window = window;
    g_keys = keys;

    glClearColor(0.0f, 0.0f, 0.0f, 0.5f);
    glShadeModel(GL_SMOOTH);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

    BuildFont(window);
    return TRUE;
}

void Deinitialize(void)
{
    KillFont();

    constantVelocity->release();
    delete(constantVelocity);
    constantVelocity = NULL;

    motionUnderGravitation->release();
    delete(motionUnderGravitation);
    motionUnderGravitation = NULL;

    massConnectedWithSpring->release();
    delete(massConnectedWithSpring);
    massConnectedWithSpring = NULL;
}

void Update(DWORD milliseconds)
{
    if (g_keys->keyDown[VK_ESCAPE] == TRUE) {
        TerminateApplication(g_window);
    }

    if (g_keys->keyDown[VK_F1] == TRUE) {
        ToggleFullscreen(g_window);
    }
    if (g_keys->keyDown[VK_F2] == TRUE) {
        slowMotionRatio = 1.0f;
    }
    if (g_keys->keyDown[VK_F3] == TRUE) {
        slowMotionRatio = 10.0f;
    }

    float dt = milliseconds / 1000.0f;
    dt /= slowMotionRatio;
    timeElapsed += dt;

    float maxPossible_dt = 0.1f;               //The maximum possible dt is 0.1 seconds
    int numOfIterations = (int)(dt / maxPossible_dt) + 1;
    if (numOfIterations != 0) {
        dt = dt / numOfIterations;
    }
    for (int i = 0; i < numOfIterations; ++i) {
        constantVelocity->operate(dt);
        motionUnderGravitation->operate(dt);
        massConnectedWithSpring->operate(dt);
    }
}

void Draw(void)
{
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    gluLookAt(0, 0, 40, 0, 0, 0, 0, 1, 0);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glColor3ub(0, 0, 255);               // Blue
    
    glBegin(GL_LINES);
    for (float x = -20; x <= 20; x += 1.0f) {
        glVertex3f(x, 20, 0);
        glVertex3f(x, -20, 0);
    }
    for (float y = -20; y <= 20; y += 1.0f)
    {
        glVertex3f(20, y, 0);
        glVertex3f(-20, y, 0);
    }
    glEnd();

    glColor3ub(255, 0, 0);
    for (int i = 0; i < constantVelocity->numOfMasses; ++i) {
        Mass* mass = constantVelocity->getMass(i);
        Vector3D* pos = &mass->pos;

        glPrint(pos->x, pos->y + 1, pos->z, "Mass with constant vel");
        glPointSize(4);
        glBegin(GL_POINTS);
            glVertex3f(pos->x, pos->y, pos->z);
        glEnd();
    }

    glColor3ub(255, 255, 0);
    for (int i = 0; i < motionUnderGravitation->numOfMasses; ++i) {
        Mass* mass = motionUnderGravitation->getMass(i);
        Vector3D* pos = &mass->pos;

        glPrint(pos->x, pos->y + 1, pos->z, "Motion under gravitation");
        glPointSize(4);
        glBegin(GL_POINTS);
            glVertex3f(pos->x, pos->y, pos->z);
        glEnd();
    }

    glColor3ub(0, 255, 0);
    for (int i = 0; i < massConnectedWithSpring->numOfMasses; ++i) {
        Mass* mass = massConnectedWithSpring->getMass(i);
        Vector3D* pos = &mass->pos;

        glPrint(pos->x, pos->y + 1, pos->z, "Mass connected with spring");
        glPointSize(8);
        glBegin(GL_POINTS);
            glVertex3f(pos->x, pos->y, pos->z);
        glEnd();

        glBegin(GL_LINES);
            glVertex3f(pos->x, pos->y, pos->z);
            pos = &massConnectedWithSpring->connectionPos;
            glVertex3f(pos->x, pos->y, pos->z);
        glEnd();
    }

    glColor3ub(255, 255, 255);
    glPrint(-5.0f, 14, 0, "Time elapsed (seconds): %.2f", timeElapsed);
    glPrint(-5.0f, 13, 0, "Slow motion ratio: %.2f", slowMotionRatio);
    glPrint(-5.0f, 12, 0, "Press F2 for normal motion");
    glPrint(-5.0f, 11, 0, "Press F3 for slow motion");

    glFlush();
}