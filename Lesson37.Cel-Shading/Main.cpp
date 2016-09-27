#include <Windows.h>
#include <GL/glew.h>
#include <GL/glut.h>
#include <GL/GLUAX.H>
#include <math.h>
#include <stdio.h>
#include "Previous.h"

#pragma comment(lib, "legacy_stdio_definitions.lib")

#ifndef CDS_FULLSCREEN
#define CDS_FULLSCREEN 4
#endif

GL_Window* g_window;
Keys* g_keys;

typedef struct tagMATRIX {
    float Data[16];                  // Matrix format
} MATRIX;

typedef struct tagVECTOR {
    float X, Y, Z;
} VECTOR;

typedef struct tagVERTEX {
    VECTOR Nor;                      // Normal
    VECTOR Pos;                      // Position
} VERTEX;

typedef struct tagPOLYGON {          // Polygon
    VERTEX Verts[3];
} POLYGON;

bool outlineDraw = true;             // Flag to draw the outline
bool outlineSmooth = false;          // Flag to Anti-Alias the lines
float outlineColor[3] = { 0.0f, 0.0f, 0.0f };     // Color of the line
float outlineWidth = 3.0f;           // Width of the lines

VECTOR lightAngle;                   // The direction of the light
bool lightRotate = false;            // Flag to see if rotate the light

float modelAngle = 0.0f;             // Y-axis angle of the model
bool modelRotate = false;            // Flag to rotate the model

POLYGON* polyData = NULL;            // Polygon data
int polyNum = 0;                     // Number of polygon

GLuint shaderTexture[1];             // Storage for one texture

BOOL ReadMesh()
{
    FILE* In = fopen("Data/model.txt", "rb");
    if (!In) {
        return FALSE;
    }
    fread(&polyNum, sizeof(int), 1, In);    // Read the header (number of polygon)
    polyData = (POLYGON*)malloc(sizeof(POLYGON) * polyNum);

    fread(&polyData[0], sizeof(POLYGON) * polyNum, 1, In);
    fclose(In);
    return TRUE;
}

inline float DotProduct(VECTOR &V1, VECTOR &V2)
{
    return V1.X * V2.X + V1.Y * V2.Y + V1.Z * V2.Z;
}

inline float Magnitude(VECTOR &V)                    // The length of the vector
{
    return sqrt(V.X * V.X + V.Y * V.Y + V.Z * V.Z);
}

void Normalize(VECTOR &V)                            // Create a vector with a unit length of 1
{
    float M = Magnitude(V);
    if (M != 0.0f) {
        V.X /= M;
        V.Y /= M;
        V.Z /= M;
    }
}

void RotateVector(MATRIX &M, VECTOR &V, VECTOR &D)            // Rotate a vector
{
    D.X = M.Data[0] * V.X + M.Data[4] * V.Y + M.Data[8] * V.Z;    // Rotate round the x axis
    D.Y = M.Data[1] * V.X + M.Data[5] * V.Y + M.Data[9] * V.Z;    // Rotate round the y axis
    D.Z = M.Data[2] * V.X + M.Data[6] * V.Y + M.Data[10] * V.Z;   // Rotate round the z axis
}

BOOL Initialize(GL_Window* window, Keys* keys)
{
    char Line[255];
    float shaderData[32][3];
    FILE *In = NULL;

    g_window = window;
    g_keys = keys;

    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);     // Realy nice perspective calculations
    glClearColor(0.1f, 0.1f, 0.1f, 0.0f);
    glClearDepth(1.0f);    

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glShadeModel(GL_SMOOTH);
    glDisable(GL_LINE_SMOOTH);
    glEnable(GL_CULL_FACE);
    glDisable(GL_LIGHTING);

    In = fopen("Data/shader.txt", "r");
    if (In) {
        for (int i = 0; i < 32; i++) {
            if (feof(In)) break;
            fgets(Line, 255, In);
            shaderData[i][0] = shaderData[i][1] = shaderData[i][2] = float(atof(Line));
        }
        fclose(In);
    }
    else {
        return FALSE;
    }

    glGenTextures(1, &shaderTexture[0]);
    glBindTexture(GL_TEXTURE_1D, shaderTexture[0]);

    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB, 32, 0, GL_RGB, GL_FLOAT, shaderData);

    lightAngle.X = 0.0f;
    lightAngle.Y = 0.0f;
    lightAngle.Z = 1.0f;

    Normalize(lightAngle);
    return ReadMesh();
}

