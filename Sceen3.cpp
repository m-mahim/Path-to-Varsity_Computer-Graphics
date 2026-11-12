#include <windows.h>
#include <mmsystem.h>
#include <GL/glut.h>
#include <cmath>
#include <cstdlib>
#include <string>

#pragma comment(lib, "winmm.lib") // Auto-link Windows Multimedia API for MSVC

using namespace std;

// ============================================================
// SCENE 3: DIU Student Bus Journey
// Windows OpenGL GLUT + Win32 PlaySound
// ============================================================

const int WINDOW_WIDTH = 1200;
const int WINDOW_HEIGHT = 720;
const float PI = 3.14159265358979323846f;

// File path for background music
const string MUSIC_PATH = "D:\\8_th_Semister\\Computer_Graphics_Practice\\S3.test1\\bus.wav";


// ----------------------------
// Global animation variables
// ----------------------------

bool animationStarted = false;
bool animationPaused = false;
bool isMusicPlaying = false;

int storyStage = 0;
int frameCounter = 0;

float roadOffset = 0.0f;
float treeOffset = 0.0f;
float buildingOffset = 0.0f;
float cloudOffset = 0.0f;
float lampOffset = 0.0f;

float busBounce = 0.0f;
float wheelAngle = 0.0f;
float birdWing = 0.0f;

float sunX = 1020.0f;
float campusX = 1500.0f;
float gateOpenAmount = 0.0f;

float studentHeadMove = 0.0f;
float passengerMove = 0.0f;
float progressBar = 0.0f;

// ----------------------------
// Sound Control (Win32)
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
        PlaySoundA(NULL, NULL, 0); // Stops sound
        isMusicPlaying = false;
    }
}

// ----------------------------
// Drawing utilities
// ----------------------------

void drawText(float x, float y, const string& text, void* font = GLUT_BITMAP_HELVETICA_18)
{
    glRasterPos2f(x, y);
    for (char c : text)
        glutBitmapCharacter(font, c);
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

void drawFilledCircle(float cx, float cy, float radius, int segments = 80)
{
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx, cy);

    for (int i = 0; i <= segments; i++)
    {
        float angle = 2.0f * PI * i / segments;
        glVertex2f(cx + cos(angle) * radius, cy + sin(angle) * radius);
    }
    glEnd();
}

// ----------------------------
// Sky and environment
// ----------------------------

void drawSky()
{
    glBegin(GL_QUADS);
    glColor3f(0.30f, 0.68f, 0.95f);
    glVertex2f(0.0f, 720.0f);
    glVertex2f(1200.0f, 720.0f);

    glColor3f(0.72f, 0.90f, 1.0f);
    glVertex2f(1200.0f, 250.0f);
    glVertex2f(0.0f, 250.0f);
    glEnd();

    glColor3f(1.0f, 0.82f, 0.15f);
    drawFilledCircle(sunX, 585.0f, 48.0f);

    glColor3f(1.0f, 0.88f, 0.25f);
    for (int i = 0; i < 12; i++)
    {
        float a = i * 30.0f * PI / 180.0f;
        drawLine(
            sunX + cos(a) * 58.0f, 585.0f + sin(a) * 58.0f,
            sunX + cos(a) * 78.0f, 585.0f + sin(a) * 78.0f, 2.0f
        );
    }
}

void drawCloud(float x, float y, float scale)
{
    glColor3f(0.96f, 0.98f, 1.0f);

    drawFilledCircle(x, y, 23.0f * scale);
    drawFilledCircle(x + 28.0f * scale, y + 9.0f * scale, 30.0f * scale);
    drawFilledCircle(x + 62.0f * scale, y, 24.0f * scale);

    drawRectangle(x, y - 18.0f * scale, x + 62.0f * scale, y + 8.0f * scale);
}

void drawClouds()
{
    drawCloud(100.0f + cloudOffset, 570.0f, 0.85f);
    drawCloud(520.0f + cloudOffset * 0.70f, 525.0f, 0.68f);
    drawCloud(900.0f + cloudOffset * 0.45f, 610.0f, 0.60f);
}

