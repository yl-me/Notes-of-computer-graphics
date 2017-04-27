#include "glFont.h"
#include <stdio.h>
#pragma comment(lib, "legacy_stdio_definitions.lib")

glFont::glFont()
{
	m_FontTexture = 0;
	m_ListBase = 0;
}

glFont::~glFont()
{
	if (m_FontTexture != 0) {
		glDeleteTextures(1, &m_FontTexture);
	}
	if (m_ListBase != 0) {
		glDeleteLists(m_ListBase, 256);
	}
}

void glFont::SetFontTexture(GLuint tex)
{
	if (tex != 0) {
		m_FontTexture = tex;
	}
}

void glFont::BuildFont(GLfloat Scale)
{
	float cx;
	float cy;

	m_ListBase = glGenLists(256);
	if (m_FontTexture != 0) {
		glBindTexture(GL_TEXTURE_2D, m_FontTexture);
		for (GLuint loop = 0; loop < 256; ++loop) {
			cx = float(loop % 16) / 16.0f;
			cy = float(loop / 16) / 16.0f;

			glNewList(m_ListBase + loop, GL_COMPILE);            // Building a list
			glBegin(GL_QUADS);
				glTexCoord2f(cx, 1 - cy - 0.0625f);
				glVertex2f(0, 0);
				glTexCoord2f(cx + 0.0625f, 1 - cy - 0.0625f);
				glVertex2f(16 * Scale, 0);
				glTexCoord2f(cx + 0.0625f, 1 - cy);
				glVertex2f(16 * Scale, 16 * Scale);
				glTexCoord2f(cx, 1 - cy);
				glVertex2f(0, 16 * Scale);
			glEnd();
			glTranslatef(10 * Scale, 0, 0);
			glEndList();
		}
	}
}

void glFont::glPrintf(GLint x, GLint y, GLint set, const char* Format, ...)
{
	char text[256];
	va_list ap;

	if (Format == NULL) {
		return;
	}
	va_start(ap, Format);
	vsprintf(text, Format, ap);
	va_end(ap);

	if (set > 1) {
		set = 1;
	}
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR);
	glBindTexture(GL_TEXTURE_2D, m_FontTexture);
	glDisable(GL_DEPTH_TEST);
	
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	
	glOrtho(0, m_WindowWidth, 0, m_WindowHeight, -1, 1);           // Set up an ortho screen
	
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glTranslatef(x, y, 0);
	glListBase(m_ListBase - 32 + (128 * set));
	glCallLists(strlen(text), GL_BYTE, text);
	
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	glDisable(GL_TEXTURE_2D);
}

void glFont::SetWindowSize(GLint width, GLint height)
{
	m_WindowWidth = width;
	m_WindowHeight = height;
}

GLuint glFont::GetTexture()
{
	GLuint result = m_FontTexture;
	return result;
}

GLuint glFont::GetListBase()
{
	GLuint result = m_ListBase;
	return result;
}