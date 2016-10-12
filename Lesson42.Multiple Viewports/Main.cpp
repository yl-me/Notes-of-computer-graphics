#include <Windows.h>
#include <GL/glew.h>
#include <GL/glut.h>
#include <GL/GLUAX.H>
#include <math.h>
#include <stdio.h>
#include "Previous.h"
// FreeType Fonts in OpenGL
#pragma comment(lib, "legacy_stdio_definitions.lib")

#ifndef CDS_FULLSCREEN
#define CDS_FULLSCREEN 4
#endif

GL_Window* g_window;
Keys* g_keys;

int mx, my;             // Used for loops
const int width = 128;       // Maze
const int height = 128;

BOOL done;              // Flag to let us know when it's done
BOOL sp;                // Spacebar pressed?

BYTE r[4], g[4], b[4];   // Random color
BYTE* tex_data;

GLfloat xrot, yrot, zrot;    // Rotation
GLUquadricObj* quadric;      // The quadric object

void UpdateTex(int dmx, int dmy)            // Update pixel dmx, dmy on the texture
{
	tex_data[0 + ((dmx + (width * dmy)) * 3)] = 255;      // Set red pixel to full bright
	tex_data[1 + ((dmx + (width * dmy)) * 3)] = 255;      // Green
	tex_data[2 + ((dmx + (width * dmy)) * 3)] = 255;      // Blue
}

void Reset(void)                        // Reset the maze,colors,start point,etc
{
	ZeroMemory(tex_data, width * height * 3);
	srand(GetTickCount());

	for (int loop = 0; loop < 4; ++loop) {         // Color
		r[loop] = rand() % 128 + 128;
		g[loop] = rand() % 128 + 128;
		b[loop] = rand() % 128 + 128;
	}

	mx = int(rand() % (width / 2)) * 2;                // Position
	my = int(rand() % (height / 2)) * 2;
}

BOOL Initialize(GL_Window* window, Keys* keys)
{
	tex_data = new BYTE[width * height * 3];
	g_window = window;
	g_keys = keys;

	Reset();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, tex_data);

	glEnable(GL_TEXTURE_2D);
	glClearColor(0.0f, 0.0f, 0.0f, 0.5f);
	glClearDepth(1.0f);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_COLOR_MATERIAL);                // Enable color material
	
	quadric = gluNewQuadric();                  // Create a pointer to the quadric bject
	gluQuadricNormals(quadric, GLU_SMOOTH);     // Create smooth normal
	gluQuadricTexture(quadric, GL_TRUE);        // Create texture coords
	glEnable(GL_LIGHT0);

	return TRUE;
}

void Deinitialize(void)
{
	delete[] tex_data;
}

void Update(DWORD milliseconds)
{
	if (g_keys->keyDown[VK_ESCAPE] == TRUE)
		TerminateApplication(g_window);
	if (g_keys->keyDown[VK_F1] == TRUE)
		ToggleFullscreen(g_window);
	if (g_keys->keyDown[' '] && sp) {
		sp = TRUE;
		Reset();
	}
	if (!g_keys->keyDown[' '])
		sp = FALSE;

	int dir;          // Direction

	xrot += (float)(milliseconds)* 0.02f;
	yrot += (float)(milliseconds)* 0.03f;
	zrot += (float)(milliseconds)* 0.015f;

	done = TRUE;
	for (int x = 0; x < width; x+=2) {
		for (int y = 0; y < height; y+=2) {
			if (tex_data[((x + (width * y)) * 3)] == 0)
				done = FALSE;                        // Check
		}
	}
	// Title
	if (done) {
		// Display a message at the top of the window, pause for a bit and then start building a new maze
		SetWindowText(g_window->hWnd, "Multiple Viewports...Maze Complete!");
		Sleep(5000);
		SetWindowText(g_window->hWnd, "Multiple Viewports...Building Maze!");
		Reset();
	}

	if (((mx > (width - 4) || tex_data[(((mx + 2) + (width*my)) * 3)] == 255)) &&
		((mx < 2 || tex_data[(((mx - 2) + (width*my)) * 3)] == 255)) &&
		((my >(height - 4) || tex_data[((mx + (width*(my + 2))) * 3)] == 255)) &&
		((my < 2 || tex_data[((mx + (width*(my - 2))) * 3)] == 255)))
	{
		do {
			mx = int(rand() % (width / 2)) * 2;
			my = int(rand() % (width / 2)) * 2;
		} while (tex_data[((mx + (width*my)) * 3)] == 0);
	}

	dir = int(rand() % 4);           // Pick a random direction
	if ((dir == 0) && (mx <= (width - 4))) {
		if (tex_data[(((mx + 2) + (width * my)) * 3)] == 0) {
			UpdateTex(mx + 1, my);  // Update the texture to show path cut out between rooms
			mx += 2;                // Move to the right
		}
	}
	if ((dir == 1) && (my <= (width - 4))) {
		if (tex_data[((mx + (width * (my + 2))) * 3)] == 0) {
			UpdateTex(mx, my + 1);
			my += 2;
		}
	}
	if ((dir == 2) && (mx >= 2)) {
		if (tex_data[(((mx - 2) + (width * my)) * 3)] == 0) {
			UpdateTex(mx - 1, my);
			mx -= 2;
		}
	}
	if ((dir == 3) && (my >= 2)) {
		if (tex_data[((mx + (width * (my - 2))) * 3)] == 0) {
			UpdateTex(mx, my - 1);
			my -= 2;
		}
	}

	UpdateTex(mx, my);
}

