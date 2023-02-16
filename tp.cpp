

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstdio>
#include <cstdlib>

#include <algorithm>
#include <GL/glut.h>
#include "src/Vec3.h"
#include "src/Camera.h"
#include "src/Mesh.h"
#include "src/Skeleton.h"



using namespace std;


// -------------------------------------------
// OpenGL/GLUT application code.
// -------------------------------------------

static GLint window;
static unsigned int SCREENWIDTH = 640;
static unsigned int SCREENHEIGHT = 480;
static Camera camera;
static bool mouseRotatePressed = false;
static bool mouseMovePressed = false;
static bool mouseZoomPressed = false;
static int lastX=0, lastY=0, lastZoom=0;
static bool fullScreen = false;

Mesh mesh;

int displayed_bone = -1;

void printUsage () {
    cerr << endl
         << "Usage : ./main" << endl
         << "Keyboard commands" << endl
         << "------------------" << endl
         << " ?: Print help" << endl
         << " w: Toggle Wireframe Mode" << endl
         << " f: Toggle full screen mode" << endl
         << " <drag>+<left button>: rotate model" << endl
         << " <drag>+<right button>: move model" << endl
         << " <drag>+<middle button>: zoom" << endl << endl;
}

void usage () {
    printUsage ();
    exit (EXIT_FAILURE);
}



// ------------------------------------

void initLight () {
    GLfloat light_position1[4] = {22.0f, 16.0f, 50.0f, 0.0f};
    GLfloat direction1[3] = {-52.0f,-16.0f,-50.0f};
    GLfloat color1[4] = {1.0f, 1.0f, 1.0f, 1.0f};
    GLfloat ambient[4] = {0.3f, 0.3f, 0.3f, 0.5f};

    glLightfv (GL_LIGHT1, GL_POSITION, light_position1);
    glLightfv (GL_LIGHT1, GL_SPOT_DIRECTION, direction1);
    glLightfv (GL_LIGHT1, GL_DIFFUSE, color1);
    glLightfv (GL_LIGHT1, GL_SPECULAR, color1);
    glLightModelfv (GL_LIGHT_MODEL_AMBIENT, ambient);
    glEnable (GL_LIGHT1);
    glEnable (GL_LIGHTING);
}

void init () {
    camera.resize (SCREENWIDTH, SCREENHEIGHT);
    initLight ();
    glCullFace (GL_BACK);
    glEnable (GL_CULL_FACE);
    glDepthFunc (GL_LESS);
    glEnable (GL_DEPTH_TEST);
    glClearColor (0.2f, 0.2f, 0.3f, 1.0f);
    glEnable(GL_COLOR_MATERIAL);

}

Vec3 vecMin, vecMax;

int calculBoite = 0;

void calculBoiteEnglobante () {
    if (calculBoite == 0) {
    float xmin, xmax, ymin, ymax, zmin, zmax, epsilon = 0.02;

    xmin = FLT_MAX;
    xmax = -FLT_MAX;

    ymin = FLT_MAX;
    ymax = -FLT_MAX;

    zmin = FLT_MAX;
    zmax = -FLT_MAX;

    for (int i = 0; i < mesh.vertices.size(); i++) {

        if (mesh.vertices[mesh.triangles[i].v[0]].position[0] < xmin) {
            xmin = mesh.vertices[mesh.triangles[i].v[0]].position[0] - epsilon;
        }
        else if (mesh.vertices[mesh.triangles[i].v[0]].position[0] > xmax) {
            xmax = mesh.vertices[mesh.triangles[i].v[0]].position[0] + epsilon;
        }

        if (mesh.vertices[mesh.triangles[i].v[0]].position[1] < ymin) {
            ymin = mesh.vertices[mesh.triangles[i].v[0]].position[1] -  epsilon;
        }
        else if (mesh.vertices[mesh.triangles[i].v[0]].position[1] > ymax) {
            ymax = mesh.vertices[mesh.triangles[i].v[0]].position[1] + epsilon;
        }

        if (mesh.vertices[mesh.triangles[i].v[0]].position[2] < zmin) {
            zmin = mesh.vertices[mesh.triangles[i].v[0]].position[2] - epsilon;
        }
        else if (mesh.vertices[mesh.triangles[i].v[0]].position[2] > zmax) {
            zmax = mesh.vertices[mesh.triangles[i].v[0]].position[2] + epsilon;
        }

    }

    vecMin = Vec3(xmin, ymin, zmin);
    vecMax = Vec3(xmax, ymax, zmax);
    calculBoite = 1;
    }
}

