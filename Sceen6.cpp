#include <windows.h>
#include <mmsystem.h>
#include <GL/glut.h>
#include <cmath>
#include <cstdlib>
#include <string>
#include <iostream>

// Link Windows Multimedia API for sound support
#pragma comment(lib, "winmm.lib")

using namespace std;

// ============================================================
// SCENE 5: 3D DIU Classroom (Windows GLUT Version) - FIXED
// ============================================================

const int WINDOW_WIDTH = 1200;
const int WINDOW_HEIGHT = 720;
const float PI = 3.14159265358979323846f;

// ------------------------------------------------------------
// Animation state
// ------------------------------------------------------------

bool started = false;
bool paused = false;

int storyStage = 0;
int frameCounter = 0;

// Student movement variables adjusted for 3-row layout
float studentX = 5.5f;
float studentZ = 7.0f;
float studentY = 0.0f;
float studentRotation = 180.0f;
float studentWalkPhase = 0.0f;
float studentSitAmount = 0.0f;

// Chair & Laptop Animation Variables
float targetChairOffsetZ = 0.0f;
float mainLaptopAngle = 0.0f; // 0 = Closed, 105 = Fully Open

// Teacher animation
float teacherArmAngle = 0.0f;
float teacherWalk = 0.0f;

// Classroom animation
float fanAngle = 0.0f;
float clockSecondAngle = 0.0f;
float curtainMove = 0.0f;
float lightIntensity = 1.0f;

// Camera
float cameraAngle = 25.0f;
float cameraHeight = 8.0f;
float cameraDistance = 22.0f;
float cameraTargetX = 0.0f;
float cameraTargetY = 2.5f;
float cameraTargetZ = 0.0f;

// ------------------------------------------------------------
// Windows Native Sound System
// ------------------------------------------------------------

void playSoundEffect(const string& path)
{
    PlaySoundA(path.c_str(), NULL, SND_FILENAME | SND_ASYNC);
}

void playLoopSound(const string& path)
{
    PlaySoundA(path.c_str(), NULL, SND_FILENAME | SND_ASYNC | SND_LOOP);
}

void stopAllSounds()
{
    PlaySoundA(NULL, NULL, 0);
}

void startWalkingSound()
{
    playLoopSound("sounds/walking.wav");
}

void startFanSound()
{
    playLoopSound("sounds/fan.wav");
}

// Updated to use your new class lecture audio path
void startClassroomSound()
{
    playLoopSound("D:\\8_th_Semister\\Computer_Graphics_Practice\\S6.test2_sound\\Class_Lecture.wav");
}

// ------------------------------------------------------------
// Utility functions
// ------------------------------------------------------------

void drawBitmapText(float x, float y, float z, const string& text, void* font = GLUT_BITMAP_HELVETICA_18)
{
    glRasterPos3f(x, y, z);
    for (char c : text) glutBitmapCharacter(font, c);
}

void draw2DText(float x, float y, const string& text, void* font = GLUT_BITMAP_HELVETICA_18)
{
    glRasterPos2f(x, y);
    for (char c : text) glutBitmapCharacter(font, c);
}

void cube(float sx, float sy, float sz)
{
    glPushMatrix();
    glScalef(sx, sy, sz);
    glutSolidCube(1.0);
    glPopMatrix();
}

void cylinder(float radius, float height, int slices = 32)
{
    GLUquadric* quad = gluNewQuadric();
    glPushMatrix();
    glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
    gluCylinder(quad, radius, radius, height, slices, 1);
    gluDisk(quad, 0.0, radius, slices, 1);
    glTranslatef(0.0f, 0.0f, height);
    gluDisk(quad, 0.0, radius, slices, 1);
    glPopMatrix();
    gluDeleteQuadric(quad);
}

void sphere(float radius)
{
    glutSolidSphere(radius, 30, 30);
}

// Enhanced Material system for specular gloss realism
void setMaterial(float r, float g, float b, float shininess = 30.0f, float specIntensity = 0.35f)
{
    GLfloat ambient[] = { r * 0.45f, g * 0.45f, b * 0.45f, 1.0f };
    GLfloat diffuse[] = { r, g, b, 1.0f };
    GLfloat specular[] = { specIntensity, specIntensity, specIntensity, 1.0f };

    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shininess);
}

// Helper to ease animations smoothly
float easeInOut(float t) {
    return t * t * (3.0f - 2.0f * t);
}

// ------------------------------------------------------------
// Classroom structure & Props
// ------------------------------------------------------------