// ----------------------------
// Moving city
// ----------------------------

void drawBuilding(float x, float w, float h, float r, float g, float b)
{
    glColor3f(r, g, b);
    drawRectangle(x, 180.0f, x + w, 180.0f + h);

    glColor3f(0.68f, 0.88f, 1.0f);
    for (float wx = x + 16.0f; wx < x + w - 16.0f; wx += 42.0f)
    {
        for (float wy = 215.0f; wy < 180.0f + h - 22.0f; wy += 45.0f)
        {
            drawRectangle(wx, wy, wx + 22.0f, wy + 26.0f);
        }
    }

    glColor3f(0.20f, 0.18f, 0.18f);
    drawRectangle(x + w * 0.42f, 180.0f, x + w * 0.62f, 240.0f);
}

void drawCityBackground()
{
    float base = fmod(buildingOffset, 1400.0f);

    drawBuilding(-base + 0.0f, 150.0f, 230.0f, 0.80f, 0.48f, 0.34f);
    drawBuilding(-base + 180.0f, 130.0f, 175.0f, 0.64f, 0.70f, 0.78f);
    drawBuilding(-base + 345.0f, 175.0f, 260.0f, 0.82f, 0.68f, 0.30f);
    drawBuilding(-base + 555.0f, 145.0f, 205.0f, 0.56f, 0.74f, 0.62f);
    drawBuilding(-base + 735.0f, 190.0f, 235.0f, 0.72f, 0.50f, 0.68f);
    drawBuilding(-base + 960.0f, 170.0f, 190.0f, 0.52f, 0.64f, 0.80f);
    drawBuilding(-base + 1170.0f, 180.0f, 245.0f, 0.83f, 0.58f, 0.42f);
    drawBuilding(-base + 1400.0f, 150.0f, 230.0f, 0.80f, 0.48f, 0.34f);
    drawBuilding(-base + 1580.0f, 130.0f, 175.0f, 0.64f, 0.70f, 0.78f);
}

// ----------------------------
// Trees
// ----------------------------

void drawTree(float x, float y, float scale)
{
    glPushMatrix();
    glTranslatef(x, y, 0.0f);
    glScalef(scale, scale, 1.0f);

    glColor3f(0.34f, 0.18f, 0.07f);
    drawRectangle(-9.0f, 0.0f, 9.0f, 75.0f);

    glColor3f(0.10f, 0.52f, 0.18f);
    drawFilledCircle(-25.0f, 88.0f, 32.0f);
    drawFilledCircle(10.0f, 100.0f, 38.0f);
    drawFilledCircle(42.0f, 82.0f, 29.0f);
    drawFilledCircle(5.0f, 130.0f, 35.0f);

    glPopMatrix();
}

void drawMovingTrees()
{
    float base = fmod(treeOffset, 320.0f);

    for (int i = -1; i < 6; i++)
    {
        float x = i * 320.0f - base;
        drawTree(x, 180.0f, 0.62f);
    }
}

// ----------------------------
// Road, bridge and river
// ----------------------------

void drawNormalRoad()
{
    glColor3f(0.22f, 0.22f, 0.24f);
    drawRectangle(0.0f, 0.0f, 1200.0f, 180.0f);

    glColor3f(0.95f, 0.82f, 0.15f);
    float offset = fmod(roadOffset, 140.0f);

    for (int i = -1; i < 11; i++)
    {
        float x = i * 140.0f - offset;
        drawRectangle(x, 80.0f, x + 75.0f, 88.0f);
    }

    glColor3f(0.82f, 0.82f, 0.82f);
    drawRectangle(0.0f, 170.0f, 1200.0f, 180.0f);
}

