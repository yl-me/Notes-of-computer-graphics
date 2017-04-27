#include <windows.h>
#include <stdio.h>
#include <math.h>
#include <gl/glew.h>
#include <gl/glut.h>
#include <GL/GLUAX.H>
#include <mmsystem.h>
#include "Previous.h"

#pragma comment(lib, "legacy_stdio_definitions.lib")

#ifndef CDS_FULLSCREEN
#define CDS_FULLSCREEN 4
#endif

#define MESH_RESOLUTION 4.0f            // Pixels per vertex
#define MESH_HEIGHTSCALE 1.0f           // Mesh height scale
#define GL_ARRAY_BUFFER_ARB 0x8892      // VBO extension definitions
#define GL_STATIC_DRAW_ARB 0x88E4

typedef void (APIENTRY * PFNGLBINDBUFFERARBPROC) (GLenum target, GLuint buffer);
typedef void (APIENTRY * PFNGLDELETEBUFFERSARBPROC) (GLsizei n, const GLuint *buffers);
typedef void (APIENTRY * PFNGLGENBUFFERSARBPROC) (GLsizei n, GLuint *buffers);
typedef void (APIENTRY * PFNGLBUFFERDATAARBPROC) (GLenum target, int size, const GLvoid *data, GLenum usage);

// VBO extension function pointers
PFNGLGENBUFFERSARBPROC glGenBuffersARB = NULL;                  // VBO name generation procedure
PFNGLBINDBUFFERARBPROC glBindBufferARB = NULL;                  // VBO bind procedure
PFNGLBUFFERDATAARBPROC glBufferDataARB = NULL;                  // VBO data loading procedure
PFNGLDELETEBUFFERSARBPROC glDeleteBuffersARB = NULL;                // VBO deletion procedure

class CVert {       // Vertex
public:
	float x;
	float y;
	float z;
};

class CTexCoord {     // Texture coordinate
public:
	float u;
	float v;
};

class CMesh {
public:
	// Mesh data
	int m_nVertexCount;                // Vertex count
	CVert* m_pVertices;                // Vertex data
	CTexCoord* m_pTexCoords;           // Texture coordinate
	unsigned int m_nTextureId;         // Texture ID
	// Vertex buffer object name 
	unsigned int m_nVBOVertices;       // Vertex VBO name
	unsigned int m_nVBOTexCoords;      // Texture coordinate VBO name
	// Temporary data
	AUX_RGBImageRec* m_pTextureImage;  // Heightmap data

public:
	CMesh();
	~CMesh();
	bool LoadHeightmap(char* szPath, float flHeightScale, float flResolution);
	float PtHeight(int nX, int nY);   // Single point height
	void BuildVBOs();     // Build function
};

bool g_fVBOSupported = false;
CMesh* g_pMesh = NULL;         // Mesh data
float g_flYRot = 0.0f;         // Rotation
float g_flXRot = 0.0f;
float DisX = 0.0f;
float DisZ = 0.0f;
int g_nFPS = 0, g_nFrames = 0;     // FPS and FPS counter
DWORD g_dwLastFPS = 0;            // Last FPS check time

GL_Window*	g_window;
Keys*		g_keys;

CMesh::CMesh()
{
	m_pTextureImage = NULL;
	m_pVertices = NULL;
	m_pTexCoords = NULL;
	m_nVertexCount = 0;
	m_nVBOVertices = m_nVBOTexCoords = m_nTextureId = 0;
}

CMesh :: ~CMesh()
{
	// Delete VBOs
	if (g_fVBOSupported) {
		unsigned int nBuffers[2] = { m_nVBOVertices, m_nVBOTexCoords };
		glDeleteBuffersARB(2, nBuffers);
	}

	if (m_pVertices) {
		delete[] m_pVertices;
	}
	m_pVertices = NULL;

	if (m_pTexCoords) {
		delete[] m_pTexCoords;
	}
	m_pTexCoords = NULL;
}