void drawFloor()
{
    setMaterial(0.65f, 0.58f, 0.45f, 15.0f, 0.1f);

    glPushMatrix();
    glTranslatef(0.0f, -0.15f, 0.0f);
    cube(18.0f, 0.3f, 16.0f);
    glPopMatrix();

    glDisable(GL_LIGHTING);
    glColor3f(0.40f, 0.35f, 0.28f);
    glLineWidth(1.5f);

    for (int x = -9; x <= 9; x++)
    {
        glBegin(GL_LINES);
        glVertex3f((float)x, 0.01f, -8.0f);
        glVertex3f((float)x, 0.01f, 8.0f);
        glEnd();
    }
    for (int z = -8; z <= 8; z++)
    {
        glBegin(GL_LINES);
        glVertex3f(-9.0f, 0.01f, (float)z);
        glVertex3f(9.0f, 0.01f, (float)z);
        glEnd();
    }
    glEnable(GL_LIGHTING);
}

void drawWalls()
{
    setMaterial(0.76f, 0.69f, 0.55f, 5.0f, 0.0f); // Cream walls

    // Back Wall
    glPushMatrix();
    glTranslatef(0.0f, 4.5f, -8.0f);
    cube(18.0f, 9.0f, 0.25f);
    glPopMatrix();

    // Left Wall
    glPushMatrix();
    glTranslatef(-9.0f, 4.5f, 0.0f);
    cube(0.25f, 9.0f, 16.0f);
    glPopMatrix();

    // Right Wall
    glPushMatrix();
    glTranslatef(9.0f, 4.5f, 0.0f);
    cube(0.25f, 9.0f, 16.0f);
    glPopMatrix();

    // Ceiling
    setMaterial(0.80f, 0.80f, 0.78f, 5.0f, 0.0f);
    glPushMatrix();
    glTranslatef(0.0f, 9.0f, 0.0f);
    cube(18.0f, 0.25f, 16.0f);
    glPopMatrix();
}

void drawDoor()
{
    setMaterial(0.35f, 0.20f, 0.10f, 30.0f, 0.2f);
    glPushMatrix();
    glTranslatef(7.8f, 3.2f, -7.82f);
    cube(2.0f, 6.4f, 0.18f);
    glPopMatrix();

    setMaterial(0.46f, 0.24f, 0.10f, 30.0f);
    glPushMatrix();
    glTranslatef(7.8f, 3.0f, -7.66f);
    cube(1.65f, 5.8f, 0.16f);
    glPopMatrix();

    setMaterial(0.34f, 0.68f, 0.86f, 100.0f, 0.9f); // Glossy door window
    glPushMatrix();
    glTranslatef(7.8f, 4.4f, -7.55f);
    cube(1.1f, 1.7f, 0.08f);
    glPopMatrix();

    setMaterial(0.82f, 0.70f, 0.18f, 100.0f, 0.8f); // Metallic handle
    glPushMatrix();
    glTranslatef(7.2f, 2.9f, -7.42f);
    glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
    cylinder(0.07f, 0.40f);
    glPopMatrix();
}

void drawWindows()
{
    for (int i = 0; i < 3; i++)
    {
        float z = -4.8f + i * 4.5f;

        setMaterial(0.20f, 0.22f, 0.25f, 40.0f); // Frame
        glPushMatrix();
        glTranslatef(-8.82f, 5.1f, z);
        cube(0.18f, 3.3f, 3.2f);
        glPopMatrix();

        setMaterial(0.40f, 0.75f, 0.95f, 120.0f, 0.9f); // Reflective glass
        glPushMatrix();
        glTranslatef(-8.68f, 5.1f, z);
        cube(0.08f, 2.9f, 2.8f);
        glPopMatrix();

        setMaterial(0.15f, 0.15f, 0.18f, 30.0f); // Pane dividers
        glPushMatrix();
        glTranslatef(-8.56f, 5.1f, z);
        cube(0.06f, 0.10f, 2.8f);
        glPopMatrix();
        glPushMatrix();
        glTranslatef(-8.56f, 5.1f, z);
        cube(0.06f, 2.9f, 0.10f);
        glPopMatrix();

        setMaterial(0.22f, 0.55f, 0.82f, 10.0f, 0.0f); // Curtains
        glPushMatrix();
        glTranslatef(-8.45f, 5.0f, z - 1.2f + sin(curtainMove + i) * 0.12f);
        cube(0.10f, 3.1f, 0.55f);
        glPopMatrix();
    }
}

