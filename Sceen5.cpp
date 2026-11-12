/* ============================================================================
   Elevator Simulation - Immediate Door Closure & Ascending Lift (With Audio Sync)
   ============================================================================ */

#include <GL/glut.h>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <windows.h>  // Required for Windows base definitions
#include <mmsystem.h> // REQUIRED for PlaySound, SND_FILENAME, SND_ASYNC, etc.

const int   WIN_W    = 1100;
const int   WIN_H    = 880;
const int   TIMER_MS = 16; // ~60 FPS
const float WORLD_W  = 100.0f;
const float WORLD_H  = 80.0f;

enum SceneState {
    WALK_TO_LIFT,
    OPEN_DOOR_ENTRY,
    ENTER_LIFT,
    CLOSE_DOOR_ENTRY,
    TRAVELING_FLOOR_5,
    OPEN_DOOR_EXIT,
    EXIT_LIFT_STEP_OUT, // Stepping forward out of the lift
    CLOSE_DOOR_QUICK,   // Doors closing right behind student
    LIFT_GOING_UP,      // Lift ascending to higher floors
    DONE
};

SceneState currentState = WALK_TO_LIFT;

const float WALK_PATH_Y = 8.0f;

// Elevator 1 State
float doorOpenProgress    = 0.0f; // 0.0 = Closed, 1.0 = Open
int   currentFloorDisplay = 1;
float stateTimer          = 0.0f;
float floorAscentTimer    = 0.0f;

// Main Student Character
float boyX = -8.0f;
float boyY = WALK_PATH_Y;
float boyAnimPhase = 0.0f;
float studentLegPhase = 0.0f; // Added for the new character leg sync

// Audio State Tracker
bool isAudioPlaying = false;
const std::string soundPath = "D:\\8_th_Semister\\Computer_Graphics_Practice\\S5.test1_sound\\Walking.wav";

// Static interior passengers inside Lift 1
struct Passenger {
    float x, y;
    float r, g, b;
};

Passenger insidePassengers[3] = {
    {21.0f, 23.0f, 0.9f, 0.4f, 0.3f},
    {31.0f, 22.5f, 0.3f, 0.8f, 0.4f},
    {24.5f, 24.0f, 0.8f, 0.3f, 0.8f}
};

// Helpers
void setColor(float r, float g, float b, float a = 1.0f) { glColor4f(r, g, b, a); }

void drawRect(float x, float y, float w, float h) {
    glBegin(GL_QUADS);
    glVertex2f(x, y);         glVertex2f(x + w, y);
    glVertex2f(x + w, y + h); glVertex2f(x, y + h);
    glEnd();
}

// Wrapper helpers matching the new drawStudent function
void drawRectangle(float x1, float y1, float x2, float y2) {
    glBegin(GL_QUADS);
    glVertex2f(x1, y1); glVertex2f(x2, y1);
    glVertex2f(x2, y2); glVertex2f(x1, y2);
    glEnd();
}

void drawGradientRectV(float x, float y, float w, float h,
                       float r1, float g1, float b1, float r2, float g2, float b2) {
    glBegin(GL_QUADS);
    glColor3f(r1, g1, b1); glVertex2f(x, y);
    glColor3f(r1, g1, b1); glVertex2f(x + w, y);
    glColor3f(r2, g2, b2); glVertex2f(x + w, y + h);
    glColor3f(r2, g2, b2); glVertex2f(x, y + h);
    glEnd();
}

void drawCircle(float cx, float cy, float r, int seg = 20) {
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx, cy);
    for (int i = 0; i <= seg; ++i) {
        float t = 2.0f * 3.14159265f * i / seg;
        glVertex2f(cx + r * cosf(t), cy + r * sinf(t));
    }
    glEnd();
}

void drawFilledCircle(float cx, float cy, float r) {
    drawCircle(cx, cy, r, 20);
}

void drawLine(float x1, float y1, float x2, float y2, float w = 2.0f) {
    glLineWidth(w);
    glBegin(GL_LINES);
    glVertex2f(x1, y1); glVertex2f(x2, y2);
    glEnd();
}

void drawText(float x, float y, const char* s, void* font = GLUT_BITMAP_HELVETICA_12) {
    glRasterPos2f(x, y);
    for (const char* c = s; *c; ++c) glutBitmapCharacter(font, *c);
}

// ---------------------------------------------------------------------------
// Audio Synchronization Controller
// ---------------------------------------------------------------------------
void manageAudio(bool shouldPlay) {
    if (shouldPlay && !isAudioPlaying) {
        PlaySoundA(soundPath.c_str(), NULL, SND_FILENAME | SND_ASYNC | SND_LOOP);
        isAudioPlaying = true;
    } else if (!shouldPlay && isAudioPlaying) {
        PlaySoundA(NULL, NULL, 0); // Stops any playing sound instantly
        isAudioPlaying = false;
    }
}