void drawRiverAndBridge()
{
    glColor3f(0.20f, 0.63f, 0.88f);
    drawRectangle(0.0f, 0.0f, 1200.0f, 100.0f);

    glColor3f(0.60f, 0.88f, 1.0f);
    float waveOffset = fmod(roadOffset, 80.0f);

    for (int i = -1; i < 18; i++)
    {
        float x = i * 80.0f - waveOffset;
        drawLine(x, 40.0f, x + 35.0f, 45.0f, 2.0f);
        drawLine(x + 15.0f, 100.0f, x + 50.0f, 105.0f, 2.0f);
    }

    glColor3f(0.35f, 0.35f, 0.38f);
    drawRectangle(0.0f, 110.0f, 1200.0f, 205.0f);

    glColor3f(0.92f, 0.82f, 0.18f);
    for (int i = 0; i < 1200; i += 120)
        drawRectangle(i, 172.0f, i + 60.0f, 179.0f);

    glColor3f(0.75f, 0.75f, 0.77f);
    for (int i = 0; i <= 1200; i += 70)
        drawRectangle(i, 205.0f, i + 6.0f, 248.0f);

    drawRectangle(0.0f, 238.0f, 1200.0f, 246.0f);
}

// ----------------------------
// Street lamps
// ----------------------------

void drawLamp(float x)
{
    glColor3f(0.18f, 0.20f, 0.24f);
    drawRectangle(x, 178.0f, x + 7.0f, 335.0f);
    drawLine(x + 3.5f, 335.0f, x + 35.0f, 350.0f, 5.0f);

    glColor3f(1.0f, 0.90f, 0.45f);
    drawFilledCircle(x + 39.0f, 348.0f, 10.0f);
}

void drawMovingLamps()
{
    float base = fmod(lampOffset, 280.0f);
    for (int i = -1; i < 7; i++)
        drawLamp(i * 280.0f - base);
}

// ----------------------------
// DIU Campus
// ----------------------------

void drawDIUCampus(float x)
{
    glPushMatrix();
    glTranslatef(x, 0.0f, 0.0f);

    // Campus ground
    glColor3f(0.20f, 0.62f, 0.25f);
    drawRectangle(0.0f, 180.0f, 700.0f, 280.0f);

    // Main building
    glColor3f(0.94f, 0.94f, 0.96f);
    drawRectangle(110.0f, 220.0f, 610.0f, 540.0f);

    glColor3f(0.18f, 0.48f, 0.82f);
    drawRectangle(95.0f, 530.0f, 625.0f, 560.0f);

    // Central glass section
    glColor3f(0.35f, 0.72f, 0.92f);
    drawRectangle(300.0f, 240.0f, 420.0f, 520.0f);

    glColor3f(0.12f, 0.33f, 0.55f);
    for (int y = 265; y < 520; y += 45)
        drawLine(300.0f, y, 420.0f, y, 2.0f);

    for (int xx = 320; xx < 420; xx += 25)
        drawLine(xx, 240.0f, xx, 520.0f, 2.0f);

    // Side windows
    glColor3f(0.52f, 0.80f, 0.95f);
    for (int y = 270; y < 510; y += 55)
    {
        for (int xx = 135; xx < 285; xx += 50)
            drawRectangle(xx, y, xx + 30.0f, y + 30.0f);

        for (int xx = 445; xx < 595; xx += 50)
            drawRectangle(xx, y, xx + 30.0f, y + 30.0f);
    }

    // DIU Sign
    glColor3f(0.06f, 0.28f, 0.55f);
    drawRectangle(205.0f, 565.0f, 515.0f, 610.0f);

    glColor3f(1.0f, 1.0f, 1.0f);
    drawText(252.0f, 580.0f, "DAFFODIL INTERNATIONAL UNIVERSITY", GLUT_BITMAP_HELVETICA_12);

    // Entrance
    glColor3f(0.18f, 0.22f, 0.28f);
    drawRectangle(332.0f, 220.0f, 390.0f, 290.0f);

    // Gate pillars
    glColor3f(0.88f, 0.88f, 0.90f);
    drawRectangle(10.0f, 180.0f, 45.0f, 330.0f);
    drawRectangle(655.0f, 180.0f, 690.0f, 330.0f);

    // Gate boards
    glColor3f(0.10f, 0.38f, 0.72f);
    drawRectangle(0.0f, 320.0f, 700.0f, 355.0f);

    glColor3f(1.0f, 1.0f, 1.0f);
    drawText(224.0f, 332.0f, "WELCOME TO DIU CAMPUS", GLUT_BITMAP_HELVETICA_18);

    // Opening gate
    glColor3f(0.16f, 0.18f, 0.22f);
    float leftGateEnd = 350.0f - gateOpenAmount;
    float rightGateStart = 350.0f + gateOpenAmount;

    for (float gx = 45.0f; gx < leftGateEnd; gx += 20.0f)
        drawLine(gx, 180.0f, gx, 285.0f, 4.0f);

    for (float gx = rightGateStart; gx < 655.0f; gx += 20.0f)
        drawLine(gx, 180.0f, gx, 285.0f, 4.0f);

    drawLine(45.0f, 285.0f, leftGateEnd, 285.0f, 5.0f);
    drawLine(rightGateStart, 285.0f, 655.0f, 285.0f, 5.0f);

    // Decorative trees
    drawTree(80.0f, 180.0f, 0.45f);
    drawTree(630.0f, 180.0f, 0.45f);

    glPopMatrix();
}

