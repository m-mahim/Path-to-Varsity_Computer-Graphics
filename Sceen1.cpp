// Scene 1: Morning Alarm in a Celebrity's Room
// C++ OpenGL / GLUT 2D animation with Synchronized Audio Control
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <string>
#include <algorithm>

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

// Platform-specific headers for system sounds and audio streaming
#ifdef _WIN32
#include <windows.h>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib") // Links Windows Multimedia library for sound functions
#endif

constexpr float WORLD_W = 1200.0f;
constexpr float WORLD_H = 700.0f;
constexpr float PI = 3.14159265358979323846f;

constexpr float ALARM_TIME = 7.0f;
constexpr float WAKE_END = 8.7f;
constexpr float SIT_END = 10.8f;
constexpr float WALK_START = 12.0f;
constexpr float WALK_END = 17.6f;
constexpr float ALARM_OFF_TIME = 18.6f;
constexpr float END_TIME = 24.0f;

// Music configuration path
const std::string MUSIC_PATH = "D:\\8_th_Semister\\Computer_Graphics_Practice\\S2.test2_sound\\morning.wav";

float sceneTime = 0.0f;
float fanAngle = 0.0f;
int lastTickMs = 0;
int lastBeepSlot = -1;

bool paused = false;
bool autoLoop = true;
bool manualAlarmOff = false;
bool manualControl = false;
bool fanRunning = true;
bool isMusicPlaying = false; // Tracks whether the music file is actively running

enum class ManualPose {
    Sleeping,
    Sitting,
    Standing
};

ManualPose manualPose = ManualPose::Sleeping;
float manualCharacterX = 600.0f;
float manualWalkPhase = 0.0f;
bool manualReach = false;

int viewportX = 0;
int viewportY = 0;
int viewportW = 1200;
int viewportH = 700;

struct Color {
    float r, g, b, a;
    Color(float red, float green, float blue, float alpha = 1.0f)
        : r(red), g(green), b(blue), a(alpha) {}
};

// Global audio control bridge to safely interact with WinMM
void ManageMusicState(bool shouldPlay) {
#ifdef _WIN32
    if (shouldPlay) {
        if (!isMusicPlaying) {
            // Begins looping background wave audio asynchronously
            PlaySound(MUSIC_PATH.c_str(), NULL, SND_FILENAME | SND_ASYNC | SND_LOOP);
            isMusicPlaying = true;
        }
    } else {
        if (isMusicPlaying) {
            // Passing NULL terminates all currently executing audio streams on the channel instantly
            PlaySound(NULL, NULL, 0);
            isMusicPlaying = false;
        }
    }
#endif
}

float clamp01(float value) {
    return std::max(0.0f, std::min(1.0f, value));
}

float lerp(float a, float b, float t) {
    return a + (b - a) * t;
}

float smooth01(float t) {
    t = clamp01(t);
    return t * t * (3.0f - 2.0f * t);
}

void setColor(const Color& c) {
    glColor4f(c.r, c.g, c.b, c.a);
}

void rectangle(float x1, float y1, float x2, float y2, const Color& c) {
    setColor(c);
    glBegin(GL_QUADS);
    glVertex2f(x1, y1);
    glVertex2f(x2, y1);
    glVertex2f(x2, y2);
    glVertex2f(x1, y2);
    glEnd();
}

void gradientRectangle(float x1, float y1, float x2, float y2,
                       const Color& bottom, const Color& top) {
    glBegin(GL_QUADS);
    setColor(bottom);
    glVertex2f(x1, y1);
    glVertex2f(x2, y1);
    setColor(top);
    glVertex2f(x2, y2);
    glVertex2f(x1, y2);
    glEnd();
}

void outlineRectangle(float x1, float y1, float x2, float y2,
                      const Color& c, float width = 2.0f) {
    setColor(c);
    glLineWidth(width);
    glBegin(GL_LINE_LOOP);
    glVertex2f(x1, y1);
    glVertex2f(x2, y1);
    glVertex2f(x2, y2);
    glVertex2f(x1, y2);
    glEnd();
    glLineWidth(1.0f);
}

void circle(float cx, float cy, float radius, const Color& c, int segments = 64) {
    setColor(c);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx, cy);
    for (int i = 0; i <= segments; ++i) {
        float angle = 2.0f * PI * static_cast<float>(i) / static_cast<float>(segments);
        glVertex2f(cx + radius * std::cos(angle), cy + radius * std::sin(angle));
    }
    glEnd();
}

void ellipse(float cx, float cy, float rx, float ry, const Color& c, int segments = 64) {
    setColor(c);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx, cy);
    for (int i = 0; i <= segments; ++i) {
        float angle = 2.0f * PI * static_cast<float>(i) / static_cast<float>(segments);
        glVertex2f(cx + rx * std::cos(angle), cy + ry * std::sin(angle));
    }
    glEnd();
}

void line(float x1, float y1, float x2, float y2,
          const Color& c, float width = 2.0f) {
    setColor(c);
    glLineWidth(width);
    glBegin(GL_LINES);
    glVertex2f(x1, y1);
    glVertex2f(x2, y2);
    glEnd();
    glLineWidth(1.0f);
}

void filledPolygon(const float points[][2], int count, const Color& c) {
    setColor(c);
    glBegin(GL_POLYGON);
    for (int i = 0; i < count; ++i) {
        glVertex2f(points[i][0], points[i][1]);
    }
    glEnd();
}

void drawText(float x, float y, const std::string& text,
              const Color& c, void* font = GLUT_BITMAP_HELVETICA_18) {
    setColor(c);
    glRasterPos2f(x, y);
    for (char ch : text) {
        glutBitmapCharacter(font, ch);
    }
}

