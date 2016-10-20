#include "glPoint.h"

glPoint::glPoint() : x(0), y(0), z(0) {}

glPoint::~glPoint() {}

void glPoint::operator=(glPoint p)
{
	x = p.x;
	y = p.y;
	z = p.z;
}

void glPoint::operator=(glVector v)
{
	x = v.i;
	y = v.j;
	z = v.k;
}

glVector glPoint::operator-(glPoint p)
{
	glVector r;
	r.i = x - p.x;
	r.j = y - p.y;
	r.k = z - p.z;

	return r;
}

glPoint glPoint::operator+(glPoint p)
{
	glPoint r;
	r.x = x + p.x;
	r.y = y + p.y;
	r.z = z + p.z;

	return r;
}

void glPoint::operator+=(glPoint p)
{
	x += p.x;
	y += p.y;
	z += p.z;
}