void drawElevatorDoors(float x, float y, float w, float h, float doorProgress) {
    float halfW = w / 2.0f;
    float slideOffset = doorProgress * (halfW - 0.4f);

    setColor(0.58f, 0.61f, 0.66f, 1.0f);
    drawRect(x - slideOffset, y, halfW, h);
    drawRect(x + halfW + slideOffset, y, halfW, h);

    setColor(0.40f, 0.43f, 0.48f);
    drawRect(x + halfW - 0.4f - slideOffset, y, 0.4f, h);
    drawRect(x + halfW + slideOffset, y, 0.4f, h);
}
void drawPassenger(const Passenger& p) {
    glPushMatrix();
    glTranslatef(p.x, p.y, 0.0f);

    // Dark leg color
    setColor(0.15f, 0.15f, 0.2f);
    // Thicker legs (line width increased from 3.0f to 6.0f)
    drawLine(-0.5f, 2.0f, -0.5f, 0.0f, 6.0f);
    drawLine( 0.5f, 2.0f,  0.5f, 0.0f, 6.0f);

    // Body rectangle
    setColor(p.r, p.g, p.b);
    drawRect(-1.3f, 2.0f, 2.6f, 3.8f);

    // Face
    setColor(0.88f, 0.68f, 0.52f);
    drawCircle(0.0f, 7.0f, 1.3f, 16);

    // Hair/hat
    setColor(0.15f, 0.1f, 0.05f);
    drawCircle(0.0f, 7.6f, 1.25f, 12);

    glPopMatrix();
}

void drawElevatorInteriorAndFrame(float x, float y, float w, float h, int floorNum, const char* floorList, bool isLift1) {
    // Cabin Rear Wall
    drawGradientRectV(x, y + 2.0f, w, h - 2.0f, 0.85f, 0.87f, 0.90f, 0.72f, 0.75f, 0.78f);

    // Seams
    setColor(0.6f, 0.62f, 0.65f);
    drawLine(x + w * 0.33f, y + 2.0f, x + w * 0.33f, y + h, 1.5f);
    drawLine(x + w * 0.66f, y + 2.0f, x + w * 0.66f, y + h, 1.5f);

    // Ceiling Light
    setColor(0.98f, 0.98f, 0.85f);
    drawRect(x + 2.0f, y + h - 1.5f, w - 4.0f, 1.2f);

    // Floor
    drawGradientRectV(x, y, w, 2.0f, 0.35f, 0.35f, 0.40f, 0.25f, 0.25f, 0.30f);

    // Render passengers inside Lift 1
    if (isLift1) {
        for (int i = 0; i < 3; ++i) drawPassenger(insidePassengers[i]);
    }

    // Outer Frame
    setColor(0.25f, 0.27f, 0.3f);
    drawRect(x - 1.2f, y, 1.2f, h + 1.8f);
    drawRect(x + w, y, 1.2f, h + 1.8f);
    drawRect(x - 1.2f, y + h, w + 2.4f, 1.8f);
    drawRect(x - 1.2f, y - 0.8f, w + 2.4f, 0.8f);

    // Status Indicator
    if (currentState == LIFT_GOING_UP) {
        setColor(0.2f, 0.8f, 0.95f);
    } else if (doorOpenProgress > 0.1f) {
        setColor(0.2f, 0.9f, 0.3f);
    } else {
        setColor(0.8f, 0.2f, 0.2f);
    }

    drawCircle(x + w / 2.0f, y + h + 3.0f, 1.1f, 16);

    // Display Screen
    setColor(0.1f, 0.1f, 0.12f);
    drawRect(x - 1.0f, y + h + 5.0f, w + 2.0f, 5.5f);

    char floorTxt[28];
    if (currentState == LIFT_GOING_UP && isLift1) {
        sprintf(floorTxt, "LIFT 1: FL %d ^", floorNum);
    } else {
        sprintf(floorTxt, "LIFT %d: FLOOR %d", isLift1 ? 1 : 2, floorNum);
    }
    setColor(0.95f, 0.95f, 0.2f);
    drawText(x + 0.8f, y + h + 8.5f, floorTxt, GLUT_BITMAP_HELVETICA_12);

    setColor(0.4f, 0.9f, 0.95f);
    drawText(x + 0.8f, y + h + 6.2f, floorList, GLUT_BITMAP_HELVETICA_10);
}// ----------------------------