bool CMesh::LoadHeightmap(char* szPath, float flHeightScale, float flResolution)
{
	FILE* fTest = fopen(szPath, "r");
	if (!fTest) {
		return false;
	}
	fclose(fTest);
	m_pTextureImage = auxDIBImageLoad(szPath);

	// (( Terrain Width / Resolution ) * ( Terrain Length / Resolution ) * 
	// 3 Vertices in a Triangle * 2 Triangles in a Square )
	m_nVertexCount = 
		(int)(m_pTextureImage->sizeX * m_pTextureImage->sizeY * 6 / (flResolution * flResolution));
	
	m_pVertices = new CVert[m_nVertexCount];          // Vertex data
	m_pTexCoords = new CTexCoord[m_nVertexCount];     // Tex coord data

	int nX, nZ, nTri, nIndex = 0;
	float flX, flZ;

	for (nZ = 0; nZ < m_pTextureImage->sizeY; nZ += (int)flResolution) {
		for (nX = 0; nX < m_pTextureImage->sizeX; nX += (int)flResolution) {
			for (nTri = 0; nTri < 6; ++nTri) {
				flX = (float)nX + ((nTri == 1 || nTri == 2 || nTri == 5) ? flResolution : 0.0f);
				flZ = (float)nZ + ((nTri == 2 || nTri == 4 || nTri == 5) ? flResolution : 0.0f);

				// Using PtHeight to obtain the Y value
				m_pVertices[nIndex].x = flX - (m_pTextureImage->sizeX / 2);
				m_pVertices[nIndex].y = PtHeight((int)flX, (int)flZ) * flHeightScale;
				m_pVertices[nIndex].z = flZ - (m_pTextureImage->sizeY / 2);

				// Texture coordinate
				m_pTexCoords[nIndex].u = flX / m_pTextureImage->sizeX;
				m_pTexCoords[nIndex].v = flZ / m_pTextureImage->sizeY;

				nIndex++;
			}
		}
	}
	glGenTextures(1, &m_nTextureId);
	glBindTexture(GL_TEXTURE_2D, m_nTextureId);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, m_pTextureImage->sizeX, m_pTextureImage->sizeY, 0, 
		GL_RGB, GL_UNSIGNED_BYTE, m_pTextureImage->data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	if (m_pTextureImage) {
		if (m_pTextureImage->data) {
			delete[] m_pTextureImage->data;
		}
		delete[] m_pTextureImage;
	}
	return true;
}

float CMesh::PtHeight(int nX, int nY)
{
	// Calculate the position in the texture
	int nPos = ((nX % m_pTextureImage->sizeX) + 
		((nY % m_pTextureImage->sizeY) * m_pTextureImage->sizeX)) * 3;
	float flR = (float)m_pTextureImage->data[nPos];
	float flG = (float)m_pTextureImage->data[nPos + 1];
	float flB = (float)m_pTextureImage->data[nPos + 2];
	// Calculate the height using the luminance algorithm
	return (0.229f * flR + 0.587f * flG + 0.114f * flB);
}

// VBO: Vertex Buffer Objects use high-performance graphics card memory 
// instead of your standard, ram-allocated memory
void CMesh::BuildVBOs()
{
	// Generate and bind the vertex buffer
	glGenBuffersARB(1, &m_nVBOVertices);
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, m_nVBOVertices);
	// Load data
	glBufferDataARB(GL_ARRAY_BUFFER_ARB, m_nVertexCount * 3 * sizeof(float), 
		m_pVertices, GL_STATIC_DRAW_ARB);
	// Generate and bind the texture coordinate buffer
	glGenBuffersARB(1, &m_nVBOTexCoords);
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, m_nVBOTexCoords);
	// Load data
	glBufferDataARB(GL_ARRAY_BUFFER_ARB, m_nVertexCount * 2 * sizeof(float),
		m_pTexCoords, GL_STATIC_DRAW_ARB);

	delete[] m_pVertices;
	m_pVertices = NULL;
	delete[] m_pTexCoords;
	m_pTexCoords = NULL;
}

bool IsExtensionSupported(char* szTargetExtension)
{
	const unsigned char* pszExtensions = NULL;
	const unsigned char* pszStart;
	unsigned char* pszWhere;
	unsigned char* pszTerminator;
	// Extension names should not have spaces
	pszWhere = (unsigned char*)strchr(szTargetExtension, ' ');
	if (!pszWhere || *szTargetExtension == '\0') {
		return false;
	}
	// Get extension string
	pszExtensions = glGetString(GL_EXTENSIONS);
	// Search the extensions string for an exact copy
	pszStart = pszExtensions;
	for (;;) {
		pszWhere = (unsigned char*)strstr((const char*)pszStart, szTargetExtension);
		if (!pszWhere) {
			break;
		}
		pszTerminator = pszWhere + strlen(szTargetExtension);
		if (pszWhere == pszStart || *(pszWhere - 1) == ' ') {
			if (*pszTerminator == ' ' || *pszTerminator == '\0') {
				return true;
			}
		}
		pszStart = pszTerminator;
	}
	return false;
}

