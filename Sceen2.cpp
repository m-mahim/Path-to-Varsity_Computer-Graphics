#define _CRT_SECURE_NO_WARNINGS

#include <GL/glut.h>
#include <cmath>
#include <cstdlib>
#include <string>
#include <windows.h>
#include <mmsystem.h>

#pragma comment(lib, "winmm.lib")

using namespace std;

// =========================================================
// Scene 2: Student Goes to the Bus Stop
// Windows OpenGL GLUT + WinMM PlaySound
// =========================================================

const int WINDOW_WIDTH = 1100;
const int WINDOW_HEIGHT = 700;
const float PI = 3.14159265358979323846f;

// File path for background music
const string MUSIC_PATH = "D:\\8_th_Semister\\Computer_Graphics_Practice\\S2.test2_sound\\Traffic.wav";

// ----------------------------
// Animation State
// ----------------------------

bool animationStarted = true;
bool animationPaused = false;
bool isMusicPlaying = false;

int storyStage = 0;
int frameCounter = 0;

float studentX = 120.0f;
float studentY = 150.0f;
float studentLegPhase = 0.0f;

float busX = -420.0f;
float busWheelAngle = 0.0f;

float car1X = 1200.0f; // Right to left car
float car2X = -300.0f; // Left to right car
float rickshawX = 1500.0f;

float cloud1X = 80.0f;
float cloud2X = 640.0f;
float birdX = -100.0f;

float treeSway = 0.0f;
float studentBoardProgress = 0.0f;

int doorCloseWait = 0;

// ----------------------------
// Sound Control
// ----------------------------

void PlayMusic(bool play)
{
    if (play)
    {
        if (!isMusicPlaying)
        {
            PlaySoundA(MUSIC_PATH.c_str(), NULL, SND_FILENAME | SND_ASYNC | SND_LOOP);
            isMusicPlaying = true;
        }
    }
    else
    {
        PlaySoundA(NULL, NULL, 0); // Stop playing sound
        isMusicPlaying = false;
    }
}

// ----------------------------
// Utility Functions
// ----------------------------

void drawText(float x, float y, const string& text, void* font = GLUT_BITMAP_HELVETICA_18)
{
    glRasterPos2f(x, y);
    for (char character : text)
    {
        glutBitmapCharacter(font, character);
    }
}

void drawRectangle(float x1, float y1, float x2, float y2)
{
    glBegin(GL_QUADS);
    glVertex2f(x1, y1);
    glVertex2f(x2, y1);
    glVertex2f(x2, y2);
    glVertex2f(x1, y2);
    glEnd();
}

void drawLine(float x1, float y1, float x2, float y2, float width = 2.0f)
{
    glLineWidth(width);
    glBegin(GL_LINES);
    glVertex2f(x1, y1);
    glVertex2f(x2, y2);
    glEnd();
    glLineWidth(1.0f);
}

void drawFilledCircle(float centerX, float centerY, float radius, int segments = 80)
{
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(centerX, centerY);
    for (int i = 0; i <= segments; i++)
    {
        float angle = 2.0f * PI * i / segments;
        glVertex2f(centerX + cos(angle) * radius, centerY + sin(angle) * radius);
    }
    glEnd();
}

// ----------------------------
// Background Elements
// ----------------------------

void drawSky()
{
    glColor3f(0.45f, 0.75f, 0.98f);
    drawRectangle(0.0f, 260.0f, 1100.0f, 700.0f);

    glColor3f(1.0f, 0.80f, 0.15f);
    drawFilledCircle(930.0f, 580.0f, 45.0f);

    for (int i = 0; i < 12; i++)
    {
        float angle = i * 30.0f * PI / 180.0f;
        drawLine(
            930.0f + cos(angle) * 55.0f, 580.0f + sin(angle) * 55.0f,
            930.0f + cos(angle) * 74.0f, 580.0f + sin(angle) * 74.0f, 2.0f
        );
    }
}

