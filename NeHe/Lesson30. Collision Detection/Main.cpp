/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
#include <windows.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <gl/glew.h>
#include <GL\glut.h>
#include "Tvector.h"
#include "Tray.h"
#include "Tmatrix.h"
#include "Image.h"
#include <mmsystem.h>

#pragma comment(lib, "legacy_stdio_definitions.lib")

/*
*  Every OpenGL program is linked to a Rendering Context.
*  A Rendering Context is what links OpenGL calls to the Device Context.
*  In order for your program to draw to a Window you need to create a Device Context.
*  The DC connects the Window to the GDI (Graphics Device Interface).
*/

HGLRC     hRC = NULL;         // Permanent rendering context
HDC       hDC = NULL;         // Private GDI device context
HWND      hWnd = NULL;        // Holds our window handle
HINSTANCE hInstance;          // Holds the instance of the application

/*
*  It's important to make this global so that each procedure knows if
*  the program is running in fullscreen mode or not.
*/

bool keys[256];         // Array used for the keyboard routine
bool active = TRUE;     // Window active flag set to TRUE by default
bool fullscreen = TRUE; // Fullscreen flag set to fullscreen mode by default

DEVMODE DMsaved;        // Saves the previous sacreen settings

GLfloat spec[] = { 1.0f, 1.0f, 1.0f, 1.0f };       // Specular highlight of balls
GLfloat posl[] = { 0, 400, 0, 1 };                 // Position of light
GLfloat amb[] = { 0.2f, 0.2f, 0.2f, 1.0f };        // Global ambient
GLfloat amb2[] = { 0.3f, 0.3f, 0.3f, 1.0f };       // Ambient of lightsource

GLfloat rotx;
GLfloat position_x;

TVector dir(0, 0, -10);                     // Initial direction of camera
TVector pos(0, -50, 1000);                  // Initial position of camera
float camera_rotation = 0;                  // Holds rotation around the Y aixs

TVector veloc(0.5, -0.1, 0.5);              // Initial velocity of balls
TVector accel(0, -0.05, 0);                 // acceleration ie. gravity of balls

TVector ArrayVel[10];                       // Holds velocity of balls
TVector ArrayPos[10];                       // Position of balls
TVector OldPos[10];                         // Old position f balls
int NrOfBalls;                              // Numbers of balls
double Time = 0.6;                          // Timestep of simulation
int hook_toball1 = 0;                       // Hook camera on ball
int sounds = 1;                             // Sound on/off

struct Plane {
    TVector    _Position;
    TVector _Normal;
};

struct Cylinder {
    TVector _Position;
    TVector _Axis;
    double _Radius;
};

struct Explosion {
    TVector _Position;
    float _Alpha;
    float _Scale;
};

Plane pl1, pl2, pl3, pl4, pl5;            // Room
Cylinder cyl1, cyl2, cyl3;                // The 3 cylinder of the room
GLUquadricObj* cylinder_obj;              // Quadratic object
GLuint texture[4];                        // Stores texture objects
GLuint displayList;                       // Display list
Explosion ExplosionArray[20];             // Holds max 20 explosion ay once

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM); // Declaration for WndProc

                                                      /* Init variables */
void InitVars()
{
    // Create planes
    pl1._Position = TVector(0, -300, 0);
    pl1._Normal = TVector(0, 1, 0);
    pl2._Position = TVector(300, 0, 0);
    pl2._Normal = TVector(-1, 0, 0);
    pl3._Position = TVector(-300, 0, 0);
    pl3._Normal = TVector(1, 0, 0);
    pl4._Position = TVector(0, 0, 300);
    pl4._Normal = TVector(0, 0, -1);
    pl5._Position = TVector(0, 0, -300);
    pl5._Normal = TVector(0, 0, 1);

    // Create cylinders
    cyl1._Position = TVector(0, 0, 0);
    cyl1._Axis = TVector(0, 1, 0);
    cyl1._Radius = 60 + 20;
    cyl2._Position = TVector(200, -300, 0);
    cyl2._Axis = TVector(0, 0, 1);
    cyl2._Radius = 60 + 20;
    cyl3._Position = TVector(-200, 0, 0);
    cyl3._Axis = TVector(0, 1, 1);
    cyl3._Axis.unit();
    cyl3._Radius = 30 + 20;

    // Create quadratic object
    cylinder_obj = gluNewQuadric();
    gluQuadricTexture(cylinder_obj, GL_TRUE);

    // Set initial position and velocities of balls
    // alse initialize array which holds explosions
    NrOfBalls = 10;

    ArrayVel[0] = veloc;
    ArrayPos[0] = TVector(199, 180, 10);
    ExplosionArray[0]._Alpha = 0;
    ExplosionArray[0]._Scale = 1;

    ArrayVel[1] = veloc;
    ArrayPos[1] = TVector(0, 150, 100);
    ExplosionArray[1]._Alpha = 0;
    ExplosionArray[1]._Scale = 1;

    ArrayVel[2] = veloc;
    ArrayPos[2] = TVector(-100, 180, -100);
    ExplosionArray[2]._Alpha = 0;
    ExplosionArray[2]._Scale = 1;

    for (int i = 3; i<10; i++)
    {
        ArrayVel[i] = veloc;
        ArrayPos[i] = TVector(-500 + i * 75, 300, -500 + i * 50);
        ExplosionArray[i]._Alpha = 0;
        ExplosionArray[i]._Scale = 1;
    }
    for (int i = 10; i<20; i++)
    {
        ExplosionArray[i]._Alpha = 0;
        ExplosionArray[i]._Scale = 1;
    }
}