void drawStar(float cx, float cy, float outerR, float innerR, const Color& c) {
    setColor(c);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx, cy);
    for (int i = 0; i <= 10; ++i) {
        float r = (i % 2 == 0) ? outerR : innerR;
        float angle = PI / 2.0f + static_cast<float>(i) * PI / 5.0f;
        glVertex2f(cx + r * std::cos(angle), cy + r * std::sin(angle));
    }
    glEnd();
}

bool isAlarmActive() {
    if (sceneTime < ALARM_TIME || manualAlarmOff) return false;
    if (manualControl) return true;
    return sceneTime < ALARM_OFF_TIME;
}

void drawCloud(float x, float y, float s) {
    Color shadow(0.80f, 0.86f, 0.92f, 0.80f);
    Color white(0.98f, 0.98f, 1.00f, 0.95f);
    ellipse(x, y - 4.0f * s, 42.0f * s, 14.0f * s, shadow);
    circle(x - 24.0f * s, y, 19.0f * s, white);
    circle(x, y + 10.0f * s, 27.0f * s, white);
    circle(x + 28.0f * s, y, 20.0f * s, white);
    ellipse(x, y - 3.0f * s, 48.0f * s, 16.0f * s, white);
}

void drawBird(float x, float y, float s, float phase) {
    float flap = 0.25f + 0.55f * std::fabs(std::sin(phase));
    Color dark(0.13f, 0.15f, 0.19f);
    line(x, y, x - 12.0f * s, y + 10.0f * s * flap, dark, 2.4f);
    line(x, y, x + 12.0f * s, y + 10.0f * s * flap, dark, 2.4f);
    ellipse(x, y - 1.0f * s, 4.0f * s, 2.2f * s, dark);
}

void drawSun(float x, float y, float radius) {
    Color ray(1.0f, 0.78f, 0.18f, 0.48f);
    Color sun(1.0f, 0.71f, 0.12f);
    for (int i = 0; i < 16; ++i) {
        float a = 2.0f * PI * static_cast<float>(i) / 16.0f;
        line(x + std::cos(a) * (radius + 5.0f),
             y + std::sin(a) * (radius + 5.0f),
             x + std::cos(a) * (radius + 18.0f),
             y + std::sin(a) * (radius + 18.0f), ray, 2.0f);
    }
    circle(x, y, radius, sun);
    circle(x - 10.0f, y + 10.0f, radius * 0.28f, Color(1.0f, 0.84f, 0.34f, 0.55f));
}

void drawWindowAndOutside() {
    gradientRectangle(790.0f, 385.0f, 1130.0f, 655.0f,
                      Color(1.0f, 0.72f, 0.46f), Color(0.37f, 0.70f, 0.94f));
    float rise = smooth01(sceneTime / 18.0f);
    float sunY = lerp(405.0f, 550.0f, rise);
    drawSun(930.0f, sunY, 31.0f);

    const float hill1[][2] = {
        {790.0f, 385.0f}, {790.0f, 435.0f}, {850.0f, 470.0f},
        {930.0f, 420.0f}, {1010.0f, 462.0f}, {1130.0f, 420.0f}, {1130.0f, 385.0f}
    };
    filledPolygon(hill1, 7, Color(0.26f, 0.55f, 0.38f));

    const float hill2[][2] = {
        {790.0f, 385.0f}, {790.0f, 414.0f}, {870.0f, 448.0f},
        {960.0f, 405.0f}, {1040.0f, 445.0f}, {1130.0f, 408.0f}, {1130.0f, 385.0f}
    };
    filledPolygon(hill2, 7, Color(0.18f, 0.44f, 0.31f));

    float cloud1X = 885.0f + 65.0f * std::sin(sceneTime * 0.25f);
    float cloud2X = 1025.0f + 50.0f * std::sin(sceneTime * 0.19f + 1.8f);
    drawCloud(cloud1X, 590.0f, 0.68f);
    drawCloud(cloud2X, 520.0f, 0.52f);

    float birdTravel = std::fmod(sceneTime * 30.0f, 250.0f);
    drawBird(825.0f + birdTravel, 555.0f + 8.0f * std::sin(sceneTime),
             0.8f, sceneTime * 7.0f);
    drawBird(860.0f + std::fmod(sceneTime * 25.0f + 80.0f, 220.0f),
             500.0f + 7.0f * std::sin(sceneTime * 1.2f + 1.0f),
             0.62f, sceneTime * 8.0f + 1.0f);
    drawBird(850.0f + std::fmod(sceneTime * 20.0f + 150.0f, 210.0f),
             610.0f + 5.0f * std::sin(sceneTime * 0.9f + 2.0f),
             0.52f, sceneTime * 9.0f + 2.0f);

    rectangle(775.0f, 370.0f, 790.0f, 670.0f, Color(0.29f, 0.18f, 0.10f));
    rectangle(1130.0f, 370.0f, 1145.0f, 670.0f, Color(0.29f, 0.18f, 0.10f));
    rectangle(775.0f, 655.0f, 1145.0f, 670.0f, Color(0.29f, 0.18f, 0.10f));
    rectangle(775.0f, 370.0f, 1145.0f, 385.0f, Color(0.29f, 0.18f, 0.10f));

    const float leftPanel[][2] = {
        {790.0f, 385.0f}, {742.0f, 360.0f}, {742.0f, 680.0f}, {790.0f, 655.0f}
    };
    filledPolygon(leftPanel, 4, Color(0.62f, 0.82f, 0.92f, 0.30f));
    line(790.0f, 385.0f, 742.0f, 360.0f, Color(0.25f, 0.17f, 0.11f), 6.0f);
    line(742.0f, 360.0f, 742.0f, 680.0f, Color(0.25f, 0.17f, 0.11f), 6.0f);
    line(742.0f, 680.0f, 790.0f, 655.0f, Color(0.25f, 0.17f, 0.11f), 6.0f);
    line(790.0f, 385.0f, 790.0f, 655.0f, Color(0.25f, 0.17f, 0.11f), 6.0f);

    const float rightPanel[][2] = {
        {1130.0f, 385.0f}, {1178.0f, 360.0f}, {1178.0f, 680.0f}, {1130.0f, 655.0f}
    };
    filledPolygon(rightPanel, 4, Color(0.62f, 0.82f, 0.92f, 0.30f));
    line(1130.0f, 385.0f, 1178.0f, 360.0f, Color(0.25f, 0.17f, 0.11f), 6.0f);
    line(1178.0f, 360.0f, 1178.0f, 680.0f, Color(0.25f, 0.17f, 0.11f), 6.0f);
    line(1178.0f, 680.0f, 1130.0f, 655.0f, Color(0.25f, 0.17f, 0.11f), 6.0f);
    line(1130.0f, 385.0f, 1130.0f, 655.0f, Color(0.25f, 0.17f, 0.11f), 6.0f);

    rectangle(760.0f, 355.0f, 1160.0f, 378.0f, Color(0.43f, 0.27f, 0.14f));
    rectangle(755.0f, 348.0f, 1165.0f, 357.0f, Color(0.30f, 0.18f, 0.10f));
    circle(757.0f, 520.0f, 5.0f, Color(0.82f, 0.62f, 0.23f));
    circle(1163.0f, 520.0f, 5.0f, Color(0.82f, 0.62f, 0.23f));
}

