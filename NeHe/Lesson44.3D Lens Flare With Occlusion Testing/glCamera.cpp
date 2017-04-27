#include "glCamera.h"

glCamera::glCamera()
{
	m_MaxPitchRate             = 0.0f;
	m_MaxHeadingRate           = 0.0f;
	m_HeadingDegrees           = 0.0f;
	m_PitchDegrees             = 0.0f;
	m_MaxForwardVelocity       = 0.0f;
	m_ForwardVelocity          = 0.0f;
	m_LightSourcePos.x         = 0.0f;
	m_LightSourcePos.y         = 0.0f;
	m_LightSourcePos.z         = 0.0f;
	m_GlowTexture              = 0;
	m_BigGlowTexture           = 0;
	m_HaloTexture              = 0;
	m_StreakTexture            = 0;
	m_MaxPointSize             = 0.0f;
}

glCamera::~glCamera()
{
	if (m_GlowTexture != 0) {
		glDeleteTextures(1, &m_GlowTexture);
	}
	if (m_HaloTexture != 0) {
		glDeleteTextures(1, &m_HaloTexture);
	}
	if (m_BigGlowTexture != 0) {
		glDeleteTextures(1, &m_BigGlowTexture);
	}
	if (m_StreakTexture != 0) {
		glDeleteTextures(1, &m_StreakTexture);
	}
}

void glCamera::SetPrespective()
{
	GLfloat Matrix[16];
	glVector v;

	glRotatef(m_HeadingDegrees, 0.0f, 1.0f, 0.0f);
	glRotatef(m_PitchDegrees, 1.0f, 0.0f, 0.0f);

	glGetFloatv(GL_MODELVIEW_MATRIX, Matrix);

	m_DirectionVector.i = Matrix[8];
	m_DirectionVector.j = Matrix[9];
	m_DirectionVector.k = -Matrix[10];

	glLoadIdentity();

	glRotatef(m_PitchDegrees, 1.0f, 0.0f, 0.0f);
	glRotatef(m_HeadingDegrees, 0.0f, 1.0f, 0.0f);

	// Scale the direction by speed
	v = m_DirectionVector;
	v *= m_ForwardVelocity;

	m_Position.x += v.i;
	m_Position.y += v.j;
	m_Position.z += v.k;

	glTranslatef(-m_Position.x, -m_Position.y, -m_Position.z);
}

void glCamera::ChangePitch(GLfloat degrees)
{
	if (fabs(degrees) < fabs(m_MaxPitchRate)) {
		m_PitchDegrees += degrees;
	}
	else {
		if (degrees < 0) {
			m_PitchDegrees -= m_MaxPitchRate;
		}
		else {
			m_PitchDegrees += m_MaxPitchRate;
		}
	}

	if (m_PitchDegrees > 360.0f) {
		m_PitchDegrees -= 360.0f;
	}
	else if (m_PitchDegrees < -360.0f) {
		m_PitchDegrees += 360.0f;
	}
}

void glCamera::ChangeHeading(GLfloat degrees)
{
	if (fabs(degrees) < fabs(m_MaxHeadingRate)) {
		if ((m_PitchDegrees > 90 && m_PitchDegrees < 270) || 
			(m_PitchDegrees < -90 && m_PitchDegrees > -270))
		{
			m_HeadingDegrees -= degrees;
		}
		else {
			m_HeadingDegrees += degrees;
		}
	}
	else {
		if (degrees < 0) {
			if ((m_PitchDegrees > 90 && m_PitchDegrees < 270) ||
				(m_PitchDegrees < -90 && m_PitchDegrees > -270))
			{
				m_HeadingDegrees += m_MaxHeadingRate;
			}
			else {
				m_HeadingDegrees -= m_MaxHeadingRate;
			}
		}
		else {
			if ((m_PitchDegrees > 90 && m_PitchDegrees < 270) ||
				(m_PitchDegrees < -90 && m_PitchDegrees > -270))
			{
				m_HeadingDegrees -= m_MaxHeadingRate;
			}
			else {
				m_HeadingDegrees += m_MaxHeadingRate;
			}
		}
	}

	if (m_HeadingDegrees > 360.0f)
	{
		m_HeadingDegrees -= 360.0f;
	}
	else if (m_HeadingDegrees < -360.0f)
	{
		m_HeadingDegrees += 360.0f;
	}
}