void drawCloud(float x, float y, float scale)
{
    glColor3f(0.95f, 0.97f, 1.0f);
    drawFilledCircle(x, y, 21.0f * scale);
    drawFilledCircle(x + 28.0f * scale, y + 8.0f * scale, 27.0f * scale);
    drawFilledCircle(x + 58.0f * scale, y, 22.0f * scale);
    drawRectangle(x, y - 18.0f * scale, x + 58.0f * scale, y + 6.0f * scale);
}

void drawClouds()
{
    drawCloud(cloud1X, 555.0f, 0.85f);
    drawCloud(cloud2X, 505.0f, 0.65f);
}

void drawBird(float x, float y, float scale, float phase)
{
    glPushMatrix();
    glTranslatef(x, y, 0.0f);
    glScalef(scale, scale, 1.0f);

    float wingLift = sin(frameCounter * 0.16f + phase) * 10.0f;

    glColor3f(0.20f, 0.38f, 0.72f);
    glBegin(GL_POLYGON);
    glVertex2f(-18.0f, 0.0f); glVertex2f(-8.0f, 8.0f);
    glVertex2f(15.0f, 7.0f); glVertex2f(28.0f, 0.0f);
    glVertex2f(14.0f, -7.0f); glVertex2f(-8.0f, -6.0f);
    glEnd();

    drawFilledCircle(24.0f, 5.0f, 7.0f, 30);

    glColor3f(0.95f, 0.62f, 0.10f);
    glBegin(GL_TRIANGLES);
    glVertex2f(30.0f, 6.0f); glVertex2f(41.0f, 3.0f); glVertex2f(30.0f, 0.0f);
    glEnd();

    glColor3f(1.0f, 1.0f, 1.0f);
    drawFilledCircle(26.0f, 7.0f, 2.1f, 20);
    glColor3f(0.02f, 0.02f, 0.02f);
    drawFilledCircle(26.5f, 7.0f, 1.0f, 15);

    glColor3f(0.30f, 0.55f, 0.90f);
    glBegin(GL_TRIANGLES);
    glVertex2f(4.0f, 3.0f); glVertex2f(-24.0f, 18.0f + wingLift); glVertex2f(12.0f, 8.0f);
    glEnd();

    glColor3f(0.15f, 0.35f, 0.68f);
    glBegin(GL_TRIANGLES);
    glVertex2f(5.0f, -1.0f); glVertex2f(-18.0f, -16.0f - wingLift * 0.55f); glVertex2f(15.0f, -5.0f);
    glEnd();

    glColor3f(0.12f, 0.28f, 0.58f);
    glBegin(GL_TRIANGLES);
    glVertex2f(-15.0f, 3.0f); glVertex2f(-34.0f, 12.0f); glVertex2f(-25.0f, 0.0f);
    glEnd();
    glBegin(GL_TRIANGLES);
    glVertex2f(-15.0f, -2.0f); glVertex2f(-34.0f, -10.0f); glVertex2f(-25.0f, 1.0f);
    glEnd();

    glPopMatrix();
}

void drawBirds()
{
    drawBird(birdX, 520.0f, 0.60f, 0.0f);
    drawBird(birdX + 90.0f, 565.0f, 0.47f, 1.8f);
    drawBird(birdX + 160.0f, 535.0f, 0.38f, 3.4f);
}

void drawGroundAndRoad()
{
    // Grass
    glColor3f(0.25f, 0.65f, 0.28f);
    drawRectangle(0.0f, 210.0f, 1100.0f, 320.0f);

    // Footpath
    glColor3f(0.65f, 0.65f, 0.65f);
    drawRectangle(0.0f, 145.0f, 1100.0f, 220.0f);

    glColor3f(0.52f, 0.52f, 0.52f);
    for (int x = 0; x < 1100; x += 90)
    {
        drawLine(static_cast<float>(x), 145.0f, static_cast<float>(x), 220.0f, 1.0f);
    }

    // Main Road
    glColor3f(0.18f, 0.20f, 0.23f);
    drawRectangle(0.0f, 0.0f, 1100.0f, 145.0f);

    // Lane Divider
    glColor3f(1.0f, 0.90f, 0.20f);
    for (int x = 0; x < 1100; x += 100)
    {
        drawRectangle(static_cast<float>(x), 68.0f, static_cast<float>(x + 55), 74.0f);
    }
}