void drawVector( Vec3 const & i_from, Vec3 const & i_to ) {

    glBegin(GL_LINES);
    glVertex3f( i_from[0] , i_from[1] , i_from[2] );
    glVertex3f( i_to[0] , i_to[1] , i_to[2] );
    glEnd();
}

Vec3 Min2, Min3, Min4, Max2, Max3, Max4;

void drawBox() {

    calculBoiteEnglobante();

    Min2 = Vec3(vecMax[0], vecMin[1], vecMin[2]);
    Min3 = Vec3(vecMax[0], vecMin[1], vecMax[2]);
    Min4 = Vec3(vecMin[0], vecMin[1], vecMax[2]);

    Max2 = Vec3(vecMax[0], vecMax[1], vecMin[2]);
    Max3 = Vec3(vecMin[0], vecMax[1], vecMin[2]);
    Max4 = Vec3(vecMin[0], vecMax[1], vecMax[2]);

    drawVector(vecMin, Min2);
    drawVector(Min2, Min3);
    drawVector(Min3, Min4);
    drawVector(Min4, vecMin);

    drawVector(vecMin, Max3);
    drawVector(Min2, Max2);
    drawVector(Min3, vecMax);
    drawVector(Min4, Max4);

    drawVector(vecMax, Max2);
    drawVector(Max2, Max3);
    drawVector(Max3, Max4);
    drawVector(Max4, vecMax);

}

float maxBoiteEnglobante() {
    calculBoiteEnglobante();

    float max = -FLT_MAX;

    float dist1 =  sqrt(pow(Min2[0]-vecMin[0], 2) + pow(Min2[1]-vecMin[1], 2) + pow(Min2[2]-vecMin[2], 2));
    float dist2 =  sqrt(pow(Min3[0]-Min2[0], 2) + pow(Min3[1]-Min2[1], 2) + pow(Min3[2]-Min2[2], 2));
    float dist3 =  sqrt(pow(Min4[0]-Min3[0], 2) + pow(Min4[1]-Min3[1], 2) + pow(Min4[2]-Min3[2], 2));
    float dist4 =  sqrt(pow(vecMin[0]-Min4[0], 2) + pow(vecMin[1]-Min4[1], 2) + pow(vecMin[2]-Min4[2], 2));

    float dist5 =  sqrt(pow(Max3[0]-vecMin[0], 2) + pow(Max3[1]-vecMin[1], 2) + pow(Max3[2]-vecMin[2], 2));
    float dist6 =  sqrt(pow(Max2[0]-Min2[0], 2) + pow(Max2[1]-Min2[1], 2) + pow(Max2[2]-Min2[2], 2));
    float dist7 =  sqrt(pow(vecMax[0]-Min3[0], 2) + pow(vecMax[1]-Min3[1], 2) + pow(vecMax[2]-Min3[2], 2));
    float dist8 =  sqrt(pow(Max4[0]-Min4[0], 2) + pow(Max4[1]-Min4[1], 2) + pow(Max4[2]-Min4[2], 2));

    float dist9 =  sqrt(pow(Max2[0]-vecMax[0], 2) + pow(Max2[1]-vecMax[1], 2) + pow(Max2[2]-vecMax[2], 2));
    float dist10 =  sqrt(pow(Max3[0]-Max2[0], 2) + pow(Max3[1]-Max2[1], 2) + pow(Max3[2]-Max2[2], 2));
    float dist11 =  sqrt(pow(Max4[0]-Max3[0], 2) + pow(Max4[1]-Max3[1], 2) + pow(Max4[2]-Max3[2], 2));
    float dist12 =  sqrt(pow(vecMax[0]-Max4[0], 2) + pow(vecMax[1]-Max4[1], 2) + pow(vecMax[2]-Max4[2], 2));

    /*std::cout << " " << std::endl;
    std::cout << dist1 << std::endl;
    std::cout << dist2 << std::endl;
    std::cout << dist3 << std::endl;
    std::cout << dist4 << std::endl;

    std::cout << dist5 << std::endl;
    std::cout << dist6 << std::endl;
    std::cout << dist7 << std::endl;
    std::cout << dist8 << std::endl;

    std::cout << dist9 << std::endl;
    std::cout << dist10 << std::endl;
    std::cout << dist11 << std::endl;
    std::cout << dist12 << std::endl;
    std::cout << " " << std::endl;*/

    if (dist1 > max) {
        max = dist1;
    }
    if (dist2 > max) {
        max = dist2;
    }
    if (dist3 > max) {
        max = dist3;
    }
    if (dist4 > max) {
        max = dist4;
    }
    if (dist5 > max) {
        max = dist5;
    }
    if (dist6 > max) {
        max = dist6;
    }
    if (dist7 > max) {
        max = dist7;
    }
    if (dist8 > max) {
        max = dist8;
    }
    if (dist9 > max) {
        max = dist9;
    }
    if (dist10 > max) {
        max = dist10;
    }
    if (dist11 > max) {
        max = dist11;
    }
    if (dist12 > max) {
        max = dist12;
    }

    return max;
}