BOOL Initialize(GL_Window* window, Keys* keys)
{
	g_window = window;
	g_keys = keys;

	g_pMesh = new CMesh();
	if (!g_pMesh->LoadHeightmap("Data/terrain.bmp",
		MESH_HEIGHTSCALE, MESH_RESOLUTION))
	{
		MessageBox(NULL, "Error Loading Heightmap", "Error", MB_OK);
		return FALSE;
	}

	// Check for VBOs supported
#ifndef NO_VBOS
	g_fVBOSupported = IsExtensionSupported("GL_ARB_vertex_buffer_object");
	if (g_fVBOSupported) {
		// Get pointer
		glGenBuffersARB = (PFNGLGENBUFFERSARBPROC)wglGetProcAddress("glGenBuffersARB");
		glBindBufferARB = (PFNGLBINDBUFFERARBPROC)wglGetProcAddress("glBindBufferARB");
		glBufferDataARB = (PFNGLBUFFERDATAARBPROC)wglGetProcAddress("glBufferDataARB");
		glDeleteBuffersARB = (PFNGLDELETEBUFFERSARBPROC)wglGetProcAddress("glDeleteBuffersARB");
		// Load vertex data into the graphics card memory
		g_pMesh->BuildVBOs();
	}
#else
	g_fVBOSupported = false;
#endif // !NO_VBOS

	glClearColor(0.0f, 0.0f, 0.0f, 0.5f);
	glClearDepth(1.0f);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_DEPTH_TEST);
	glShadeModel(GL_SMOOTH);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	glEnable(GL_TEXTURE_2D);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	return TRUE;
}

void Deinitialize(void)
{
	if (g_pMesh) {
		delete g_pMesh;
	}
	g_pMesh = NULL;
}

void Update(DWORD milliseconds)
{
//	g_flYRot += (float)(milliseconds) / 1000.0f * 25.0f;    // Consistantly rotate
	if (g_keys->keyDown[VK_ESCAPE] == TRUE) {
		TerminateApplication(g_window);
	}
	if (g_keys->keyDown[VK_F1] == TRUE) {
		ToggleFullscreen(g_window);
	}
	if (g_keys->keyDown[VK_UP]) {
		g_flXRot -= 0.1f;
		if (g_flXRot <= -20.0f) {
			g_flXRot = -20.0f;
		}
	}
	if (g_keys->keyDown[VK_DOWN]) {
		g_flXRot += 0.1f;
		if (g_flXRot >= 20.0f) {
			g_flXRot = 20.0f;
		}
	}

	if (g_keys->keyDown[VK_LEFT]) {
		g_flYRot -= 0.1f;
	}
	if (g_keys->keyDown[VK_RIGHT]) {
		g_flYRot += 0.1f;
	}
	if (g_keys->keyDown['W']) {
		DisZ += 0.1f;
	}
	if (g_keys->keyDown['S']) {
		DisZ -= 0.1f;
	}
	if (g_keys->keyDown['A']) {
		DisX += 0.1f;
	}
	if (g_keys->keyDown['D']) {
		DisX -= 0.1f;
	}
}

void Draw(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	if (GetTickCount() - g_dwLastFPS >= 1000) {
		g_dwLastFPS = GetTickCount();             // Update time virable
		g_nFPS = g_nFrames;                       // Save the FPS
		g_nFrames = 0;                            // Reset the FPS counter

		char szTitle[256] = { 0 };
		sprintf(szTitle, "VBO - %d Triangles, %d FPS", 
			g_pMesh->m_nVertexCount / 3, g_nFPS);
		if (g_fVBOSupported) {
			strcat(szTitle, ", Using VBOs");
		}
		else {
			strcat(szTitle, ", Not Using VBOs");
		}
		SetWindowText(g_window->hWnd, szTitle);
	}
	g_nFrames++;

	glTranslatef(DisX, 0, DisZ);
	glTranslatef(0.0f, -220.0f, 0.0f);
	glRotatef(g_flXRot, 1.0f, 0.0f, 0.0f);
	glRotatef(g_flYRot, 0.0f, 1.0f, 0.0f);

	// Enable pointer
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	// Set pointer to data
	if (g_fVBOSupported) {
		glBindBufferARB(GL_ARRAY_BUFFER_ARB, g_pMesh->m_nVBOVertices);
		glVertexPointer(3, GL_FLOAT, 0, (char*)NULL);
		glBindBufferARB(GL_ARRAY_BUFFER_ARB, g_pMesh->m_nVBOTexCoords);
		glTexCoordPointer(2, GL_FLOAT, 0, (char*)NULL);
	}
	else {
		glVertexPointer(3, GL_FLOAT, 0, g_pMesh->m_pVertices);
		glTexCoordPointer(2, GL_FLOAT, 0, g_pMesh->m_pTexCoords);
	}

	glDrawArrays(GL_TRIANGLES, 0, g_pMesh->m_nVertexCount);    // Render

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}