void drawRoomShell() {
    gradientRectangle(0.0f, 210.0f, WORLD_W, WORLD_H,
                      Color(0.91f, 0.83f, 0.74f), Color(0.75f, 0.84f, 0.90f));
    gradientRectangle(0.0f, 0.0f, WORLD_W, 210.0f,
                      Color(0.40f, 0.24f, 0.14f), Color(0.62f, 0.39f, 0.22f));
    line(0.0f, 210.0f, WORLD_W, 210.0f, Color(0.25f, 0.15f, 0.09f), 5.0f);

    for (int y = 35; y < 210; y += 42) {
        line(0.0f, static_cast<float>(y), WORLD_W, static_cast<float>(y),
             Color(0.34f, 0.20f, 0.12f, 0.55f), 1.2f);
    }
    for (int x = -150; x < 1300; x += 120) {
        line(static_cast<float>(x), 0.0f, static_cast<float>(x + 90), 210.0f,
             Color(0.31f, 0.18f, 0.11f, 0.50f), 1.0f);
    }

    rectangle(0.0f, 202.0f, WORLD_W, 216.0f, Color(0.80f, 0.70f, 0.59f));
    rectangle(0.0f, 202.0f, WORLD_W, 207.0f, Color(0.48f, 0.32f, 0.20f));
}
/*
void drawMorningLight() {
    float glow = 0.12f + 0.16f * smooth01(sceneTime / 18.0f);
    const float beam1[][2] = {
        {820.0f, 380.0f}, {900.0f, 380.0f}, {760.0f, 20.0f}, {510.0f, 20.0f}
    };
    filledPolygon(beam1, 4, Color(1.0f, 0.86f, 0.42f, glow));

    const float beam2[][2] = {
        {970.0f, 380.0f}, {1090.0f, 380.0f}, {1190.0f, 60.0f}, {920.0f, 60.0f}
    };
    filledPolygon(beam2, 4, Color(1.0f, 0.88f, 0.48f, glow * 0.2f));
}
*/
void drawRug() {
    ellipse(610.0f, 118.0f, 255.0f, 82.0f, Color(0.18f, 0.34f, 0.43f, 0.85f));
    ellipse(610.0f, 118.0f, 225.0f, 62.0f, Color(0.87f, 0.60f, 0.28f, 0.75f));
    ellipse(610.0f, 118.0f, 190.0f, 45.0f, Color(0.22f, 0.43f, 0.50f, 0.70f));
}

void drawBookshelf() {
    rectangle(25.0f, 230.0f, 165.0f, 625.0f, Color(0.29f, 0.16f, 0.08f));
    rectangle(35.0f, 242.0f, 155.0f, 612.0f, Color(0.48f, 0.29f, 0.14f));
    rectangle(25.0f, 620.0f, 165.0f, 640.0f, Color(0.23f, 0.12f, 0.06f));
    rectangle(18.0f, 225.0f, 172.0f, 242.0f, Color(0.23f, 0.12f, 0.06f));

    const float shelfY[] = {320.0f, 405.0f, 490.0f, 575.0f};
    for (float y : shelfY) {
        rectangle(30.0f, y, 160.0f, y + 12.0f, Color(0.24f, 0.13f, 0.07f));
    }

    const Color bookColors[] = {
        Color(0.76f, 0.20f, 0.18f), Color(0.12f, 0.43f, 0.63f),
        Color(0.92f, 0.62f, 0.14f), Color(0.24f, 0.56f, 0.31f),
        Color(0.48f, 0.24f, 0.58f), Color(0.87f, 0.38f, 0.19f)
    };

    for (int shelf = 0; shelf < 4; ++shelf) {
        float bottom = 246.0f + shelf * 85.0f;
        float x = 42.0f;
        for (int b = 0; b < 6; ++b) {
            float width = 12.0f + static_cast<float>((b + shelf) % 3) * 4.0f;
            float height = 48.0f + static_cast<float>((b * 7 + shelf * 5) % 18);
            rectangle(x, bottom, x + width, bottom + height,
                      bookColors[(b + shelf) % 6]);
            line(x + 3.0f, bottom + 8.0f, x + width - 3.0f, bottom + 8.0f,
                 Color(1.0f, 1.0f, 1.0f, 0.35f), 1.0f);
            x += width + 4.0f;
        }
    }

    rectangle(70.0f, 638.0f, 120.0f, 657.0f, Color(0.67f, 0.32f, 0.13f));
    const float pot[][2] = {{76.0f, 657.0f}, {114.0f, 657.0f}, {108.0f, 684.0f}, {82.0f, 684.0f}};
    filledPolygon(pot, 4, Color(0.80f, 0.42f, 0.20f));
    ellipse(86.0f, 676.0f, 10.0f, 17.0f, Color(0.18f, 0.51f, 0.25f));
    ellipse(104.0f, 678.0f, 10.0f, 18.0f, Color(0.15f, 0.45f, 0.22f));
    ellipse(96.0f, 686.0f, 9.0f, 14.0f, Color(0.23f, 0.60f, 0.29f));
}