// ----------------------------
// Buildings, House & Trees
// ----------------------------

void drawBuilding(float x, float width, float height, float r, float g, float b)
{
    glColor3f(r, g, b);
    drawRectangle(x, 220.0f, x + width, 220.0f + height);

    glColor3f(0.18f, 0.22f, 0.28f);
    drawRectangle(x + width * 0.40f, 220.0f, x + width * 0.62f, 280.0f);

    for (float wx = x + 18.0f; wx < x + width - 20.0f; wx += 42.0f)
    {
        for (float wy = 300.0f; wy < 220.0f + height - 25.0f; wy += 48.0f)
        {
            glColor3f(0.82f, 0.92f, 1.0f);
            drawRectangle(wx, wy, wx + 20.0f, wy + 25.0f);

            glColor3f(0.25f, 0.32f, 0.40f);
            drawLine(wx + 10.0f, wy, wx + 10.0f, wy + 25.0f, 1.0f);
        }
    }
}

void drawBuildings()
{
    drawBuilding(0.0f, 150.0f, 220.0f, 0.82f, 0.45f, 0.30f);
    drawBuilding(170.0f, 125.0f, 175.0f, 0.70f, 0.72f, 0.80f);
    drawBuilding(320.0f, 180.0f, 245.0f, 0.88f, 0.75f, 0.36f);
    drawBuilding(530.0f, 130.0f, 195.0f, 0.55f, 0.70f, 0.62f);
    drawBuilding(690.0f, 175.0f, 230.0f, 0.76f, 0.48f, 0.65f);
    drawBuilding(900.0f, 200.0f, 190.0f, 0.55f, 0.62f, 0.78f);
}

void drawHouse()
{
    glColor3f(0.95f, 0.72f, 0.42f);
    drawRectangle(30.0f, 220.0f, 220.0f, 400.0f);

    glColor3f(0.62f, 0.18f, 0.12f);
    glBegin(GL_TRIANGLES);
    glVertex2f(15.0f, 400.0f);
    glVertex2f(125.0f, 495.0f);
    glVertex2f(235.0f, 400.0f);
    glEnd();

    glColor3f(0.30f, 0.17f, 0.08f);
    drawRectangle(85.0f, 220.0f, 145.0f, 315.0f);

    glColor3f(0.95f, 0.80f, 0.20f);
    drawFilledCircle(132.0f, 267.0f, 4.0f);

    glColor3f(0.65f, 0.88f, 1.0f);
    drawRectangle(160.0f, 305.0f, 205.0f, 360.0f);

    glColor3f(0.20f, 0.25f, 0.30f);
    drawLine(182.5f, 305.0f, 182.5f, 360.0f, 2.0f);
    drawLine(160.0f, 332.0f, 205.0f, 332.0f, 2.0f);

    glColor3f(0.18f, 0.20f, 0.22f);
    for (float x = 245.0f; x <= 325.0f; x += 16.0f)
    {
        drawLine(x, 220.0f, x, 300.0f, 4.0f);
    }
    drawLine(240.0f, 220.0f, 330.0f, 220.0f, 5.0f);
    drawLine(240.0f, 300.0f, 330.0f, 300.0f, 5.0f);
}

void drawTree(float x, float y, float scale)
{
    glPushMatrix();
    glTranslatef(x, y, 0.0f);
    glRotatef(treeSway, 0.0f, 0.0f, 1.0f);
    glScalef(scale, scale, 1.0f);

    glColor3f(0.35f, 0.18f, 0.06f);
    drawRectangle(-10.0f, 0.0f, 10.0f, 75.0f);

    glColor3f(0.12f, 0.55f, 0.22f);
    drawFilledCircle(-25.0f, 85.0f, 32.0f);
    drawFilledCircle(15.0f, 95.0f, 38.0f);
    drawFilledCircle(42.0f, 78.0f, 30.0f);
    drawFilledCircle(0.0f, 125.0f, 34.0f);

    glPopMatrix();
}

