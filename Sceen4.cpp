#include <GL/glut.h>
#include <math.h>
#include <stdlib.h>
#include <windows.h>
#include <mmsystem.h>
#include <string>

#pragma comment(lib, "winmm.lib")

using namespace std;

// ============================================================
// SCENE 4: DIU Campus Arrival
// Traditional C++ OpenGL GLUT scene
// ============================================================

const int W = 1200;
const int H = 720;
const float PI = 3.14159265358979323846f;

// ------------------------------------------------------------
// Animation state
// ------------------------------------------------------------

bool paused = false;
int frameCounter = 0;
float cloudOffset = 0.0f;
float globalLegPhase = 0.0f;

// ------------------------------------------------------------
// Sound Implementation (Windows MMSystem Synchronized)
// ------------------------------------------------------------

bool isAudioPlaying = false;

void PlayMusic(const string& path) {
    PlaySoundA(path.c_str(), NULL, SND_FILENAME | SND_ASYNC | SND_LOOP);
}

void StopMusic() {
    PlaySoundA(NULL, NULL, 0);
}

// Controls music state directly aligned with simulation playback status
void updateAudioState() {
    if (paused) {
        if (isAudioPlaying) {
            StopMusic();
            isAudioPlaying = false;
        }
    } else {
        if (!isAudioPlaying) {
            // Re-trigger playback loop using your specific absolute semester paths
            PlayMusic("D:\\8_th_Semister\\Computer_Graphics_Practice\\S4.test5\\Playground.wav");
            // Note: Windows PlaySound API only allows one active wave file via waveOut device concurrently.
            isAudioPlaying = true;
        }
    }
}

// ------------------------------------------------------------
// Basic drawing helper functions
// ------------------------------------------------------------

void drawFilledCircle(float cx, float cy, float r, int n = 40) {
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx, cy);
    for (int i = 0; i <= n; i++) {
        float a = 2.0f * PI * i / n;
        glVertex2f(cx + cos(a) * r, cy + sin(a) * r);
    }
    glEnd();
}

void drawRectangle(float x1, float y1, float x2, float y2) {
    glBegin(GL_QUADS);
    glVertex2f(x1, y1); glVertex2f(x2, y1);
    glVertex2f(x2, y2); glVertex2f(x1, y2);
    glEnd();
}

void drawLine(float x1, float y1, float x2, float y2, float width = 2.0f) {
    glLineWidth(width);
    glBegin(GL_LINES);
    glVertex2f(x1, y1); glVertex2f(x2, y2);
    glEnd();
    glLineWidth(1.0f);
}

void text(float x, float y, const char* value, void* font = GLUT_BITMAP_HELVETICA_18) {
    glRasterPos2f(x, y);
    for (int i = 0; value[i] != '\0'; i++) {
        glutBitmapCharacter(font, value[i]);
    }
}

// ------------------------------------------------------------
// Harmonized Student Walk Cycles (Built from Smooth Grounded Walk Base)
// ------------------------------------------------------------
void drawSidePerson(float x, float y, float scale, float r, float g, float b, float legPhase, bool facingRight, bool isCustom) {
    glPushMatrix();

    glTranslatef(x, y, 0.0f);
    glScalef(facingRight ? scale : -scale, scale, 1.0f);

    float legSwing = sinf(legPhase) * 12.0f;
    float armSwing = sinf(legPhase) * 14.0f;

    // Shadow
    glColor4f(0.0f, 0.0f, 0.0f, 0.25f);
    drawFilledCircle(0.0f, 0.0f, 25.0f);

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
    if (isCustom) glColor3f(0.12f, 0.42f, 0.78f);
    else glColor3f(r, g, b);
    glBegin(GL_POLYGON);
    glVertex2f(-18.0f, 133.0f);
    glVertex2f(18.0f, 133.0f);
    glVertex2f(14.0f, 72.0f);
    glVertex2f(-14.0f, 72.0f);
    glEnd();

    // Backpack
    if (isCustom) {
        glColor3f(0.20f, 0.22f, 0.27f);
        drawRectangle(-28.0f, 83.0f, -16.0f, 125.0f);
    }

    // Arms (thinner)
    glColor3f(0.82f, 0.59f, 0.40f);
    drawLine(-18.0f, 122.0f, -26.0f - armSwing, 80.0f, 4.0f);
    drawLine(18.0f, 122.0f, 26.0f + armSwing, 80.0f, 4.0f);

    // Trousers (narrower)
    if (isCustom) glColor3f(0.09f, 0.14f, 0.25f);
    else glColor3f(r * 0.6f, g * 0.6f, b * 0.6f);
    drawRectangle(-14.0f, 55.0f, 14.0f, 78.0f);

    // Legs (thinner)
    drawLine(-8.0f, 58.0f, -10.0f + legSwing, 8.0f, 4.0f);
    drawLine(8.0f, 58.0f, 10.0f - legSwing, 8.0f, 4.0f);

    // Shoes (adjusted)
    glColor3f(0.04f, 0.04f, 0.05f);
    drawRectangle(-18.0f + legSwing, 2.0f, -6.0f + legSwing, 8.0f);
    drawRectangle(6.0f - legSwing, 2.0f, 18.0f - legSwing, 8.0f);

    glPopMatrix();
}

