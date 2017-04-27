#ifndef AFX_GLFONT_H__F5069B5F_9D05_4832_8200_1EC9B4BFECE6__INCLUDED_
#define AFX_GLFONT_H__F5069B5F_9D05_4832_8200_1EC9B4BFECE6__INCLUDED_

#include <Windows.h>
#include <GL\glew.h>
#include <GL\glut.h>

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class glFont {
public:
	glFont();
	virtual ~glFont();

public:
	GLuint GetListBase(void);
	GLuint GetTexture(void);
	void SetWindowSize(GLint width, GLint height);
	void glPrintf(GLint x, GLint y, GLint set, const char* Format, ...);
	void BuildFont(GLfloat Scale = 1.0f);
	void SetFontTexture(GLuint tex);                    // Texture ID

protected:
	GLdouble m_WindowWidth;
	GLdouble m_WindowHeight;
	GLuint m_ListBase;
	GLuint m_FontTexture;
};


#endif // !AFX_GLFONT_H__F5069B5F_9D05_4832_8200_1EC9B4BFECE6__INCLUDED_
