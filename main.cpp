#include <GL/glut.h>
#include <cmath>
#include <algorithm>

// --------------------------- Canvas / Timing ---------------------------
static const int W = 1000;
static const int H = 600;

static const int TIMER_MS = 16;   // ~60 FPS
static const float DT = 0.016f;

// --------------------------- Modes ---------------------------
static bool gNight = false;

// --------------------------- Utility ---------------------------
static inline int iround(float x) { return (int)std::lround(x); }

// Plot a point (used by custom algorithms)
static void plotPoint(int x, int y)
{
    glVertex2i(x, y);
}

// --------------------------- DDA Line Algorithm ---------------------------
static void lineDDA(float x1, float y1, float x2, float y2)
{
    float dx = x2 - x1;
    float dy = y2 - y1;

    float steps = std::max(std::fabs(dx), std::fabs(dy));
    if (steps < 1.0f) steps = 1.0f;

    float xInc = dx / steps;
    float yInc = dy / steps;

    float x = x1;
    float y = y1;

    for (int i = 0; i <= (int)steps; i++)
    {
        plotPoint(iround(x), iround(y));
        x += xInc;
        y += yInc;
    }
}

// --------------------------- Bresenham Line Algorithm ---------------------------
static void lineBresenham(int x1, int y1, int x2, int y2)
{
    int dx = std::abs(x2 - x1);
    int dy = std::abs(y2 - y1);

    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;

    int err = dx - dy;

    while (true)
    {
        plotPoint(x1, y1);
        if (x1 == x2 && y1 == y2) break;

        int e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x1 += sx; }
        if (e2 <  dx) { err += dx; y1 += sy; }
    }
}

// --------------------------- Midpoint Circle Algorithm ---------------------------
static void circleMidpoint(int xc, int yc, int r)
{
    int x = 0;
    int y = r;
    int d = 1 - r;

    auto plot8 = [&](int px, int py)
    {
        plotPoint(xc + px, yc + py);
        plotPoint(xc - px, yc + py);
        plotPoint(xc + px, yc - py);
        plotPoint(xc - px, yc - py);
        plotPoint(xc + py, yc + px);
        plotPoint(xc - py, yc + px);
        plotPoint(xc + py, yc - px);
        plotPoint(xc - py, yc - px);
    };

    plot8(x, y);

    while (x < y)
    {
        x++;
        if (d < 0)
        {
            d += 2 * x + 1;
        }
        else
        {
            y--;
            d += 2 * (x - y) + 1;
        }
        plot8(x, y);
    }
}

// --------------------------- Drawing Helpers ---------------------------
static void setColor(float r, float g, float b)
{
    glColor3f(r, g, b);
}

static void rectFilled(float x, float y, float w, float h)
{
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + w, y);
    glVertex2f(x + w, y + h);
    glVertex2f(x, y + h);
    glEnd();
}

static void rectOutline(float x, float y, float w, float h)
{
    glBegin(GL_LINE_LOOP);
    glVertex2f(x, y);
    glVertex2f(x + w, y);
    glVertex2f(x + w, y + h);
    glVertex2f(x, y + h);
    glEnd();
}

// Use algorithms to draw a rectangle outline with Bresenham (pixel lines)
static void rectOutlineBresenham(int x, int y, int w, int h)
{
    glBegin(GL_POINTS);
    lineBresenham(x, y, x + w, y);
    lineBresenham(x + w, y, x + w, y + h);
    lineBresenham(x + w, y + h, x, y + h);
    lineBresenham(x, y + h, x, y);
    glEnd();
}

// Use algorithms to draw a rectangle outline with DDA (pixel lines)
static void rectOutlineDDA(int x, int y, int w, int h)
{
    glBegin(GL_POINTS);
    lineDDA((float)x, (float)y, (float)(x + w), (float)y);
    lineDDA((float)(x + w), (float)y, (float)(x + w), (float)(y + h));
    lineDDA((float)(x + w), (float)(y + h), (float)x, (float)(y + h));
    lineDDA((float)x, (float)(y + h), (float)x, (float)y);
    glEnd();
}

// --------------------------- Scene Objects ---------------------------