void drawFrontPerson(float x, float y, float scale, float r, float g, float b, float legPhase, bool isCustom) {
    glPushMatrix();
    glTranslatef(x, y, 0.0f);
    glScalef(scale, scale, 1.0f);

    float legSwing = sinf(legPhase) * 10.0f;
    float armSwing = cosf(legPhase) * 8.0f;

    // Shadow
    glColor4f(0.0f, 0.0f, 0.0f, 0.25f);
    drawFilledCircle(0.0f, 0.0f, 25.0f);

    // Head
    glColor3f(0.82f, 0.59f, 0.40f);
    drawFilledCircle(0.0f, 160.0f, 27.0f);

    // Hair
    glColor3f(0.10f, 0.06f, 0.03f);
    glBegin(GL_POLYGON);
    glVertex2f(-26.0f, 160.0f);
    glVertex2f(-20.0f, 188.0f);
    glVertex2f(0.0f, 195.0f);
    glVertex2f(20.0f, 188.0f);
    glVertex2f(26.0f, 160.0f);
    glEnd();

    // Eyes
    glColor3f(0.06f, 0.04f, 0.02f);
    drawFilledCircle(-8.0f, 160.0f, 2.5f);
    drawFilledCircle(8.0f, 160.0f, 2.5f);

    // Body (slimmer)
    if (isCustom) glColor3f(0.12f, 0.42f, 0.78f);
    else glColor3f(r, g, b);
    drawRectangle(-16.0f, 72.0f, 16.0f, 133.0f);

    // Straps
    if (isCustom) {
        glColor3f(0.20f, 0.22f, 0.27f);
        drawRectangle(-12.0f, 72.0f, -8.0f, 133.0f);
        drawRectangle(8.0f, 72.0f, 12.0f, 133.0f);
    }

    // Arms (thinner)
    glColor3f(0.82f, 0.59f, 0.40f);
    drawLine(-20.0f, 122.0f, -24.0f, 80.0f + armSwing, 4.0f);
    drawLine(20.0f, 122.0f, 24.0f, 80.0f - armSwing, 4.0f);

    // Trousers (narrower)
    if (isCustom) glColor3f(0.09f, 0.14f, 0.25f);
    else glColor3f(r * 0.6f, g * 0.6f, b * 0.6f);
    drawRectangle(-14.0f, 55.0f, 14.0f, 78.0f);

    // Legs (thinner)
    drawLine(-8.0f, 58.0f, -8.0f, 8.0f + legSwing, 4.0f);
    drawLine(8.0f, 58.0f, 8.0f, 8.0f - legSwing, 4.0f);

    // Shoes (adjusted)
    glColor3f(0.04f, 0.04f, 0.05f);
    drawRectangle(-14.0f, 0.0f + legSwing, -4.0f, 8.0f + legSwing);
    drawRectangle(4.0f, 0.0f - legSwing, 14.0f, 8.0f - legSwing);

    glPopMatrix();
}

void drawBackPerson(float x, float y, float scale, float r, float g, float b, float legPhase, bool isCustom) {
    glPushMatrix();
    glTranslatef(x, y, 0.0f);
    glScalef(scale, scale, 1.0f);

    float legSwing = sinf(legPhase) * 10.0f;
    float armSwing = cosf(legPhase) * 8.0f;

    // Shadow
    glColor4f(0.0f, 0.0f, 0.0f, 0.25f);
    drawFilledCircle(0.0f, 0.0f, 25.0f);

    // Hair/Head Back
    glColor3f(0.10f, 0.06f, 0.03f);
    drawFilledCircle(0.0f, 160.0f, 27.0f);

    // Body
    if (isCustom) glColor3f(0.12f, 0.42f, 0.78f);
    else glColor3f(r, g, b);
    drawRectangle(-16.0f, 72.0f, 16.0f, 133.0f);  // narrower body

    // Backpack
    if (isCustom) {
        glColor3f(0.20f, 0.22f, 0.27f);
        drawRectangle(-14.0f, 80.0f, 14.0f, 125.0f);  // slimmer backpack
    }

    // Arms (thinner lines)
    glColor3f(0.82f, 0.59f, 0.40f);
    drawLine(-20.0f, 122.0f, -22.0f, 80.0f + armSwing, 3.0f);
    drawLine(20.0f, 122.0f, 22.0f, 80.0f - armSwing, 3.0f);

    // Trousers (narrower)
    if (isCustom) glColor3f(0.09f, 0.14f, 0.25f);
    else glColor3f(r * 0.6f, g * 0.6f, b * 0.6f);
    drawRectangle(-14.0f, 55.0f, 14.0f, 78.0f);

    // Legs (thinner lines)
    drawLine(-8.0f, 58.0f, -8.0f, 8.0f + legSwing, 3.0f);
    drawLine(8.0f, 58.0f, 8.0f, 8.0f - legSwing, 3.0f);

    // Shoes (adjusted to match thinner legs)
    glColor3f(0.04f, 0.04f, 0.05f);
    drawRectangle(-14.0f, 0.0f + legSwing, -4.0f, 8.0f + legSwing);
    drawRectangle(4.0f, 0.0f - legSwing, 14.0f, 8.0f - legSwing);

    glPopMatrix();
}