void glCamera::ChangeVelocity(GLfloat vel)
{
	if (fabs(vel) < fabs(m_MaxForwardVelocity)) {
		m_ForwardVelocity += vel;
	}
	else {
		if (vel < 0) {
			m_ForwardVelocity -= -m_MaxForwardVelocity;
		}
		else {
			m_ForwardVelocity += m_MaxForwardVelocity;
		}
	}
}

void glCamera::UpdateFrustum()
{
	GLfloat clip[16];
	GLfloat proj[16];
	GLfloat modl[16];
	GLfloat t;

	// Get current matrix
	glGetFloatv(GL_PROJECTION_MATRIX, proj);
	glGetFloatv(GL_MODELVIEW_MATRIX, modl);

	// Multiply
	clip[0] = modl[0] * proj[0] + modl[1] * proj[4] + modl[2] * proj[8] + modl[3] * proj[12];
	clip[1] = modl[0] * proj[1] + modl[1] * proj[5] + modl[2] * proj[9] + modl[3] * proj[13];
	clip[2] = modl[0] * proj[2] + modl[1] * proj[6] + modl[2] * proj[10] + modl[3] * proj[14];
	clip[3] = modl[0] * proj[3] + modl[1] * proj[7] + modl[2] * proj[11] + modl[3] * proj[15];

	clip[4] = modl[4] * proj[0] + modl[5] * proj[4] + modl[6] * proj[8] + modl[7] * proj[12];
	clip[5] = modl[4] * proj[1] + modl[5] * proj[5] + modl[6] * proj[9] + modl[7] * proj[13];
	clip[6] = modl[4] * proj[2] + modl[5] * proj[6] + modl[6] * proj[10] + modl[7] * proj[14];
	clip[7] = modl[4] * proj[3] + modl[5] * proj[7] + modl[6] * proj[11] + modl[7] * proj[15];

	clip[8] = modl[8] * proj[0] + modl[9] * proj[4] + modl[10] * proj[8] + modl[11] * proj[12];
	clip[9] = modl[8] * proj[1] + modl[9] * proj[5] + modl[10] * proj[9] + modl[11] * proj[13];
	clip[10] = modl[8] * proj[2] + modl[9] * proj[6] + modl[10] * proj[10] + modl[11] * proj[14];
	clip[11] = modl[8] * proj[3] + modl[9] * proj[7] + modl[10] * proj[11] + modl[11] * proj[15];

	clip[12] = modl[12] * proj[0] + modl[13] * proj[4] + modl[14] * proj[8] + modl[15] * proj[12];
	clip[13] = modl[12] * proj[1] + modl[13] * proj[5] + modl[14] * proj[9] + modl[15] * proj[13];
	clip[14] = modl[12] * proj[2] + modl[13] * proj[6] + modl[14] * proj[10] + modl[15] * proj[14];
	clip[15] = modl[12] * proj[3] + modl[13] * proj[7] + modl[14] * proj[11] + modl[15] * proj[15];

	// Extract the numbers for the right plane
	m_Frustum[0][0] = clip[3] - clip[0];
	m_Frustum[0][1] = clip[7] - clip[4];
	m_Frustum[0][2] = clip[11] - clip[8];
	m_Frustum[0][3] = clip[15] - clip[12];

	// Normal
	t = GLfloat(sqrt(m_Frustum[0][0] * m_Frustum[0][0] + m_Frustum[0][1] * m_Frustum[0][1] +
		m_Frustum[0][2] * m_Frustum[0][2]));
	m_Frustum[0][0] /= t;
	m_Frustum[0][1] /= t;
	m_Frustum[0][2] /= t;
	m_Frustum[0][3] /= t;

	// Extract the numbers for the left plane
	m_Frustum[1][0] = clip[3] + clip[0];
	m_Frustum[1][1] = clip[7] + clip[4];
	m_Frustum[1][2] = clip[11] + clip[8];
	m_Frustum[1][3] = clip[15] + clip[12];

	// Normal
	t = GLfloat(sqrt(m_Frustum[1][0] * m_Frustum[1][0] + m_Frustum[1][1] * m_Frustum[1][1] +
		m_Frustum[1][2] * m_Frustum[1][2]));
	m_Frustum[1][0] /= t;
	m_Frustum[1][1] /= t;
	m_Frustum[1][2] /= t;
	m_Frustum[1][3] /= t;

	// Extract the numbers for the bottm plane
	m_Frustum[2][0] = clip[3] + clip[1];
	m_Frustum[2][1] = clip[7] + clip[5];
	m_Frustum[2][2] = clip[11] + clip[9];
	m_Frustum[2][3] = clip[15] + clip[13];

	// Normal
	t = GLfloat(sqrt(m_Frustum[2][0] * m_Frustum[2][0] + m_Frustum[2][1] * m_Frustum[2][1] +
		m_Frustum[2][2] * m_Frustum[2][2]));
	m_Frustum[2][0] /= t;
	m_Frustum[2][1] /= t;
	m_Frustum[2][2] /= t;
	m_Frustum[2][3] /= t;

	// Extract the numbers for the top plane
	m_Frustum[3][0] = clip[3] - clip[1];
	m_Frustum[3][1] = clip[7] - clip[5];
	m_Frustum[3][2] = clip[11] - clip[9];
	m_Frustum[3][3] = clip[15] - clip[13];

	// Normal
	t = GLfloat(sqrt(m_Frustum[3][0] * m_Frustum[3][0] + m_Frustum[3][1] * m_Frustum[3][1] +
		m_Frustum[3][2] * m_Frustum[3][2]));
	m_Frustum[3][0] /= t;
	m_Frustum[3][1] /= t;
	m_Frustum[3][2] /= t;
	m_Frustum[3][3] /= t;

	// Extract the numbers for the far plane
	m_Frustum[4][0] = clip[3] - clip[2];
	m_Frustum[4][1] = clip[7] - clip[6];
	m_Frustum[4][2] = clip[11] - clip[10];
	m_Frustum[4][3] = clip[15] - clip[14];

	// Normal
	t = GLfloat(sqrt(m_Frustum[4][0] * m_Frustum[4][0] + m_Frustum[4][1] * m_Frustum[4][1] +
		m_Frustum[4][2] * m_Frustum[4][2]));
	m_Frustum[4][0] /= t;
	m_Frustum[4][1] /= t;
	m_Frustum[4][2] /= t;
	m_Frustum[4][3] /= t;

	// Extract the numbers for the near plane
	m_Frustum[5][0] = clip[3] + clip[2];
	m_Frustum[5][1] = clip[7] + clip[6];
	m_Frustum[5][2] = clip[11] + clip[10];
	m_Frustum[5][3] = clip[15] + clip[14];

	// Normal
	t = GLfloat(sqrt(m_Frustum[5][0] * m_Frustum[5][0] + m_Frustum[5][1] * m_Frustum[5][1] +
		m_Frustum[5][2] * m_Frustum[5][2]));
	m_Frustum[5][0] /= t;
	m_Frustum[5][1] /= t;
	m_Frustum[5][2] /= t;
	m_Frustum[5][3] /= t;
}

