#include <windows.h>
#include <stdio.h>
#include <math.h>
#include <gl/glew.h>
#include <gl/glut.h>
#include <GL/GLUAX.H>
#include <mmsystem.h>
#include "Previous.h"
#include "ArcBall.h"

#pragma comment(lib, "legacy_stdio_definitions.lib")

GL_Window*	g_window;
Keys*		g_keys;

GLUquadricObj *quadratic;
const float PI2 = 2.0*3.1415926535f;

Matrix4fT Transform = { 1.0f,  0.0f,  0.0f,  0.0f, 
	0.0f,  1.0f,  0.0f,  0.0f,
	0.0f,  0.0f,  1.0f,  0.0f,
	0.0f,  0.0f,  0.0f,  1.0f };

Matrix3fT   LastRot = { 1.0f,  0.0f,  0.0f,
	0.0f,  1.0f,  0.0f,
	0.0f,  0.0f,  1.0f };

Matrix3fT   ThisRot = { 1.0f,  0.0f,  0.0f,
	0.0f,  1.0f,  0.0f,
	0.0f,  0.0f,  1.0f };

ArcBallT    ArcBall(640.0f, 480.0f);
Point2fT    MousePt;
bool        isClicked = false;
bool        isRClicked = false;
bool        isDragging = false;

BOOL Initialize(GL_Window* window, Keys* keys)
{
	g_window = window;
	g_keys = keys;
	
	isClicked = false;
	isDragging = false;

	glClearColor(0.0f, 0.0f, 0.0f, 0.5f);
	glClearDepth(1.0f);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_DEPTH_TEST);
	glShadeModel(GL_FLAT);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	quadratic = gluNewQuadric();
	gluQuadricNormals(quadratic, GLU_SMOOTH);
	gluQuadricTexture(quadratic, GL_TRUE);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHTING);
	glEnable(GL_COLOR_MATERIAL);

	return TRUE;
}

void Deinitialize(void)
{
	gluDeleteQuadric(quadratic);
}

void Update(DWORD milliseconds)
{
	if (g_keys->keyDown[VK_ESCAPE] == TRUE) {
		TerminateApplication(g_window);
	}
	if (g_keys->keyDown[VK_F1] == TRUE) {
		ToggleFullscreen(g_window);
	}
	if (isRClicked) {
		Matrix3fSetIdentity(&LastRot);
		Matrix3fSetIdentity(&ThisRot);
		Matrix4fSetRotationFromMatrix3f(&Transform, &ThisRot);
	}
	if (!isDragging) {
		if (isClicked){
			isDragging = true;
			LastRot = ThisRot;
			ArcBall.click(&MousePt);
		}
	}
	else {
		if (isClicked) {
			Quat4fT ThisQuat;

			ArcBall.drag(&MousePt, &ThisQuat);
			Matrix3fSetRotationFromQuat4f(&ThisRot, &ThisQuat);
			Matrix3fMulMatrix3f(&ThisRot, &LastRot);
			Matrix4fSetRotationFromMatrix3f(&Transform, &ThisRot);
		}
		else {
			isDragging = false;
		}
	}
}

void Torus(float MinorRadius, float MajorRadius)					// Draw a torus with normals
{
	int i, j;
	glBegin(GL_TRIANGLE_STRIP);
	for (i = 0; i<20; i++) {
		for (j = -1; j<20; j++) {
			float wrapFrac = (j % 20) / (float)20;
			float phi = PI2*wrapFrac;
			float sinphi = float(sin(phi));
			float cosphi = float(cos(phi));

			float r = MajorRadius + MinorRadius*cosphi;

			glNormal3f(float(sin(PI2*(i % 20 + wrapFrac) / (float)20))*cosphi, sinphi, float(cos(PI2*(i % 20 + wrapFrac) / (float)20))*cosphi);
			glVertex3f(float(sin(PI2*(i % 20 + wrapFrac) / (float)20))*r, MinorRadius*sinphi, float(cos(PI2*(i % 20 + wrapFrac) / (float)20))*r);

			glNormal3f(float(sin(PI2*(i + 1 % 20 + wrapFrac) / (float)20))*cosphi, sinphi, float(cos(PI2*(i + 1 % 20 + wrapFrac) / (float)20))*cosphi);
			glVertex3f(float(sin(PI2*(i + 1 % 20 + wrapFrac) / (float)20))*r, MinorRadius*sinphi, float(cos(PI2*(i + 1 % 20 + wrapFrac) / (float)20))*r);
		}
	}
	glEnd();
}

void Draw(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	glTranslatef(-1.5f, 0.0f, -6.0f);
	glPushMatrix();
	glMultMatrixf(Transform.M);
	glColor3f(0.75f, 0.75f, 1.0f);
	Torus(0.30f, 1.00f);
	glPopMatrix();
	glLoadIdentity();
	glTranslatef(1.5f, 0.0f, -6.0f);
	glPushMatrix();
	glMultMatrixf(Transform.M);
	glColor3f(1.0f, 0.75f, 0.75f);
	gluSphere(quadratic, 1.3f, 20, 20);
	glPopMatrix();
	glFlush();
}