// ------------------------------------------------------------
// Environment
// ------------------------------------------------------------

void drawSky() {
    glBegin(GL_QUADS);
    glColor3f(0.32f, 0.61f, 0.94f); glVertex2f(0.0f, 720.0f); glVertex2f(1200.0f, 720.0f);
    glColor3f(0.72f, 0.91f, 1.0f);  glVertex2f(1200.0f, 260.0f); glVertex2f(0.0f, 260.0f);
    glEnd();
}

void drawCloud(float x, float y, float s) {
    glColor3f(0.97f, 0.97f, 0.98f);
    drawFilledCircle(x, y, 22.0f * s); drawFilledCircle(x + 28.0f * s, y + 10.0f * s, 29.0f * s);
    drawFilledCircle(x + 60.0f * s, y, 23.0f * s); drawRectangle(x, y - 17.0f * s, x + 60.0f * s, y + 7.0f * s);
}

void drawClouds() {
    drawCloud(140.0f + cloudOffset, 575.0f, 0.85f);
    drawCloud(510.0f + cloudOffset * 0.55f, 620.0f, 0.65f);
    drawCloud(930.0f + cloudOffset * 0.35f, 565.0f, 0.75f);
}

void drawMainAcademicBuilding() {
    glColor3f(0.92f, 0.93f, 0.94f); drawRectangle(250.0f, 285.0f, 355.0f, 500.0f);
    glColor3f(0.27f, 0.05f, 0.05f); drawRectangle(262.0f, 292.0f, 330.0f, 493.0f);
    glColor3f(0.86f, 0.89f, 0.91f);
    for (int y = 315; y <= 455; y += 34)
        for (int x = 275; x <= 307; x += 16) drawRectangle((float)x, (float)y, (float)x + 8.0f, (float)y + 19.0f);
    glColor3f(0.92f, 0.92f, 0.93f); drawRectangle(355.0f, 300.0f, 700.0f, 485.0f);
    glColor3f(0.72f, 0.02f, 0.02f);
    for (int row = 0; row < 4; row++) {
        float yy = 325.0f + row * 43.0f;
        drawRectangle(370.0f, yy, 435.0f, yy + 27.0f); drawRectangle(455.0f, yy, 525.0f, yy + 27.0f); drawRectangle(545.0f, yy, 615.0f, yy + 27.0f);
    }
    glColor3f(0.91f, 0.92f, 0.93f); drawRectangle(615.0f, 300.0f, 760.0f, 430.0f);
    glColor3f(0.73f, 0.02f, 0.02f);
    for (int row = 0; row < 3; row++) {
        float yy = 325.0f + row * 38.0f; drawRectangle(630.0f, yy, 742.0f, yy + 23.0f);
    }
    glColor3f(0.05f, 0.34f, 0.63f); drawRectangle(395.0f, 492.0f, 670.0f, 520.0f);
    glColor3f(1.0f, 1.0f, 1.0f); text(408.0f, 502.0f, "DAFFODIL INTERNATIONAL UNIVERSITY", GLUT_BITMAP_HELVETICA_10);
    glColor3f(0.38f, 0.82f, 0.90f); drawRectangle(438.0f, 300.0f, 466.0f, 440.0f);
    glColor3f(0.10f, 0.40f, 0.48f);
    for (int y = 320; y < 440; y += 25) drawLine(438.0f, (float)y, 466.0f, (float)y, 1.0f);
}

void drawLeftFacility() {
    glColor3f(0.88f, 0.89f, 0.90f); drawRectangle(20.0f, 280.0f, 220.0f, 390.0f);
    glColor3f(0.10f, 0.65f, 0.16f);
    for (int x = 20; x < 220; x += 30) drawRectangle((float)x, 390.0f, (float)x + 16.0f, 410.0f);
    glColor3f(0.10f, 0.50f, 0.74f); drawRectangle(55.0f, 280.0f, 86.0f, 338.0f); drawRectangle(135.0f, 280.0f, 166.0f, 338.0f);
    glColor3f(0.84f, 0.05f, 0.04f); drawRectangle(20.0f, 280.0f, 220.0f, 315.0f);
}