void glCamera::UpdateFrustumFaster()
{
	GLfloat clip[16];
	GLfloat proj[16];
	GLfloat modl[16];
	GLfloat t;

	glGetFloatv(GL_PROJECTION_MATRIX, proj);
	glGetFloatv(GL_MODELVIEW_MATRIX, modl);

	// Multiply
	// This function will only work if you do not rotate or translate your projection matrix
	clip[0] = modl[0] * proj[0];
	clip[1] = modl[1] * proj[5];
	clip[2] = modl[2] * proj[10] + modl[3] * proj[14];
	clip[3] = modl[2] * proj[11];

	clip[4] = modl[4] * proj[0];
	clip[5] = modl[5] * proj[5];
	clip[6] = modl[6] * proj[10] + modl[7] * proj[14];
	clip[7] = modl[6] * proj[11];

	clip[8] = modl[8] * proj[0];
	clip[9] = modl[9] * proj[5];
	clip[10] = modl[10] * proj[10] + modl[11] * proj[14];
	clip[11] = modl[10] * proj[11];

	clip[12] = modl[12] * proj[0];
	clip[13] = modl[13] * proj[5];
	clip[14] = modl[14] * proj[10] + modl[15] * proj[14];
	clip[15] = modl[14] * proj[11];

	// Extract the numbers for the right plane
	m_Frustum[0][0] = clip[3] - clip[0];
	m_Frustum[0][1] = clip[7] - clip[4];
	m_Frustum[0][2] = clip[11] - clip[8];
	m_Frustum[0][3] = clip[15] - clip[12];

	// Normal
	t = GLfloat(sqrt(m_Frustum[0][0] * m_Frustum[0][0] + m_Frustum[0][1] * m_Frustum[0][1] +
		m_Frustum[0][2] * m_Frustum[0][2]));
	m_Frustum[0][0] /= t;
	m_Frustum[0][1] /= t;
	m_Frustum[0][2] /= t;
	m_Frustum[0][3] /= t;

	// Extract the numbers for the left plane
	m_Frustum[1][0] = clip[3] + clip[0];
	m_Frustum[1][1] = clip[7] + clip[4];
	m_Frustum[1][2] = clip[11] + clip[8];
	m_Frustum[1][3] = clip[15] + clip[12];

	// Normal
	t = GLfloat(sqrt(m_Frustum[1][0] * m_Frustum[1][0] + m_Frustum[1][1] * m_Frustum[1][1] +
		m_Frustum[1][2] * m_Frustum[1][2]));
	m_Frustum[1][0] /= t;
	m_Frustum[1][1] /= t;
	m_Frustum[1][2] /= t;
	m_Frustum[1][3] /= t;

	// Extract the numbers for the bottm plane
	m_Frustum[2][0] = clip[3] + clip[1];
	m_Frustum[2][1] = clip[7] + clip[5];
	m_Frustum[2][2] = clip[11] + clip[9];
	m_Frustum[2][3] = clip[15] + clip[13];

	// Normal
	t = GLfloat(sqrt(m_Frustum[2][0] * m_Frustum[2][0] + m_Frustum[2][1] * m_Frustum[2][1] +
		m_Frustum[2][2] * m_Frustum[2][2]));
	m_Frustum[2][0] /= t;
	m_Frustum[2][1] /= t;
	m_Frustum[2][2] /= t;
	m_Frustum[2][3] /= t;

	// Extract the numbers for the top plane
	m_Frustum[3][0] = clip[3] - clip[1];
	m_Frustum[3][1] = clip[7] - clip[5];
	m_Frustum[3][2] = clip[11] - clip[9];
	m_Frustum[3][3] = clip[15] - clip[13];

	// Normal
	t = GLfloat(sqrt(m_Frustum[3][0] * m_Frustum[3][0] + m_Frustum[3][1] * m_Frustum[3][1] +
		m_Frustum[3][2] * m_Frustum[3][2]));
	m_Frustum[3][0] /= t;
	m_Frustum[3][1] /= t;
	m_Frustum[3][2] /= t;
	m_Frustum[3][3] /= t;

	// Extract the numbers for the far plane
	m_Frustum[4][0] = clip[3] - clip[2];
	m_Frustum[4][1] = clip[7] - clip[6];
	m_Frustum[4][2] = clip[11] - clip[10];
	m_Frustum[4][3] = clip[15] - clip[14];

	// Normal
	t = GLfloat(sqrt(m_Frustum[4][0] * m_Frustum[4][0] + m_Frustum[4][1] * m_Frustum[4][1] +
		m_Frustum[4][2] * m_Frustum[4][2]));
	m_Frustum[4][0] /= t;
	m_Frustum[4][1] /= t;
	m_Frustum[4][2] /= t;
	m_Frustum[4][3] /= t;

	// Extract the numbers for the near plane
	m_Frustum[5][0] = clip[3] + clip[2];
	m_Frustum[5][1] = clip[7] + clip[6];
	m_Frustum[5][2] = clip[11] + clip[10];
	m_Frustum[5][3] = clip[15] + clip[14];

	// Normal
	t = GLfloat(sqrt(m_Frustum[5][0] * m_Frustum[5][0] + m_Frustum[5][1] * m_Frustum[5][1] +
		m_Frustum[5][2] * m_Frustum[5][2]));
	m_Frustum[5][0] /= t;
	m_Frustum[5][1] /= t;
	m_Frustum[5][2] /= t;
	m_Frustum[5][3] /= t;
}