void drawTrees()
{
    drawTree(380.0f, 220.0f, 0.70f);
    drawTree(575.0f, 220.0f, 0.82f);
    drawTree(980.0f, 220.0f, 0.75f);
}

void drawStreetLight(float x)
{
    glColor3f(0.20f, 0.22f, 0.25f);
    drawRectangle(x, 210.0f, x + 8.0f, 350.0f);
    drawLine(x + 4.0f, 350.0f, x + 35.0f, 365.0f, 5.0f);

    glColor3f(1.0f, 0.90f, 0.40f);
    drawFilledCircle(x + 38.0f, 363.0f, 12.0f);
}

void drawBusStop()
{
    float x = 780.0f;

    // Bus Stop Roof
    glColor3f(0.12f, 0.30f, 0.55f);
    drawRectangle(x, 315.0f, x + 220.0f, 335.0f);

    // Pillars
    glColor3f(0.18f, 0.20f, 0.23f);
    drawRectangle(x + 15.0f, 220.0f, x + 25.0f, 315.0f);
    drawRectangle(x + 195.0f, 220.0f, x + 205.0f, 315.0f);

    // Glass Backdrop
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.55f, 0.82f, 0.95f, 0.45f);
    drawRectangle(x + 25.0f, 225.0f, x + 195.0f, 315.0f);
    glDisable(GL_BLEND);

    // Bench
    glColor3f(0.48f, 0.25f, 0.10f);
    drawRectangle(x + 45.0f, 245.0f, x + 175.0f, 260.0f);
    drawRectangle(x + 55.0f, 220.0f, x + 65.0f, 245.0f);
    drawRectangle(x + 155.0f, 220.0f, x + 165.0f, 245.0f);

    // Sign Post
    glColor3f(0.12f, 0.18f, 0.25f);
    drawRectangle(731.0f, 220.0f, 739.0f, 345.0f);

    // Sign Board Box
    glColor3f(0.05f, 0.28f, 0.62f);
    drawRectangle(645.0f, 345.0f, 825.0f, 385.0f);

    // MIRPUR BUS STOP Text
    glColor3f(1.0f, 1.0f, 1.0f);
    drawText(662.0f, 359.0f, "MIRPUR BUS STOP", GLUT_BITMAP_HELVETICA_18);
}

void drawTrafficSignal()
{
    glColor3f(0.12f, 0.12f, 0.14f);
    drawRectangle(1015.0f, 180.0f, 1025.0f, 360.0f);
    drawRectangle(985.0f, 340.0f, 1055.0f, 455.0f);

    glColor3f(0.25f, 0.10f, 0.10f);
    drawFilledCircle(1020.0f, 425.0f, 14.0f);

    glColor3f(0.25f, 0.22f, 0.08f);
    drawFilledCircle(1020.0f, 397.0f, 14.0f);

    glColor3f(0.10f, 1.0f, 0.12f);
    drawFilledCircle(1020.0f, 369.0f, 14.0f);
}

// ----------------------------
// Vehicles
// ----------------------------

void drawCar(float x, float y, float scale, float r, float g, float b)
{
    glPushMatrix();
    glTranslatef(x, y, 0.0f);
    glScalef(scale, scale, 1.0f);

    glColor3f(r, g, b);
    drawRectangle(0.0f, 25.0f, 150.0f, 65.0f);

    glBegin(GL_POLYGON);
    glVertex2f(30.0f, 65.0f);
    glVertex2f(58.0f, 100.0f);
    glVertex2f(112.0f, 100.0f);
    glVertex2f(138.0f, 65.0f);
    glEnd();

    glColor3f(0.60f, 0.85f, 1.0f);
    drawRectangle(60.0f, 70.0f, 85.0f, 94.0f);
    drawRectangle(90.0f, 70.0f, 112.0f, 94.0f);

    glColor3f(0.08f, 0.08f, 0.09f);
    drawFilledCircle(35.0f, 25.0f, 18.0f);
    drawFilledCircle(118.0f, 25.0f, 18.0f);

    glColor3f(0.72f, 0.72f, 0.72f);
    drawFilledCircle(35.0f, 25.0f, 8.0f);
    drawFilledCircle(118.0f, 25.0f, 8.0f);

    glPopMatrix();
}