void drawPrayerBuilding() {
    glColor3f(0.57f, 0.57f, 0.58f); glBegin(GL_POLYGON);
    glVertex2f(830.0f, 280.0f); glVertex2f(875.0f, 405.0f); glVertex2f(1075.0f, 405.0f); glVertex2f(1120.0f, 280.0f); glEnd();
    glColor3f(0.76f, 0.76f, 0.77f); drawRectangle(825.0f, 402.0f, 1125.0f, 414.0f);
    glColor3f(0.28f, 0.03f, 0.02f); drawRectangle(902.0f, 280.0f, 1045.0f, 382.0f);
    glColor3f(1.0f, 0.97f, 0.89f); drawRectangle(930.0f, 280.0f, 1018.0f, 352.0f);
    glColor3f(0.02f, 0.40f, 0.43f); glBegin(GL_POLYGON);
    glVertex2f(959.0f, 280.0f); glVertex2f(959.0f, 320.0f); glVertex2f(974.0f, 338.0f); glVertex2f(989.0f, 320.0f); glVertex2f(989.0f, 280.0f); glEnd();
}

void drawPineTree(float x, float y, float s) {
    glPushMatrix(); glTranslatef(x, y, 0.0f); glScalef(s, s, 1.0f);
    glColor3f(0.25f, 0.12f, 0.04f); drawRectangle(-8.0f, 0.0f, 8.0f, 72.0f);
    glColor3f(0.02f, 0.37f, 0.10f);
    glBegin(GL_TRIANGLES); glVertex2f(0.0f, 155.0f); glVertex2f(-55.0f, 68.0f); glVertex2f(55.0f, 68.0f); glEnd();
    glBegin(GL_TRIANGLES); glVertex2f(0.0f, 125.0f); glVertex2f(-48.0f, 45.0f); glVertex2f(48.0f, 45.0f); glEnd();
    glBegin(GL_TRIANGLES); glVertex2f(0.0f, 95.0f); glVertex2f(-42.0f, 25.0f); glVertex2f(42.0f, 25.0f); glEnd();
    glPopMatrix();
}

void drawRoundTree(float x, float y, float s) {
    glPushMatrix(); glTranslatef(x, y, 0.0f); glScalef(s, s, 1.0f);
    glColor3f(0.28f, 0.13f, 0.04f); drawRectangle(-7.0f, 0.0f, 7.0f, 60.0f);
    glColor3f(0.03f, 0.45f, 0.10f);
    drawFilledCircle(-24.0f, 75.0f, 28.0f); drawFilledCircle(10.0f, 85.0f, 34.0f); drawFilledCircle(38.0f, 72.0f, 25.0f); drawFilledCircle(4.0f, 112.0f, 29.0f);
    glPopMatrix();
}

void drawBush(float x, float y, float s) {
    glColor3f(0.03f, 0.48f, 0.10f);
    drawFilledCircle(x, y, 18.0f * s); drawFilledCircle(x + 17.0f * s, y + 4.0f * s, 20.0f * s); drawFilledCircle(x + 35.0f * s, y, 17.0f * s);
}

const float ROAD_Y_BOTTOM = 210.0f; const float ROAD_Y_TOP = 285.0f;
void drawCampusRoad() { glColor3f(0.96f, 0.79f, 0.58f); drawRectangle(0.0f, ROAD_Y_BOTTOM, 1200.0f, ROAD_Y_TOP); }

void drawMainGate() {
    glColor3f(0.87f, 0.88f, 0.89f); drawRectangle(505.0f, 260.0f, 545.0f, 350.0f); drawRectangle(655.0f, 260.0f, 695.0f, 350.0f);
    glColor3f(0.10f, 0.44f, 0.66f); drawRectangle(525.0f, 340.0f, 675.0f, 370.0f);
    glColor3f(1.0f, 1.0f, 1.0f); text(552.0f, 349.0f, "DIU MAIN GATE", GLUT_BITMAP_HELVETICA_12);
    glColor3f(0.16f, 0.18f, 0.20f);
    for (float x = 545.0f; x < 550.0f; x += 12.0f) drawLine(x, 260.0f, x, 330.0f, 3.0f);
    for (float x = 650.0f; x < 655.0f; x += 12.0f) drawLine(x, 260.0f, x, 330.0f, 3.0f);
    drawLine(545.0f, 328.0f, 550.0f, 328.0f, 4.0f); drawLine(650.0f, 328.0f, 655.0f, 328.0f, 4.0f);
}

const float BRANCH_LEFT = 75.0f; const float BRANCH_RIGHT = 125.0f;