// Background buildings (scaled + DDA outlines)
static void drawBuildings()
{
    // Buildings base layer
    struct B { float x, y, w, h, s; };
    B bs[] = {
        {  40, 230, 120, 170, 1.0f },
        { 180, 230,  90, 140, 1.0f },
        { 290, 230, 140, 190, 1.0f },
        { 460, 230, 110, 160, 1.0f },
        { 590, 230, 160, 210, 1.0f },
        { 780, 230, 120, 175, 1.0f }
    };

    for (auto &b : bs)
    {
        glPushMatrix();
        glTranslatef(b.x, b.y, 0);
        glScalef(b.s, b.s, 1.0f); // Scaling (required)

        // Fill
        if (!gNight) setColor(0.78f, 0.80f, 0.86f);
        else         setColor(0.15f, 0.17f, 0.22f);
        rectFilled(0, 0, b.w, b.h);

        // Outline using DDA (required)
        if (!gNight) setColor(0.30f, 0.35f, 0.45f);
        else         setColor(0.65f, 0.70f, 0.80f);
        rectOutlineDDA(0, 0, (int)b.w, (int)b.h);

        // Windows
        int cols = 4, rows = 5;
        float wx = b.w / (cols + 1);
        float wy = b.h / (rows + 1);
        for (int r = 1; r <= rows; r++)
        {
            for (int c = 1; c <= cols; c++)
            {
                float px = c * wx - 10;
                float py = r * wy - 8;
                float ww = 18, wh = 14;

                if (!gNight) setColor(0.55f, 0.70f, 0.90f);
                else         setColor(0.95f, 0.85f, 0.40f); // warm lights at night
                rectFilled(px, py, ww, wh);
            }
        }

        glPopMatrix();
    }
}


// Sun / Moon using midpoint circle
static void drawSunMoon()
{
    glPointSize(2.0f);
    glBegin(GL_POINTS);
    if (!gNight)
    {
        setColor(1.0f, 0.85f, 0.20f);
        circleMidpoint(880, 520, 35);
    }
    else
    {
        setColor(0.90f, 0.90f, 0.95f);
        circleMidpoint(880, 520, 30);
        // Crescent effect (simple)
        setColor(0.10f, 0.10f, 0.15f);
        circleMidpoint(892, 528, 26);
    }
    glEnd();
}

// Cloud made from 3 circles + a base (translation used externally)
static void drawCloud()
{
    if (!gNight) setColor(1.0f, 1.0f, 1.0f);
    else         setColor(0.75f, 0.78f, 0.85f);

    rectFilled(-35, -10, 90, 22);

    glPointSize(2.0f);
    glBegin(GL_POINTS);
    circleMidpoint(-20,  2, 18);
    circleMidpoint(  5, 10, 22);
    circleMidpoint( 30,  2, 18);
    glEnd();
}


// Station + platform
static void drawStation()
{
    // Platform
    if (!gNight) setColor(0.60f, 0.60f, 0.62f);
    else         setColor(0.25f, 0.25f, 0.28f);
    rectFilled(0, 150, W, 80);

    // Platform edge line using Bresenham
    if (!gNight) setColor(0.95f, 0.90f, 0.20f);
    else         setColor(0.90f, 0.85f, 0.30f);
    glPointSize(2.0f);
    glBegin(GL_POINTS);
    lineBresenham(0, 150, W, 150);
    glEnd();

    // Station building (simple)
    if (!gNight) setColor(0.88f, 0.88f, 0.90f);
    else         setColor(0.18f, 0.18f, 0.22f);
    rectFilled(680, 230, 280, 170);

    // Outline (Bresenham)
    if (!gNight) setColor(0.25f, 0.30f, 0.40f);
    else         setColor(0.65f, 0.70f, 0.80f);
    glPointSize(2.0f);
    rectOutlineBresenham(680, 230, 280, 170);

    // Station sign
    if (!gNight) setColor(0.20f, 0.40f, 0.80f);
    else         setColor(0.30f, 0.50f, 0.90f);
    rectFilled(740, 350, 160, 40);

    if (!gNight) setColor(1.0f, 1.0f, 1.0f);
    else         setColor(1.0f, 1.0f, 1.0f);
    // Simple "METRO" letters using DDA lines (pixel style)
    glPointSize(2.0f);
    glBegin(GL_POINTS);
    // M
    lineDDA(755, 360, 755, 380);
    lineDDA(755, 380, 765, 370);
    lineDDA(765, 370, 775, 380);
    lineDDA(775, 380, 775, 360);
    // E
    lineDDA(790, 360, 790, 380);
    lineDDA(790, 380, 810, 380);
    lineDDA(790, 370, 805, 370);
    lineDDA(790, 360, 810, 360);
    // T
    lineDDA(825, 380, 845, 380);
    lineDDA(835, 380, 835, 360);
    // R
    lineDDA(860, 360, 860, 380);
    lineDDA(860, 380, 878, 380);
    lineDDA(878, 380, 878, 370);
    lineDDA(878, 370, 860, 370);
    lineDDA(860, 370, 880, 360);
    // O
    circleMidpoint(915, 370, 10);
    glEnd();
}

