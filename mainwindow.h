#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "common.h"

// ---------------------- Hauptfenster-Buttons ----------------------
extern Button startBtn, stopBtn, manageBtn;

// ---------------------- Hauptfenster-Funktionen ----------------------
void drawText(float x, float y, void* font, const string& s);
int textWidth(void* font, const string& s);
bool inside(const Button& b, int x, int y, int winH);
void drawButton(const Button& b, void* font = GLUT_BITMAP_HELVETICA_18);

void relayoutMain();
void displayMain();
void reshapeMain(int w, int h);
void mouseMain(int button, int state, int x, int y);
void passiveMain(int x, int y);

#endif // MAINWINDOW_H