void drawIntersection() {
    glColor3f(0.94f, 0.77f, 0.56f); drawRectangle(BRANCH_LEFT, 0.0f, BRANCH_RIGHT, ROAD_Y_BOTTOM);
    glColor3f(0.93f, 0.75f, 0.54f); drawRectangle(BRANCH_LEFT, ROAD_Y_BOTTOM - 6.0f, BRANCH_RIGHT, ROAD_Y_BOTTOM);
    glColor3f(0.98f, 0.96f, 0.90f); drawLine(BRANCH_LEFT, 0.0f, BRANCH_LEFT, ROAD_Y_BOTTOM, 2.0f); drawLine(BRANCH_RIGHT, 0.0f, BRANCH_RIGHT, ROAD_Y_BOTTOM, 2.0f);
    float midX = (BRANCH_LEFT + BRANCH_RIGHT) * 0.5f;
    for (float y = 10.0f; y < ROAD_Y_BOTTOM - 15.0f; y += 30.0f) drawLine(midX, y, midX, y + 16.0f, 3.0f);
    float midY = (ROAD_Y_BOTTOM + ROAD_Y_TOP) * 0.5f;
    for (float x = BRANCH_RIGHT + 25.0f; x < 1170.0f; x += 34.0f) drawLine(x, midY, x + 18.0f, midY, 3.0f);
}

// ------------------------------------------------------------
// Field & Goalpost
// ------------------------------------------------------------
const float FIELD_TOP_Y = 45.0f; const float FIELD_BOTTOM_Y = 195.0f;
const float FIELD_TL_X = 230.0f; const float FIELD_TR_X = 1120.0f;
const float FIELD_BL_X = 300.0f; const float FIELD_BR_X = 1150.0f;

void fieldPoint(float u, float v, float& outX, float& outY) {
    float y = FIELD_TOP_Y + (FIELD_BOTTOM_Y - FIELD_TOP_Y) * v;
    float xLeft = FIELD_TL_X + (FIELD_BL_X - FIELD_TL_X) * v;
    float xRight = FIELD_TR_X + (FIELD_BR_X - FIELD_TR_X) * v;
    outX = xLeft + (xRight - xLeft) * u; outY = y;
}

void drawPitchBox(float u0, float u1, float v0, float v1) {
    float x = 0.0f, y = 0.0f;
    glBegin(GL_LINE_LOOP);
    fieldPoint(u0, v0, x, y); glVertex2f(x, y); fieldPoint(u1, v0, x, y); glVertex2f(x, y);
    fieldPoint(u1, v1, x, y); glVertex2f(x, y); fieldPoint(u0, v1, x, y); glVertex2f(x, y);
    glEnd();
}

void drawGoalFrame(float x0, float y0, float x1, float y1, float depth) {
    float h = 40.0f;
    glColor3f(0.92f, 0.95f, 0.95f);
    drawLine(x0 + depth, y0, x1 + depth, y1, 2.0f); drawLine(x0, y0, x0 + depth, y0, 2.0f); drawLine(x1, y1, x1 + depth, y1, 2.0f);
    drawLine(x0, y0, x0, y0 + h, 4.0f); drawLine(x1, y1, x1, y1 + h, 4.0f); drawLine(x0, y0 + h, x1, y1 + h, 4.0f);
    glColor3f(0.85f, 0.90f, 0.90f);
    for (int i = 1; i <= 6; i++) {
        float px = x0 + (x1 - x0) * i / 7.0f; float py = y0 + (y1 - y0) * i / 7.0f;
        drawLine(px, py + h, px + depth, py, 1.0f);
    }
    for (int i = 1; i <= 5; i++) {
        float ch = h * (1.0f - i / 6.0f); float cd = depth * (i / 6.0f);
        drawLine(x0 + cd, y0 + ch, x1 + cd, y1 + ch, 1.0f);
    }
    drawLine(x0, y0 + h, x0 + depth, y0, 1.5f); drawLine(x1, y1 + h, x1 + depth, y1, 1.5f);
    for(int i = 1; i <= 4; i++) {
        float frac = i / 5.0f;
        drawLine(x0, y0, x0 + depth * frac, y0 + h * (1.0f - frac), 1.0f);
        drawLine(x1, y1, x1 + depth * frac, y1 + h * (1.0f - frac), 1.0f);
    }
}