void drawWallPicture() {
    rectangle(355.0f, 500.0f, 695.0f, 645.0f, Color(0.27f, 0.15f, 0.08f));
    rectangle(369.0f, 514.0f, 681.0f, 631.0f, Color(0.96f, 0.91f, 0.76f));
    outlineRectangle(369.0f, 514.0f, 681.0f, 631.0f,
                      Color(0.55f, 0.39f, 0.19f), 2.0f);

    rectangle(369.0f, 601.0f, 681.0f, 631.0f, Color(0.19f, 0.40f, 0.56f));
    circle(387.0f, 617.0f, 5.0f, Color(0.86f, 0.15f, 0.12f));
    circle(663.0f, 617.0f, 5.0f, Color(0.86f, 0.15f, 0.12f));
    drawText(452.0f, 610.0f, "CLASS REMINDER",
             Color(1.0f, 1.0f, 1.0f), GLUT_BITMAP_HELVETICA_18);

    drawText(397.0f, 566.0f, "8:30 e Shah jalal sir er",
             Color(0.16f, 0.18f, 0.22f), GLUT_BITMAP_HELVETICA_18);
    drawText(472.0f, 535.0f, "class ase!!",
             Color(0.75f, 0.10f, 0.08f), GLUT_BITMAP_HELVETICA_18);

    circle(650.0f, 548.0f, 17.0f, Color(0.98f, 0.98f, 0.96f));
    circle(650.0f, 548.0f, 17.0f, Color(0.25f, 0.28f, 0.32f), 48);
    circle(650.0f, 548.0f, 13.5f, Color(0.98f, 0.98f, 0.96f), 48);
    line(650.0f, 548.0f, 650.0f, 558.0f, Color(0.12f, 0.13f, 0.15f), 2.0f);
    line(650.0f, 548.0f, 658.0f, 543.0f, Color(0.12f, 0.13f, 0.15f), 2.0f);
    circle(650.0f, 548.0f, 2.3f, Color(0.75f, 0.10f, 0.08f));
}

void drawBed() {
    rectangle(112.0f, 205.0f, 145.0f, 405.0f, Color(0.30f, 0.17f, 0.09f));
    rectangle(126.0f, 220.0f, 575.0f, 246.0f, Color(0.25f, 0.13f, 0.07f));
    rectangle(120.0f, 240.0f, 585.0f, 302.0f, Color(0.88f, 0.87f, 0.82f));
    outlineRectangle(120.0f, 240.0f, 585.0f, 302.0f, Color(0.52f, 0.44f, 0.37f), 2.0f);
    rectangle(120.0f, 220.0f, 585.0f, 242.0f, Color(0.42f, 0.24f, 0.12f));

    rectangle(135.0f, 155.0f, 153.0f, 223.0f, Color(0.32f, 0.18f, 0.09f));
    rectangle(548.0f, 155.0f, 566.0f, 223.0f, Color(0.32f, 0.18f, 0.09f));

    ellipse(235.0f, 317.0f, 82.0f, 31.0f, Color(0.97f, 0.96f, 0.91f));
    outlineRectangle(170.0f, 297.0f, 300.0f, 333.0f, Color(0.78f, 0.76f, 0.71f), 1.0f);

    rectangle(295.0f, 287.0f, 570.0f, 350.0f, Color(0.17f, 0.48f, 0.62f));
    ellipse(430.0f, 345.0f, 140.0f, 28.0f, Color(0.23f, 0.59f, 0.70f));
    rectangle(295.0f, 287.0f, 570.0f, 305.0f, Color(0.10f, 0.34f, 0.48f));

    for (int x = 320; x <= 540; x += 44) {
        line(static_cast<float>(x), 298.0f, static_cast<float>(x + 25), 342.0f,
             Color(1.0f, 1.0f, 1.0f, 0.16f), 2.0f);
    }
}

void drawSleepingCelebrity(float wakeAmount) {
    float lift = 9.0f * smooth01(wakeAmount);
    float headX = 263.0f;
    float headY = 334.0f + lift;

    Color skin(0.93f, 0.68f, 0.51f);
    Color jacket(0.08f, 0.12f, 0.20f);

    ellipse(308.0f, 329.0f + lift * 0.35f, 48.0f, 26.0f, jacket);

    // Sleeping Neck
    rectangle(headX + 8.0f, headY - 12.0f, headX + 22.0f, headY - 2.0f, skin);

    circle(headX, headY, 29.0f, skin);
    ellipse(headX - 7.0f, headY + 18.0f, 29.0f, 17.0f, Color(0.12f, 0.08f, 0.05f));
    circle(headX - 27.0f, headY + 1.0f, 7.0f, skin);

    if (wakeAmount < 0.55f) {
        line(headX + 4.0f, headY + 4.0f, headX + 14.0f, headY + 4.0f,
             Color(0.18f, 0.10f, 0.08f), 2.0f);
        line(headX + 17.0f, headY - 8.0f, headX + 24.0f, headY - 5.0f,
             Color(0.55f, 0.16f, 0.14f), 1.5f);
    } else {
        circle(headX + 9.0f, headY + 5.0f, 3.0f, Color(0.10f, 0.08f, 0.07f));
        line(headX + 15.0f, headY - 8.0f, headX + 23.0f, headY - 8.0f,
             Color(0.55f, 0.16f, 0.14f), 1.5f);
    }

    line(312.0f, 338.0f, 365.0f, 322.0f, skin, 10.0f);
    circle(368.0f, 321.0f, 7.0f, skin);

    if (sceneTime < ALARM_TIME) {
        float zBob = 4.0f * std::sin(sceneTime * 2.0f);
        drawText(318.0f, 390.0f + zBob, "Z", Color(0.20f, 0.30f, 0.50f), GLUT_BITMAP_TIMES_ROMAN_24);
        drawText(345.0f, 415.0f + zBob, "z", Color(0.20f, 0.30f, 0.50f), GLUT_BITMAP_HELVETICA_18);
        drawText(367.0f, 435.0f + zBob, "z", Color(0.20f, 0.30f, 0.50f), GLUT_BITMAP_HELVETICA_12);
    }
}

