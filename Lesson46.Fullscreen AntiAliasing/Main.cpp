#include <windows.h>
#include <stdio.h>
#include <math.h>
#include <gl/glew.h>
#include <gl/glut.h>
#include <GL/GLUAX.H>
#include <mmsystem.h>
#include "Previous.h"
#include "arbMultiSample.h"

// Fullscreen AntiAliasing
#pragma comment(lib, "legacy_stdio_definitions.lib")

#ifndef CDS_FULLSCREEN
#define CDS_FULLSCREEN 4
#endif

bool domulti = true;
bool doangle = true;

float angle = 0.0f;

GL_Window*	g_window;
Keys*		g_keys;

BOOL Initialize(GL_Window* window, Keys* keys)
{
	g_window = window;
	g_keys = keys;

	angle = 0.0f;

	glViewport(0, 0, window->init.width, window->init.height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(50, (float)window->init.width / (float)window->init.height, 5, 2000);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glEnable(GL_DEPTH_TEST);
	glShadeModel(GL_SMOOTH);
	glClearColor(0.0f, 0.0f, 0.0f, 0.5f);

	return TRUE;
}

void Deinitialize(void) { }

void Update(DWORD milliseconds)
{
	if (g_keys->keyDown[VK_ESCAPE] == TRUE) {
		TerminateApplication(g_window);
	}
	if (g_keys->keyDown[VK_F1] == TRUE) {
		ToggleFullscreen(g_window);
	}
	if (g_keys->keyDown[VK_SPACE] == TRUE) {
		domulti = !domulti;
	}
	if (g_keys->keyDown['M'] == TRUE) {
		doangle = !doangle;
	}
}

void Draw(void)
{
	if (domulti) {
		glEnable(GL_MULTISAMPLE_ARB);
	}

	glClearColor(0.0f, 0.0f, 0.0f, 0.5);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	for (float i = -10; i < 10; ++i) {
		for (float j = -10; j < 10; ++j) {
			glPushMatrix();
			glTranslatef(i * 2.0f, j * 2.0f, -5.0f);
			glRotatef(angle, 0.0f, 0.0f, 1.0f);
			glBegin(GL_QUADS);
				glColor3f(1.0f, 0.0f, 0.0f); glVertex3f(i, j, 0.0f);
				glColor3f(0.0f, 1.0f, 0.0f); glVertex3f(i + 2.0f, j, 0.0f);
				glColor3f(0.0f, 0.0f, 1.0f); glVertex3f(i + 2.0f, j + 2.0f, 0.0f);
				glColor3f(1.0f, 1.0f, 1.0f); glVertex3f(i, j + 2.0f, 0.0f);
			glEnd();
			glPopMatrix();
		}
	}

	if (doangle) {
		angle += 0.05f;
	}
	glFlush();
	if (domulti) {
		glDisable(GL_MULTISAMPLE_ARB);
	}
}