void drawRickshaw(float x, float y, float scale)
{
    glPushMatrix();
    glTranslatef(x, y, 0.0f);
    glScalef(scale, scale, 1.0f);

    glColor3f(0.15f, 0.55f, 0.30f);
    glBegin(GL_POLYGON);
    glVertex2f(25.0f, 35.0f);
    glVertex2f(90.0f, 35.0f);
    glVertex2f(78.0f, 100.0f);
    glVertex2f(42.0f, 100.0f);
    glEnd();

    glColor3f(0.80f, 0.20f, 0.18f);
    glBegin(GL_TRIANGLES);
    glVertex2f(37.0f, 100.0f);
    glVertex2f(83.0f, 100.0f);
    glVertex2f(60.0f, 130.0f);
    glEnd();

    glColor3f(0.08f, 0.08f, 0.09f);
    drawFilledCircle(25.0f, 30.0f, 19.0f);
    drawFilledCircle(88.0f, 30.0f, 19.0f);

    drawLine(90.0f, 40.0f, 130.0f, 65.0f, 4.0f);
    drawFilledCircle(135.0f, 68.0f, 8.0f);

    glPopMatrix();
}

void drawBus(float x, float y)
{
    glPushMatrix();
    glTranslatef(x, y, 0.0f);

    // 1. Main Bus Body
    glColor3f(0.02f, 0.45f, 0.82f);
    drawRectangle(0.0f, 25.0f, 390.0f, 150.0f);

    // 2. Front Windshield Slant Area
    glBegin(GL_POLYGON);
    glVertex2f(390.0f, 25.0f);
    glVertex2f(400.0f, 45.0f);
    glVertex2f(395.0f, 150.0f);
    glVertex2f(390.0f, 150.0f);
    glEnd();

    // 3. Top Route Signboard Box
    glColor3f(0.02f, 0.12f, 0.28f);
    drawRectangle(10.0f, 150.0f, 390.0f, 162.0f);

    // Top Route Text
    glColor3f(0.95f, 0.75f, 0.20f);
    drawText(130.0f, 152.0f, "MIRPUR - DIU CAMPUS", GLUT_BITMAP_HELVETICA_10);

    // 4. Yellow Stripe
    glColor3f(0.95f, 0.75f, 0.08f);
    drawRectangle(0.0f, 38.0f, 390.0f, 46.0f);

    // 5. Windows
    for (int i = 0; i < 6; i++)
    {
        float wx = 12.0f + i * 50.0f;
        glColor3f(0.65f, 0.88f, 0.98f);
        drawRectangle(wx, 82.0f, wx + 42.0f, 138.0f);

        // Center Vertical Line
        glColor3f(0.02f, 0.35f, 0.65f);
        drawLine(wx + 21.0f, 82.0f, wx + 21.0f, 138.0f, 2.0f);
    }

    // Driver Window
    glColor3f(0.65f, 0.88f, 0.98f);
    drawRectangle(318.0f, 82.0f, 375.0f, 138.0f);
    glColor3f(0.02f, 0.35f, 0.65f);
    drawLine(350.0f, 82.0f, 350.0f, 138.0f, 2.0f);

    // Windshield Front Glass
    glColor3f(0.65f, 0.88f, 0.98f);
    glBegin(GL_POLYGON);
    glVertex2f(382.0f, 82.0f);
    glVertex2f(392.0f, 82.0f);
    glVertex2f(391.0f, 138.0f);
    glVertex2f(382.0f, 138.0f);
    glEnd();

    // 6. Bus Text
    glColor3f(1.0f, 1.0f, 1.0f);
    drawText(125.0f, 56.0f, "DIU STUDENT BUS", GLUT_BITMAP_HELVETICA_18);

    // 7. Taillight
    glColor3f(0.85f, 0.10f, 0.15f);
    drawFilledCircle(5.0f, 62.0f, 5.0f);

    // 8. Headlight
    glColor3f(0.98f, 0.88f, 0.20f);
    drawFilledCircle(393.0f, 58.0f, 7.0f);

    // 9. Wheels
    // Rear Wheel
    glPushMatrix();
    glTranslatef(68.0f, 25.0f, 0.0f);
    glRotatef(busWheelAngle, 0.0f, 0.0f, 1.0f);
    glColor3f(0.06f, 0.06f, 0.08f);
    drawFilledCircle(0.0f, 0.0f, 26.0f);
    glColor3f(0.80f, 0.80f, 0.82f);
    drawFilledCircle(0.0f, 0.0f, 10.0f);

    glColor3f(0.06f, 0.06f, 0.08f);
    for (int i = 0; i < 8; i++) {
        float angle = i * (PI / 4.0f);
        drawLine(0.0f, 0.0f, cos(angle) * 10.0f, sin(angle) * 10.0f, 1.5f);
    }
    glPopMatrix();

    // Front Wheel
    glPushMatrix();
    glTranslatef(320.0f, 25.0f, 0.0f);
    glRotatef(busWheelAngle, 0.0f, 0.0f, 1.0f);
    glColor3f(0.06f, 0.06f, 0.08f);
    drawFilledCircle(0.0f, 0.0f, 26.0f);
    glColor3f(0.80f, 0.80f, 0.82f);
    drawFilledCircle(0.0f, 0.0f, 10.0f);

    glColor3f(0.06f, 0.06f, 0.08f);
    for (int i = 0; i < 8; i++) {
        float angle = i * (PI / 4.0f);
        drawLine(0.0f, 0.0f, cos(angle) * 10.0f, sin(angle) * 10.0f, 1.5f);
    }
    glPopMatrix();

    glPopMatrix();
}