// ----------------------------
// Bus exterior
// ----------------------------

void drawWheel(float x, float y, float radius)
{
    glPushMatrix();
    glTranslatef(x, y, 0.0f);
    glRotatef(wheelAngle, 0.0f, 0.0f, 1.0f);

    glColor3f(0.05f, 0.05f, 0.06f);
    drawFilledCircle(0.0f, 0.0f, radius);

    glColor3f(0.66f, 0.68f, 0.72f);
    drawFilledCircle(0.0f, 0.0f, radius * 0.42f);

    glColor3f(0.18f, 0.18f, 0.20f);
    for (int i = 0; i < 8; i++)
    {
        float a = i * 45.0f * PI / 180.0f;
        drawLine(0.0f, 0.0f, cos(a) * radius * 0.80f, sin(a) * radius * 0.80f, 2.0f);
    }

    glPopMatrix();
}

void drawBus()
{
    glPushMatrix();
    glTranslatef(220.0f, 115.0f + busBounce, 0.0f);

    // Shadow
    glColor4f(0.0f, 0.0f, 0.0f, 0.18f);
    drawFilledCircle(350.0f, 20.0f, 200.0f, 90);

    // Body
    glColor3f(0.08f, 0.43f, 0.78f);
    drawRectangle(0.0f, 40.0f, 700.0f, 250.0f);

    // Front
    glColor3f(0.06f, 0.33f, 0.64f);
    glBegin(GL_POLYGON);
    glVertex2f(700.0f, 40.0f);
    glVertex2f(760.0f, 65.0f);
    glVertex2f(745.0f, 250.0f);
    glVertex2f(700.0f, 250.0f);
    glEnd();

    // Roof strip
    glColor3f(0.04f, 0.25f, 0.52f);
    drawRectangle(15.0f, 250.0f, 735.0f, 270.0f);

    // Lower stripe
    glColor3f(0.94f, 0.76f, 0.12f);
    drawRectangle(0.0f, 75.0f, 735.0f, 92.0f);

    // Windows
    glColor3f(0.62f, 0.86f, 0.98f);
    for (float wx = 25.0f; wx < 590.0f; wx += 92.0f)
    {
        drawRectangle(wx, 155.0f, wx + 72.0f, 232.0f);

        glColor3f(0.10f, 0.30f, 0.50f);
        drawLine(wx + 36.0f, 155.0f, wx + 36.0f, 232.0f, 2.0f);

        glColor3f(0.62f, 0.86f, 0.98f);
    }

    // Driver window
    drawRectangle(610.0f, 150.0f, 690.0f, 232.0f);
    drawRectangle(705.0f, 145.0f, 735.0f, 230.0f);

    // Bus label
    glColor3f(1.0f, 1.0f, 1.0f);
    drawText(225.0f, 112.0f, "DIU STUDENT BUS", GLUT_BITMAP_HELVETICA_18);

    // Route sign
    glColor3f(0.03f, 0.08f, 0.12f);
    drawRectangle(230.0f, 250.0f, 495.0f, 268.0f);

    glColor3f(1.0f, 0.88f, 0.20f);
    drawText(274.0f, 254.0f, "MIRPUR  -  DIU CAMPUS", GLUT_BITMAP_HELVETICA_12);

    // Front lights
    glColor3f(1.0f, 0.94f, 0.45f);
    drawFilledCircle(747.0f, 115.0f, 11.0f);

    glColor3f(0.85f, 0.12f, 0.10f);
    drawFilledCircle(7.0f, 120.0f, 8.0f);

    // Wheels
    drawWheel(125.0f, 42.0f, 42.0f);
    drawWheel(600.0f, 42.0f, 42.0f);

    glPopMatrix();
}