void thickSegment(float x1, float y1, float x2, float y2,
                  float width, const Color& c) {
    line(x1, y1, x2, y2, c, width);
    circle(x1, y1, width * 0.45f, c, 24);
    circle(x2, y2, width * 0.45f, c, 24);
}

void drawCelebrityHead(float x, float y) {
    Color skin(0.93f, 0.68f, 0.51f);

    // Neck
    rectangle(x - 8.0f, y - 36.0f, x + 8.0f, y - 22.0f, skin);

    circle(x, y, 27.0f, skin);
    ellipse(x - 2.0f, y + 20.0f, 29.0f, 15.0f, Color(0.12f, 0.08f, 0.05f));
    circle(x - 23.0f, y + 5.0f, 7.0f, Color(0.12f, 0.08f, 0.05f));
    circle(x + 23.0f, y + 5.0f, 7.0f, skin);

    rectangle(x - 21.0f, y + 2.0f, x - 3.0f, y + 12.0f, Color(0.08f, 0.10f, 0.13f));
    rectangle(x + 3.0f, y + 2.0f, x + 21.0f, y + 12.0f, Color(0.08f, 0.10f, 0.13f));
    line(x - 3.0f, y + 8.0f, x + 3.0f, y + 8.0f, Color(0.08f, 0.10f, 0.13f), 2.0f);
    line(x - 21.0f, y + 8.0f, x - 27.0f, y + 11.0f, Color(0.08f, 0.10f, 0.13f), 2.0f);
    line(x + 21.0f, y + 8.0f, x + 27.0f, y + 11.0f, Color(0.08f, 0.10f, 0.13f), 2.0f);

    line(x - 7.0f, y - 10.0f, x + 9.0f, y - 10.0f, Color(0.55f, 0.16f, 0.14f), 1.8f);
}

void drawStandingCelebrity(float x, float baseY, float walkPhase, float reach) {
    Color skin(0.93f, 0.68f, 0.51f);
    Color jacket(0.08f, 0.12f, 0.20f);
    Color shirt(0.93f, 0.90f, 0.78f);
    Color pants(0.13f, 0.19f, 0.31f);
    Color shoe(0.06f, 0.07f, 0.08f);

    float stride = std::sin(walkPhase) * 18.0f * (1.0f - reach);
    float hipY = baseY + 59.0f;
    float shoulderY = baseY + 111.0f;

    float leftKneeX = x - 9.0f + stride * 0.38f;
    float rightKneeX = x + 9.0f - stride * 0.38f;
    float leftFootX = x - 11.0f + stride;
    float rightFootX = x + 11.0f - stride;

    thickSegment(x - 8.0f, hipY, leftKneeX, baseY + 30.0f, 13.0f, pants);
    thickSegment(leftKneeX, baseY + 30.0f, leftFootX, baseY + 4.0f, 12.0f, pants);

    thickSegment(x + 8.0f, hipY, rightKneeX, baseY + 30.0f, 13.0f, pants);
    thickSegment(rightKneeX, baseY + 30.0f, rightFootX, baseY + 4.0f, 12.0f, pants);

    ellipse(leftFootX + 5.0f, baseY + 2.0f, 15.0f, 6.0f, shoe);
    ellipse(rightFootX + 5.0f, baseY + 2.0f, 15.0f, 6.0f, shoe);

    const float torso[][2] = {
        {x - 25.0f, hipY - 2.0f}, {x - 24.0f, shoulderY},
        {x + 24.0f, shoulderY}, {x + 25.0f, hipY - 2.0f}
    };
    filledPolygon(torso, 4, jacket);

    const float innerShirt[][2] = {
        {x - 10.0f, hipY + 5.0f}, {x - 9.0f, shoulderY - 2.0f},
        {x + 9.0f, shoulderY - 2.0f}, {x + 10.0f, hipY + 5.0f}
    };
    filledPolygon(innerShirt, 4, shirt);

    drawStar(x, baseY + 88.0f, 9.0f, 4.2f, Color(1.0f, 0.70f, 0.10f));

    float armSwing = -stride * 0.75f;
    float leftShoulderX = x - 22.0f;
    float rightShoulderX = x + 22.0f;

    float leftHandX = x - 30.0f + armSwing;
    float leftHandY = baseY + 70.0f;
    float leftElbowX = (leftShoulderX + leftHandX) * 0.5f - 5.0f;
    float leftElbowY = (shoulderY + leftHandY) * 0.5f;
    thickSegment(leftShoulderX, shoulderY - 2.0f, leftElbowX, leftElbowY, 10.0f, jacket);
    thickSegment(leftElbowX, leftElbowY, leftHandX, leftHandY, 8.0f, skin);

    float normalRightHandX = x + 31.0f - armSwing;
    float normalRightHandY = baseY + 72.0f;
    float targetRightHandX = x + 48.0f;
    float targetRightHandY = baseY + 138.0f;

    float rightHandX = lerp(normalRightHandX, targetRightHandX, reach);
    float rightHandY = lerp(normalRightHandY, targetRightHandY, reach);

    float rightElbowX = lerp((rightShoulderX + normalRightHandX) * 0.5f + 5.0f,
                             rightShoulderX + 16.0f, reach);
    float rightElbowY = lerp((shoulderY + normalRightHandY) * 0.5f,
                             shoulderY + 12.0f, reach);

    thickSegment(rightShoulderX, shoulderY - 2.0f, rightElbowX, rightElbowY, 10.0f, jacket);
    thickSegment(rightElbowX, rightElbowY, rightHandX, rightHandY, 8.0f, skin);

    drawCelebrityHead(x, baseY + 145.0f);
}