void quantification(int qp) {
    std::vector<MeshVertex> verticesQuantifies;
    verticesQuantifies.resize(mesh.vertices.size());

    float range = maxBoiteEnglobante();

    for (int i = 0; i < mesh.vertices.size(); i++) {

        if (i == 0) {
            std::cout << mesh.vertices[i].position[0] << " " << mesh.vertices[i].position[1] << " " << mesh.vertices[i].position[2] << std::endl;
        }

        verticesQuantifies[i].position[0] = (mesh.vertices[i].position[0]-vecMin[0]) * pow(2, qp) / range;
        verticesQuantifies[i].position[1] = (mesh.vertices[i].position[1]-vecMin[1]) * pow(2, qp) / range;
        verticesQuantifies[i].position[2] = (mesh.vertices[i].position[2]-vecMin[2]) * pow(2, qp) / range;

         if (i == 0) {
            std::cout << verticesQuantifies[i].position[0] << " " << verticesQuantifies[i].position[1] << " " << verticesQuantifies[i].position[2] << std::endl;
        }
    }

    for (int i = 0; i < mesh.vertices.size(); i++) {
        mesh.vertices[i].position = Vec3(verticesQuantifies[i].position[0], verticesQuantifies[i].position[1], verticesQuantifies[i].position[2]);
    }

}

void dequantification(int qp) {
    std::vector<MeshVertex> verticesDeQuantifies;
    verticesDeQuantifies.resize(mesh.vertices.size());

    float range = maxBoiteEnglobante();

    for (int i = 0; i < mesh.vertices.size(); i++) {
        verticesDeQuantifies[i].position[0] = mesh.vertices[i].position[0] * range / pow(2, qp) + vecMin[0];
        verticesDeQuantifies[i].position[1] = mesh.vertices[i].position[1] * range / pow(2, qp) + vecMin[1];
        verticesDeQuantifies[i].position[2] = mesh.vertices[i].position[2] * range / pow(2, qp) + vecMin[2];

        if (i == 0) {
            std::cout << verticesDeQuantifies[i].position[0] << " " << verticesDeQuantifies[i].position[1] << " " << verticesDeQuantifies[i].position[2] << std::endl;
        }
    }

    for (int i = 0; i < mesh.vertices.size(); i++) {
        mesh.vertices[i].position = Vec3(verticesDeQuantifies[i].position[0], verticesDeQuantifies[i].position[1], verticesDeQuantifies[i].position[2]);
    }
}