// ----------------------------
// Interface
// ----------------------------

string getStoryText()
{
    switch (storyStage)
    {
        case 0: return "Press S to begin the DIU bus journey.";
        case 1: return "The DIU student bus starts moving from Mirpur.";
        case 2: return "The bus is travelling through the city road.";
        case 3: return "The bus crosses a bridge over the river.";
        case 4: return "The bus continues through a green countryside road.";
        case 5: return "Daffodil International University is now visible.";
        case 6: return "The bus arrives at the DIU campus gate.";
        case 7: return "Scene 3 completed. Press R to replay.";
        default: return "";
    }
}

void drawInterface()
{
    glColor3f(0.03f, 0.06f, 0.10f);
    drawRectangle(0.0f, 665.0f, 1200.0f, 720.0f);

    glColor3f(1.0f, 1.0f, 1.0f);
    drawText(25.0f, 687.0f, "Scene 3: Journey from Mirpur to DIU Campus", GLUT_BITMAP_HELVETICA_18);

    glColor3f(0.06f, 0.08f, 0.12f);
    drawRectangle(0.0f, 0.0f, 1200.0f, 42.0f);

    glColor3f(1.0f, 0.92f, 0.65f);
    drawText(18.0f, 16.0f, getStoryText(), GLUT_BITMAP_HELVETICA_18);

    if (animationPaused)
    {
        glColor3f(1.0f, 0.85f, 0.12f);
        drawText(550.0f, 635.0f, "PAUSED", GLUT_BITMAP_HELVETICA_18);
    }
}

// ----------------------------
// Display
// ----------------------------

void display()
{
    glClear(GL_COLOR_BUFFER_BIT);

    drawSky();
    drawClouds();

    if (storyStage == 3)
    {
        drawRiverAndBridge();
    }
    else
    {
        if (storyStage <= 2)
            drawCityBackground();
        else if (storyStage == 4)
            drawMovingTrees();

        drawNormalRoad();
        drawMovingLamps();
    }

    if (storyStage >= 5)
        drawDIUCampus(campusX);

    drawBus();
    drawInterface();

    glutSwapBuffers();
}

// ----------------------------
// Reset
// ----------------------------

void resetScene()
{
    PlayMusic(false);

    animationStarted = false;
    animationPaused = false;

    storyStage = 0;
    frameCounter = 0;

    roadOffset = 0.0f;
    treeOffset = 0.0f;
    buildingOffset = 0.0f;
    cloudOffset = 0.0f;
    lampOffset = 0.0f;

    busBounce = 0.0f;
    wheelAngle = 0.0f;
    birdWing = 0.0f;

    campusX = 1500.0f;
    gateOpenAmount = 0.0f;

    studentHeadMove = 0.0f;
    passengerMove = 0.0f;
    progressBar = 0.0f;

    glutPostRedisplay();
}

// ----------------------------
// Story update
// ----------------------------

