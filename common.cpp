#include "common.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using namespace std;
using namespace std::chrono;

// ---------------------- Globale Variablen ----------------------
int mainW = 854, mainH = 480;
int manaW = 0, manaH = 0;

int mainWinId = -1;
int verwaltenWinId = -1;

// ---------------------- Fahrten-Daten ----------------------
vector<Fahrt> letzteFahrten;

// ---------------------- Tarif ----------------------
const double grundgebuehr = 1.00;
const double preisProMinute = 0.25;

// ---------------------- Zeit-/Preis-Status ----------------------
steady_clock::time_point startTime;
double elapsedSecs = 0.0;
bool running = false;
bool stopped = false;

// ---------------------- Logo-Variablen ----------------------
GLuint logoTexture = 0;
int logoW = 0, logoH = 0;

// ---------------------- Hilfsfunktionen ----------------------
void drawFilledRect(float x, float y, float w, float h, float r, float g, float b) {
    glColor3f(r, g, b);
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + w, y);
    glVertex2f(x + w, y + h);
    glVertex2f(x, y + h);
    glEnd();
}

void drawRect(float x, float y, float w, float h, float r, float g, float b, float thickness) {
    glColor3f(r, g, b);
    glLineWidth(thickness);
    glBegin(GL_LINE_LOOP);
    glVertex2f(x, y);
    glVertex2f(x + w, y);
    glVertex2f(x + w, y + h);
    glVertex2f(x, y + h);
    glEnd();
}

// ---------------------- Texturlade-Funktion ----------------------
GLuint loadTexture(const char* filename, int& w, int& h) {
    int n;
    unsigned char* data = stbi_load(filename, &w, &h, &n, 4);
    if (!data) return 0;

    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    stbi_image_free(data);
    return tex;
}

// ---------------------- Ressourcen-Bereinigung ----------------------
void cleanupResources() {
    if (logoTexture != 0) {
        glDeleteTextures(1, &logoTexture);
        logoTexture = 0;
    }
}