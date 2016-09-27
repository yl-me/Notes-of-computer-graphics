#ifndef _3DOBJECT_H_
#define _3DOBJECT_H

#include <iostream>
#include <stdio.h>
#include <math.h>
#include <GL\glut.h>
#pragma comment(lib, "legacy_stdio_definitions.lib")

//3D-coordinate
struct Point3f {
    float x, y, z;
};

// Plane equation ax + by + cz + d = 0
struct PlaneEq {
    float a, b, c, d;
};

// Object's face
struct Face {
    int vertexIndices[3];      // Index of each vertex within the triangle of this face
    Point3f normals[3];        // Normals of each vertex
    PlaneEq PlaneEquation;
    int neighbourIndices[3];   // Index of each face that neighbour
    bool visible;              // Is the face visible by the light
};

struct ShadowedObject {
    int nVertices;
    Point3f* pVertices;        // Dynamically Allocated
    int nFaces;
    Face* pFaces;              // Dynamically Allocated
};

bool readObject(const char* filename, ShadowedObject& object)
{
    FILE* pInputFile = fopen(filename, "r");

    if (!pInputFile) {
        std::cerr << "Unable to open the object file: " << filename << std::endl;
        return false;
    }
    // Read vertices
    fscanf(pInputFile, "%d", &object.nVertices);
    object.pVertices = new Point3f[object.nVertices];

    for (int i = 0; i < object.nVertices; ++i) {
        fscanf(pInputFile, "%f", &object.pVertices[i].x);
        fscanf(pInputFile, "%f", &object.pVertices[i].y);
        fscanf(pInputFile, "%f", &object.pVertices[i].z);
    }
    // Read faces
    fscanf(pInputFile, "%d", &object.nFaces);
    object.pFaces = new Face[object.nFaces];

    for (int i = 0; i < object.nFaces; ++i) {
        Face* pFace = &object.pFaces[i];
        for (int j = 0; j < 3; ++j) {
            pFace->neighbourIndices[j] = 0;     // No neighbour set up
            fscanf(pInputFile, "%d", &pFace->vertexIndices[j]);
            pFace->PlaneEquation.a = pFace->PlaneEquation.b = pFace->PlaneEquation.c = pFace->PlaneEquation.d = 0;
            pFace->visible = false;
//            pFace->vertexIndices[j]--;
        }

        for (int j = 0; j < 3; ++j) {
            fscanf(pInputFile, "%f", &pFace->normals[j].x);
            fscanf(pInputFile, "%f", &pFace->normals[j].y);
            fscanf(pInputFile, "%f", &pFace->normals[j].z);
        }
    }
    return true;
}

void killObject(ShadowedObject& object)
{
    delete[] object.pFaces;
    object.nFaces = 0;
    object.pFaces = NULL;
    delete[] object.pVertices;
    object.nVertices = 0;
    object.pVertices = NULL;
}

void setConnectivity(ShadowedObject& object)
{
    for (int faceA = 0; faceA < object.nFaces; ++faceA) {
        for (int edgeA = 0; edgeA < 3; ++edgeA) {
            if (!object.pFaces[faceA].neighbourIndices[edgeA]) {
                for (int faceB = faceA; faceB < object.nFaces; ++faceB) {
                    for (int edgeB = 0; edgeB < 3; ++edgeB) {
                        int vertA1 = object.pFaces[faceA].vertexIndices[edgeA];
                        int vertA2 = object.pFaces[faceA].vertexIndices[(edgeA + 1) % 3];
                        int vertB1 = object.pFaces[faceB].vertexIndices[edgeB];
                        int vertB2 = object.pFaces[faceB].vertexIndices[(edgeB + 1) % 3];
                        // Check If They Are Neighbours - IE, The Edges Are The Same
                        if ((vertA1 == vertB1 && vertA2 == vertB2) || (vertA1 == vertB2 && vertA2 == vertB1))
                        {
                            // They are neighbours
                            object.pFaces[faceA].neighbourIndices[edgeA] = faceB + 1;
                            object.pFaces[faceB].neighbourIndices[edgeB] = faceA + 1;
                            //edgeFound = true;
                            //break;
                        }
                    }
                }
            }
        }
    }    
}

void drawObject(const ShadowedObject& object)
{
    glBegin(GL_TRIANGLES);
    for (int i = 0; i < object.nFaces; ++i) {
        const Face& face = object.pFaces[i];
        for (int j = 0; j < 3; ++j) {
            const Point3f& vertex = object.pVertices[face.vertexIndices[j]-1];
            glNormal3f(face.normals[j].x, face.normals[j].y, face.normals[j].z);
            glVertex3f(vertex.x, vertex.y, vertex.z);
        }
    }
    glEnd();
}