void drawBoard()
{
    setMaterial(0.70f, 0.70f, 0.75f, 80.0f, 0.8f);
    glPushMatrix();
    glTranslatef(0.0f, 5.3f, -7.70f);
    cube(8.7f, 3.7f, 0.22f);
    glPopMatrix();

    setMaterial(0.95f, 0.95f, 0.95f, 100.0f, 0.6f);
    glPushMatrix();
    glTranslatef(0.0f, 5.3f, -7.53f);
    cube(8.4f, 3.4f, 0.12f);
    glPopMatrix();

    setMaterial(0.65f, 0.65f, 0.70f, 60.0f, 0.5f);
    glPushMatrix();
    glTranslatef(0.0f, 3.55f, -7.30f);
    cube(8.5f, 0.12f, 0.45f);
    glPopMatrix();

    setMaterial(0.1f, 0.1f, 0.8f, 50.0f, 0.5f);
    glPushMatrix(); glTranslatef(-2.0f, 3.65f, -7.25f); glRotatef(15.0f, 0, 1, 0); cube(0.6f, 0.06f, 0.06f); glPopMatrix();
    setMaterial(0.8f, 0.1f, 0.1f, 50.0f, 0.5f);
    glPushMatrix(); glTranslatef(-1.8f, 3.65f, -7.15f); glRotatef(-10.0f, 0, 1, 0); cube(0.6f, 0.06f, 0.06f); glPopMatrix();
    setMaterial(0.2f, 0.2f, 0.2f, 10.0f, 0.1f);
    glPushMatrix(); glTranslatef(1.5f, 3.65f, -7.2f); cube(0.4f, 0.1f, 0.15f); glPopMatrix();

    glDisable(GL_LIGHTING);
    float textZ = -7.45f;
    glColor3f(0.05f, 0.05f, 0.05f);
    drawBitmapText(-2.7f, 5.9f, textZ, "COMPUTER GRAPHICS", GLUT_BITMAP_HELVETICA_18);
    glColor3f(0.15f, 0.15f, 0.60f);
    drawBitmapText(-3.1f, 5.2f, textZ, "2D Transformation  |  3D Transformation", GLUT_BITMAP_HELVETICA_12);
    glColor3f(0.60f, 0.15f, 0.15f);
    drawBitmapText(-2.4f, 4.6f, textZ, "Daffodil International University", GLUT_BITMAP_HELVETICA_12);
    glEnable(GL_LIGHTING);
}

void drawAC()
{
    setMaterial(0.9f, 0.9f, 0.92f, 80.0f, 0.5f);
    glPushMatrix();
    glTranslatef(7.5f, 7.5f, -2.0f);
    cube(0.5f, 1.2f, 3.5f);

    setMaterial(0.1f, 0.1f, 0.1f, 10.0f, 0.0f);
    glTranslatef(-0.26f, -0.2f, 0.0f);
    cube(0.02f, 0.2f, 3.0f);
    glPopMatrix();
}

void drawProjector()
{
    setMaterial(0.8f, 0.8f, 0.8f, 50.0f, 0.3f);
    glPushMatrix();
    glTranslatef(0.0f, 8.5f, -3.0f);
    cylinder(0.05f, 1.5f);

    glTranslatef(0.0f, 0.0f, 1.5f);
    cube(1.0f, 0.4f, 1.2f);

    setMaterial(0.1f, 0.1f, 0.1f, 90.0f, 1.0f);
    glTranslatef(0.0f, 0.0f, -0.62f);
    glRotatef(90.0f, 1, 0, 0);
    cylinder(0.15f, 0.1f);
    glPopMatrix();
}

void drawLaptop(float x, float y, float z, float rotation, float openAngle = 105.0f)
{
    glPushMatrix();
    glTranslatef(x, y, z);
    glRotatef(rotation, 0.0f, 1.0f, 0.0f);

    // Laptop Base
    setMaterial(0.15f, 0.15f, 0.15f, 60.0f, 0.5f);
    cube(0.8f, 0.04f, 0.6f);

    // Laptop Screen with hinge logic
    glTranslatef(0.0f, 0.02f, -0.28f);
    glRotatef(90.0f - openAngle, 1.0f, 0.0f, 0.0f);
    glTranslatef(0.0f, 0.35f, 0.0f);
    cube(0.8f, 0.7f, 0.04f);

    // Screen Glow
    if (openAngle > 10.0f) {
        glDisable(GL_LIGHTING);
        glColor3f(0.8f, 0.9f, 1.0f);
        glTranslatef(0.0f, 0.0f, 0.025f);
        glBegin(GL_QUADS);
            glVertex3f(-0.35f, -0.3f, 0.0f);
            glVertex3f( 0.35f, -0.3f, 0.0f);
            glVertex3f( 0.35f,  0.3f, 0.0f);
            glVertex3f(-0.35f,  0.3f, 0.0f);
        glEnd();
        glEnable(GL_LIGHTING);
    }

    glPopMatrix();
}

void drawTeacherTable()
{
    setMaterial(0.45f, 0.25f, 0.10f, 35.0f, 0.1f);
    glPushMatrix();
    glTranslatef(0.0f, 1.55f, -5.3f);
    cube(5.0f, 0.25f, 1.8f);
    glPopMatrix();

    for (float x : {-2.2f, 2.2f}) {
        for (float z : {-5.95f, -4.65f}) {
            glPushMatrix();
            glTranslatef(x, 0.75f, z);
            cube(0.22f, 1.5f, 0.22f);
            glPopMatrix();
        }
    }
    drawLaptop(-1.0f, 1.70f, -5.3f, 25.0f, 105.0f);
}