// Student Character (Smooth Grounded Walk Cycle)
// ----------------------------
void drawStudent(float x, float y, float scale, bool walking, bool boarding)
{
    glPushMatrix();

    // Kept perfectly stable at the simulation's targeted Y coordinate
    glTranslatef(x, y, 0.0f);
    glScalef(scale, scale, 1.0f);

    // legSwing creates a natural, restricted angular range for clean strides
    float legSwing = walking ? sinf(studentLegPhase) * 12.0f : 0.0f;
    float armSwing = walking ? sinf(studentLegPhase) * 14.0f : 0.0f;

    if (boarding)
    {
        glRotatef(-8.0f, 0.0f, 0.0f, 1.0f);
    }

    // Shadow - Solidly locked to the floor base
    setColor(0.0f, 0.0f, 0.0f, 0.25f);
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

    // Arms (Alternating smoothly in harmony with walking velocity)
    glColor3f(0.82f, 0.59f, 0.40f);
    drawLine(-22.0f, 122.0f, -32.0f - armSwing, 80.0f, 9.0f);  // Left Arm
    drawLine(22.0f, 122.0f, 32.0f + armSwing, 80.0f, 9.0f);    // Right Arm

    // Trousers
    glColor3f(0.09f, 0.14f, 0.25f);
    drawRectangle(-20.0f, 55.0f, 20.0f, 78.0f);

    // Legs (Perfectly out-of-phase: one extends forward while the other slips back)
    drawLine(-10.0f, 58.0f, -14.0f + legSwing, 8.0f, 12.0f);   // Left Leg
    drawLine(10.0f, 58.0f, 14.0f - legSwing, 8.0f, 12.0f);     // Right Leg

    // Shoes (Securely anchored to the swinging leg angles)
    glColor3f(0.04f, 0.04f, 0.05f);
    drawRectangle(-27.0f + legSwing, 2.0f, -6.0f + legSwing, 10.0f); // Left Shoe
    drawRectangle(6.0f - legSwing, 2.0f, 27.0f - legSwing, 10.0f);   // Right Shoe

    glPopMatrix();
}



// ---------------------------------------------------------------------------
// Environment & Elevator Construction
// ---------------------------------------------------------------------------
void drawLobbyBackground() {
    drawGradientRectV(0.0f, 20.0f, WORLD_W, 60.0f, 0.82f, 0.84f, 0.86f, 0.65f, 0.68f, 0.72f);
    drawGradientRectV(0.0f, 0.0f, WORLD_W, 20.0f, 0.25f, 0.28f, 0.32f, 0.45f, 0.48f, 0.52f);

    setColor(0.2f, 0.2f, 0.22f);
    drawRect(0.0f, 19.2f, WORLD_W, 1.0f);
}