void drawSeatedCelebrity(float progress) {
    Color skin(0.93f, 0.68f, 0.51f);
    Color jacket(0.08f, 0.12f, 0.20f);
    Color shirt(0.93f, 0.90f, 0.78f);
    Color pants(0.13f, 0.19f, 0.31f);
    Color shoe(0.06f, 0.07f, 0.08f);

    float bob = 4.0f * std::sin(progress * PI);
    float x = 600.0f;
    float seatY = 250.0f;

    thickSegment(x - 12.0f, seatY, x - 5.0f, 230.0f, 13.0f, pants);
    thickSegment(x - 5.0f, 230.0f, x + 3.0f, 207.0f, 12.0f, pants);

    thickSegment(x + 12.0f, seatY, x + 25.0f, 233.0f, 13.0f, pants);
    thickSegment(x + 25.0f, 233.0f, x + 35.0f, 207.0f, 12.0f, pants);

    ellipse(x + 8.0f, 205.0f, 15.0f, 6.0f, shoe);
    ellipse(x + 40.0f, 205.0f, 15.0f, 6.0f, shoe);

    const float torso[][2] = {
        {x - 23.0f, seatY}, {x - 20.0f, seatY + 73.0f + bob},
        {x + 22.0f, seatY + 70.0f + bob}, {x + 26.0f, seatY}
    };
    filledPolygon(torso, 4, jacket);

    const float innerShirt[][2] = {
        {x - 9.0f, seatY + 5.0f}, {x - 8.0f, seatY + 70.0f + bob},
        {x + 8.0f, seatY + 68.0f + bob}, {x + 9.0f, seatY + 5.0f}
    };
    filledPolygon(innerShirt, 4, shirt);

    drawStar(x + 1.0f, seatY + 42.0f + bob, 9.0f, 4.2f, Color(1.0f, 0.70f, 0.10f));

    thickSegment(x - 18.0f, seatY + 62.0f, x - 36.0f, seatY + 38.0f, 9.0f, jacket);
    thickSegment(x - 36.0f, seatY + 38.0f, x - 24.0f, seatY + 83.0f + bob, 7.0f, skin);

    thickSegment(x + 18.0f, seatY + 61.0f, x + 42.0f, seatY + 30.0f, 9.0f, jacket);
    thickSegment(x + 42.0f, seatY + 30.0f, x + 50.0f, seatY + 4.0f, 7.0f, skin);

    drawCelebrityHead(x - 2.0f, seatY + 101.0f + bob);
}

void drawCharacter() {
    constexpr float CHAR_SCALE = 1.30f;

    if (manualControl) {
        if (manualPose == ManualPose::Sleeping) {
            glPushMatrix();
            glTranslatef(300.0f, 320.0f, 0.0f);
            glScalef(CHAR_SCALE, CHAR_SCALE, 1.0f);
            glTranslatef(-300.0f, -320.0f, 0.0f);
            drawSleepingCelebrity(isAlarmActive() ? 0.35f : 0.0f);
            glPopMatrix();
        } else if (manualPose == ManualPose::Sitting) {
            glPushMatrix();
            glTranslatef(600.0f, 205.0f, 0.0f);
            glScalef(CHAR_SCALE, CHAR_SCALE, 1.0f);
            glTranslatef(-600.0f, -205.0f, 0.0f);
            drawSeatedCelebrity(1.0f);
            glPopMatrix();
        } else {
            glPushMatrix();
            glTranslatef(manualCharacterX, 205.0f, 0.0f);
            glScalef(CHAR_SCALE, CHAR_SCALE, 1.0f);
            drawStandingCelebrity(0.0f, 0.0f, manualWalkPhase, manualReach ? 1.0f : 0.0f);
            glPopMatrix();
        }
        return;
    }

    if (sceneTime < WAKE_END) {
        float wake = smooth01((sceneTime - ALARM_TIME) / (WAKE_END - ALARM_TIME));
        glPushMatrix();
        glTranslatef(300.0f, 320.0f, 0.0f);
        glScalef(CHAR_SCALE, CHAR_SCALE, 1.0f);
        glTranslatef(-300.0f, -320.0f, 0.0f);
        drawSleepingCelebrity(wake);
        glPopMatrix();
    } else if (sceneTime < SIT_END) {
        float p = smooth01((sceneTime - WAKE_END) / (SIT_END - WAKE_END));
        glPushMatrix();
        glTranslatef(600.0f, 205.0f, 0.0f);
        glScalef(CHAR_SCALE, CHAR_SCALE, 1.0f);
        glTranslatef(-600.0f, -205.0f, 0.0f);
        drawSeatedCelebrity(p);
        glPopMatrix();
    } else if (sceneTime < WALK_START) {
        glPushMatrix();
        glTranslatef(600.0f, 205.0f, 0.0f);
        glScalef(CHAR_SCALE, CHAR_SCALE, 1.0f);
        drawStandingCelebrity(0.0f, 0.0f, 0.0f, 0.0f);
        glPopMatrix();
    } else if (sceneTime < WALK_END) {
        float p = smooth01((sceneTime - WALK_START) / (WALK_END - WALK_START));
        float x = lerp(600.0f, 838.0f, p);
        glPushMatrix();
        glTranslatef(x, 205.0f, 0.0f);
        glScalef(CHAR_SCALE, CHAR_SCALE, 1.0f);
        drawStandingCelebrity(0.0f, 0.0f, sceneTime * 9.0f, 0.0f);
        glPopMatrix();
    } else {
        float reach = 0.0f;
        if (sceneTime < ALARM_OFF_TIME) {
            reach = smooth01((sceneTime - WALK_END) / (ALARM_OFF_TIME - WALK_END));
        } else if (sceneTime < ALARM_OFF_TIME + 1.4f) {
            reach = 1.0f - 0.75f * smooth01((sceneTime - ALARM_OFF_TIME) / 1.4f);
        } else {
            reach = 0.25f;
        }
        glPushMatrix();
        glTranslatef(838.0f, 205.0f, 0.0f);
        glScalef(CHAR_SCALE, CHAR_SCALE, 1.0f);
        drawStandingCelebrity(0.0f, 0.0f, 0.0f, reach);
        glPopMatrix();
    }
}