void drawDesk(float x, float z)
{
    setMaterial(0.55f, 0.32f, 0.15f, 40.0f, 0.2f);
    glPushMatrix();
    glTranslatef(x, 1.75f, z);
    cube(3.0f, 0.20f, 1.4f);
    glPopMatrix();

    setMaterial(0.20f, 0.22f, 0.24f, 50.0f, 0.4f);
    for (float dx : {-1.28f, 1.28f}) {
        for (float dz : {-0.50f, 0.50f}) {
            glPushMatrix();
            glTranslatef(x + dx, 0.85f, z + dz);
            cube(0.16f, 1.7f, 0.16f);
            glPopMatrix();
        }
    }
}

void drawChair(float x, float z)
{
    setMaterial(0.15f, 0.30f, 0.55f, 60.0f, 0.4f);
    glPushMatrix();
    glTranslatef(x, 1.0f, z);
    cube(1.2f, 0.18f, 1.1f);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(x, 1.75f, z + 0.48f);
    cube(1.2f, 1.4f, 0.15f);
    glPopMatrix();

    setMaterial(0.20f, 0.22f, 0.24f, 50.0f, 0.4f);
    for (float dx : {-0.48f, 0.48f}) {
        for (float dz : {-0.38f, 0.38f}) {
            glPushMatrix();
            glTranslatef(x + dx, 0.48f, z + dz);
            cube(0.12f, 0.95f, 0.12f);
            glPopMatrix();
        }
    }
}

void drawClassroomFurniture()
{
    float rowZ[] = {-1.5f, 1.0f, 3.5f};
    float colX[] = {-5.4f, -1.8f, 1.8f, 5.4f};

    for (float z : rowZ)
    {
        for (float x : colX)
        {
            drawDesk(x, z);

            bool hasStaticStudent = false;
            if (abs(x - (-5.4f)) < 0.1f && abs(z - (-1.5f)) < 0.1f) hasStaticStudent = true;
            if (abs(x - (-1.8f)) < 0.1f && abs(z - 1.0f) < 0.1f) hasStaticStudent = true;
            if (abs(x - 5.4f) < 0.1f && abs(z - (-1.5f)) < 0.1f) hasStaticStudent = true;
            if (abs(x - 5.4f) < 0.1f && abs(z - 3.5f) < 0.1f) hasStaticStudent = true;

            if (hasStaticStudent) {
                drawLaptop(x + 0.3f, 1.87f, z, 10.0f, 105.0f);
            }

            if (abs(x - 1.8f) < 0.1f && abs(z - 3.5f) < 0.1f) {
                drawLaptop(x + 0.3f, 1.87f, z, 10.0f, mainLaptopAngle);
                drawChair(x, z + 1.25f + targetChairOffsetZ);
            } else {
                drawChair(x, z + 1.25f);
            }
        }
    }
}

void drawFan()
{
    glPushMatrix();
    glTranslatef(0.0f, 8.3f, 0.0f);

    setMaterial(0.8f, 0.8f, 0.8f, 60.0f, 0.5f);
    glPushMatrix();
    glTranslatef(0.0f, 0.15f, 0.0f);
    cylinder(0.10f, 0.55f);
    glPopMatrix();

    glTranslatef(0.0f, -0.25f, 0.0f);
    glRotatef(fanAngle, 0.0f, 1.0f, 0.0f);
    sphere(0.30f);

    for (int i = 0; i < 4; i++)
    {
        glPushMatrix();
        glRotatef(i * 90.0f, 0.0f, 1.0f, 0.0f);
        glTranslatef(1.35f, 0.0f, 0.0f);
        glRotatef(15.0f, 1.0f, 0.0f, 0.0f);
        cube(2.4f, 0.05f, 0.42f);
        glPopMatrix();
    }
    glPopMatrix();
}

void drawTubeLight(float x, float z)
{
    setMaterial(0.95f * lightIntensity, 0.95f * lightIntensity, 0.90f * lightIntensity, 100.0f, 1.0f);
    glPushMatrix();
    glTranslatef(x, 8.75f, z);
    cube(3.2f, 0.15f, 0.35f);

    glDisable(GL_LIGHTING);
    glColor3f(1.0f * lightIntensity, 1.0f * lightIntensity, 0.9f * lightIntensity);
    glTranslatef(0.0f, -0.08f, 0.0f);
    cube(3.1f, 0.05f, 0.25f);
    glEnable(GL_LIGHTING);
    glPopMatrix();
}