/* Find if any of the current balls intersect with each other in the current timestep*/
/* Return the index of the 2 intersecting balls, the point and time of intersecting. */
int FindBallCol(TVector& point, double& TimePoint, double& Time2, int& BallNr1, int& BallNr2)
{
    TVector RelativeV;
    TRay rays;
    double MyTime = 0.0;
    double Add = Time2 / 150.0;
    double Timedummy = 10000;
    double Timedummy2 = -1;
    TVector posi;

    // Test all balls against each other in 150 small steps
    for (int i = 0; i < NrOfBalls - 1; ++i) {
        for (int j = i + 1; j < NrOfBalls; ++j) {
            RelativeV = ArrayVel[i] - ArrayVel[j];
            rays = TRay(OldPos[i], TVector::unit(RelativeV));
            MyTime = 0.0;

            if ((rays.dist(OldPos[j])) > 40)
                continue;
            while (MyTime < Time2) {
                MyTime += Add;
                posi = OldPos[i] + RelativeV * MyTime;
                if (posi.dist(OldPos[j]) <= 40) {
                    point = posi;
                    if ((MyTime - Add) < Timedummy)
                        Timedummy = MyTime - Add;
                    BallNr1 = i;
                    BallNr2 = j;
                    break;
                }
            }
        }
    }
    if (Timedummy != 10000) {
        TimePoint = Timedummy;
        return 1;
    }
    return 0;
}

// Fast intersection function between ray/plane
int TestIntersectionPlane(const Plane& plane, const TVector& position, const TVector& direction,
    double& lambda, TVector& pNormal)
{
    double DotProduct = direction.dot(plane._Normal);
    double l2;

    // Determine if ray paralle to plane
    if ((DotProduct < ZERO) && (DotProduct > -ZERO))
        return 0;

    l2 = (plane._Normal.dot(plane._Position - position)) / DotProduct;

    if (l2 < -ZERO)
        return 0;
    pNormal = plane._Normal;
    lambda = l2;
    return 1;
}

/* Fast intersection function between ray/cylinder */
int TestIntersectionCylinder(const Cylinder& cylinder, const TVector& position, const TVector& direction,
    double& lambda, TVector& pNormal, TVector& newposition)
{
    TVector RC;
    double d;
    double t, s;
    TVector n, D, O;
    double ln;
    double in, out;

    TVector::subtract(position, cylinder._Position, RC);
    TVector::cross(direction, cylinder._Axis, n);

    ln = n.mag();
    if ((ln < ZERO) && (ln > -ZERO))
        return 0;

    n.unit();
    d = fabs(RC.dot(n));

    if (d <= cylinder._Radius) {
        TVector::cross(RC, cylinder._Axis, O);
        t = -O.dot(n) / ln;
        TVector::cross(n, cylinder._Axis, O);
        O.unit();
        s = fabs(sqrt(cylinder._Radius*cylinder._Radius - d * d) / direction.dot(O));

        in = t - s;
        out = t + s;
        if (in < -ZERO) {
            if (out < -ZERO)
                return 0;
            else
                lambda = out;
        }
        else {
            if (out<-ZERO) {
                lambda = in;
            }
            else {
                if (in<out)
                    lambda = in;
                else
                    lambda = out;
            }
        }
        newposition = position + direction * lambda;
        TVector HB = newposition - cylinder._Position;
        pNormal = HB - cylinder._Axis * (HB.dot(cylinder._Axis));
        pNormal.unit();
        return 1;
    }
    return 0;
}