void drawTable() {
    rectangle(850.0f, 274.0f, 1095.0f, 302.0f, Color(0.30f, 0.16f, 0.08f));
    rectangle(862.0f, 225.0f, 1083.0f, 275.0f, Color(0.49f, 0.28f, 0.13f));
    outlineRectangle(862.0f, 225.0f, 1083.0f, 275.0f, Color(0.26f, 0.14f, 0.07f), 2.0f);

    circle(972.5f, 249.0f, 5.0f, Color(0.84f, 0.66f, 0.27f));

    rectangle(868.0f, 90.0f, 889.0f, 227.0f, Color(0.34f, 0.18f, 0.09f));
    rectangle(1056.0f, 90.0f, 1077.0f, 227.0f, Color(0.34f, 0.18f, 0.09f));
    ellipse(878.5f, 88.0f, 18.0f, 6.0f, Color(0.25f, 0.13f, 0.07f));
    ellipse(1066.5f, 88.0f, 18.0f, 6.0f, Color(0.25f, 0.13f, 0.07f));

    line(1040.0f, 303.0f, 1040.0f, 365.0f, Color(0.26f, 0.25f, 0.22f), 6.0f);
    ellipse(1040.0f, 303.0f, 24.0f, 7.0f, Color(0.26f, 0.25f, 0.22f));

    const float shade[][2] = {
        {1015.0f, 365.0f}, {1065.0f, 365.0f}, {1053.0f, 405.0f}, {1027.0f, 405.0f}
    };
    filledPolygon(shade, 4, Color(0.87f, 0.66f, 0.30f));
    line(1015.0f, 365.0f, 1065.0f, 365.0f, Color(0.42f, 0.28f, 0.12f), 2.0f);

    rectangle(998.0f, 305.0f, 1022.0f, 336.0f, Color(0.93f, 0.91f, 0.85f));
    ellipse(1010.0f, 336.0f, 12.0f, 4.0f, Color(0.76f, 0.73f, 0.67f));
    circle(1023.0f, 320.0f, 9.0f, Color(0.93f, 0.91f, 0.85f));
    circle(1023.0f, 320.0f, 5.0f, Color(0.49f, 0.28f, 0.13f));
}

void drawAlarmClock() {
    bool active = isAlarmActive();
    float shake = active ? 4.0f * std::sin(sceneTime * 45.0f) : 0.0f;

    float x1 = 868.0f + shake;
    float x2 = 945.0f + shake;
    float y1 = 302.0f;
    float y2 = 357.0f;

    line(x1 + 10.0f, y1, x1 + 5.0f, y1 - 8.0f, Color(0.10f, 0.12f, 0.16f), 5.0f);
    line(x2 - 10.0f, y1, x2 - 5.0f, y1 - 8.0f, Color(0.10f, 0.12f, 0.16f), 5.0f);

    rectangle(x1 + 26.0f, y2, x1 + 51.0f, y2 + 7.0f, Color(0.12f, 0.14f, 0.18f));
    rectangle(x1, y1, x2, y2, Color(0.18f, 0.21f, 0.27f));
    outlineRectangle(x1, y1, x2, y2, Color(0.04f, 0.05f, 0.07f), 3.0f);

    rectangle(x1 + 9.0f, y1 + 11.0f, x2 - 9.0f, y2 - 10.0f, Color(0.03f, 0.07f, 0.08f));

    Color digits = active ? Color(1.0f, 0.23f, 0.16f) : Color(0.37f, 1.0f, 0.58f);
    drawText(x1 + 14.0f, y1 + 25.0f, "06:30", digits, GLUT_BITMAP_HELVETICA_18);

    circle(x2 - 5.0f, y2 - 5.0f, 3.0f,
           active ? Color(1.0f, 0.12f, 0.08f) : Color(0.20f, 0.75f, 0.35f));

    if (active) {
        float pulse = 0.55f + 0.45f * std::sin(sceneTime * 18.0f);
        Color ring(0.94f, 0.16f, 0.10f, 0.55f + 0.35f * pulse);

        line(x1 - 8.0f, y1 + 10.0f, x1 - 22.0f, y1 + 2.0f, ring, 3.0f);
        line(x1 - 9.0f, y1 + 28.0f, x1 - 28.0f, y1 + 28.0f, ring, 3.0f);
        line(x1 - 8.0f, y2 - 6.0f, x1 - 22.0f, y2 + 5.0f, ring, 3.0f);

        line(x2 + 8.0f, y1 + 10.0f, x2 + 22.0f, y1 + 2.0f, ring, 3.0f);
        line(x2 + 9.0f, y1 + 28.0f, x2 + 28.0f, y1 + 28.0f, ring, 3.0f);
        line(x2 + 8.0f, y2 - 6.0f, x2 + 22.0f, y2 + 5.0f, ring, 3.0f);

        drawText(x1 - 10.0f, 388.0f, "RING! RING!", Color(0.82f, 0.08f, 0.05f), GLUT_BITMAP_HELVETICA_18);
    }
}