void drawLights()
{
    drawTubeLight(-4.5f, -3.0f);
    drawTubeLight(4.5f, -3.0f);
    drawTubeLight(-4.5f, 3.0f);
    drawTubeLight(4.5f, 3.0f);
}

void drawClock()
{
    glPushMatrix();
    glTranslatef(6.7f, 6.5f, -7.45f);

    setMaterial(0.2f, 0.2f, 0.2f, 60.0f, 0.5f);
    sphere(0.65f);

    glTranslatef(0.0f, 0.0f, 0.48f);
    setMaterial(0.95f, 0.95f, 0.90f, 40.0f, 0.2f);
    sphere(0.50f);

    glDisable(GL_LIGHTING);
    glColor3f(0.06f, 0.06f, 0.07f);

    for (int i=0; i<12; i++) {
        glPushMatrix();
        glRotatef(i * 30.0f, 0.0f, 0.0f, 1.0f);
        glTranslatef(0.0f, 0.38f, 0.08f);
        cube(0.04f, 0.1f, 0.02f);
        glPopMatrix();
    }

    glLineWidth(4.0f);
    glBegin(GL_LINES); glVertex3f(0, 0, 0.12f); glVertex3f(0.18f, 0.22f, 0.12f); glEnd();
    glLineWidth(3.0f);
    glBegin(GL_LINES); glVertex3f(0, 0, 0.13f); glVertex3f(-0.30f, 0.12f, 0.13f); glEnd();

    glPushMatrix();
    glRotatef(clockSecondAngle, 0.0f, 0.0f, 1.0f);
    glColor3f(0.85f, 0.05f, 0.05f);
    glLineWidth(2.0f);
    glBegin(GL_LINES); glVertex3f(0, 0, 0.14f); glVertex3f(0, 0.35f, 0.14f); glEnd();
    glPopMatrix();

    glEnable(GL_LIGHTING);
    glPopMatrix();
}

void drawHuman(float x, float y, float z, float rotation, float scale, float shirtR, float shirtG, float shirtB, bool walking, bool sitting, bool teacher, float sitProgress = 1.0f)
{
    glPushMatrix();
    glTranslatef(x, y, z);
    glRotatef(rotation, 0.0f, 1.0f, 0.0f);
    glScalef(scale, scale, scale);

    float legSwing = walking ? sin(studentWalkPhase) * 35.0f : 0.0f;
    float armSwing = walking ? -legSwing : 0.0f;
    if (teacher) armSwing = teacherArmAngle;

    float currentThighAngle = sitting ? (90.0f * easeInOut(sitProgress)) : 0.0f;
    float currentKneeAngle = sitting ? (-90.0f * easeInOut(sitProgress)) : 0.0f;
    float currentTorsoTilt = sitting ? (8.0f * (1.0f - easeInOut(sitProgress))) : 0.0f;

    setMaterial(shirtR, shirtG, shirtB, 30.0f, 0.1f);
    glPushMatrix();
    glTranslatef(0.0f, 2.65f - (sitting ? 0.3f * easeInOut(sitProgress) : 0.0f), 0.0f);
    glRotatef(currentTorsoTilt, 1.0f, 0.0f, 0.0f);

    glPushMatrix();
    glScalef(1.0f, 1.0f, 0.8f);
    cube(0.95f, 1.55f, 0.65f);
    glPopMatrix();

    setMaterial(0.76f, 0.53f, 0.35f, 35.0f);
    glPushMatrix();
    glTranslatef(0.0f, 0.85f, 0.0f);
    cube(0.25f, 0.3f, 0.25f);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.0f, 1.25f, 0.0f);
    sphere(0.42f);

    setMaterial(0.12f, 0.08f, 0.05f, 40.0f, 0.6f);
    glPushMatrix();
    glTranslatef(0.0f, 0.25f, -0.05f);
    glScalef(1.05f, 0.5f, 1.08f);
    sphere(0.42f);
    glPopMatrix();

    glDisable(GL_LIGHTING);
    glColor3f(0.05f, 0.05f, 0.05f);
    glPushMatrix(); glTranslatef(-0.16f, 0.05f, 0.38f); sphere(0.04f); glPopMatrix();
    glPushMatrix(); glTranslatef(0.16f, 0.05f, 0.38f); sphere(0.04f); glPopMatrix();
    glEnable(GL_LIGHTING);
    glPopMatrix();

    setMaterial(shirtR, shirtG, shirtB, 30.0f, 0.1f);
    float armRestAngle = sitting ? (25.0f * easeInOut(sitProgress)) : armSwing;

    for(float side : {-1.0f, 1.0f}) {
        glPushMatrix();
        glTranslatef(side * 0.55f, 0.55f, 0.0f);
        glRotatef(side < 0 ? armRestAngle : (teacher ? -armSwing : armRestAngle), 1.0f, 0.0f, 0.0f);

        sphere(0.22f);

        setMaterial(0.76f, 0.53f, 0.35f, 35.0f);
        glTranslatef(0.0f, -0.65f, 0.0f);
        cube(0.20f, 1.2f, 0.20f);
        glPopMatrix();
    }
    glPopMatrix();

    setMaterial(0.12f, 0.15f, 0.22f, 30.0f, 0.1f);

    for (float legSide : {-0.25f, 0.25f}) {
        glPushMatrix();
        if (!sitting) {
            glTranslatef(legSide, 1.55f, 0.0f);
            glRotatef(legSide < 0 ? legSwing : -legSwing, 1.0f, 0.0f, 0.0f);

            glTranslatef(0.0f, -0.7f, 0.0f);
            cube(0.32f, 1.4f, 0.35f);

            setMaterial(0.05f, 0.05f, 0.05f, 60.0f, 0.4f);
            glTranslatef(0.0f, -0.75f, 0.1f);
            cube(0.38f, 0.22f, 0.55f);
        } else {
            glTranslatef(legSide, 1.85f - (0.35f * easeInOut(sitProgress)), 0.0f);
            glRotatef(currentThighAngle, 1.0f, 0.0f, 0.0f);

            sphere(0.2f);
            glTranslatef(0.0f, -0.4f, 0.0f);
            cube(0.32f, 0.8f, 0.35f);

            glTranslatef(0.0f, -0.45f, 0.0f);
            sphere(0.18f);
            glRotatef(currentKneeAngle, 1.0f, 0.0f, 0.0f);

            glTranslatef(0.0f, -0.4f, 0.0f);
            cube(0.28f, 0.8f, 0.32f);

            setMaterial(0.05f, 0.05f, 0.05f, 60.0f, 0.4f);
            glTranslatef(0.0f, -0.45f, 0.15f);
            cube(0.35f, 0.22f, 0.55f);
        }
        setMaterial(0.12f, 0.15f, 0.22f, 30.0f, 0.1f);
        glPopMatrix();
    }
    glPopMatrix();
}