// Track with sleepers (Bresenham)
static void drawTrack()
{
    if (!gNight) setColor(0.25f, 0.25f, 0.25f);
    else         setColor(0.55f, 0.55f, 0.60f);

    glPointSize(2.0f);
    glBegin(GL_POINTS);
    lineBresenham(0, 120, W, 120);
    lineBresenham(0,  95, W,  95);
    glEnd();

    // Sleepers (ties)
    if (!gNight) setColor(0.45f, 0.30f, 0.20f);
    else         setColor(0.35f, 0.25f, 0.20f);

    for (int x = 0; x < W; x += 35)
        rectFilled((float)x, 92.0f, 18.0f, 32.0f);
}

// Signal light (red/green state)
static void drawSignal(bool green)
{
    // Pole
    if (!gNight) setColor(0.20f, 0.20f, 0.22f);
    else         setColor(0.65f, 0.65f, 0.70f);
    rectFilled(610, 150, 12, 140);

    // Head box
    if (!gNight) setColor(0.12f, 0.12f, 0.14f);
    else         setColor(0.20f, 0.20f, 0.24f);
    rectFilled(590, 260, 55, 85);

    // Lights (Midpoint circle)
    glPointSize(2.0f);
    glBegin(GL_POINTS);
    if (!green)
    {
        setColor(1.0f, 0.15f, 0.15f); circleMidpoint(617, 320, 12); // Red ON
        if (!gNight) setColor(0.10f, 0.35f, 0.10f);
        else         setColor(0.10f, 0.25f, 0.10f);
        circleMidpoint(617, 285, 12); // Green OFF
    }
    else
    {
        if (!gNight) setColor(0.35f, 0.10f, 0.10f);
        else         setColor(0.25f, 0.10f, 0.10f);
        circleMidpoint(617, 320, 12); // Red OFF
        setColor(0.15f, 1.0f, 0.20f); circleMidpoint(617, 285, 12); // Green ON
    }
    glEnd();
}

// --------------------------- Train + Passengers (State Machine) ---------------------------
enum TrainState
{
    TS_MOVING_TO_STATION,
    TS_ARRIVING,
    TS_STOPPED_SIGNAL_RED,
    TS_DOORS_OPENING,
    TS_PASSENGERS_BOARDING,
    TS_DOORS_CLOSING,
    TS_SIGNAL_GREEN_WAIT,
    TS_MOVING_AWAY
};

static TrainState gState = TS_MOVING_TO_STATION;

// Train positioning and animation
static float gTrainX = -520.0f;     // left start
static const float TRAIN_Y = 135.0f;

static float gTrainSpeed = 220.0f;  // px/sec
static float gWheelAngle = 0.0f;    // degrees

static float gDoorOpen = 0.0f;      // 0 closed, 1 fully open
static bool  gSignalGreen = true;

static const float STATION_STOP_X = 420.0f;     // stop target for train front alignment
static const float TRAIN_LENGTH = 520.0f;       // approximate total

// Passenger system (2 passengers)
struct Passenger
{
    bool active = true;
    float x = 0;
    float y = 0;
    float speed = 90.0f;
    float legPhase = 0.0f;   // for walking animation
};

static Passenger p1, p2;
static int gCycle = 0;

// Train door target x (in world coords)
static float trainDoorWorldX()
{
    // Door placed on 2nd coach area.
    // Door local x is around 240 from train origin (gTrainX)
    return gTrainX + 240.0f;
}

static void spawnPassengers()
{
    // Put two passengers on platform near station
    p1.active = true; p1.x = 760.0f; p1.y = 170.0f; p1.speed = 90.0f; p1.legPhase = 0.0f;
    p2.active = true; p2.x = 820.0f; p2.y = 170.0f; p2.speed = 80.0f; p2.legPhase = 1.2f;
}