void calculatePlane(const ShadowedObject object, Face& face)
{
    // Get shortened names for the vertices of the face
    const Point3f& v1 = object.pVertices[face.vertexIndices[0]-1];
    const Point3f& v2 = object.pVertices[face.vertexIndices[1]-1];
    const Point3f& v3 = object.pVertices[face.vertexIndices[2]-1];

    face.PlaneEquation.a = v1.y * (v2.z - v3.z) + v2.y * (v3.z - v1.z) + v3.y * (v1.z - v2.z);
    face.PlaneEquation.b = v1.z * (v2.x - v3.x) + v2.z * (v3.x - v1.x) + v3.z * (v1.x - v2.x);
    face.PlaneEquation.c = v1.x * (v2.y - v3.y) + v2.x * (v3.y - v1.y) + v3.x * (v1.y - v2.y);
    face.PlaneEquation.d = -(v1.x * (v2.y * v3.z - v3.y * v2.z) + v2.x * (v3.y * v1.z - v1.y * v3.z) +
        v3.x * (v1.y * v2.z - v2.y * v1.z));
}

void doShadowPass(ShadowedObject& object, GLfloat* lightPosition)
{
    for (int i = 0; i < object.nFaces; ++i) {
        const Face&    face = object.pFaces[i];
        if (face.visible) {
            // Go through each edge
            for (int j = 0; j < 3; ++j) {
                int neighbourIndex = face.neighbourIndices[j];
                if (!neighbourIndex || object.pFaces[neighbourIndex-1].visible == false) {
                    // Get the points on the edge
                    const Point3f& v1 = object.pVertices[face.vertexIndices[j]-1];
                    const Point3f& v2 = object.pVertices[face.vertexIndices[(j + 1) % 3]-1];
                    // Calculate the two vertices in distance
                    Point3f v3, v4;
                    v3.x = (v1.x - lightPosition[0]) * 10000000;
                    v3.y = (v1.y - lightPosition[1]) * 10000000;
                    v3.z = (v1.z - lightPosition[2]) * 10000000;

                    v4.x = (v2.x - lightPosition[0]) * 10000000;
                    v4.y = (v2.y - lightPosition[1]) * 10000000;
                    v4.z = (v2.z - lightPosition[2]) * 10000000;

                    // Draw the quadrilateral (as a triangle strip)
                    glBegin(GL_TRIANGLE_STRIP);
                        glVertex3f(v1.x, v1.y, v1.z);
                        glVertex3f(v1.x + v3.x, v1.y + v3.y, v1.z + v3.z);
                        glVertex3f(v2.x, v2.y, v2.z);
                        glVertex3f(v2.x + v4.x, v2.y + v4.y, v2.z + v4.z);
                    glEnd();
                }
            }
        }
    }
}


void castShadow(ShadowedObject& object, GLfloat* lightPosition)
{
    // Determine which faces are visible by the light
    for (int i = 0; i < object.nFaces; ++i) {
        const PlaneEq& plane = object.pFaces[i].PlaneEquation;
        GLfloat side = plane.a * lightPosition[0] + plane.b * lightPosition[1] +
            plane.c * lightPosition[2] + plane.d;
        if (side > 0)
            object.pFaces[i].visible = true;
        else
            object.pFaces[i].visible = false;
    }
    glPushAttrib(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_ENABLE_BIT | GL_POLYGON_BIT |
        GL_STENCIL_BUFFER_BIT);
    glDisable(GL_LIGHTING);
    glDepthMask(GL_FALSE);      // Turn off writing to the depth-buffer
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_STENCIL_TEST);  // Trun on
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    glStencilFunc(GL_ALWAYS, 1, 0xFFFFFFFFL);
    //First pass.Increase stencil value int the shadow
    glFrontFace(GL_CCW);
    glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);    
    doShadowPass(object, lightPosition);
    // Scend pass. Decrease stencil value in the shadow
    glFrontFace(GL_CW);
    glStencilOp(GL_KEEP, GL_KEEP, GL_DECR);
    doShadowPass(object, lightPosition);

    glFrontFace(GL_CCW);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);       // Enable rendering
    // Draw a shadowing rectangle covering the entire screen
    glColor4f(0.0f, 0.0f, 0.0f, 0.4f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glStencilFunc(GL_NOTEQUAL, 0, 0xFFFFFFFFL);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    glPushMatrix();
    glLoadIdentity();
        glBegin(GL_TRIANGLE_STRIP);
            glVertex3f(-0.1f, 0.1f, -0.1f);
            glVertex3f(-0.1f, -0.1f, -0.1f);
            glVertex3f(0.1f, 0.1f, -0.1f);
            glVertex3f(0.1f, -0.1f, -0.1f);
        glEnd();
    glPopMatrix();
    glPopAttrib();
}
#endif // !_3DOBJECT_H_

3Dobject.h