void drawPeople()
{
    bool studentWalking = (storyStage == 1 || storyStage == 2);
    bool studentSitting = (storyStage >= 4);

    drawHuman(studentX, studentY, studentZ, studentRotation, 0.82f,
              0.15f, 0.25f, 0.62f, studentWalking, studentSitting, false, studentSitAmount);

    drawHuman(teacherWalk, 0.0f, -5.0f, 180.0f, 0.88f,
              0.82f, 0.82f, 0.85f, false, false, true, 0.0f);

    drawHuman(-5.4f, 0.0f, -0.25f, 180.0f, 0.68f, 0.2f, 0.6f, 0.3f, false, true, false, 1.0f);
    drawHuman(-1.8f, 0.0f, 2.25f, 180.0f, 0.68f, 0.7f, 0.2f, 0.2f, false, true, false, 1.0f);
    drawHuman(5.4f, 0.0f, -0.25f, 180.0f, 0.68f, 0.6f, 0.2f, 0.7f, false, true, false, 1.0f);
    drawHuman(5.4f, 0.0f, 4.75f, 180.0f, 0.68f, 0.2f, 0.5f, 0.7f, false, true, false, 1.0f);
}

string stageText()
{
    switch (storyStage) {
        case 0: return "Press S to start Scene 5.";
        case 1: return "The student enters the 3D classroom.";
        case 2: return "The student walks toward the assigned desk.";
        case 3: return "The student pulls the chair backward.";
        case 4: return "The student sits down and pulls the chair in.";
        case 5: return "The student opens their laptop.";
        case 6: return "The teacher begins the Computer Graphics lecture.";
        case 7: return "Scene 5 completed. Ready for class.";
        default: return "";
    }
}