void drawFootballField() {
    glColor3f(0.02f, 0.58f, 0.05f); drawRectangle(0.0f, 0.0f, 1200.0f, ROAD_Y_BOTTOM);
    glColor3f(0.97f, 0.97f, 0.95f); glLineWidth(2.0f);
    float x = 0.0f, y = 0.0f;
    glBegin(GL_LINE_LOOP);
    fieldPoint(0.0f, 0.0f, x, y); glVertex2f(x, y); fieldPoint(1.0f, 0.0f, x, y); glVertex2f(x, y);
    fieldPoint(1.0f, 1.0f, x, y); glVertex2f(x, y); fieldPoint(0.0f, 1.0f, x, y); glVertex2f(x, y);
    glEnd();
    float hx0, hy0, hx1, hy1;
    fieldPoint(0.5f, 0.0f, hx0, hy0); fieldPoint(0.5f, 1.0f, hx1, hy1); drawLine(hx0, hy0, hx1, hy1, 2.0f);
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i <= 64; i++) {
        float a = 2.0f * PI * i / 64.0f;
        float u = 0.5f + 0.0871f * cos(a); float v = 0.5f + 0.1346f * sin(a);
        fieldPoint(u, v, x, y); glVertex2f(x, y);
    }
    glEnd();
    fieldPoint(0.5f, 0.5f, x, y); drawFilledCircle(x, y, 2.0f);
    drawPitchBox(0.0f, 0.1571f, 0.2035f, 0.7965f); drawPitchBox(1.0f - 0.1571f, 1.0f, 0.2035f, 0.7965f);
    drawPitchBox(0.0f, 0.0524f, 0.3653f, 0.6347f); drawPitchBox(1.0f - 0.0524f, 1.0f, 0.3653f, 0.6347f);
    glLineWidth(1.0f);
    float gx0, gy0, gx1, gy1;
    fieldPoint(0.0f, 0.4297f, gx0, gy0); fieldPoint(0.0f, 0.5703f, gx1, gy1); drawGoalFrame(gx0, gy0, gx1, gy1, -44.0f);
    fieldPoint(1.0f, 0.4297f, gx0, gy0); fieldPoint(1.0f, 0.5703f, gx1, gy1); drawGoalFrame(gx0, gy0, gx1, gy1, 44.0f);
}

// ------------------------------------------------------------
// Student Structures & State
// ------------------------------------------------------------

const int STUDENT_COUNT = 8;
const float BASE_SPEED = 0.50f;
const float NPC_SCALE = 0.20f;

struct MainStudent { float x, y, scale, speed, r, g, b; };
MainStudent mainStudents[STUDENT_COUNT];

struct CustomStudent { float x, y, scale, speed; int state; };
CustomStudent customStudent;

struct TurningStudent { float x, y, scale, speed, r, g, b; int route, state; };
const int TURN_STUDENT_COUNT = 4;
TurningStudent turnStudents[TURN_STUDENT_COUNT];

void initStudents() {
    const float colorR[] = { 0.80f, 0.10f, 0.82f, 0.20f, 0.90f, 0.55f, 0.40f, 0.70f, 0.30f, 0.85f };
    const float colorG[] = { 0.20f, 0.70f, 0.45f, 0.80f, 0.50f, 0.20f, 0.45f, 0.30f, 0.60f, 0.10f };
    const float colorB[] = { 0.12f, 0.22f, 0.12f, 0.20f, 0.10f, 0.70f, 0.80f, 0.50f, 0.40f, 0.60f };

    for (int i = 0; i < STUDENT_COUNT; i++) {
        mainStudents[i].x = -80.0f - i * 160.0f;
        mainStudents[i].y = 215.0f + ((i % 3) * 14.0f);
        mainStudents[i].scale = NPC_SCALE;
        mainStudents[i].speed = BASE_SPEED;
        mainStudents[i].r = colorR[i];
        mainStudents[i].g = colorG[i];
        mainStudents[i].b = colorB[i];
    }

    customStudent.x = -200.0f;
    customStudent.y = 222.0f;
    customStudent.scale = 0.22f;
    customStudent.speed = BASE_SPEED;
    customStudent.state = 0;

    turnStudents[0] = { 105.0f, 50.0f, NPC_SCALE, BASE_SPEED * 0.9f, colorR[2], colorG[2], colorB[2], 0, 0 };
    turnStudents[1] = { 85.0f, -150.0f, NPC_SCALE, BASE_SPEED * 0.8f, colorR[3], colorG[3], colorB[3], 1, 0 };
    turnStudents[2] = { -100.0f, 246.0f, NPC_SCALE, BASE_SPEED * 0.85f, colorR[4], colorG[4], colorB[4], 2, 0 };
    turnStudents[3] = { 1300.0f, 236.0f, NPC_SCALE, BASE_SPEED * 0.95f, colorR[5], colorG[5], colorB[5], 3, 0 };
}

