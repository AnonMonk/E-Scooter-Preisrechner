#include <GL/freeglut.h>
#include <chrono>
#include <string>
#include <sstream>
#include <cmath>

using namespace std;
using namespace std::chrono;

// --- State ---
steady_clock::time_point startTime;
bool running = false;
bool stopped = false;
double elapsedSeconds = 0.0;
double finalPrice = 0.0;

// --- Tarif ---
const double grundgebuehr = 1.00;
const double preisProMinute = 0.25;

// --- Fenstergröße (Pixel) ---
int winW = 854, winH = 480;

// --- Buttons in Pixel ---
struct Button {
    float x, y, w, h;      // Pixel
    string label;
    bool hover = false;
} startBtn, stopBtn, resetBtn;

// ---- Zeichnen von Bitmap-Text in Pixeln (oben links = (0,0) -> hier nutzen wir unten links = (0,0)) ---
void drawTextPx(float x, float y, void* font, const string& s) {
    glRasterPos2f(x, y);
    for (unsigned char c : s) glutBitmapCharacter(font, c);
}
int textWidthPx(void* font, const string& s) {
    return glutBitmapLength(font, reinterpret_cast<const unsigned char*>(s.c_str()));
}

// --- Layout neu berechnen (zentrieren) ---
void relayout() {
    // Buttons: 3 nebeneinander, mittig unten
    const float btnW = 150.f, btnH = 48.f, gap = 24.f;
    float totalW = 3 * btnW + 2 * gap;
    float startX = (winW - totalW) * 0.5f;
    float y = 24.f; // 24 px vom unteren Rand

    startBtn = { startX,            y, btnW, btnH, "Start" };
    stopBtn = { startX + btnW + gap, y, btnW, btnH, "Stop" };
    resetBtn = { startX + 2 * (btnW + gap), y, btnW, btnH, "Reset" };
}

// --- Anzeige ---
void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    glColor3f(1, 1, 1);

    // Zeit aktualisieren, wenn laufend
    if (running) {
        auto now = steady_clock::now();
        elapsedSeconds = duration<double>(now - startTime).count();
        stopped = false;
    }

    // mm:ss
    int totalSec = static_cast<int>(elapsedSeconds);
    int mm = totalSec / 60;
    int ss = totalSec % 60;

    // Preis pro angefangene Minute
    double minGerundet = ceil(elapsedSeconds / 60.0);
    if (minGerundet < 0) minGerundet = 0;
    double aktuellerPreis = grundgebuehr + minGerundet * preisProMinute;
    if (stopped) finalPrice = aktuellerPreis;

    // Überschriften mittig
    void* fontBig = GLUT_BITMAP_TIMES_ROMAN_24;
    void* fontMed = GLUT_BITMAP_HELVETICA_18;

    // Zeit-String
    ostringstream oss;
    oss << "Zeit: " << (mm < 10 ? "0" : "") << mm << ":" << (ss < 10 ? "0" : "") << ss << " (mm:ss)";
    string timeStr = oss.str();
    int tw = textWidthPx(fontBig, timeStr);
    drawTextPx((winW - tw) * 0.5f, winH - 100.f, fontBig, timeStr);

    // Preis-String
    ostringstream pss; pss.setf(std::ios::fixed); pss.precision(2);
    if (stopped) pss << "Endpreis: " << finalPrice << " EUR";
    else         pss << "Aktueller Preis: " << aktuellerPreis << " EUR";
    string priceStr = pss.str();
    int pw = textWidthPx(fontMed, priceStr);
    drawTextPx((winW - pw) * 0.5f, winH - 140.f, fontMed, priceStr);

    // Buttons
    auto drawButton = [](const Button& b) {
        glColor3f(b.hover ? 0.3f : 0.2f, b.hover ? 0.3f : 0.2f, b.hover ? 0.3f : 0.2f);
        glBegin(GL_QUADS);
        glVertex2f(b.x, b.y);
        glVertex2f(b.x + b.w, b.y);
        glVertex2f(b.x + b.w, b.y + b.h);
        glVertex2f(b.x, b.y + b.h);
        glEnd();

        glColor3f(1, 1, 1);
        int lw = textWidthPx(GLUT_BITMAP_HELVETICA_18, b.label);
        drawTextPx(b.x + (b.w - lw) * 0.5f, b.y + (b.h - 18) * 0.5f + 6, GLUT_BITMAP_HELVETICA_18, b.label);
        };

    drawButton(startBtn);
    drawButton(stopBtn);
    drawButton(resetBtn);

    glutSwapBuffers();
}

// --- Maus ---
bool inside(const Button& b, int x, int y) {
    return x >= b.x && x <= b.x + b.w && (winH - y) >= b.y && (winH - y) <= b.y + b.h;
}
void mouse(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        if (inside(startBtn, x, y)) {
            startTime = steady_clock::now();
            running = true;
            stopped = false;
        }
        else if (inside(stopBtn, x, y)) {
            running = false;
            stopped = true;
        }
        else if (inside(resetBtn, x, y)) {
            running = false;
            stopped = false;
            elapsedSeconds = 0.0;
            finalPrice = 0.0;
        }
    }
}
void passiveMotion(int x, int y) {
    startBtn.hover = inside(startBtn, x, y);
    stopBtn.hover = inside(stopBtn, x, y);
    resetBtn.hover = inside(resetBtn, x, y);
}

// --- Resize: Pixel-Projection + Re-Layout ---
void reshape(int w, int h) {
    winW = (w > 1 ? w : 1);
    winH = (h > 1 ? h : 1);
    glViewport(0, 0, winW, winH);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, winW, 0, winH);   // Pixel-Koordinaten
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    relayout();                     // Buttons und Layout mittig neu setzen
}

void tick(int) {
    glutPostRedisplay();
    glutTimerFunc(100, tick, 0);
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(winW, winH);
    glutCreateWindow("E-Scooter Fahrpreisrechner");

    glClearColor(0, 0, 0, 1);

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutMouseFunc(mouse);
    glutPassiveMotionFunc(passiveMotion);
    glutTimerFunc(100, tick, 0);

    relayout(); // Initial
    glutMainLoop();
    return 0;
}