void drawOverlay()
{
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0.0, WINDOW_WIDTH, 0.0, WINDOW_HEIGHT);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);

    glColor3f(0.02f, 0.05f, 0.09f);
    glBegin(GL_QUADS);
        glVertex2f(0.0f, 675.0f); glVertex2f(1200.0f, 675.0f);
        glVertex2f(1200.0f, 720.0f); glVertex2f(0.0f, 720.0f);
    glEnd();

    glColor3f(1.0f, 1.0f, 1.0f);
    draw2DText(20.0f, 692.0f, "Scene 5: 3D DIU Computer Graphics Classroom (Fixed Props Edition)", GLUT_BITMAP_HELVETICA_18);

    glColor3f(0.02f, 0.04f, 0.07f);
    glBegin(GL_QUADS);
        glVertex2f(0.0f, 0.0f); glVertex2f(1200.0f, 0.0f);
        glVertex2f(1200.0f, 42.0f); glVertex2f(0.0f, 42.0f);
    glEnd();

    glColor3f(1.0f, 0.93f, 0.66f);
    draw2DText(18.0f, 16.0f, stageText(), GLUT_BITMAP_HELVETICA_18);

    glColor3f(0.90f, 0.90f, 0.92f);
    draw2DText(670.0f, 16.0f, "S Start | P Pause | R Reset | Arrows Camera | +/- Zoom | N Finish | Esc Exit", GLUT_BITMAP_HELVETICA_12);

    if (paused) {
        glColor3f(1.0f, 0.82f, 0.10f);
        draw2DText(560.0f, 645.0f, "PAUSED", GLUT_BITMAP_HELVETICA_18);
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

void setupLighting()
{
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHT1);
    glEnable(GL_LIGHT2);

    GLfloat ambient[] = { 0.25f * lightIntensity, 0.25f * lightIntensity, 0.28f * lightIntensity, 1.0f };
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient);

    GLfloat light0Pos[] = {0.0f, 8.0f, 0.0f, 1.0f};
    GLfloat light0Diff[] = {0.80f * lightIntensity, 0.80f * lightIntensity, 0.85f * lightIntensity, 1.0f};
    GLfloat light0Spec[] = {1.0f * lightIntensity, 1.0f * lightIntensity, 1.0f * lightIntensity, 1.0f};
    glLightfv(GL_LIGHT0, GL_POSITION, light0Pos);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light0Diff);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light0Spec);

    GLfloat light1Pos[] = {-8.0f, 4.0f, 2.0f, 1.0f};
    GLfloat light1Diff[] = {0.55f * lightIntensity, 0.45f * lightIntensity, 0.35f * lightIntensity, 1.0f};
    GLfloat light1Spec[] = {0.4f * lightIntensity, 0.3f * lightIntensity, 0.2f * lightIntensity, 1.0f};
    glLightfv(GL_LIGHT1, GL_POSITION, light1Pos);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, light1Diff);
    glLightfv(GL_LIGHT1, GL_SPECULAR, light1Spec);

    GLfloat light2Pos[] = {0.0f, 6.0f, -5.0f, 1.0f};
    GLfloat light2Diff[] = {0.30f * lightIntensity, 0.30f * lightIntensity, 0.35f * lightIntensity, 1.0f};
    glLightfv(GL_LIGHT2, GL_POSITION, light2Pos);
    glLightfv(GL_LIGHT2, GL_DIFFUSE, light2Diff);
}

void setupCamera()
{
    float rad = cameraAngle * PI / 180.0f;
    float eyeX = cameraTargetX + sin(rad) * cameraDistance;
    float eyeZ = cameraTargetZ + cos(rad) * cameraDistance;

    gluLookAt(eyeX, cameraHeight, eyeZ, cameraTargetX, cameraTargetY, cameraTargetZ, 0.0f, 1.0f, 0.0f);
}

void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    setupCamera();
    setupLighting();

    drawFloor();
    drawWalls();
    drawDoor();
    drawWindows();
    drawBoard();
    drawTeacherTable();
    drawProjector();
    drawAC();
    drawClassroomFurniture();
    drawFan();
    drawLights();
    drawClock();
    drawPeople();
    drawOverlay();

    glutSwapBuffers();
}

void updateStory()
{
    if (!started || paused) return;

    frameCounter++;

    switch (storyStage)
    {
        case 0:
            storyStage = 1;
            frameCounter = 0;
            startWalkingSound();
            break;

        case 1:
            studentZ -= 0.045f;
            studentWalkPhase += 0.22f;
            if (studentZ <= 5.20f) {
                studentZ = 5.20f;
                storyStage = 2;
                frameCounter = 0;
            }
            break;

        case 2:
            studentX -= 0.055f;
            studentWalkPhase += 0.22f;
            if (studentX <= 1.8f) {
                studentX = 1.8f;
                studentRotation = 180.0f;
                storyStage = 3;
                frameCounter = 0;
                stopAllSounds();
            }
            break;

        case 3:
            targetChairOffsetZ += 0.025f;
            if (targetChairOffsetZ >= 0.85f) {
                targetChairOffsetZ = 0.85f;
                storyStage = 4;
                frameCounter = 0;
                playSoundEffect("sounds/chair.wav");
            }
            break;

        case 4:
            studentSitAmount += 0.025f;
            if (studentSitAmount > 1.0f) studentSitAmount = 1.0f;

            studentY = -0.45f * easeInOut(studentSitAmount);
            studentZ = 5.20f - (0.45f * easeInOut(studentSitAmount));
            targetChairOffsetZ = 0.85f * (1.0f - easeInOut(studentSitAmount));

            if (studentSitAmount >= 1.0f) {
                studentSitAmount = 1.0f;
                studentY = -0.45f;
                studentZ = 4.75f;
                targetChairOffsetZ = 0.0f;
                storyStage = 5;
                frameCounter = 0;
            }
            break;

        case 5:
            mainLaptopAngle += 2.5f;
            if (mainLaptopAngle >= 105.0f) {
                mainLaptopAngle = 105.0f;
                storyStage = 6;
                frameCounter = 0;

                playSoundEffect("sounds/bell.wav");
                startClassroomSound(); // Starts your custom audio loop when lecture begins
            }
            break;

        case 6:
            teacherArmAngle = 35.0f + sin(frameCounter * 0.06f) * 24.0f;
            teacherWalk = sin(frameCounter * 0.018f) * 1.0f;

            if (frameCounter > 500) {
                storyStage = 7;
                frameCounter = 0;
                teacherArmAngle = 0.0f;
            }
            break;

        case 7:
            break;
    }
}