BOOL glCamera::SphereInFrustum(glPoint p, GLfloat Radius)
{
	for (int i = 0; i < 6; ++i) {
		if (m_Frustum[i][0] * p.x + m_Frustum[i][1] * p.y + m_Frustum[i][2] * p.z + m_Frustum[i][3] <= -Radius) {
			return FALSE;
		}
	}
	return TRUE;
}

BOOL glCamera::PointInFrustum(glPoint p)
{
	for (int i = 0; i < 6; ++i) {
		if (m_Frustum[i][0] * p.x + m_Frustum[i][1] * p.y + m_Frustum[i][2] * p.z + m_Frustum[i][3] <= 0) {
			return FALSE;
		}
	}
	return TRUE;
}

BOOL glCamera::SphereInFrustum(GLfloat x, GLfloat y, GLfloat z, GLfloat Radius)
{
	for (int i = 0; i < 6; ++i) {
		if (m_Frustum[i][0] * x + m_Frustum[i][1] * y + m_Frustum[i][2] * z + m_Frustum[i][3] <= -Radius) {
			return FALSE;
		}
	}
	return TRUE;
}

BOOL glCamera::PointInFrustum(GLfloat x, GLfloat y, GLfloat z)
{
	for (int i = 0; i < 6; ++i) {
		if (m_Frustum[i][0] * x + m_Frustum[i][1] * y + m_Frustum[i][2] * z + m_Frustum[i][3] <= 0) {
			return FALSE;
		}
	}
	return TRUE;
}