/* Main loop of simulation moves. finds the collisions */
/* and responses of the objects in the current time step */
void idle()
{
    double rt, rt2, rt4, lambda = 10000;
    TVector norm, uveloc;
    TVector normal, point, time;
    double RestTime, BallTime;
    TVector Pos2;
    int BallNr = 0;
    int dummy = 0;
    int BallColNr1, BallColNr2;
    TVector Nc;

    RestTime = Time;
    lambda = 10000;

    // Compute velocity for next tiemstep using Euler equation
    for (int j = 0; j < NrOfBalls; ++j)
        ArrayVel[j] += accel * RestTime;

    // While timestep not over
    while (RestTime > ZERO) {
        lambda = 10000;
        // For all the balls find closest intersection between balls and planes/cylinders
        for (int i = 0; i < NrOfBalls; ++i) {
            // Compute new position and distance
            OldPos[i] = ArrayPos[i];
            TVector::unit(ArrayVel[i], uveloc);
            ArrayPos[i] = ArrayPos[i] + ArrayVel[i] * RestTime;
            rt2 = OldPos[i].dist(ArrayPos[i]);

            // Test if collision occured between ball and ball 5 planes
            if (TestIntersectionPlane(pl1, OldPos[i], uveloc, rt, norm)) {
                // Find intersection time
                rt4 = rt * RestTime / rt2;
                // If smaller than the one already stored replace and in tiemstep
                if (rt4 <= lambda) {
                    if (rt4 <= RestTime + ZERO) {
                        if (!((rt < ZERO) && (uveloc.dot(norm) > ZERO))) {
                            normal = norm;
                            point = OldPos[i] + uveloc * rt;
                            lambda = rt4;
                            BallNr = i;
                        }
                    }
                }
            }
            if (TestIntersectionPlane(pl2, OldPos[i], uveloc, rt, norm)) {
                rt4 = rt * RestTime / rt2;
                if (rt4 <= lambda) {
                    if (rt4 <= RestTime + ZERO) {
                        if (!((rt <= ZERO) && (uveloc.dot(norm)>ZERO)))
                        {
                            normal = norm;
                            point = OldPos[i] + uveloc * rt;
                            lambda = rt4;
                            BallNr = i;
                            dummy = 1;
                        }
                    }
                }
            }
            if (TestIntersectionPlane(pl3, OldPos[i], uveloc, rt, norm)) {
                rt4 = rt*RestTime / rt2;
                if (rt4 <= lambda) {
                    if (rt4 <= RestTime + ZERO) {
                        if (!((rt <= ZERO) && (uveloc.dot(norm)>ZERO))) {
                            normal = norm;
                            point = OldPos[i] + uveloc * rt;
                            lambda = rt4;
                            BallNr = i;
                        }
                    }
                }
            }
            if (TestIntersectionPlane(pl4, OldPos[i], uveloc, rt, norm)) {
                rt4 = rt*RestTime / rt2;
                if (rt4 <= lambda) {
                    if (rt4 <= RestTime + ZERO) {
                        if (!((rt <= ZERO) && (uveloc.dot(norm)>ZERO))) {
                            normal = norm;
                            point = OldPos[i] + uveloc * rt;
                            lambda = rt4;
                            BallNr = i;
                        }
                    }
                }
            }
            if (TestIntersectionPlane(pl5, OldPos[i], uveloc, rt, norm)) {
                rt4 = rt*RestTime / rt2;
                if (rt4 <= lambda) {
                    if (rt4 <= RestTime + ZERO) {
                        if (!((rt <= ZERO) && (uveloc.dot(norm)>ZERO))) {
                            normal = norm;
                            point = OldPos[i] + uveloc * rt;
                            lambda = rt4;
                            BallNr = i;
                        }
                    }
                }
            }
            // Now test intersection with the 3 cylinders
            if (TestIntersectionCylinder(cyl1, OldPos[i], uveloc, rt, norm, Nc)) {
                rt4 = rt * RestTime / rt2;
                if (rt4 <= lambda) {
                    if (rt4 <= RestTime + ZERO) {
                        if (!((rt <= ZERO) && (uveloc.dot(norm)>ZERO))) {
                            normal = norm;
                            point = Nc;
                            lambda = rt4;
                            BallNr = i;
                        }
                    }
                }
            }
            if (TestIntersectionCylinder(cyl2, OldPos[i], uveloc, rt, norm, Nc)) {
                rt4 = rt * RestTime / rt2;
                if (rt4 <= lambda) {
                    if (rt4 <= RestTime + ZERO) {
                        if (!((rt <= ZERO) && (uveloc.dot(norm)>ZERO))) {
                            normal = norm;
                            point = Nc;
                            lambda = rt4;
                            BallNr = i;
                        }
                    }
                }
            }
            if (TestIntersectionCylinder(cyl3, OldPos[i], uveloc, rt, norm, Nc)) {
                rt4 = rt * RestTime / rt2;
                if (rt4 <= lambda) {
                    if (rt4 <= RestTime + ZERO) {
                        if (!((rt <= ZERO) && (uveloc.dot(norm)>ZERO))) {
                            normal = norm;
                            point = Nc;
                            lambda = rt4;
                            BallNr = i;
                        }
                    }
                }
            }
        }
        // After all balls were teste with planes/cylinder test for collision
        // between them and replace if collision time smaller
        if (FindBallCol(Pos2, BallTime, RestTime, BallColNr1, BallColNr2)) {
            if (sounds)
                PlaySound("Data/Explode.wav", NULL, SND_FILENAME | SND_ASYNC);
            if ((lambda == 10000) || (lambda > BallTime)) {
                RestTime = RestTime - BallTime;

                TVector pb1, pb2, xaxis, U1x, U1y, U2x, U2y, V1x, V1y, V2x, V2y;
                double a, b;

                pb1 = OldPos[BallColNr1] + ArrayVel[BallColNr1] * BallTime;
                pb2 = OldPos[BallColNr2] + ArrayVel[BallColNr2] * BallTime;
                xaxis = (pb2 - pb1).unit();

                a = xaxis.dot(ArrayVel[BallColNr1]);
                U1x = xaxis * a;
                U1y = ArrayVel[BallColNr1] - U1x;

                xaxis = (pb1 - pb2).unit();
                b = xaxis.dot(ArrayVel[BallColNr2]);
                U2x = xaxis * b;
                U2y = ArrayVel[BallColNr2] - U2x;

                V1x = (U1x + U2x - (U1x - U2x)) * 0.5;
                V2x = (U1x + U2x - (U2x - U1x)) * 0.5;
                V1y = U1y;
                V2y = U2y;

                for (int j = 0; j < NrOfBalls; j++)
                    ArrayPos[j] = OldPos[j] + ArrayVel[j] * BallTime;

                ArrayVel[BallColNr1] = V1x + V1y;
                ArrayVel[BallColNr2] = V2x + V2y;

                // Update explosion array
                for (int j = 0; j < 20; j++) {
                    if (ExplosionArray[j]._Alpha <= 0) {
                        ExplosionArray[j]._Alpha = 1;
                        ExplosionArray[j]._Position = ArrayPos[BallColNr1];
                        ExplosionArray[j]._Scale = 1;
                        break;
                    }
                }
                continue;
            }
        }
        // End of tests 
        // If test occured move simulation for the correct timestep
        // and compute response for the colliding ball
        if (lambda != 10000)
        {
            RestTime -= lambda;

            for (int j = 0; j < NrOfBalls; j++)
                ArrayPos[j] = OldPos[j] + ArrayVel[j] * lambda;

            rt2 = ArrayVel[BallNr].mag();
            ArrayVel[BallNr].unit();
            ArrayVel[BallNr] = TVector::unit((normal * (2 * normal.dot(-ArrayVel[BallNr]))) + ArrayVel[BallNr]);
            ArrayVel[BallNr] = ArrayVel[BallNr] * rt2;

            // Update explosion array
            for (int j = 0; j < 20; j++) {
                if (ExplosionArray[j]._Alpha <= 0) {
                    ExplosionArray[j]._Alpha = 1;
                    ExplosionArray[j]._Position = point;
                    ExplosionArray[j]._Scale = 1;
                    break;
                }
            }
        }
        else {
            RestTime = 0;
        }
    }
}

