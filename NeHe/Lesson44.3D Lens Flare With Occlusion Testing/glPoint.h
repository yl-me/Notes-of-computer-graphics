#ifndef AFX_GLPOINT_H__ADADC708_0176_471A_8241_5DD4D700BCB2__INCLUDED_
#define AFX_GLPOINT_H__ADADC708_0176_471A_8241_5DD4D700BCB2__INCLUDED_

#include <Windows.h>
#include <GL\glew.h>
#include <GL\glut.h>
#include "glVector.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class glPoint {
public:
	glPoint();
	virtual ~glPoint();

public:
	void operator+=(glPoint p);
	glPoint operator+(glPoint p);
	glVector operator-(glPoint p);
	void operator=(glVector v);
	void operator=(glPoint p);

	GLfloat x;
	GLfloat y;
	GLfloat z;
};


#endif // !AFX_GLPOINT_H__ADADC708_0176_471A_8241_5DD4D700BCB2__INCLUDED_