// Draw passenger (simple body + head circle), walking legs by tiny rotation
static void drawPassenger(const Passenger& p, float scale = 1.0f)
{
    if (!p.active) return;

    glPushMatrix();
    glTranslatef(p.x, p.y, 0);       // Translation (required)
    glScalef(scale, scale, 1.0f);    // Scaling (required)

    // Body
    if (!gNight) setColor(0.20f, 0.35f, 0.85f);
    else         setColor(0.35f, 0.55f, 0.95f);
    rectFilled(-6, 0, 12, 26);

    // Head (midpoint circle)
    glPointSize(2.0f);
    glBegin(GL_POINTS);
    if (!gNight) setColor(1.0f, 0.85f, 0.70f);
    else         setColor(0.95f, 0.80f, 0.65f);
    circleMidpoint(0, 34, 8);
    glEnd();

    // Legs (animated)
    float a = std::sin(p.legPhase) * 22.0f; // +- degrees
    if (!gNight) setColor(0.10f, 0.10f, 0.12f);
    else         setColor(0.85f, 0.85f, 0.90f);

    // Left leg
    glPushMatrix();
    glTranslatef(-3, 0, 0);
    glRotatef(a, 0, 0, 1);
    rectFilled(-2, -14, 4, 14);
    glPopMatrix();

    // Right leg
    glPushMatrix();
    glTranslatef(3, 0, 0);
    glRotatef(-a, 0, 0, 1);
    rectFilled(-2, -14, 4, 14);
    glPopMatrix();

    glPopMatrix();
}

// Draw wheels (rotation required)
static void drawWheel(float cx, float cy, float r)
{
    // Wheel outline via midpoint circle, spokes via DDA
    glPushMatrix();
    glTranslatef(cx, cy, 0);
    glRotatef(gWheelAngle, 0, 0, 1);  // Rotation (required)

    if (!gNight) setColor(0.05f, 0.05f, 0.05f);
    else         setColor(0.90f, 0.90f, 0.95f);

    glPointSize(2.0f);
    glBegin(GL_POINTS);
    circleMidpoint(0, 0, (int)r);
    // Spokes (DDA)
    lineDDA(0, 0, r, 0);
    lineDDA(0, 0, -r, 0);
    lineDDA(0, 0, 0, r);
    lineDDA(0, 0, 0, -r);
    glEnd();

    glPopMatrix();
}

// Train drawing with multiple coaches + doors
static void drawTrain()
{
    glPushMatrix();
    glTranslatef(gTrainX, TRAIN_Y, 0); // Translation (required)

    // Coaches
    const int coaches = 3;
    const float coachW = 170.0f;
    const float coachH = 70.0f;
    const float gap = 8.0f;

    for (int i = 0; i < coaches; i++)
    {
        float ox = i * (coachW + gap);

        // Body
        if (!gNight) setColor(0.92f, 0.22f, 0.22f);
        else         setColor(0.75f, 0.18f, 0.20f);
        rectFilled(ox, 20, coachW, coachH);

        // Roof
        if (!gNight) setColor(0.80f, 0.15f, 0.15f);
        else         setColor(0.60f, 0.12f, 0.14f);
        rectFilled(ox, 85, coachW, 12);

        // Window strip
        if (!gNight) setColor(0.55f, 0.75f, 0.95f);
        else         setColor(0.95f, 0.85f, 0.40f);
        rectFilled(ox + 15, 55, coachW - 30, 22);

        // Outline using Bresenham (required)
        if (!gNight) setColor(0.20f, 0.20f, 0.22f);
        else         setColor(0.85f, 0.85f, 0.90f);
        glPointSize(2.0f);
        rectOutlineBresenham((int)ox, 20, (int)coachW, (int)coachH + 12);

        // Doors on middle coach only (i==1)
        if (i == 1)
        {
            float doorX = ox + 65;
            float doorY = 22;
            float doorW = 40;
            float doorH = 65;

            // Door frame
            if (!gNight) setColor(0.18f, 0.18f, 0.20f);
            else         setColor(0.90f, 0.90f, 0.95f);
            rectOutline(doorX, doorY, doorW, doorH);

            // Sliding doors: left + right panels move outward as gDoorOpen increases
            float slide = (doorW * 0.5f) * gDoorOpen;

            // Left panel
            if (!gNight) setColor(0.93f, 0.93f, 0.95f);
            else         setColor(0.30f, 0.30f, 0.35f);
            rectFilled(doorX, doorY, doorW * 0.5f - slide, doorH);

            // Right panel
            rectFilled(doorX + doorW * 0.5f + slide, doorY,
                       doorW * 0.5f - slide, doorH);
        }
    }

    // Front cabin (extra)
    if (!gNight) setColor(0.85f, 0.20f, 0.20f);
    else         setColor(0.65f, 0.16f, 0.18f);
    rectFilled(coaches * (coachW + gap), 30, 70, 60);

    // Cabin window
    if (!gNight) setColor(0.55f, 0.75f, 0.95f);
    else         setColor(0.95f, 0.85f, 0.40f);
    rectFilled(coaches * (coachW + gap) + 20, 60, 35, 18);

    // Wheels (under each coach)
    for (int i = 0; i < coaches; i++)
    {
        float ox = i * (coachW + gap);
        drawWheel(ox + 35, 18, 12);
        drawWheel(ox + coachW - 35, 18, 12);
    }
    // Wheels under cabin
    drawWheel(coaches * (coachW + gap) + 20, 18, 12);
    drawWheel(coaches * (coachW + gap) + 55, 18, 12);

    glPopMatrix();
}