void updateStory()
{
    if (!animationStarted || animationPaused)
        return;

    frameCounter++;

    switch (storyStage)
    {
        case 0:
        {
            storyStage = 1;
            frameCounter = 0;
            PlayMusic(true); // Start background music on journey start
            break;
        }

        case 1:
        {
            progressBar += 0.08f;
            if (frameCounter > 120)
            {
                storyStage = 2;
                frameCounter = 0;
            }
            break;
        }

        case 2:
        {
            progressBar += 0.055f;
            if (frameCounter > 360)
            {
                storyStage = 3;
                frameCounter = 0;
            }
            break;
        }

        case 3:
        {
            progressBar += 0.075f;
            if (frameCounter > 250)
            {
                storyStage = 4;
                frameCounter = 0;
            }
            break;
        }

        case 4:
        {
            progressBar += 0.055f;
            if (frameCounter > 330)
            {
                storyStage = 5;
                frameCounter = 0;
                campusX = 1250.0f;
            }
            break;
        }

        case 5:
        {
            progressBar += 0.08f;
            campusX -= 1.70f;

            if (campusX <= 500.0f)
            {
                campusX = 500.0f;
                storyStage = 6;
                frameCounter = 0;
                PlayMusic(false); // Stop music upon arrival at campus
            }
            break;
        }

        case 6:
        {
            if (gateOpenAmount < 210.0f)
                gateOpenAmount += 1.5f;

            if (progressBar < 100.0f)
                progressBar += 0.10f;

            if (frameCounter > 260)
            {
                storyStage = 7;
                frameCounter = 0;
                progressBar = 100.0f;
                PlayMusic(false);
            }
            break;
        }

        case 7:
        {
            break;
        }
    }

    if (progressBar > 100.0f)
        progressBar = 100.0f;
}

// ----------------------------
// Timer
// ----------------------------

void timer(int value)
{
    if (!animationPaused)
    {
        if (animationStarted && storyStage < 6)
        {
            roadOffset += 5.5f;
            treeOffset += 4.2f;
            buildingOffset += 2.4f;
            lampOffset += 5.0f;
            wheelAngle -= 8.0f;
        }

        cloudOffset -= 0.18f;
        if (cloudOffset < -1300.0f)
            cloudOffset = 200.0f;

        birdWing += 0.18f;
        busBounce = sin(frameCounter * 0.12f) * 2.0f;

        updateStory();
    }

    glutPostRedisplay();
    glutTimerFunc(16, timer, 0);
}

// ----------------------------
// Keyboard
// ----------------------------

void keyboard(unsigned char key, int x, int y)
{
    switch (key)
    {
        case 's':
        case 'S':
        {
            animationStarted = true;
            animationPaused = false;
            PlayMusic(true);
            break;
        }

        case 'p':
        case 'P':
        {
            animationPaused = !animationPaused;

            if (animationPaused)
            {
                PlayMusic(false);
            }
            else if (animationStarted && storyStage < 6)
            {
                PlayMusic(true);
            }
            break;
        }

        case 'r':
        case 'R':
        {
            resetScene();
            break;
        }

        case 'n':
        case 'N':
        {
            storyStage = 7;
            progressBar = 100.0f;
            campusX = 500.0f;
            gateOpenAmount = 210.0f;
            PlayMusic(false);
            break;
        }

        case 27: // ESC key
        {
            PlayMusic(false);
            exit(0);
        }
    }

    glutPostRedisplay();
}

// ----------------------------
// OpenGL setup
// ----------------------------

void initialize()
{
    glClearColor(0.35f, 0.72f, 0.96f, 1.0f);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0.0, WINDOW_WIDTH, 0.0, WINDOW_HEIGHT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_POINT_SMOOTH);
    glEnable(GL_LINE_SMOOTH);
}

// ----------------------------
// Main
// ----------------------------

int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    glutInitWindowPosition(40, 25);
    glutCreateWindow("Scene 3 - DIU Student Bus Journey");

    initialize();

    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutTimerFunc(16, timer, 0);

    glutMainLoop();

    return 0;
}