bool glCamera::IsOccluded(glPoint p)
{
	GLint viewport[4];
	GLdouble mvmatrix[16], projmatrix[16];
	GLdouble winx, winy, winz;
	GLdouble flareZ;                            // The transformed flare Z
	GLfloat bufferZ;                            // The read Z from the buffer

	glGetIntegerv(GL_VIEWPORT, viewport);
	glGetDoublev(GL_MODELVIEW_MATRIX, mvmatrix);
	glGetDoublev(GL_PROJECTION_MATRIX, projmatrix);

	// This asks OGL to guess the 2D position of a 3D point inside the viewport
	gluProject(p.x, p.y, p.z, mvmatrix, projmatrix, viewport, &winx, &winy, &winz);
	flareZ = winz;

	glReadPixels(winx, winy, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &bufferZ);

	// If the buffer Z is lower than our flare guessed Z then don't draw
	if (bufferZ < flareZ) {
		return true;
	}
	else {
		return false;
	}
}

void glCamera::RenderLensFlare()
{
	GLfloat Length = 0.0f;

	if (SphereInFrustum(m_LightSourcePos, 1.0f) == TRUE) {
		// Lets compute the vector that points to the camera from the light source
		vLightSourceToCamera = m_Position - m_LightSourcePos;

		Length = vLightSourceToCamera.Magnitude();
		ptIntersect = m_DirectionVector * Length;

		ptIntersect += m_Position;

		// Lets compute the vector that points to the Intersect point from the light source
		vLightSourceToIntersect = ptIntersect - m_LightSourcePos;
		Length = vLightSourceToIntersect.Magnitude();
		vLightSourceToIntersect.Normalize();

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_TEXTURE_2D);

		if (!IsOccluded(m_LightSourcePos)) {
			RenderBigGlow(0.60f, 0.60f, 0.8f, 1.0f, m_LightSourcePos, 16.0f);
			RenderStreaks(0.60f, 0.60f, 0.8f, 1.0f, m_LightSourcePos, 16.0f);
			
			RenderGlow(0.8f, 0.8f, 1.0f, 0.5f, m_LightSourcePos, 3.5f);
			/* Lets compute a point that is 20% away from the light source 
			*  in the direction of the intersection point.
			*/
			pt = vLightSourceToIntersect * (Length * 0.1f);
			pt += m_LightSourcePos;

			RenderGlow(0.9f, 0.6f, 0.4f, 0.5f, pt, 0.6f);
			/* Lets compute a point that is 30% away from the light source 
			*  in the direction of the intersection point
			*/
			pt = vLightSourceToIntersect * (Length * 0.15f);
			pt += m_LightSourcePos;

			RenderHalo(0.8f, 0.5f, 0.6f, 0.5f, pt, 1.7f);

			pt = vLightSourceToIntersect * (Length * 0.175f);  // 35%
			pt += m_LightSourcePos;

			RenderHalo(0.9f, 0.2f, 0.1f, 0.5f, pt, 0.83f);
			pt = vLightSourceToIntersect * (Length * 0.285f);  // 57%
			pt += m_LightSourcePos;

			RenderHalo(0.7f, 0.7f, 0.4f, 0.5f, pt, 1.6f);

			pt = vLightSourceToIntersect * (Length * 0.2755f);  // 55.1%
			pt += m_LightSourcePos;

			RenderGlow(0.9f, 0.9f, 0.2f, 0.5f, pt, 0.8f);

			pt = vLightSourceToIntersect * (Length * 0.4775f);  // 95.5%
			pt += m_LightSourcePos;

			RenderGlow(0.93f, 0.82f, 0.73f, 0.5f, pt, 1.0f);

			pt = vLightSourceToIntersect * (Length * 0.49f);    // 98%
			pt += m_LightSourcePos;

			RenderHalo(0.7f, 0.6f, 0.5f, 0.5f, pt, 1.4f);

			pt = vLightSourceToIntersect * (Length * 0.65f);  // 130%
			pt += m_LightSourcePos;

			RenderGlow(0.7f, 0.8f, 0.3f, 0.5f, pt, 1.8f);

			pt = vLightSourceToIntersect * (Length * 0.63f);  // 126%
			pt += m_LightSourcePos;

			RenderGlow(0.4f, 0.3f, 0.2f, 0.5f, pt, 1.4f);

			pt = vLightSourceToIntersect * (Length * 0.8f);  // 160%
			pt += m_LightSourcePos;

			RenderHalo(0.8f, 0.5f, 0.1f, 0.5f, pt, 0.6f);

			pt = vLightSourceToIntersect * (Length * 1.0f);  // 200%
			pt += m_LightSourcePos;

			RenderGlow(0.5f, 0.5f, 0.7f, 0.5f, pt, 1.7f);

			pt = vLightSourceToIntersect * (Length * 0.975f);  // 195%
			pt += m_LightSourcePos;

			RenderHalo(0.4f, 0.1f, 0.9f, 0.5f, pt, 2.0f);
		}
		glDisable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);
		glDisable(GL_TEXTURE_2D);
	}
}