// ----------------------------
// Student Character
// ----------------------------

void drawStudent(float x, float y, float scale, bool walking, bool boarding)
{
    glPushMatrix();
    glTranslatef(x, y, 0.0f);
    glScalef(scale, scale, 1.0f);

    float legSwing = walking ? sin(studentLegPhase) * 14.0f : 0.0f;
    float armSwing = walking ? -legSwing : 0.0f;

    if (boarding)
    {
        glRotatef(-8.0f, 0.0f, 0.0f, 1.0f);
    }

    // Head
    glColor3f(0.82f, 0.59f, 0.40f);
    drawFilledCircle(0.0f, 160.0f, 27.0f);

    // Hair
    glColor3f(0.10f, 0.06f, 0.03f);
    glBegin(GL_POLYGON);
    glVertex2f(-26.0f, 168.0f);
    glVertex2f(-18.0f, 188.0f);
    glVertex2f(0.0f, 195.0f);
    glVertex2f(22.0f, 187.0f);
    glVertex2f(28.0f, 170.0f);
    glVertex2f(10.0f, 180.0f);
    glVertex2f(-12.0f, 180.0f);
    glEnd();

    // Eye
    glColor3f(0.06f, 0.04f, 0.02f);
    drawFilledCircle(9.0f, 160.0f, 2.5f);

    // Body
    glColor3f(0.12f, 0.42f, 0.78f);
    glBegin(GL_POLYGON);
    glVertex2f(-25.0f, 133.0f);
    glVertex2f(25.0f, 133.0f);
    glVertex2f(20.0f, 72.0f);
    glVertex2f(-20.0f, 72.0f);
    glEnd();

    // Backpack
    glColor3f(0.20f, 0.22f, 0.27f);
    drawRectangle(-34.0f, 83.0f, -20.0f, 125.0f);

    // Arms
    glColor3f(0.82f, 0.59f, 0.40f);
    drawLine(-22.0f, 122.0f, -32.0f + armSwing, 80.0f, 9.0f);
    drawLine(22.0f, 122.0f, 32.0f - armSwing, 80.0f, 9.0f);

    // Trousers
    glColor3f(0.09f, 0.14f, 0.25f);
    drawRectangle(-20.0f, 55.0f, 20.0f, 78.0f);

    // Legs
    drawLine(-10.0f, 58.0f, -14.0f + legSwing, 8.0f, 12.0f);
    drawLine(10.0f, 58.0f, 14.0f - legSwing, 8.0f, 12.0f);

    // Shoes
    glColor3f(0.04f, 0.04f, 0.05f);
    drawRectangle(-27.0f + legSwing, 2.0f, -6.0f + legSwing, 10.0f);
    drawRectangle(6.0f - legSwing, 2.0f, 27.0f - legSwing, 10.0f);

    glPopMatrix();
}

