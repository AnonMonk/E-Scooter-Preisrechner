#ifndef COMMON_H
#define COMMON_H

#include <windows.h>
#include <commdlg.h>
#include <GL/freeglut.h>
#include <chrono>
#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include <cmath>
#include <iomanip>
#include <algorithm>

using namespace std;
using namespace std::chrono;

// ---------------------- Globale Fenstergrößen ----------------------
extern int mainW, mainH;
extern int manaW, manaH;

// ---------------------- Fenster IDs ----------------------
extern int mainWinId;
extern int verwaltenWinId;

// ---------------------- Button-Struktur ----------------------
struct Button {
    float x, y, w, h;
    string label;
    bool hover = false;
};

// ---------------------- Fahrten-Daten ----------------------
struct Fahrt {
    int minuten;
    int sekunden;
    double preis;
};

extern vector<Fahrt> letzteFahrten;

// ---------------------- Tarif ----------------------
extern const double grundgebuehr;
extern const double preisProMinute;

// ---------------------- Zeit-/Preis-Status ----------------------
extern steady_clock::time_point startTime;
extern double elapsedSecs;
extern bool running;
extern bool stopped;

// ---------------------- Logo-Variablen ----------------------
extern GLuint logoTexture;
extern int logoW, logoH;

// ---------------------- Hilfsfunktionen ----------------------
void drawFilledRect(float x, float y, float w, float h, float r, float g, float b);
void drawRect(float x, float y, float w, float h, float r, float g, float b, float thickness = 2.0f);

// ---------------------- Texturlade-Funktion ----------------------
GLuint loadTexture(const char* filename, int& w, int& h);
void cleanupResources();

#endif // COMMON_H