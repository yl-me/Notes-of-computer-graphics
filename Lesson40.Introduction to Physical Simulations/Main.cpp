#include <Windows.h>
#include <GL/glew.h>
#include <GL/glut.h>
#include <GL/GLUAX.H>
#include <math.h>
#include <stdio.h>
#include "Previous.h"
#include "Physics2.h"

#pragma comment(lib, "legacy_stdio_definitions.lib")

#ifndef CDS_FULLSCREEN
#define CDS_FULLSCREEN 4
#endif

GL_Window* g_window;
Keys* g_keys;

RopeSimulation* ropeSimulation = new RopeSimulation(
	80,                 // Particles
	0.05f,              // Each particle has a weight of 50 grams
	1000.0f,            // SpringConstant
	0.05f,              // Normal length of string in the rope
	0.2f,               // Spring inner friction constant
	Vector3D(0, -9.81f, 0),  // Gravitational acceleration
	0.02f,              // Air friction constant
	100.0f,             // Ground repel constant
	0.2f,               // Ground slide friction
	2.0f,               // Ground absoption constant
	-1.5f);             // Height of ground

BOOL Initialize(GL_Window* window, Keys* keys)
{
	g_window = window;
	g_keys = keys;

	ropeSimulation->getMass(ropeSimulation->numOfMasses - 1)->vel.z = 10.0f;

	glClearColor(0.0f, 0.0f, 0.0f, 0.5f);
	glClearDepth(1.0f);
	glShadeModel(GL_SMOOTH);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	return TRUE;
}

void Deinitialize(void)
{
	ropeSimulation->release();
	delete(ropeSimulation);
	ropeSimulation = NULL;
}

void Update(DWORD milliseconds)
{
	if (g_keys->keyDown[VK_ESCAPE] == TRUE)
		TerminateApplication(g_window);
	if (g_keys->keyDown[VK_F1] == TRUE)
		ToggleFullscreen(g_window);

	Vector3D ropeConnectionVel;
	if (g_keys->keyDown[VK_RIGHT] == TRUE)
		ropeConnectionVel.x += 3.0f;
	if (g_keys->keyDown[VK_LEFT] == TRUE)
		ropeConnectionVel.x -= 3.0f;
	if (g_keys->keyDown[VK_UP] == TRUE)
		ropeConnectionVel.z -= 3.0f;
	if (g_keys->keyDown[VK_DOWN] == TRUE)
		ropeConnectionVel.z += 3.0f;
	if (g_keys->keyDown[VK_HOME] == TRUE)
		ropeConnectionVel.y += 3.0f;
	if (g_keys->keyDown[VK_END] == TRUE)
		ropeConnectionVel.y -= 3.0f;

	ropeSimulation->setRopeConnectionVel(ropeConnectionVel);

	float dt = milliseconds / 1000.0f;
	float maxPossible_dt = 0.002f;                     // Maximum possible dt is 0.002 seconds
	
	int numOfIteration = (int)(dt / maxPossible_dt) + 1;
	if (numOfIteration != 0) {
		dt /= numOfIteration;
	}

	for (int i = 0; i < numOfIteration; ++i) {
		ropeSimulation->operate(dt);
	}
}

void Draw(void)
{
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(0, 0, 4, 0, 0, 0, 0, 1, 0);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glBegin(GL_QUADS);
		glColor3ub(0, 0, 255);
		glVertex3f(20, ropeSimulation->groundHeight, 20);
		glVertex3f(-20, ropeSimulation->groundHeight, 20);
		glColor3ub(0, 0, 0);
		glVertex3f(-20, ropeSimulation->groundHeight, -20);
		glVertex3f(20, ropeSimulation->groundHeight, -20);
	glEnd();
	// Shadow of rope
	glColor3ub(0, 0, 0);
	for (int i = 0; i < ropeSimulation->numOfMasses - 1; ++i) {
		Mass* mass1 = ropeSimulation->getMass(i);
		Vector3D* pos1 = &mass1->pos;

		Mass* mass2 = ropeSimulation->getMass(i + 1);
		Vector3D* pos2 = &mass2->pos;

		glLineWidth(2);
		glBegin(GL_LINES);
			glVertex3f(pos1->x, ropeSimulation->groundHeight, pos1->z);
			glVertex3f(pos2->x, ropeSimulation->groundHeight, pos2->z);
		glEnd();
	}

	glColor3ub(255, 255, 0);
	for (int i = 0; i < ropeSimulation->numOfMasses - 1; ++i) {
		Mass* mass1 = ropeSimulation->getMass(i);
		Vector3D* pos1 = &mass1->pos;

		Mass* mass2 = ropeSimulation->getMass(i + 1);
		Vector3D* pos2 = &mass2->pos;

		glLineWidth(4);
		glBegin(GL_LINES);
			glVertex3f(pos1->x, pos1->y, pos1->z);
			glVertex3f(pos2->x, pos2->y, pos2->z);
		glEnd();
	}
	glFlush();
}