void drawSceneStudent()
{
    if (storyStage == 4)
    {
        float doorWorldX = busX + 132.0f;
        float boardX = studentX + (doorWorldX - studentX) * studentBoardProgress;
        float boardY = studentY + studentBoardProgress * 20.0f;
        float scale = 0.52f - studentBoardProgress * 0.15f;

        drawStudent(boardX, boardY, scale, true, true);
    }
    else if (storyStage >= 5)
    {
        // Boarded and Vanished
    }
    else
    {
        bool walking = (storyStage == 1);
        drawStudent(studentX, studentY, 0.52f, walking, false);
    }
}

// ----------------------------
// User Interface
// ----------------------------

string getStoryText()
{
    switch (storyStage)
    {
        case 0: return "Press S to start Scene 2.";
        case 1: return "The student is walking from home to the bus stop.";
        case 2: return "The student is waiting at the bus stop.";
        case 3: return "The university bus is arriving.";
        case 4: return "The student is boarding the bus.";
        case 5: return "The student has boarded the bus.";
        case 6: return "The bus leaves for the university.";
        case 7: return "Scene 2 completed. Press R to replay.";
        default: return "";
    }
}

void drawInterface()
{
    glColor3f(0.04f, 0.07f, 0.12f);
    drawRectangle(0.0f, 650.0f, 1100.0f, 700.0f);

    glColor3f(1.0f, 1.0f, 1.0f);
    drawText(25.0f, 670.0f, "Scene 2: Journey to the Bus Stop", GLUT_BITMAP_HELVETICA_18);
    drawText(400.0f, 670.0f, getStoryText(), GLUT_BITMAP_HELVETICA_18);

    if (animationPaused)
    {
        glColor3f(1.0f, 0.85f, 0.10f);
        drawText(505.0f, 625.0f, "PAUSED", GLUT_BITMAP_HELVETICA_18);
    }
}

// ----------------------------
// Display Logic (Layering)
// ----------------------------

void display()
{
    glClear(GL_COLOR_BUFFER_BIT);

    // Background Layer
    drawSky();
    drawClouds();
    drawBirds();
    drawBuildings();
    drawHouse();
    drawTrees();
    drawStreetLight(450.0f);
    drawStreetLight(665.0f);

    // Midground Layer
    drawGroundAndRoad();
    drawBusStop();
    drawTrafficSignal();

    // 1. Student
    drawSceneStudent();

    // 2. Bus
    drawBus(busX, 70.0f);

    // 3. Foreground Traffic
    drawCar(car1X, 10.0f, 0.80f, 0.82f, 0.18f, 0.15f);
    drawCar(car2X, 85.0f, 0.75f, 0.18f, 0.40f, 0.82f);
    drawRickshaw(rickshawX, 5.0f, 0.75f);

    // UI
    //drawInterface();

    glutSwapBuffers();
}

// ----------------------------
// Reset Scene
// ----------------------------

void resetScene()
{
    PlayMusic(false);

    animationStarted = true;
    animationPaused = false;

    storyStage = 0;
    frameCounter = 0;

    studentX = 120.0f;
    studentY = 150.0f;
    studentLegPhase = 0.0f;

    busX = -420.0f;
    busWheelAngle = 0.0f;

    car1X = 1200.0f;
    car2X = -300.0f;
    rickshawX = 1500.0f;

    cloud1X = 80.0f;
    cloud2X = 640.0f;
    birdX = -100.0f;

    treeSway = 0.0f;
    studentBoardProgress = 0.0f;
    doorCloseWait = 0;

    glutPostRedisplay();
}