void Deinitialize(void)
{
    glDeleteTextures(1, &shaderTexture[0]);
    free(polyData);
}

void Update(DWORD milliseconds)
{
    if (g_keys->keyDown[' '] == TRUE) {
        modelRotate = !modelRotate;                  // Toggle model rotation
        g_keys->keyDown[' '] = FALSE;
    }
    if (g_keys->keyDown['1'] == TRUE) {
        outlineDraw = !outlineDraw;                  // Tpggle outline drawing
        g_keys->keyDown['1'] = FALSE;
    }
    if (g_keys->keyDown['2'] == TRUE) {
        outlineSmooth = !outlineSmooth;              // Toggle Anti-Aliasing
        g_keys->keyDown['2'] = FALSE;
    }
    if (g_keys->keyDown[VK_UP] == TRUE) {
        outlineWidth++;                              // Line width
        g_keys->keyDown[VK_UP] = FALSE;
    }
    if (g_keys->keyDown[VK_DOWN] == TRUE) {
        outlineWidth--;                              // Line width
        g_keys->keyDown[VK_DOWN] = FALSE;
    }
    if (modelRotate) {
        modelAngle += (float)(milliseconds) / 10.0f;
    }
}

void Draw()
{
    float TmpShade;                // Temporary shader value
    MATRIX TmpMatrix;
    VECTOR TmpVector, TmpNormal;
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    if (outlineSmooth) {           // Check
        glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
        glEnable(GL_LINE_SMOOTH);                // Enable Anti-Aliasing
    }
    else {
        glDisable(GL_LINE_SMOOTH);
    }
    glTranslatef(0.0f, 0.0f, -2.0f);
    glRotatef(modelAngle, 0.0f, 1.0f, 0.0f);
    glGetFloatv(GL_MODELVIEW_MATRIX, TmpMatrix.Data);      // Get the generatef matrix
    // Cel-shading code
    glEnable(GL_TEXTURE_1D);
    glBindTexture(GL_TEXTURE_1D, shaderTexture[0]);
    glColor3f(0.8f, 0.2f, 0.2f);

    glBegin(GL_TRIANGLES);
    for (int i = 0; i < polyNum; ++i) {
        for (int j = 0; j < 3; ++j) {
            TmpNormal.X = polyData[i].Verts[j].Nor.X;
            TmpNormal.Y = polyData[i].Verts[j].Nor.Y;
            TmpNormal.Z = polyData[i].Verts[j].Nor.Z;

            RotateVector(TmpMatrix, TmpNormal, TmpVector);     // Rotate
            Normalize(TmpVector);
            
            TmpShade = DotProduct(TmpVector, lightAngle);
            if (TmpShade < 0.0f) {
                TmpShade = 0.0f;
            }

            glTexCoord1f(TmpShade);          // Set the texture coordinate as the shade value
            glVertex3fv(&polyData[i].Verts[j].Pos.X);
        }
    }
    glEnd();
    glDisable(GL_TEXTURE_1D);

    if (outlineDraw) {             // Check
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);   // Blend mode

        glPolygonMode(GL_BACK, GL_LINE);           // Draw backfacing polygon as wireframes
        glLineWidth(outlineWidth);
        glCullFace(GL_FRONT);                      // Don't draw any front-face polygon
        glDepthFunc(GL_LEQUAL);
        glColor3fv(&outlineColor[0]);              // Set the outline color

        glBegin(GL_TRIANGLES);
        for (int i = 0; i < polyNum; ++i) {
            for (int j = 0; j < 3; ++j) {
                glVertex3fv(&polyData[i].Verts[j].Pos.X);
            }
        }
        glEnd();

        glDepthFunc(GL_LESS);               // Reset
        glCullFace(GL_BACK);                // Reset
        glPolygonMode(GL_BACK, GL_FILL);    // Reset
        glDisable(GL_BLEND);
    }
}