void Draw(void)
{
	RECT rect;
	GetClientRect(g_window->hWnd, &rect);            // Get window dimensions
	int window_width = rect.right - rect.left;
	int window_height = rect.bottom - rect.top;

	// Update texture
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, tex_data);

	glClear(GL_COLOR_BUFFER_BIT);
	
	for (int loop = 0; loop < 4; ++loop) {
		glColor3ub(r[loop], g[loop], b[loop]);            // Assign color to current view

		if (loop == 0) {
			glViewport(0, window_height / 2, window_width / 2, window_height / 2);
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			gluOrtho2D(0, window_width / 2, window_height / 2, 0);
		}
		if (loop == 1) {
			glViewport(window_width / 2, window_height / 2, window_width / 2, window_height / 2);
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			gluPerspective(45.0f, (GLfloat)(width) / (GLfloat)(height), 0.1f, 500.0f);
		}
		if (loop == 2) {
			glViewport(window_width / 2, 0, window_width / 2, window_height / 2);
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			gluPerspective(45.0f, (GLfloat)(width) / (GLfloat)(height), 0.1f, 500.0f);
		}
		if (loop == 3) {
			glViewport(0, 0, window_width / 2, window_height / 2);
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			gluPerspective(45.0f, (GLfloat)(width) / (GLfloat)(height), 0.1f, 500.0f);
		}

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glClear(GL_DEPTH_BUFFER_BIT);

		if (loop == 0) {
			glBegin(GL_QUADS);
				glTexCoord2f(1.0f, 0.0f); glVertex2f(window_width / 2, 0);
				glTexCoord2f(0.0f, 0.0f); glVertex2f(0, 0);
				glTexCoord2f(0.0f, 1.0f); glVertex2f(0, window_width / 2);
				glTexCoord2f(1.0f, 1.0f); glVertex2f(window_width / 2, window_width / 2);
			glEnd();
		}
		if (loop == 1) {
			glTranslatef(0.0f, 0.0f, -14.0f);
			glRotatef(xrot, 1.0f, 0.0f, 0.0f);
			glRotatef(yrot, 0.0f, 1.0f, 0.0f);
			glRotatef(zrot, 0.0f, 0.0f, 1.0f);

			glEnable(GL_LIGHTING);
			gluSphere(quadric, 4.0f, 32, 32);
			glDisable(GL_LIGHTING);
		}
		if (loop == 2) {
			glTranslatef(0.0f, 0.0f, -2.0f);
			glRotatef(-45.0f, 1.0f, 0.0f, 0.0f);
			glRotatef(zrot / 1.5f, 0.0f, 0.0f, 1.0f);
			glBegin(GL_QUADS);
				glTexCoord2f(1.0f, 1.0f); glVertex3f(1.0f, 1.0f, 0.0f);
				glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f, 1.0f, 0.0f);
				glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f, 0.0f);
				glTexCoord2f(1.0f, 0.0f); glVertex3f(1.0f, -1.0f, 0.0f);
			glEnd();
		}
		if (loop == 3) {
			glTranslatef(0.0f, 0.0f, -7.0f);
			glRotatef(-xrot / 2, 1.0f, 0.0f, 0.0f);
			glRotatef(-yrot / 2, 0.0f, 1.0f, 0.0f);
			glRotatef(-zrot / 2, 0.0f, 0.0f, 1.0f);
			glEnable(GL_LIGHTING);
			glTranslatef(0.0f, 0.0f, -2.0f);
			gluCylinder(quadric, 1.5f, 1.5f, 4.0f, 32, 16);
			glDisable(GL_LIGHTING);
		}
	}
	glFlush();
}