void drawFan() {
    line(520.0f, 700.0f, 520.0f, 650.0f, Color(0.24f, 0.25f, 0.27f), 7.0f);
    ellipse(520.0f, 695.0f, 18.0f, 7.0f, Color(0.18f, 0.19f, 0.21f));

    glPushMatrix();
    glTranslatef(520.0f, 645.0f, 0.0f);

    for (int i = 0; i < 4; ++i) {
        float angleRad = (fanAngle + i * 90.0f) * PI / 180.0f;
        float cosA = std::cos(angleRad);
        float sinA = std::sin(angleRad);

        float tipX = cosA * 110.0f;
        float tipY = sinA * 30.0f;
        float perpX = -sinA * 12.0f;
        float perpY = cosA * 4.0f;

        float blade[4][2] = {
            { perpX * 0.5f,  perpY * 0.5f},
            { tipX + perpX,  tipY + perpY},
            { tipX - perpX,  tipY - perpY},
            {-perpX * 0.5f, -perpY * 0.5f}
        };
        filledPolygon(blade, 4, Color(0.27f, 0.31f, 0.35f));
    }

    ellipse(0.0f, 0.0f, 22.0f, 10.0f, Color(0.16f, 0.18f, 0.20f));
    ellipse(0.0f, 0.0f, 10.0f, 4.5f, Color(0.68f, 0.55f, 0.25f));
    glPopMatrix();
}

void drawSmallRoomObjects() {
    const float pot[][2] = {
        {700.0f, 212.0f}, {752.0f, 212.0f}, {744.0f, 270.0f}, {708.0f, 270.0f}
    };
    filledPolygon(pot, 4, Color(0.62f, 0.29f, 0.12f));

    ellipse(716.0f, 292.0f, 16.0f, 38.0f, Color(0.20f, 0.52f, 0.24f));
    ellipse(739.0f, 304.0f, 15.0f, 42.0f, Color(0.14f, 0.44f, 0.20f));
    ellipse(730.0f, 330.0f, 13.0f, 38.0f, Color(0.24f, 0.60f, 0.29f));
    ellipse(704.0f, 325.0f, 12.0f, 34.0f, Color(0.18f, 0.49f, 0.22f));

    ellipse(430.0f, 190.0f, 25.0f, 9.0f, Color(0.30f, 0.16f, 0.10f));
    ellipse(475.0f, 184.0f, 25.0f, 9.0f, Color(0.30f, 0.16f, 0.10f));
    line(414.0f, 194.0f, 442.0f, 187.0f, Color(0.75f, 0.53f, 0.29f), 5.0f);
    line(459.0f, 188.0f, 487.0f, 181.0f, Color(0.75f, 0.53f, 0.29f), 5.0f);
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    drawRoomShell();
    drawWindowAndOutside();
    //drawMorningLight();
    drawRug();
    drawWallPicture();
    drawBookshelf();
    drawSmallRoomObjects();
    drawBed();
    drawTable();
    drawAlarmClock();
    drawCharacter();
    drawFan();

    glutSwapBuffers();
}

void resetScene() {
    sceneTime = 0.0f;
    fanAngle = 0.0f;
    manualAlarmOff = false;
    manualControl = false;
    manualPose = ManualPose::Sleeping;
    manualCharacterX = 600.0f;
    manualWalkPhase = 0.0f;
    manualReach = false;
    fanRunning = true;
    lastBeepSlot = -1;

    // Reset music loop on scene rewind
    ManageMusicState(false);
    if (!paused) {
        ManageMusicState(true);
    }
}

void timer(int) {
    int now = glutGet(GLUT_ELAPSED_TIME);
    float dt = (now - lastTickMs) / 1000.0f;
    lastTickMs = now;
    dt = std::min(dt, 0.05f);

    if (!paused) {
        // Run audio sync layer if the animation isn't paused
        ManageMusicState(true);

        sceneTime += dt;

        if (fanRunning) {
            fanAngle = std::fmod(fanAngle - 210.0f * dt, 360.0f);
        }

        if (isAlarmActive()) {
            int beepSlot = static_cast<int>((sceneTime - ALARM_TIME) * 3.0f);
            if (beepSlot != lastBeepSlot) {
                std::cout << '\a' << std::flush; // Legacy terminal alert

                // Real Audio Processing Layer
                #ifdef _WIN32
                    // Triggers an actual 800 Hz alert frequency tone for 120 milliseconds asynchronously
                    Beep(800, 120);
                #elif __APPLE__
                    // Non-blocking macOS internal bell sound trigger
                    std::system("osascript -e 'beep' &");
                #else
                    // Linux alternative command block
                    std::system("spd-say 'ring' &");
                #endif

                lastBeepSlot = beepSlot;
            }
        } else {
            lastBeepSlot = -1;
        }

        if (autoLoop && !manualControl && sceneTime >= END_TIME) {
            resetScene();
        }
    } else {
        // Instantly silences the audio playback device if execution pauses
        ManageMusicState(false);
    }

    glutPostRedisplay();
    glutTimerFunc(16, timer, 0);
}

// Added template implementation for keyboard integration to allow toggling pause
void keyboard(unsigned char key, int x, int y) {
    if (key == 'p' || key == 'P') { // Pause/Play key switch
        paused = !paused;
    }
    if (key == 'r' || key == 'R') { // Reset switch
        resetScene();
    }
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(viewportW, viewportH);
    glutCreateWindow("Celebrity Morning Scene - Audio Sync");

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, WORLD_W, 0, WORLD_H);

    glutDisplayFunc(display);
    glutTimerFunc(16, timer, 0);
    glutKeyboardFunc(keyboard);

    lastTickMs = glutGet(GLUT_ELAPSED_TIME);

    // Begin initial playback stream
    ManageMusicState(true);

    glutMainLoop();

    // Final hardware clearing safely upon context termination
    ManageMusicState(false);
    return 0;
}
