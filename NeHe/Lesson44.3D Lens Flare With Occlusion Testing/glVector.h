#ifndef AFX_GLVECTOR_H__F526A5CF_89B5_4F20_8F2C_517D83879D35__INCLUDED_
#define AFX_GLVECTOR_H__F526A5CF_89B5_4F20_8F2C_517D83879D35__INCLUDED_

#include <Windows.h>
#include <GL\glew.h>
#include <GL\glut.h>
#include <math.h>

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class glVector {
public:
	glVector();
	virtual ~glVector();

public:
	void operator=(glVector v);
	glVector operator+(glVector v);
	glVector operator*(GLfloat scalar);
	void Normalize(void);
	GLfloat Magnitude(void);
	GLfloat m_Msg;
	void operator*=(GLfloat scalar);
	
	GLfloat i;
	GLfloat j;
	GLfloat k;
};

#endif // !AFX_GLVECTOR_H__F526A5CF_89B5_4F20_8F2C_517D83879D35__INCLUDED_
