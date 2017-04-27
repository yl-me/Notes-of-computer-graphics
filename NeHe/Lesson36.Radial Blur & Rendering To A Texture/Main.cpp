#include <Windows.h>
#include <GL/glew.h>
#include <GL/glut.h>
#include <GL/GLUAX.H>
#include <math.h>
#include "Previous.h"


#ifndef CDS_FULLSCREEN
#define CDS_FULLSCREEN 4
#endif

GL_Window* g_window;
Keys* g_keys;

float angle;                    // Used to rotate the helix
float vertexes[4][3];
float normal[3];
GLuint BlurTexture;             // Stored the texture number

GLuint EmptyTexture()           // Create a empty texture
{
    GLuint textureID;
    unsigned int* data;
    
    data = (unsigned int*)malloc(128 * 128 * 4 * sizeof(unsigned int));
    ZeroMemory(data, (128 * 128 * 4 * sizeof(unsigned int)));

    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, 4, 128, 128, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

    free(data);
    return textureID;
}

void ReduceToUnit(float vector[3])            // Reduce a normal vector
{
    float length;
    length = (float)sqrt((vector[0] * vector[0]) * (vector[1] * vector[1]) * (vector[2] * vector[2]));
    
    if (length == 0.0f)
        length = 1.0f;
    vector[0] /= length;
    vector[1] /= length;
    vector[2] /= length;
}

void calcNormal(float v[3][3], float out[3])        //Calculates normal for a quad using 3 points
{
    float v1[3], v2[3];
    static const int x = 0;                         // Coordinate
    static const int y = 0;
    static const int z = 0;

    // Finds the vector bewteen 2 points
    v1[x] = v[0][x] - v[1][x];
    v1[y] = v[0][y] - v[1][y];
    v1[z] = v[0][z] - v[1][z];

    v2[x] = v[1][x] - v[2][x];
    v2[y] = v[1][y] - v[2][y];
    v2[z] = v[1][z] - v[2][z];

    out[x] = v1[y] * v2[z] - v1[z] * v2[y];          // Cross product for Y - Z
    out[y] = v1[z] * v2[x] - v1[x] * v2[z];          // Cross product for X - Z
    out[z] = v1[x] * v2[y] - v1[y] * v2[x];          // Cross product for X - Y

    ReduceToUnit(out);
}

void ProcessHelix()               // Draw a helix
{
    GLfloat x, y, z;              // Coordniate
    GLfloat phi;                  // Angle
    GLfloat theta;                // Angle
    GLfloat v, u;                 // Angles
    GLfloat r = 1.5;              // Radius of twist
    int twists = 5;               // Numbers

    GLfloat glfMaterialColor[] = { 0.4f, 0.2f, 0.8f, 1.0f };        // The material color
    GLfloat specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };                // Specular lighting

    glLoadIdentity();
    gluLookAt(0, 5, 50, 0, 0, 0, 0, 1, 0);
    glPushMatrix();
    
    glTranslatef(0, 0, -50);
    glRotatef(angle / 2.0f, 1, 0, 0);
    glRotatef(angle / 3.0f, 0, 1, 0);

    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, glfMaterialColor);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);

    glBegin(GL_QUADS);
        for (phi = 0; phi <= 360; phi += 20.0f) {
            for (theta = 0; theta <= 360 * twists; theta += 20.0f) {
                // First
                v = phi / 180.0f * 3.1415f;                // rad
                u = theta / 180.0f * 3.1415f;              // rad

                x = float(cos(u) * (2.0f + cos(v))) * r;   // The coordniate of point 
                y = float(sin(u) * (2.0f + cos(v))) * r;
                z = float(((u - (2.0f * 3.1415f)) + sin(v)) * r);

                vertexes[0][0] = x;
                vertexes[0][1] = y;
                vertexes[0][2] = z;
                // Second
                v = phi / 180.0f * 3.1415f;
                u = (theta + 20) / 180.0f * 3.1415f;

                x = float(cos(u) * (2.0f + cos(v))) * r;   // The coordniate of point 
                y = float(sin(u) * (2.0f + cos(v))) * r;
                z = float(((u - (2.0f * 3.1415f)) + sin(v)) * r);

                vertexes[1][0] = x;
                vertexes[1][1] = y;
                vertexes[1][2] = z;
                // Third
                v = (phi + 20) / 180.0f * 3.1415f;
                u = (theta + 20) / 180.0f * 3.1415f;

                x = float(cos(u) * (2.0f + cos(v))) * r;   // The coordniate of point 
                y = float(sin(u) * (2.0f + cos(v))) * r;
                z = float(((u - (2.0f * 3.1415f)) + sin(v)) * r);

                vertexes[2][0] = x;
                vertexes[2][1] = y;
                vertexes[2][2] = z;
                // Fourth
                v = (phi + 20) / 180.0f * 3.1415f;
                u = theta / 180.0f * 3.1415f;

                x = float(cos(u) * (2.0f + cos(v))) * r;   // The coordniate of point 
                y = float(sin(u) * (2.0f + cos(v))) * r;
                z = float(((u - (2.0f * 3.1415f)) + sin(v)) * r);

                vertexes[3][0] = x;
                vertexes[3][1] = y;
                vertexes[3][2] = z;

                calcNormal(vertexes, normal);
                glNormal3f(normal[0], normal[1], normal[2]);// Set normal

                // Render
                glVertex3f(vertexes[0][0], vertexes[0][1], vertexes[0][2]);
                glVertex3f(vertexes[1][0], vertexes[1][1], vertexes[1][2]);
                glVertex3f(vertexes[2][0], vertexes[2][1], vertexes[2][2]);
                glVertex3f(vertexes[3][0], vertexes[3][1], vertexes[3][2]);
            }
        }
    glEnd();
    glPopMatrix();
}

