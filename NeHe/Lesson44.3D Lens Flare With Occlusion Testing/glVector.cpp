#include "glVector.h"

glVector::glVector() : i(0), j(0), k(0) {}

glVector::~glVector() {}

void glVector::operator*=(GLfloat scalar)
{
	i *= scalar;
	j *= scalar;
	k *= scalar;
}

GLfloat glVector::Magnitude()
{
	GLfloat result;
	result = GLfloat(sqrt(i * i + j * j + k * k));
	m_Msg = result;
	return result;
}

void glVector::Normalize()
{
	if (m_Msg != 0.0f) {
		i /= m_Msg;
		j /= m_Msg;
		k /= m_Msg;
		Magnitude();
	}
}

glVector glVector::operator*(GLfloat scalar)
{
	glVector r;
	r.i = i * scalar;
	r.j = j * scalar;
	r.k = k * scalar;

	return r;
}

glVector glVector::operator+(glVector v)
{
	glVector r;
	r.i = i + v.i;
	r.j = j + v.j;
	r.k = k + v.k;

	return r;
}

void glVector::operator=(glVector v)
{
	i = v.i;
	j = v.j;
	k = v.k;
	m_Msg = v.m_Msg;
}