void updateStudents() {
    globalLegPhase += BASE_SPEED * 0.22f;

    for (int i = 0; i < STUDENT_COUNT; i++) {
        mainStudents[i].x += mainStudents[i].speed;
        if (mainStudents[i].x > 1250.0f) mainStudents[i].x = -150.0f;
    }

    if (customStudent.state == 0) {
        customStudent.x += customStudent.speed;
        if (customStudent.x >= 102.0f) {
            customStudent.x = 102.0f;
            customStudent.state = 1;
        }
    } else if (customStudent.state == 1) {
        customStudent.y -= customStudent.speed;
        if (customStudent.y < -80.0f) {
            customStudent.x = -1200.0f;
            customStudent.y = 222.0f;
            customStudent.state = 0;
        }
    }

    for (int i = 0; i < TURN_STUDENT_COUNT; i++) {
        if (turnStudents[i].route == 0) {
            if (turnStudents[i].state == 0) {
                turnStudents[i].y += turnStudents[i].speed;
                if (turnStudents[i].y >= 218.0f) { turnStudents[i].y = 218.0f; turnStudents[i].state = 1; }
            } else {
                turnStudents[i].x += turnStudents[i].speed;
                if (turnStudents[i].x > 1250.0f) { turnStudents[i].x = 105.0f; turnStudents[i].y = -100.0f; turnStudents[i].state = 0; }
            }
        } else if (turnStudents[i].route == 1) {
            if (turnStudents[i].state == 0) {
                turnStudents[i].y += turnStudents[i].speed;
                if (turnStudents[i].y >= 232.0f) { turnStudents[i].y = 232.0f; turnStudents[i].state = 1; }
            } else {
                turnStudents[i].x -= turnStudents[i].speed;
                if (turnStudents[i].x < -100.0f) { turnStudents[i].x = 85.0f; turnStudents[i].y = -150.0f; turnStudents[i].state = 0; }
            }
        } else if (turnStudents[i].route == 2) {
            if (turnStudents[i].state == 0) {
                turnStudents[i].x += turnStudents[i].speed;
                if (turnStudents[i].x >= 115.0f) { turnStudents[i].x = 115.0f; turnStudents[i].state = 1; }
            } else {
                turnStudents[i].y -= turnStudents[i].speed;
                if (turnStudents[i].y < -100.0f) { turnStudents[i].x = -150.0f; turnStudents[i].y = 246.0f; turnStudents[i].state = 0; }
            }
        } else if (turnStudents[i].route == 3) {
            if (turnStudents[i].state == 0) {
                turnStudents[i].x -= turnStudents[i].speed;
                if (turnStudents[i].x <= 95.0f) { turnStudents[i].x = 95.0f; turnStudents[i].state = 1; }
            } else {
                turnStudents[i].y -= turnStudents[i].speed;
                if (turnStudents[i].y < -100.0f) { turnStudents[i].x = 1350.0f; turnStudents[i].y = 236.0f; turnStudents[i].state = 0; }
            }
        }
    }
}

// Struct to aid with depth/layering sorting
struct DrawablePerson {
    float x, y, scale, r, g, b, legPhase;
    int type;
    bool isCustom, facingRight;
};

void drawWalkingStudents() {
    DrawablePerson people[20];
    int pCount = 0;

    for (int i = 0; i < STUDENT_COUNT; i++) {
        if (mainStudents[i].x >= -80.0f && mainStudents[i].x <= 1220.0f) {
            people[pCount].x = mainStudents[i].x;
            people[pCount].y = mainStudents[i].y;
            people[pCount].scale = mainStudents[i].scale;
            people[pCount].r = mainStudents[i].r;
            people[pCount].g = mainStudents[i].g;
            people[pCount].b = mainStudents[i].b;
            people[pCount].legPhase = globalLegPhase + i * 1.2f;
            people[pCount].type = 0;
            people[pCount].isCustom = false;
            people[pCount].facingRight = true;
            pCount++;
        }
    }

    for (int i = 0; i < TURN_STUDENT_COUNT; i++) {
        int type = 0; bool facingR = true;
        if (turnStudents[i].route == 0) {
            if (turnStudents[i].state == 0) type = 2; else { type = 0; facingR = true; }
        } else if (turnStudents[i].route == 1) {
            if (turnStudents[i].state == 0) type = 2; else { type = 0; facingR = false; }
        } else if (turnStudents[i].route == 2) {
            if (turnStudents[i].state == 0) { type = 0; facingR = true; } else type = 1;
        } else if (turnStudents[i].route == 3) {
            if (turnStudents[i].state == 0) { type = 0; facingR = false; } else type = 1;
        }

        people[pCount].x = turnStudents[i].x;
        people[pCount].y = turnStudents[i].y;
        people[pCount].scale = turnStudents[i].scale;
        people[pCount].r = turnStudents[i].r;
        people[pCount].g = turnStudents[i].g;
        people[pCount].b = turnStudents[i].b;
        people[pCount].legPhase = globalLegPhase + i * 2.0f;
        people[pCount].type = type;
        people[pCount].isCustom = false;
        people[pCount].facingRight = facingR;
        pCount++;
    }

    people[pCount].x = customStudent.x;
    people[pCount].y = customStudent.y;
    people[pCount].scale = customStudent.scale;
    people[pCount].r = 0.0f;
    people[pCount].g = 0.0f;
    people[pCount].b = 0.0f;
    people[pCount].legPhase = globalLegPhase;
    people[pCount].type = customStudent.state == 0 ? 0 : 1;
    people[pCount].isCustom = true;
    people[pCount].facingRight = true;
    pCount++;

    // Bubble Sort for Depth Sorting
    for (int i = 0; i < pCount - 1; i++) {
        for (int j = 0; j < pCount - i - 1; j++) {
            if (people[j].y < people[j + 1].y) {
                DrawablePerson temp = people[j];
                people[j] = people[j + 1];
                people[j + 1] = temp;
            }
        }
    }

    // Draw sorted people using unified walk system configurations
    for (int i = 0; i < pCount; i++) {
        if (people[i].type == 0) {
            drawSidePerson(people[i].x, people[i].y, people[i].scale, people[i].r, people[i].g, people[i].b, people[i].legPhase, people[i].facingRight, people[i].isCustom);
        } else if (people[i].type == 1) {
            drawFrontPerson(people[i].x, people[i].y, people[i].scale, people[i].r, people[i].g, people[i].b, people[i].legPhase, people[i].isCustom);
        } else if (people[i].type == 2) {
            drawBackPerson(people[i].x, people[i].y, people[i].scale, people[i].r, people[i].g, people[i].b, people[i].legPhase, people[i].isCustom);
        }
    }
}

