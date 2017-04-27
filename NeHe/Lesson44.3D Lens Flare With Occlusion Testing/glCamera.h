#ifndef AFX_GLCAMERA_H__8E3CD02E_6D82_437E_80DA_50023C60C146__INCLUDED_
#define AFX_GLCAMERA_H__8E3CD02E_6D82_437E_80DA_50023C60C146__INCLUDED_

#include <windows.h>
#include <GL\glew.h>
#include <gl\glut.h>
#include <math.h>
#include "glVector.h"
#include "glPoint.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class glCamera {
public:
	glCamera();
	virtual ~glCamera();

	BOOL SphereInFrustum(glPoint p, GLfloat Radius);
	BOOL SphereInFrustum(GLfloat x, GLfloat y, GLfloat z, GLfloat Radius);
	BOOL PointInFrustum(GLfloat x, GLfloat y, GLfloat z);
	BOOL PointInFrustum(glPoint p);

	void RenderLensFlare(void);                                  // Render lens flare
	void RenderStreaks(GLfloat r, GLfloat g, GLfloat b, GLfloat a, glPoint p, GLfloat scale);
	void RenderBigGlow(GLfloat r, GLfloat g, GLfloat b, GLfloat a, glPoint p, GLfloat scale);
	void RenderGlow(GLfloat r, GLfloat g, GLfloat b, GLfloat a, glPoint p, GLfloat scale);
	void RenderHalo(GLfloat r, GLfloat g, GLfloat b, GLfloat a, glPoint p, GLfloat scale);

	void UpdateFrustumFaster(void);            // Update the frustum
	void UpdateFrustum(void);

	void ChangeVelocity(GLfloat vel);           // Change volcity and speed
	void ChangeHeading(GLfloat degrees);
	void ChangePitch(GLfloat degrees);
	void SetPrespective(void);

	bool glCamera::IsOccluded(glPoint p);                // Occlusion

	glVector vLightSourceToCamera, vLightSourceToIntersect;
	glPoint ptIntersect, pt;
	GLsizei m_WindowHeight;
	GLsizei m_WindowWidth;
	GLuint m_StreakTexture;
	GLuint m_HaloTexture;
	GLuint m_GlowTexture;
	GLuint m_BigGlowTexture;
	GLfloat m_MaxPointSize;
	GLfloat m_Frustum[6][4];
	glPoint m_LightSourcePos;
	GLfloat m_MaxPitchRate;
	GLfloat m_MaxHeadingRate;
	GLfloat m_HeadingDegrees;
	GLfloat m_PitchDegrees;
	GLfloat m_MaxForwardVelocity;
	GLfloat m_ForwardVelocity;
	glPoint m_Position;
	glVector m_DirectionVector;
};

#endif // !AFX_GLCAMERA_H__8E3CD02E_6D82_437E_80DA_50023C60C146__INCLUDED_
