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