// ----------------------------
// Story Sequence Update
// ----------------------------

void updateStory()
{
    if (!animationStarted || animationPaused)
        return;

    frameCounter++;

    switch (storyStage)
    {
        case 0:
            storyStage = 1;
            frameCounter = 0;
            PlayMusic(true); // Start music at scene start
            break;

        case 1:
            studentX += 1.10f;
            studentLegPhase += 0.20f;

            if (studentX >= 760.0f)
            {
                studentX = 760.0f;
                storyStage = 2;
                frameCounter = 0;
            }
            break;

        case 2:
            if (frameCounter > 120)
            {
                storyStage = 3;
                frameCounter = 0;
            }
            break;

        case 3:
            busX += 2.40f;
            busWheelAngle -= 6.0f;

            if (busX >= 500.0f)
            {
                busX = 500.0f;
                storyStage = 4;
                frameCounter = 0;
            }
            break;

        case 4:
            studentBoardProgress += 0.015f;
            studentLegPhase += 0.22f;

            if (studentBoardProgress >= 1.0f)
            {
                studentBoardProgress = 1.0f;
                storyStage = 5;
                frameCounter = 0;
                doorCloseWait = 0;
            }
            break;

        case 5:
            doorCloseWait++;
            if (doorCloseWait > 60)
            {
                storyStage = 6;
                frameCounter = 0;
            }
            break;

        case 6:
            busX += 2.70f;
            busWheelAngle -= 7.0f;

            if (busX > 1120.0f)
            {
                storyStage = 7;
                frameCounter = 0;
                PlayMusic(false); // Stop music when bus leaves scene
            }
            break;

        case 7:
            break;
    }
}

// ----------------------------
// Timer & Controls
// ----------------------------

void timer(int value)
{
    if (!animationPaused)
    {
        cloud1X += 0.18f;
        cloud2X += 0.12f;

        if (cloud1X > 1120.0f) cloud1X = -120.0f;
        if (cloud2X > 1120.0f) cloud2X = -120.0f;

        birdX += 0.85f;
        if (birdX > 1120.0f) birdX = -180.0f;

        treeSway = sin(frameCounter * 0.025f) * 2.4f;

        // Vehicle Movement and Spacing
        car1X -= 2.20f;
        if (car1X < -250.0f) car1X = 1400.0f;

        car2X += 1.70f;
        if (car2X > 1200.0f) car2X = -400.0f;

        rickshawX -= 1.15f;
        if (rickshawX < -200.0f) rickshawX = 1600.0f;

        updateStory();
    }

    glutPostRedisplay();
    glutTimerFunc(16, timer, 0);
}

void keyboard(unsigned char key, int x, int y)
{
    switch (key)
    {
        case 's':
        case 'S':
            animationStarted = true;
            animationPaused = false;
            if (storyStage == 0) frameCounter = 0;
            PlayMusic(true);
            break;

        case 'p':
        case 'P':
            animationPaused = !animationPaused;
            if (animationPaused)
            {
                PlayMusic(false);
            }
            else if (animationStarted && storyStage < 7)
            {
                PlayMusic(true);
            }
            break;

        case 'r':
        case 'R':
            resetScene();
            break;

        case 27: // ESC Key
            PlayMusic(false);
            exit(0);
    }
    glutPostRedisplay();
}

void initialize()
{
    glClearColor(0.45f, 0.75f, 0.98f, 1.0f);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0.0, WINDOW_WIDTH, 0.0, WINDOW_HEIGHT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glEnable(GL_POINT_SMOOTH);
    glEnable(GL_LINE_SMOOTH);
}

int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    glutInitWindowPosition(80, 40);
    glutCreateWindow("Scene 2 - Journey to the Bus Stop");

    initialize();

    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutTimerFunc(16, timer, 0);

    glutMainLoop();
    return 0;
}