/* Load bitmaps and convert to textures */
void LoadGLTextures()
{
    Image *image1, *image2, *image3, *image4;

    // allocate space for texture
    image1 = (Image *)malloc(sizeof(Image));
    image2 = (Image *)malloc(sizeof(Image));
    image3 = (Image *)malloc(sizeof(Image));
    image4 = (Image *)malloc(sizeof(Image));
    if (image1 == NULL || image2 == NULL || image3 == NULL || image4 == NULL) {
        printf("Error allocating space for image");
        exit(0);
    }

    if (!ImageLoad("data/marble.bmp", image1) || !ImageLoad("data/spark.bmp", image2) ||
        !ImageLoad("data/boden.bmp", image3) || !ImageLoad("data/wand.bmp", image4)) {
        exit(1);
    }

    glGenTextures(2, &texture[0]);
    glBindTexture(GL_TEXTURE_2D, texture[0]);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexImage2D(GL_TEXTURE_2D, 0, 3, image1->sizeX, image1->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, image1->data);

    glBindTexture(GL_TEXTURE_2D, texture[1]);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexImage2D(GL_TEXTURE_2D, 0, 3, image2->sizeX, image2->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, image2->data);


    glGenTextures(2, &texture[2]);
    glBindTexture(GL_TEXTURE_2D, texture[2]);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexImage2D(GL_TEXTURE_2D, 0, 3, image3->sizeX, image3->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, image3->data);

    glBindTexture(GL_TEXTURE_2D, texture[3]);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexImage2D(GL_TEXTURE_2D, 0, 3, image4->sizeX, image4->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, image4->data);

    free(image1->data);
    free(image1);
    free(image2->data);
    free(image2);
    free(image3->data);
    free(image3);
    free(image4->data);
    free(image4);
}

int ProcessKeys()
{
    if (keys['W'])
        pos += TVector(0, 0, -1);
    if (keys['S'])
        pos += TVector(0, 0, 1);
    if (keys['A']) 
        position_x += 1;
    if (keys['D'])
        position_x -= 1;
    if (keys[VK_UP])
        rotx += 0.1;
    if (keys[VK_DOWN])
        rotx -= 0.1;
    if (keys[VK_LEFT])
        camera_rotation += 0.1;
    if (keys[VK_RIGHT])
        camera_rotation -= 0.1;
    if (keys[VK_ADD]) {
        Time += 0.1;
        if (Time >= 10.0)
            Time = 10.0;
        keys[VK_ADD] = FALSE;
    }
    if (keys[VK_SUBTRACT]) {
        Time -= 0.1;
        if (Time <= 0.1)
            Time = 0.0;
        keys[VK_SUBTRACT] = FALSE;
    }
    if (keys[VK_F3]) {
        sounds ^= 1;
        keys[VK_F3] = FALSE;
    }
    if (keys[VK_F2]) {
        hook_toball1 ^= 1;
        camera_rotation = 0;
        keys[VK_F2] = FALSE;
    }
    return 1;
}


/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
GLvoid ReSizeGLScene(GLsizei width, GLsizei height)   // Resize and initialize the GL window
{
    if (height == 0) {                                // Prevent a divide by zero by
        height = 1;                                   // Making height equal one
    }

    glViewport(0, 0, width, height);                  // Reset the current viewport

    /*
    *  The following lines set the screen up for a perspective view.
    *  Meaning things in the distance get smaller. This creates a realistic looking scene.
    *  The perspective is calculated with a 45 degree viewing angle based on
    *  the windows width and height. The 0.1f, 100.0f is the starting point and
    *  ending point for how deep we can draw into the screen.
    *
    *  The projection matrix is responsible for adding perspective to our scene.
    *  glLoadIdentity() restores the selected matrix to it's original state.
    *  The modelview matrix is where our object information is stored.
    *   Lastly we reset the modelview matrix.
    */

    glMatrixMode(GL_PROJECTION);                      // Select the projection matrix
    glLoadIdentity();                                 // Reset the projection matrix

                                                      // Calculate the aspect ratio of the window
    gluPerspective(50.0f, (GLfloat)width / (GLfloat)height, 10.0f, 1700.0f);
    //    glOrtho(0.0f, width, height, 0.0f, -1.0f, 1.0f);  // Create orhto 640X480 view (0, 0, at the top)

    glMatrixMode(GL_MODELVIEW);                       // Seclet the modelview matrix
    glLoadIdentity();                                 // Reset the modelview matrix
}
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/

int InitGL(GLvoid)                                    // All setup for OpenGL goes here
{
    float df = 100.0f;

    glClearDepth(1.0f);                               // Depth buffer setup
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);             // Background
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glShadeModel(GL_SMOOTH);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

    glMaterialfv(GL_FRONT, GL_SPECULAR, spec);
    glMaterialfv(GL_FRONT, GL_SHININESS, &df);

    glEnable(GL_LIGHTING);
    glLightfv(GL_LIGHT0, GL_POSITION, posl);
    glLightfv(GL_LIGHT0, GL_AMBIENT, amb2);
    glEnable(GL_LIGHT0);

    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, amb);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    glEnable(GL_TEXTURE_2D);
    LoadGLTextures();

    // Construct billboared explosion primitive as display list
    // 4 quads at right angles to each other
    glNewList(displayList = glGenLists(1), GL_COMPILE);
    glBegin(GL_QUADS);
    glRotatef(-45, 0, 1, 0);
    glNormal3f(0, 0, 1);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-50, -40, 0);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(50, -40, 0);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(50, 40, 0);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-50, 40, 0);
    glNormal3f(0, 0, -1);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-50, 40, 0);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(50, 40, 0);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(50, -40, 0);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-50, -40, 0);

    glNormal3f(1, 0, 0);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(0, -40, 50);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(0, -40, -50);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(0, 40, -50);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(0, 40, 50);
    glNormal3f(-1, 0, 0);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(0, 40, 50);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(0, 40, -50);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(0, -40, -50);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(0, -40, 50);
    glEnd();
    glEndList();

    return TRUE;
}