// --------------------------- Cloud animation ---------------------------
static float c1x = 120.0f, c2x = 520.0f, c3x = 860.0f;
static float cloudSpeed = 25.0f;

// --------------------------- State Machine Update ---------------------------
static float stateTimer = 0.0f;

static void updateStateMachine(float dt)
{
    stateTimer += dt;

    // Wheel rotation increases while moving
    auto wheelAdvance = [&](float speedFactor)
    {
        gWheelAngle -= 360.0f * speedFactor * dt;  // negative for forward
        if (gWheelAngle < -360.0f) gWheelAngle += 360.0f;
    };

    switch (gState)
    {
        case TS_MOVING_TO_STATION:
        {
            gSignalGreen = true;
            gDoorOpen = 0.0f;

            gTrainX += gTrainSpeed * dt;
            wheelAdvance(1.2f);

            // When near station stop point -> arriving (slowdown)
            if (gTrainX >= STATION_STOP_X)
            {
                gTrainX = STATION_STOP_X;
                gState = TS_ARRIVING;
                stateTimer = 0.0f;
            }
        } break;

        case TS_ARRIVING:
        {
            // Small pause to feel like arrival
            gSignalGreen = true;
            if (stateTimer > 0.35f)
            {
                gState = TS_STOPPED_SIGNAL_RED;
                stateTimer = 0.0f;
            }
        } break;

        case TS_STOPPED_SIGNAL_RED:
        {
            gSignalGreen = false;
            // Wait then open doors
            if (stateTimer > 0.6f)
            {
                gState = TS_DOORS_OPENING;
                stateTimer = 0.0f;
            }
        } break;

        case TS_DOORS_OPENING:
        {
            gSignalGreen = false;
            gDoorOpen = std::min(1.0f, gDoorOpen + 1.3f * dt);

            if (gDoorOpen >= 1.0f && stateTimer > 0.2f)
            {
                gState = TS_PASSENGERS_BOARDING;
                stateTimer = 0.0f;
            }
        } break;

        case TS_PASSENGERS_BOARDING:
        {
            gSignalGreen = false;

            float doorX = trainDoorWorldX() + 65.0f; // door frame-ish center
            // Move passengers toward door; when inside => disappear
            auto movePassenger = [&](Passenger& p)
            {
                if (!p.active) return;
                float targetX = doorX;
                float dx = targetX - p.x;
                float step = p.speed * dt;

                if (std::fabs(dx) <= step)
                    p.x = targetX;
                else
                    p.x += (dx > 0 ? step : -step);

                p.legPhase += 8.0f * dt;

                // "Enter train" condition (near door + doors open)
                if (std::fabs(p.x - targetX) < 2.0f && gDoorOpen > 0.95f)
                {
                    p.active = false; // disappears after boarding (required)
                }
            };

            movePassenger(p1);
            movePassenger(p2);

            // When both boarded, close doors
            if (!p1.active && !p2.active && stateTimer > 0.4f)
            {
                gState = TS_DOORS_CLOSING;
                stateTimer = 0.0f;
            }
        } break;

        case TS_DOORS_CLOSING:
        {
            gSignalGreen = false;
            gDoorOpen = std::max(0.0f, gDoorOpen - 1.3f * dt);

            if (gDoorOpen <= 0.0f)
            {
                gState = TS_SIGNAL_GREEN_WAIT;
                stateTimer = 0.0f;
            }
        } break;

        case TS_SIGNAL_GREEN_WAIT:
        {
            // Turn signal green, then depart
            gSignalGreen = true;
            if (stateTimer > 0.5f)
            {
                gState = TS_MOVING_AWAY;
                stateTimer = 0.0f;
            }
        } break;

        case TS_MOVING_AWAY:
        {
            gSignalGreen = true;
            gTrainX += gTrainSpeed * dt;
            wheelAdvance(1.2f);

            // Once fully off screen to right, reset cycle
            if (gTrainX > (float)W + 50.0f)
            {
                gTrainX = -TRAIN_LENGTH;
                gDoorOpen = 0.0f;

                // New passengers each cycle (required)
                gCycle++;
                spawnPassengers();

                gState = TS_MOVING_TO_STATION;
                stateTimer = 0.0f;
            }
        } break;
    }
}