void ViewOrtho()                  // Set up an ortho view
{
    glMatrixMode(GL_PROJECTION);  // Select projection
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, 640, 480, 0, -1, 1);   // Select ortho mode
    glMatrixMode(GL_MODELVIEW);       // Select modelview matrix
    glPushMatrix();
    glLoadIdentity();
}

void ViewPerspective()                // Set up a perspective view
{
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}


void RenderToTexture()                // Render to a texture
{
    glViewport(0, 0, 128, 128);       // Set viewport (match texture size)
    ProcessHelix();                   // Render the helix
    glBindTexture(GL_TEXTURE_2D, BlurTexture);
    //Copy  viewport to the blur texture (from 0, 0 to 128, 128...No border)
    glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, 0, 0, 128, 128, 0);
    glClearColor(0.0f, 0.0f, 0.4f, 0.5f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, 640, 480);
}

void DrawBlur(int times, float inc)     // Draw the blurred image
{
    float spost = 0.0f;                 // Starting texture coordinate offset
    float alphainc = 0.0f / times;      // Fade speed for alpha blending
    float alpha = 0.2f;                 // Starting alpha value

    // Disable autoTexture cordinates
    glDisable(GL_TEXTURE_GEN_S);
    glDisable(GL_TEXTURE_GEN_T);

    glEnable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glEnable(GL_BLEND);
    glBindTexture(GL_TEXTURE_2D, BlurTexture);

    ViewOrtho();

    alphainc = alpha / times;                      // To render blur
    glBegin(GL_QUADS);
        for (int num = 0; num < times; ++num) {
            glColor4f(1.0f, 1.0f, 1.0f, alpha);
            glTexCoord2f(0 + spost, 1 - spost);
            glVertex2f(0, 0);

            glTexCoord2f(0 + spost, 0 + spost);
            glVertex2f(0, 480);

            glTexCoord2f(1 - spost, 0 + spost);
            glVertex2f(640, 480);

            glTexCoord2f(1 - spost, 1 - spost);
            glVertex2f(640, 0);

            spost += inc;                          // Zooming closer to texture center
            alpha -= alphainc;                     // Gradually fading image out
        }
    glEnd();
    
    ViewPerspective();

    glEnable(GL_DEPTH_TEST);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);
    glBindTexture(GL_TEXTURE_2D, 0);
}

BOOL Initialize(GL_Window* window, Keys* keys)             // Any GL init code & User initialiazation goes here
{
    g_window = window;
    g_keys = keys;

    angle = 0.0f;
    BlurTexture = EmptyTexture();
    glViewport(0, 0, window->init.width, window->init.height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(50, (float)window->init.width / (float)window->init.height, 5, 2000);  // Set perspective
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glEnable(GL_DEPTH_TEST);

    GLfloat global_ambient[4] = { 0.2f, 0.2f, 0.2f, 1.0f };
    GLfloat light0pos[4] = { 0.0f, 5.0f, 10.0f, 1.0f };
    GLfloat light0ambient[4] = { 0.2f, 0.2f, 0.2f, 1.0f };
    GLfloat light0diffuse[4] = { 0.3f, 0.3f, 0.3f, 1.0f };
    GLfloat light0specular[4] = { 0.8f, 0.8f, 0.8f, 1.0f };

    GLfloat lmodel_ambient[] = { 0.2f, 0.2f, 0.2f, 1.0f };
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);    // Set the model

    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, global_ambient);
    glLightfv(GL_LIGHT0, GL_POSITION, light0pos);
    glLightfv(GL_LIGHT0, GL_AMBIENT, light0ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light0diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light0specular);

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    glShadeModel(GL_SMOOTH);
    glMateriali(GL_FRONT, GL_SHININESS, 128);
    glClearColor(0.0f, 0.0f, 0.0f, 0.5f);
    return TRUE;
}

void Deinitialize(void)
{
    glDeleteTextures(1, &BlurTexture);
}

void Update(DWORD milliseconds)                 // Perform motion updates here
{
    if (g_keys->keyDown[VK_ESCAPE] == TRUE) {
        TerminateApplication(g_window);
    }
    if (g_keys->keyDown[VK_F1] == TRUE) {
        ToggleFullscreen(g_window);
    }
    angle += (float)(milliseconds) / 5.0f;
}

void Draw(void)
{
    glClearColor(0.0f, 0.0f, 0.0f, 0.5f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    RenderToTexture();
    ProcessHelix();
    DrawBlur(25, 0.02f);
    glFlush();
}