void glCamera::RenderHalo(GLfloat r, GLfloat g, GLfloat b, GLfloat a, glPoint p, GLfloat scale)
{
	glPoint q[4];

	q[0].x = p.x - scale;
	q[0].y = p.y - scale;

	q[1].x = p.x - scale;
	q[1].y = p.y + scale;

	q[2].x = p.x + scale;
	q[2].y = p.y - scale;

	q[3].x = p.x + scale;
	q[3].y = p.y + scale;

	glPushMatrix();
	glTranslatef(p.x, p.y, p.z);
	glRotatef(-m_HeadingDegrees, 0.0f, 1.0f, 0.0f);
	glRotatef(-m_PitchDegrees, 1.0f, 0.0f, 0.0f);
	glBindTexture(GL_TEXTURE_2D, m_HaloTexture);
	glColor4f(r, g, b, a);

	glBegin(GL_TRIANGLE_STRIP);
		glTexCoord2f(0.0f, 0.0f);
		glVertex2f(q[0].x, q[0].y);
		glTexCoord2f(0.0f, 1.0f);
		glVertex2f(q[1].x, q[1].y);
		glTexCoord2f(1.0f, 0.0f);
		glVertex2f(q[2].x, q[2].y);
		glTexCoord2f(1.0f, 1.0f);
		glVertex2f(q[3].x, q[3].y);
	glEnd();
	glPopMatrix();
}