void draw () {

    glColor3f(0.8, 0.8, 0.8);

    mesh.draw( displayed_bone );

    //drawBox();

}


void display () {
    glLoadIdentity ();
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    camera.apply ();
    draw ();
    glFlush ();
    glutSwapBuffers ();
}

void idle () {
    glutPostRedisplay ();
}

void key (unsigned char keyPressed, int x, int y) {
    switch (keyPressed) {
    case 'f':
        if (fullScreen == true) {
            glutReshapeWindow (SCREENWIDTH, SCREENHEIGHT);
            fullScreen = false;
        } else {
            glutFullScreen ();
            fullScreen = true;
        }
        break;

    case 'w':
        GLint polygonMode[2];
        glGetIntegerv(GL_POLYGON_MODE, polygonMode);
        if(polygonMode[0] != GL_FILL)
            glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
        else
            glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);
        break;
    
    case 'a' : 
        quantification(20);
        break;

    case 'z' : 
        dequantification(20);
        break;

    default:
        printUsage ();
        break;
    }
    idle ();
}
void specialKey(int key, int x, int y)
{
    switch (key)
    {
    case GLUT_KEY_DOWN :
        --displayed_bone;
        displayed_bone = std::max( displayed_bone , -1 );
        break;
    }
    idle ();
}

void mouse (int button, int state, int x, int y) {
    if (state == GLUT_UP) {
        mouseMovePressed = false;
        mouseRotatePressed = false;
        mouseZoomPressed = false;
    } else {
        if (button == GLUT_LEFT_BUTTON) {
            camera.beginRotate (x, y);
            mouseMovePressed = false;
            mouseRotatePressed = true;
            mouseZoomPressed = false;
        } else if (button == GLUT_RIGHT_BUTTON) {
            lastX = x;
            lastY = y;
            mouseMovePressed = true;
            mouseRotatePressed = false;
            mouseZoomPressed = false;
        } else if (button == GLUT_MIDDLE_BUTTON) {
            if (mouseZoomPressed == false) {
                lastZoom = y;
                mouseMovePressed = false;
                mouseRotatePressed = false;
                mouseZoomPressed = true;
            }
        }
    }
    idle ();
}

void motion (int x, int y) {
    if (mouseRotatePressed == true) {
        camera.rotate (x, y);
    }
    else if (mouseMovePressed == true) {
        camera.move ((x-lastX)/static_cast<float>(SCREENWIDTH), (lastY-y)/static_cast<float>(SCREENHEIGHT), 0.0);
        lastX = x;
        lastY = y;
    }
    else if (mouseZoomPressed == true) {
        camera.zoom (float (y-lastZoom)/SCREENHEIGHT);
        lastZoom = y;
    }
}


void reshape(int w, int h) {
    camera.resize (w, h);
}




int main (int argc, char ** argv) {
    if (argc > 2) {
        printUsage ();
        exit (EXIT_FAILURE);
    }
    glutInit (&argc, argv);
    glutInitDisplayMode (GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
    glutInitWindowSize (SCREENWIDTH, SCREENHEIGHT);
    window = glutCreateWindow ("TP");

    init ();
    glutIdleFunc (idle);
    glutDisplayFunc (display);
    glutKeyboardFunc (key);
    glutSpecialFunc(specialKey);
    glutReshapeFunc (reshape);
    glutMotionFunc (motion);
    glutMouseFunc (mouse);
    key ('?', 0, 0);


    mesh.loadOFF("data/bunny.off");

    glutMainLoop ();
    return EXIT_SUCCESS;
}