// --------------------------- Display ---------------------------
static void drawSky()
{
    if (!gNight) setColor(0.55f, 0.80f, 0.98f);
    else         setColor(0.08f, 0.10f, 0.16f);
    rectFilled(0, 0, W, H);

    // Ground
    if (!gNight) setColor(0.45f, 0.75f, 0.45f);
    else         setColor(0.10f, 0.18f, 0.10f);
    rectFilled(0, 0, W, 150);
}

static void display()
{
    glClear(GL_COLOR_BUFFER_BIT);

    drawSky();
    drawSunMoon();
    drawBuildings();
    drawStation();
    drawTrack();
    drawSignal(gSignalGreen);

    // Moving clouds (translation required)
    glPushMatrix();
    glTranslatef(c1x, 520.0f, 0); drawCloud();
    glPopMatrix();

    glPushMatrix();
    glTranslatef(c2x, 480.0f, 0); glScalef(1.1f, 1.1f, 1.0f); drawCloud(); // scaling
    glPopMatrix();

    glPushMatrix();
    glTranslatef(c3x, 540.0f, 0); glScalef(0.9f, 0.9f, 1.0f); drawCloud(); // scaling
    glPopMatrix();

    // Passengers
    drawPassenger(p1, 1.0f);
    drawPassenger(p2, 1.0f);

    // Train
    drawTrain();

    glutSwapBuffers();
}

// --------------------------- Timer / Animation ---------------------------
static void timer(int)
{
    // Clouds move
    c1x += cloudSpeed * DT;
    c2x += (cloudSpeed * 0.8f) * DT;
    c3x += (cloudSpeed * 1.1f) * DT;

    if (c1x > W + 60) c1x = -60;
    if (c2x > W + 60) c2x = -60;
    if (c3x > W + 60) c3x = -60;

    // State machine update
    updateStateMachine(DT);

    glutPostRedisplay();
    glutTimerFunc(TIMER_MS, timer, 0);
}

// --------------------------- Input ---------------------------
static void keyboard(unsigned char key, int, int)
{
    if (key == 27) exit(0);
    if (key == 'd' || key == 'D') gNight = false;
    if (key == 'n' || key == 'N') gNight = true;
}

// --------------------------- Init ---------------------------
static void initGL()
{
    glClearColor(0, 0, 0, 1);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, W, 0, H);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glDisable(GL_DEPTH_TEST);
    glPointSize(2.0f);

    // Start passengers for first cycle
    spawnPassengers();
    gState = TS_MOVING_TO_STATION;
    stateTimer = 0.0f;
}

// --------------------------- Main ---------------------------
int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(W, H);
    glutInitWindowPosition(80, 60);
    glutCreateWindow("Metro Rail Simulation (C++ / OpenGL GLUT) - State Machine");

    initGL();

    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutTimerFunc(TIMER_MS, timer, 0);

    glutMainLoop();
    return 0;
}