void glCamera::RenderGlow(GLfloat r, GLfloat g, GLfloat b, GLfloat a, glPoint p, GLfloat scale)
{
	glPoint q[4];

	q[0].x = p.x - scale;
	q[0].y = p.y - scale;

	q[1].x = p.x - scale;
	q[1].y = p.y + scale;

	q[2].x = p.x + scale;
	q[2].y = p.y - scale;

	q[3].x = p.x + scale;
	q[3].y = p.y + scale;

	glPushMatrix();
	glTranslatef(p.x, p.y, p.z);
	glRotatef(-m_HeadingDegrees, 0.0f, 1.0f, 0.0f);
	glRotatef(-m_PitchDegrees, 1.0f, 0.0f, 0.0f);
	glBindTexture(GL_TEXTURE_2D, m_GlowTexture);
	glColor4f(r, g, b, a);

	glBegin(GL_TRIANGLE_STRIP);
		glTexCoord2f(0.0f, 0.0f);
		glVertex2f(q[0].x, q[0].y);
		glTexCoord2f(0.0f, 1.0f);
		glVertex2f(q[1].x, q[1].y);
		glTexCoord2f(1.0f, 0.0f);
		glVertex2f(q[2].x, q[2].y);
		glTexCoord2f(1.0f, 1.0f);
		glVertex2f(q[3].x, q[3].y);
	glEnd();
	glPopMatrix();
}

void glCamera::RenderBigGlow(GLfloat r, GLfloat g, GLfloat b, GLfloat a, glPoint p, GLfloat scale)
{
	glPoint q[4];

	q[0].x = p.x - scale;
	q[0].y = p.y - scale;

	q[1].x = p.x - scale;
	q[1].y = p.y + scale;

	q[2].x = p.x + scale;
	q[2].y = p.y - scale;

	q[3].x = p.x + scale;
	q[3].y = p.y + scale;

	glPushMatrix();
	glTranslatef(p.x, p.y, p.z);
	glRotatef(-m_HeadingDegrees, 0.0f, 1.0f, 0.0f);
	glRotatef(-m_PitchDegrees, 1.0f, 0.0f, 0.0f);
	glBindTexture(GL_TEXTURE_2D, m_BigGlowTexture);
	glColor4f(r, g, b, a);

	glBegin(GL_TRIANGLE_STRIP);
		glTexCoord2f(0.0f, 0.0f);
		glVertex2f(q[0].x, q[0].y);
		glTexCoord2f(0.0f, 1.0f);
		glVertex2f(q[1].x, q[1].y);
		glTexCoord2f(1.0f, 0.0f);
		glVertex2f(q[2].x, q[2].y);
		glTexCoord2f(1.0f, 1.0f);
		glVertex2f(q[3].x, q[3].y);
	glEnd();
	glPopMatrix();
}

void glCamera::RenderStreaks(GLfloat r, GLfloat g, GLfloat b, GLfloat a, glPoint p, GLfloat scale)
{
	glPoint q[4];

	q[0].x = p.x - scale;
	q[0].y = p.y - scale;

	q[1].x = p.x - scale;
	q[1].y = p.y + scale;

	q[2].x = p.x + scale;
	q[2].y = p.y - scale;

	q[3].x = p.x + scale;
	q[3].y = p.y + scale;

	glPushMatrix();
	glTranslatef(p.x, p.x, p.z);
	glRotatef(-m_HeadingDegrees, 0.0f, 1.0f, 0.0f);
	glRotatef(-m_PitchDegrees, 1.0f, 0.0f, 0.0f);
	glBindTexture(GL_TEXTURE_2D, m_StreakTexture);
	glColor4f(r, g, b, a);

	glBegin(GL_TRIANGLE_STRIP);
		glTexCoord2f(0.0f, 0.0f);
		glVertex2f(q[0].x, q[0].y);
		glTexCoord2f(0.0f, 1.0f);
		glVertex2f(q[1].x, q[1].y);
		glTexCoord2f(1.0f, 0.0f);
		glVertex2f(q[2].x, q[2].y);
		glTexCoord2f(1.0f, 1.0f);
		glVertex2f(q[3].x, q[3].y);
	glEnd();
	glPopMatrix();
}