// ---------------------------------------------------------------------------
// Main Animation State Updates
// ---------------------------------------------------------------------------
void updateSimulation() {
    float liftCenter1 = 27.0f;

    // Continuous linear walk updates when student departs from elevator onto walkway
    if (currentState >= EXIT_LIFT_STEP_OUT && boyX < 110.0f) {
        if (boyY <= WALK_PATH_Y) {
            boyX += 0.35f;
            studentLegPhase += 0.25f;
            manageAudio(true); // Keep playing audio while exiting lobby
        }
    }

    switch (currentState) {

        case WALK_TO_LIFT:
            manageAudio(true);
            if (boyX < liftCenter1) {
                boyX += 0.35f;
                studentLegPhase += 0.25f;
            } else {
                studentLegPhase = 0.0f;
                manageAudio(false); // Stop when waiting for doors
                currentState = OPEN_DOOR_ENTRY;
            }
            break;

        case OPEN_DOOR_ENTRY:
            manageAudio(false);
            if (doorOpenProgress < 1.0f) {
                doorOpenProgress += 0.04f;
            } else {
                doorOpenProgress = 1.0f;
                currentState = ENTER_LIFT;
            }
            break;

        case ENTER_LIFT:
            manageAudio(true); // Play audio while stepping up into the lift
            if (boyY < 21.5f) {
                boyY += 0.25f;
                studentLegPhase += 0.25f;
            } else {
                studentLegPhase = 0.0f;
                manageAudio(false); // Inside lift, stop sound
                currentState = CLOSE_DOOR_ENTRY;
            }
            break;

        case CLOSE_DOOR_ENTRY:
            manageAudio(false);
            if (doorOpenProgress > 0.0f) {
                doorOpenProgress -= 0.04f;
            } else {
                doorOpenProgress = 0.0f;
                stateTimer = 0.0f;
                currentState = TRAVELING_FLOOR_5;
            }
            break;

        case TRAVELING_FLOOR_5:
            manageAudio(false);
            stateTimer += TIMER_MS / 1000.0f;
            if (stateTimer >= 1.2f) {
                currentFloorDisplay = 5;
                currentState = OPEN_DOOR_EXIT;
            }
            break;

        case OPEN_DOOR_EXIT:
            manageAudio(false);
            if (doorOpenProgress < 1.0f) {
                doorOpenProgress += 0.04f;
            } else {
                doorOpenProgress = 1.0f;
                currentState = EXIT_LIFT_STEP_OUT;
            }
            break;

        case EXIT_LIFT_STEP_OUT:
            manageAudio(true); // Play audio as walking shifts downwards
            if (boyY > WALK_PATH_Y) {
                boyY -= 0.25f;
                studentLegPhase += 0.25f;
            } else {
                currentState = CLOSE_DOOR_QUICK;
            }
            break;

        case CLOSE_DOOR_QUICK:
            // Lift doors close immediately behind the student while he keeps walking outward
            if (doorOpenProgress > 0.0f) {
                doorOpenProgress -= 0.05f;
            } else {
                doorOpenProgress = 0.0f;
                floorAscentTimer = 0.0f;
                currentState = LIFT_GOING_UP;
            }
            break;

        case LIFT_GOING_UP:
            floorAscentTimer += TIMER_MS / 1000.0f;
            if (floorAscentTimer >= 0.7f) {
                floorAscentTimer = 0.0f;
                if (currentFloorDisplay < 11) {
                    currentFloorDisplay += 2;
                } else {
                    currentState = DONE;
                }
            }
            break;

        case DONE:
            if (boyX >= 110.0f) {
                manageAudio(false); // Stop sound entirely once character completely leaves screen
            }
            break;
    }
}

// ---------------------------------------------------------------------------
// Display Callback (Fixed Dynamic Layering)
// ---------------------------------------------------------------------------
void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // 1. Background
    drawLobbyBackground();

    // 2. Elevator Cabins & Frame Interior
    drawElevatorInteriorAndFrame(18.0f, 20.0f, 18.0f, 28.0f, currentFloorDisplay, "Floors: 5, 7, 9, 11", true);
    drawElevatorInteriorAndFrame(62.0f, 20.0f, 18.0f, 28.0f, 6, "Floors: 6, 8, 10, 12, 14", false);

    // 3. Condition-Based Door Layering to prevent Hair Clipping
    if (currentState == ENTER_LIFT || currentState == CLOSE_DOOR_ENTRY || currentState == TRAVELING_FLOOR_5) {
        // When the student is INSIDE the lift, draw the student FIRST, then doors on top
        if (boyX < 110.0f) {
            bool isMoving = (currentState == WALK_TO_LIFT || currentState == ENTER_LIFT || currentState >= EXIT_LIFT_STEP_OUT);
            bool isBoarding = (currentState == ENTER_LIFT);
            drawStudent(boyX, boyY, 0.065f, isMoving, isBoarding);
        }
        drawElevatorDoors(18.0f, 20.0f, 18.0f, 28.0f, doorOpenProgress);
        drawElevatorDoors(62.0f, 20.0f, 18.0f, 28.0f, 0.0f);
    }
    else {
        // When the student is OUTSIDE in the lobby or exiting, draw doors BEHIND him
        drawElevatorDoors(18.0f, 20.0f, 18.0f, 28.0f, doorOpenProgress);
        drawElevatorDoors(62.0f, 20.0f, 18.0f, 28.0f, 0.0f);

        if (boyX < 110.0f) {
            bool isMoving = (currentState == WALK_TO_LIFT || currentState == ENTER_LIFT || currentState >= EXIT_LIFT_STEP_OUT);
            bool isBoarding = (currentState == ENTER_LIFT);
            drawStudent(boyX, boyY, 0.065f, isMoving, isBoarding);
        }
    }

    glutSwapBuffers();
}

void timerFunc(int) {
    updateSimulation();
    glutPostRedisplay();
    glutTimerFunc(TIMER_MS, timerFunc, 0);
}

void reshape(int w, int h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, WORLD_W, 0.0, WORLD_H, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void init() {
    glClearColor(0.1f, 0.12f, 0.15f, 1.0f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowSize(WIN_W, WIN_H);
    glutCreateWindow("Elevator Lobby - Dynamic Character & Audio Sync");

    init();
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutTimerFunc(TIMER_MS, timerFunc, 0);

    glutMainLoop();
    return 0;
}