// ------------------------------------------------------------
// Display
// ------------------------------------------------------------

void display() {
    glClear(GL_COLOR_BUFFER_BIT);

    drawSky();
    drawClouds();
    drawMainAcademicBuilding();
    drawLeftFacility();
    drawPrayerBuilding();

    drawFootballField();
    drawCampusRoad();
    drawIntersection();
    drawMainGate();

    drawPineTree(170.0f, 285.0f, 0.75f); drawPineTree(1080.0f, 285.0f, 0.72f);
    drawRoundTree(330.0f, 285.0f, 0.55f); drawRoundTree(790.0f, 285.0f, 0.55f); drawRoundTree(1145.0f, 285.0f, 0.50f);

    float treeXPositions[] = {170.0f, 330.0f, 790.0f, 1080.0f, 1145.0f};

    for (int i = 0; i < 12; i++) {
        float bx = 15.0f + i * 100.0f;

        if (bx > BRANCH_LEFT - 25.0f && bx < BRANCH_RIGHT + 25.0f) continue;
        if (bx > 450.0f && bx < 750.0f) continue;

        bool nearTree = false;
        for (int t = 0; t < 5; t++) {
            if (fabs(bx - treeXPositions[t]) < 50.0f) { nearTree = true; break; }
        }
        if (nearTree) continue;

        drawBush(bx, 285.0f, 0.72f);
    }

    drawBush(470.0f, 285.0f, 0.72f);
    drawBush(720.0f, 285.0f, 0.72f);

    drawWalkingStudents();

    if (paused) {
        glColor3f(1.0f, 0.85f, 0.10f);
        text(555.0f, 690.0f, "PAUSED", GLUT_BITMAP_HELVETICA_18);
    }
    glutSwapBuffers();
}

void timer(int value) {
    updateAudioState(); // Continually verify state matches active runtime status
    if (!paused) {
        cloudOffset += 0.10f;
        if (cloudOffset > 1200.0f) cloudOffset = -260.0f;
        frameCounter++;
        updateStudents();
    }
    glutPostRedisplay();
    glutTimerFunc(16, timer, 0);
}

void resetScene() {
    cloudOffset = 0.0f;
    frameCounter = 0;
    initStudents();
    glutPostRedisplay();
}

void keyboard(unsigned char key, int x, int y) {
    switch (key) {
        case 'p': case 'P':
            paused = !paused;
            updateAudioState();
            break;
        case 'r': case 'R': resetScene(); break;
        case 27: StopMusic(); exit(0);
    }
    glutPostRedisplay();
}

void init() {
    glClearColor(0.38f, 0.72f, 0.95f, 1.0f);
    glMatrixMode(GL_PROJECTION); glLoadIdentity(); gluOrtho2D(0.0, W, 0.0, H);
    glMatrixMode(GL_MODELVIEW); glLoadIdentity();
    glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_LINE_SMOOTH); glEnable(GL_POINT_SMOOTH);
    initStudents();
    updateAudioState(); // Initialize sound immediately upon startup configuration matching animation flow
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(W, H);
    glutInitWindowPosition(40, 25);
    glutCreateWindow("Scene 4 - DIU Campus Arrival");
    init();
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutTimerFunc(16, timer, 0);
    glutMainLoop();
    return 0;
}