/*
*  For now all we will do is clear the screen to the color we previously decided on,
*  clear the depth buffer and reset the scene. We wont draw anything yet.
*/
bool DrawGLScene(GLvoid)                                  // Here's where we do all the drawing
{
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Set camera in hookhmode
    if (hook_toball1) {
        TVector unit_followvector = ArrayVel[0];
        unit_followvector.unit();
        gluLookAt(ArrayPos[0].X() + 250, ArrayPos[0].Y() + 250, ArrayPos[0].Z(),
            ArrayPos[0].X() + ArrayVel[0].X(), ArrayPos[0].Y() + ArrayVel[0].Y(),
            ArrayPos[0].Z() + ArrayVel[0].Z(), 0, 1, 0);
    }
    else {
        gluLookAt(pos.X(), pos.Y(), pos.Z(), pos.X() + dir.X(), pos.Y() + dir.Y(),
            pos.Z() + dir.Z(), 0.0, 1.0, 0.0);
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glRotatef(camera_rotation, 0, 1, 0);
    glRotatef(rotx, 1, 0, 0);
    glTranslatef(position_x, 0, 0);
    // Render balls
    for (int i = 0; i < NrOfBalls; ++i) {
        switch (i) {
        case 1: glColor3f(1.0f, 1.0f, 1.0f); break;
        case 2: glColor3f(1.0f, 1.0f, 0.0f); break;
        case 3: glColor3f(0.0f, 1.0f, 1.0f); break;
        case 4: glColor3f(0.0f, 1.0f, 0.0f); break;
        case 5: glColor3f(0.0f, 0.0f, 1.0f); break;
        case 6: glColor3f(0.65f, 0.2f, 0.3f); break;
        case 7: glColor3f(1.0f, 0.0f, 1.0f); break;
        case 8: glColor3f(0.0f, 0.7f, 0.4f); break;
        default: glColor3f(1.0f, 0, 0);
        }
        glPushMatrix();
        glTranslated(ArrayPos[i].X(), ArrayPos[i].Y(), ArrayPos[i].Z());
        gluSphere(cylinder_obj, 20, 20, 20);
        glPopMatrix();
    }

    glEnable(GL_TEXTURE_2D);

    // Render walls with texture
    glBindTexture(GL_TEXTURE_2D, texture[3]);
    glColor3f(1, 1, 1);
    glBegin(GL_QUADS);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(320, 320, 320);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(320, -320, 320);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-320, -320, 320);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-320, 320, 320);

    glTexCoord2f(1.0f, 0.0f); glVertex3f(-320, 320, -320);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(-320, -320, -320);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(320, -320, -320);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(320, 320, -320);

    glTexCoord2f(1.0f, 0.0f); glVertex3f(320, 320, -320);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(320, -320, -320);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(320, -320, 320);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(320, 320, 320);

    glTexCoord2f(1.0f, 0.0f); glVertex3f(-320, 320, 320);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(-320, -320, 320);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-320, -320, -320);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-320, 320, -320);
    glEnd();

    // Render floor with color
    glBindTexture(GL_TEXTURE_2D, texture[2]);
    glBegin(GL_QUADS);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-320, -320, 320);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(320, -320, 320);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(320, -320, -320);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-320, -320, -320);
    glEnd();


    // Render columns
    glBindTexture(GL_TEXTURE_2D, texture[0]);
    glColor3f(0.5, 0.5, 0.5);
    glPushMatrix();
    glRotatef(90, 1, 0, 0);
    glTranslatef(0, 0, -500);
    gluCylinder(cylinder_obj, 60, 60, 1000, 20, 2);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(200, -300, -500);
    gluCylinder(cylinder_obj, 60, 60, 1000, 20, 2);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-200, 0, 0);
    glRotatef(135, 1, 0, 0);
    glTranslatef(0, 0, -500);
    gluCylinder(cylinder_obj, 30, 30, 1000, 20, 2);
    glPopMatrix();

    // Render explosions
    glEnable(GL_BLEND);
    glDepthMask(GL_FALSE);
    glBindTexture(GL_TEXTURE_2D, texture[1]);
    for (int i = 0; i < 20; i++)
    {
        if (ExplosionArray[i]._Alpha >= 0)
        {
            glPushMatrix();
            ExplosionArray[i]._Alpha -= 0.01f;
            ExplosionArray[i]._Scale += 0.03f;
            glColor4f(1, 1, 0, ExplosionArray[i]._Alpha);
            glScalef(ExplosionArray[i]._Scale, ExplosionArray[i]._Scale, ExplosionArray[i]._Scale);
            glTranslatef((float)ExplosionArray[i]._Position.X() / ExplosionArray[i]._Scale,
                (float)ExplosionArray[i]._Position.Y() / ExplosionArray[i]._Scale,
                (float)ExplosionArray[i]._Position.Z() / ExplosionArray[i]._Scale);
            glCallList(displayList);
            glPopMatrix();
        }
    }
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);
    return true;
}
//******************************************************************************************************************************************/
//******************************************************************************************************************************************/
/*
*  The job of KillGLWindow() is to release the Rendering Context,
*  the Device Context and finally the Window Handle.
*/