void timer(int value)
{
    if (!paused)
    {
        fanAngle += 10.0f;
        if (fanAngle >= 360.0f) fanAngle -= 360.0f;

        clockSecondAngle -= 1.0f;
        if (clockSecondAngle <= -360.0f) clockSecondAngle += 360.0f;

        curtainMove += 0.035f;
        updateStory();
    }
    glutPostRedisplay();
    glutTimerFunc(16, timer, 0);
}

void resetScene()
{
    stopAllSounds();
    started = false;
    paused = false;
    storyStage = 0;
    frameCounter = 0;

    studentX = 5.5f;
    studentZ = 7.0f;
    studentY = 0.0f;
    studentRotation = 180.0f;
    studentWalkPhase = 0.0f;
    studentSitAmount = 0.0f;
    targetChairOffsetZ = 0.0f;

    mainLaptopAngle = 0.0f;

    teacherArmAngle = 0.0f;
    teacherWalk = 0.0f;
    fanAngle = 0.0f;
    clockSecondAngle = 0.0f;
    curtainMove = 0.0f;
    lightIntensity = 1.0f;

    cameraAngle = 25.0f;
    cameraHeight = 8.0f;
    cameraDistance = 22.0f;

    glutPostRedisplay();
}

void keyboard(unsigned char key, int x, int y)
{
    switch (key)
    {
        case 's': case 'S':
            started = true; paused = false;
            break;
        case 'p': case 'P':
            paused = !paused;
            if (paused) {
                stopAllSounds(); // Stops the custom audio on pause
            } else {
                if (started) {
                    if (storyStage == 1 || storyStage == 2) startWalkingSound();
                    else if (storyStage >= 6) startClassroomSound(); // Resumes your audio if paused mid-lecture
                }
            }
            break;
        case 'r': case 'R': resetScene(); break;
        case 'n': case 'N':
            storyStage = 7;
            studentX = 1.8f;
            studentY = -0.45f;
            studentZ = 4.75f;
            studentSitAmount = 1.0f;
            targetChairOffsetZ = 0.0f;
            studentRotation = 180.0f;

            mainLaptopAngle = 105.0f;

            teacherArmAngle = 0.0f;
            stopAllSounds();
            break;
        case '+': case '=':
            cameraDistance -= 0.8f;
            if (cameraDistance < 10.0f) cameraDistance = 10.0f;
            break;
        case '-': case '_':
            cameraDistance += 0.8f;
            if (cameraDistance > 34.0f) cameraDistance = 34.0f;
            break;
        case 'l': case 'L':
            lightIntensity = (lightIntensity > 0.7f) ? 0.35f : 1.0f;
            break;
        case 27:
            stopAllSounds();
            exit(0);
    }
    glutPostRedisplay();
}

void specialKeys(int key, int x, int y)
{
    switch (key)
    {
        case GLUT_KEY_LEFT: cameraAngle -= 4.0f; break;
        case GLUT_KEY_RIGHT: cameraAngle += 4.0f; break;
        case GLUT_KEY_UP:
            cameraHeight += 0.5f;
            if (cameraHeight > 16.0f) cameraHeight = 16.0f;
            break;
        case GLUT_KEY_DOWN:
            cameraHeight -= 0.5f;
            if (cameraHeight < 3.0f) cameraHeight = 3.0f;
            break;
    }
    glutPostRedisplay();
}

void reshape(int width, int height)
{
    if (height == 0) height = 1;
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(55.0, static_cast<double>(width) / static_cast<double>(height), 0.1, 100.0);
    glMatrixMode(GL_MODELVIEW);
}

void initialize()
{
    glClearColor(0.12f, 0.15f, 0.20f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_NORMALIZE);
    glEnable(GL_SMOOTH);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    glutInitWindowPosition(40, 25);
    glutCreateWindow("Scene 5 - 3D DIU Computer Graphics Classroom");

    initialize();
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(specialKeys);
    glutTimerFunc(16, timer, 0);

    glutMainLoop();
    return 0;
}