GLvoid KillGLWindow(GLvoid)                             // Properly kill the window
{
    if (fullscreen) {                                   // Are we in fullscreen mode
        if (!ChangeDisplaySettings(NULL, CDS_TEST)) {   // If the shortcut doesn't work
            ChangeDisplaySettings(NULL, CDS_RESET);     // Do it anyway (to get the values out of the registry)
            ChangeDisplaySettings(&DMsaved, CDS_RESET);
        }
        else {
            ChangeDisplaySettings(NULL, CDS_RESET);                  // If so switch back to the desktop
        }
        /*
        *  We use ChangeDisplaySettings(NULL,0) to return us to our original desktop.
        *  After we've switched back to the desktop we make the cursor visible again.
        */
        ShowCursor(TRUE);                                // Show mouse pointer
    }

    if (hRC) {                                           // Do we have a rendering context
        if (!wglMakeCurrent(NULL, NULL)) {                // Are we able to release the DC and RC contexts
            MessageBox(NULL, "Release of DC and RC failed.", "SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
        }

        if (!wglDeleteContext(hRC)) {                     // Are we able to delete the RC
            MessageBox(NULL, "Release rendering context failed.", "SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
            hRC = NULL;                                  // Set RC to NULL
        }

        if (hDC && !ReleaseDC(hWnd, hDC)) {              // Are we able to release the DC
            MessageBox(NULL, "Release device context failed.", "SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
            hDC = NULL;                                  // Set DC to NULL
        }
        if (hWnd && !DestroyWindow(hWnd)) {              // Are we able to destroy the window
            MessageBox(NULL, "Could not release hWnd.", "SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
            hWnd = NULL;                                 // Set hWnd to NULL
        }

        if (!UnregisterClass("OpenGL", hInstance)) {     // Are we able to unregister class
            MessageBox(NULL, "Could not register class.", "SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
            hInstance = NULL;                            // Set hInstance to NULL
        }
    }
}
/*
* The next section of code creates our OpenGL Window.
*/

BOOL CreateGLWindow(char* title, int width, int height, int bits, bool fullscreenflag)
{
    /*
    * Find  a pixel format that matches the one we want
    */
    GLuint PixelFormat;                                  // Holds the result after serching for a match

    /*
    * Before you create a window, you MUST register a Class for the window
    */
    WNDCLASS wc;                                         // Windows class structure

    /*
    *  dwExStyle and dwStyle will store the Extended and normal Window Style Information.
    */
    DWORD dwExStyle;                                     // Window extend style
    DWORD dwStyle;                                       // Window style


    RECT WindowRect;                                     // Grabs rectangle upper left/lower right values
    WindowRect.left = (long)0;                           // Set left value to 0
    WindowRect.right = (long)width;                      // Set right value to requested width
    WindowRect.top = (long)0;                            // Set top value to 0
    WindowRect.bottom = (long)height;                    // Set bottom value to requested height

    fullscreen = fullscreenflag;                         // Set the global fullscreen flag

    /*
    *  The style CS_HREDRAW and CS_VREDRAW force the Window to redraw whenever it is resized.
    *  CS_OWNDC creates a private DC for the Window. Meaning the DC is not shared across applications.
    *  WndProc is the procedure that watches for messages in our program.
    *  No extra Window data is used so we zero the two fields. Then we set the instance.
    *  Next we set hIcon to NULL meaning we don't want an ICON in the Window,
    *  and for a mouse pointer we use the standard arrow. The background color doesn't matter
    *  (we set that in GL). We don't want a menu in this Window so we set it to NULL,
    *  and the class name can be any name you want. I'll use "OpenGL" for simplicity.
    */
    hInstance = GetModuleHandle(NULL);                   // Grab an instance for our window
    wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;       // Redraw on move, and own DC for window
    wc.lpfnWndProc = (WNDPROC)WndProc;                   // WndProc handles message
    wc.cbClsExtra = 0;                                   // No extra window date
    wc.cbWndExtra = 0;                                   // No extra window date
    wc.hInstance = hInstance;                            // set the instance
    wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);              // Load the default icon
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);            // Load the arrow pointer
    wc.hbrBackground = NULL;                             // No background requried for GL
    wc.lpszMenuName = NULL;                              // We don't want a menu
    wc.lpszClassName = "OpenGL";                         // set the class name

    EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &DMsaved);  // Save the current display state

    if (!RegisterClass(&wc)) {                           // Attempt to register the window class
        MessageBox(NULL, "Failed to register the window class.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
        return FALSE;                                    // Exit and return false
    }

    if (fullscreen) {                                    // attempt fullsreen model

        /*
        *  There are a few very important things you should keep in mind when switching to full screen mode.
        *  Make sure the width and height that you use in fullscreen mode is the same as
        *  the width and height you plan to use for your window, and most importantly,
        *  set fullscreen mode BEFORE you create your window.
        */
        DEVMODE dmScreenSettings;                        // Device mode
        memset(&dmScreenSettings, 0, sizeof(dmScreenSettings)); // Make sure memory's cleared
        dmScreenSettings.dmSize = sizeof(dmScreenSettings);     // Size of devmode structure
        dmScreenSettings.dmPelsWidth = width;            // Select window width
        dmScreenSettings.dmPelsHeight = height;          // Select window height
        dmScreenSettings.dmBitsPerPel = bits;            // Select bits per pixel
        dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

        /*
        *  In the line below ChangeDisplaySettings tries to switch to a mode that matches
        *  what we stored in dmScreenSettings. I use the parameter CDS_FULLSCREEN when switching modes,
        *  because it's supposed to remove the start bar at the bottom of the screen,
        *  plus it doesn't move or resize the windows on your desktop when you switch to
        *  fullscreen mode and back.
        */
        //Try to set selected mode and get results. Note: CDS_FULLSCREEN gets rid of start bar
        if (ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL) {
            //If the mode fails, offer two options. Quit or run in a window
            if (MessageBox(NULL, "The requested fullscreen mode is not supported by\n your video card. Use"
                "windowed mode instead?", "GL", MB_YESNO | MB_ICONEXCLAMATION) == IDYES)
            {
                fullscreen = FALSE;                       // Select windowed mode (fullscreen=FLASE)
            }
            else {
                // Pop up a message box letting user know the programe is closing.
                MessageBox(NULL, "Program will now close.", "ERROR", MB_OK | MB_ICONSTOP);
                return FALSE;                             // Exit and return FALSE
            }
        }
    }

    if (fullscreen) {                                     // Are we still in fullscreen mode

        /*
        *  If we are still in fullscreen mode we'll set the extended style to WS_EX_APPWINDOW,
        *  which force a top level window down to the taskbar once our window is visible.
        *  For the window style we'll create a WS_POPUP window.
        *  This type of window has no border around it, making it perfect for fullscreen mode.

        *  Finally, we disable the mouse pointer. If your program is not interactive,
        *  it's usually nice to disable the mouse pointer when in fullscreen mode. It's up to you though.
        */
        dwExStyle = WS_EX_APPWINDOW;                      // Window extended style
        dwStyle = WS_POPUP;                               // Window style
        ShowCursor(FALSE);                                // Hide mosue pointer 
    }
    else {

        /*
        *  If we're using a window instead of fullscreen mode,
        *  we'll add WS_EX_WINDOWEDGE to the extended style. This gives the window a more 3D look.
        *  For style we'll use WS_OVERLAPPEDWINDOW instead of WS_POPUP.
        *  WS_OVERLAPPEDWINDOW creates a window with a title bar, sizing border,
        *  window menu, and minimize / maximize buttons.
        */
        dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;   // Window extended style
        dwStyle = WS_OVERLAPPEDWINDOW;                    // Window style
    }

    /*
    *  By using the AdjustWindowRectEx command none of our OpenGL scene will be covered up by the borders,
    *  instead, the window will be made larger to account for the pixels needed to draw the window border.
    *  In fullscreen mode, this command has no effect.
    */
    AdjustWindowRectEx(&WindowRect, dwStyle, FALSE, dwExStyle);  // Adjust window to true resqusted

    /*
    *  WS_CLIPSIBLINGS and WS_CLIPCHILDREN are both REQUIRED for OpenGL to work properly.
    *  These styles prevent other windows from drawing over or into our OpenGL Window.
    */
    if (!(hWnd = CreateWindowEx(dwExStyle,                // Extended style for the window
        "OpenGL",                                         // Class name
        title,                                            // Window title
        WS_CLIPSIBLINGS |                                 // Requried window style
        WS_CLIPCHILDREN |                                 // Requried window style
        dwStyle,                                          // Select window style
        0, 0,                                             // Window position
        WindowRect.right - WindowRect.left,               // Calculate adjusted window width
        WindowRect.bottom - WindowRect.top,               // Calculate adjusted window height
        NULL,                                             // No parent window
        NULL,                                             // No menu
        hInstance,                                        // Instance
        NULL)))                                           // Don't pass anything to WM_CREATE
    {
        KillGLWindow();                                   //Reset the display
        MessageBox(NULL, "Window creation error.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
        return FALSE;                                     // Retrurn FALSE;
    }

    /*
    *  aside from the stencil buffer and the (slow) accumulation buffer
    */
    static PIXELFORMATDESCRIPTOR pfd =                    // pfd tells windows how we want things to be 
    {
        sizeof(PIXELFORMATDESCRIPTOR),                    // Size of this pixel format descriptor
        1,                                                // Version number
        PFD_DRAW_TO_WINDOW |                              // Format must support window
        PFD_SUPPORT_OPENGL |                              // Format must support OpenGL
        PFD_DOUBLEBUFFER,                                 // Must support double buffer
        PFD_TYPE_RGBA,                                    // Request an RGBA format
        bits,                                             // Select our color depth
        0, 0, 0, 0, 0, 0,                                 // Color bits ignored
        0,                                                // No alpha buffer
        0,                                                // shift bit ignored
        0,                                                // No accumulation buffer
        0, 0, 0, 0,                                       // Accumulation bits ignored
        16,                                               // 16Bits Z_Buffer (depth buffer)
        0,                                                // No stencil buffer
        0,                                                // No auxiliary buffer
        PFD_MAIN_PLANE,                                   // Main drawing layer
        0,                                                // Reserved
        0, 0, 0                                           // Layer makes ignored
    };

    if (!(hDC = GetDC(hWnd))) {                           // Did we get a device context
        KillGLWindow();                                   // Reset the display
        MessageBox(NULL, "Can't create a GL device context.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
        return FALSE;                                     // Return FALSE
    }

    if (!(PixelFormat = ChoosePixelFormat(hDC, &pfd))) {  // Did window find a matching pixel format
        KillGLWindow();                                   // Reset the display
        MessageBox(NULL, "Can't find a suitable pixelformat.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
        return FALSE;                                     // Return FALSE;
    }

    if (!SetPixelFormat(hDC, PixelFormat, &pfd)) {        // Are we able to set the pixel format
        KillGLWindow();                                   // Reset the display
        MessageBox(NULL, "Can't set the pixelformat.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
        return FALSE;                                     // Return FALSE;
    }

    if (!(hRC = wglCreateContext(hDC))) {                 // Are we able to rendering context
        KillGLWindow();                                   // Reset the display
        MessageBox(NULL, "Can't create a GL rendering context.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
        return FALSE;                                     // Return FASLE;
    }

    if (!wglMakeCurrent(hDC, hRC)) {                      // Try to activate the rendering context
        KillGLWindow();                                   // Reset the display
        MessageBox(NULL, "Can't activate the GL rendering context.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
        return FALSE;                                     // Return FALSE    
    }

    /*
    *  ReSizeGLScene passing the screen width and height to set up our perspective OpenGL screen.
    */
    ShowWindow(hWnd, SW_SHOW);                            // Show the window
    SetForegroundWindow(hWnd);                            // slightly higher priority
    SetFocus(hWnd);                                       // Sets keyboard focus to the window
    ReSizeGLScene(width, height);                         // Set up our perspective GL screen

    /*
    *  we can set up lighting, textures, and anything else that needs to be setup in InitGL().
    */
    if (!InitGL()) {                                      // Initialize our newly created GL window
        KillGLWindow();                                   // Reset the display
        MessageBox(NULL, "Initialize Failed.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
        return FALSE;                                     // Return FALSE
    }
    return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd,                       // Handle for this window
    UINT uMsg,                                            // Message for this window
    WPARAM wParam,                                        // Additional message information
    LPARAM lParam)                                        // Additional message information
{
    switch (uMsg) {                                       // Check for window message
    case WM_ACTIVATE: {                               // Check minimization state
        if (!HIWORD(wParam)) {
            active = TRUE;                            // Program is active
        }
        else {
            active = FALSE;                           // Program is no longer active
        }
        return 0;                                     // Return to the message loop
    }
    case WM_SYSCOMMAND: {                             // Intercept system commands
        switch (wParam) {                             // Check system calls
        case SC_SCREENSAVE:                       // Screensaver trying to start
        case SC_MONITORPOWER:                     // Monitor trying to enter powersave
            return 0;                                 // Prevent form happening
        }
        break;                                        // Exit
    }
    case WM_CLOSE: {                                  // Did we receive a close message
        PostQuitMessage(0);                           // Send a quit message
        return 0;
    }
    case WM_KEYDOWN: {                                // Is a key being held down
        keys[wParam] = TRUE;                          // if so, mark it as TRUE
        return 0;                                     // Jump back
    }
    case WM_KEYUP: {                                  // Has a key been released
        keys[wParam] = FALSE;                         // if so, mark it as FALSE
        return 0;                                     // Jump back
    }
    case WM_SIZE: {                                   // Resize the OpenGL window
        ReSizeGLScene(LOWORD(lParam), HIWORD(lParam));   // LoWord = width HiWord = height
        return 0;                                     // Jump back
    }
    }
    return DefWindowProc(hWnd, uMsg, wParam, lParam);     // Pass all unhandled message to DefWindwProc
}

int WINAPI WinMain(HINSTANCE hInstance,                   // Instance
    HINSTANCE hPrevInstance,                              // Previous instance
    LPSTR lpCmdLine,                                      // Command line parameters
    int nCmdShow)                                         // Window show state
{
    MSG msg;                                              // Window message structure
    BOOL done = FALSE;                                    // Bool variable to exit loop
                                                          // Ask the user which screen mode they prefer
    if (MessageBox(NULL, "Would you like to run in fullscreen mode?",
        "Start fullscreen?", MB_YESNO | MB_ICONQUESTION) == IDNO)
    {
        fullscreen = FALSE;                               // Window mode
    }
//******************************************************************************************************************************************/
//******************************************************************************************************************************************/
    InitVars();           // Initialize variables

                          // Create our OpenGL window
    if (!CreateGLWindow("3D Shapes", 800, 600, 32, fullscreen)) {  // (Modified)
        return 0;                                         // Quit if window was not create
    }
    while (!done) {                                       // Loop that runs until donw = TRUE
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {   // Is there a message wating
            if (msg.message == WM_QUIT) {                 // Havw we received a quit message
                done = TRUE;                              // if so done  = TRUE
            }
            else {                                        // If not, deal with window message
                TranslateMessage(&msg);                   // Translate message
                DispatchMessage(&msg);                    // Dispatch message
            }
        }
        else {
            // Draw the scene. Watch for ESC key and quit message from DrawGLScene()
            if (active) {                                 // Program active
                if (keys[VK_ESCAPE]) {                    // Was ESC pressed
                    done = TRUE;                          // ESC signalled a quit
                }
                else {                                    // Not time to quit, update screen
                    idle();                             // Advance Simulation
                    DrawGLScene();                        // Draw scene
                    SwapBuffers(hDC);                     // Swap buffers (double buffering)
                }
            }
//******************************************************************************************************************************************/
//******************************************************************************************************************************************/
            if (!ProcessKeys())
                return 0;
            /*
            *  It allows us to press the F1 key to switch from fullscreen mode to
            *  windowed mode or windowed mode to fullscreen mode.
            */
            if (keys[VK_F1]) {                            // Is F1 being pressed
                keys[VK_F1] = FALSE;                      // If so make key FASLE
                KillGLWindow();                           // Kill our current window
                fullscreen = !fullscreen;                 // Toggle fullscreen / window mode
                                                          //Recreate our OpenGL window(modified)
                if (!CreateGLWindow("3D Shapes", 640, 480, 16, fullscreen)) {
                    return 0;                             // Quit if window was not create
                }
            }
        }
    }
    // Shutdown
    KillGLWindow();                                       // Kill the window
    glDeleteTextures(4, texture);
//******************************************************************************************************************************************/
//******************************************************************************************************************************************/
    return (msg.wParam);                                  // Exit the program
}