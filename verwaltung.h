#ifndef VERWALTUNG_H
#define VERWALTUNG_H

#include "common.h"

// ---------------------- Verwaltungs-Buttons ----------------------
extern Button exportBtn, closeBtn;

// ---------------------- CSV Export ----------------------
void exportCSV();

// ---------------------- Verwaltungs-Funktionen ----------------------
void relayoutVerwalten();
void displayVerwalten();
void reshapeVerwalten(int w, int h);
void mouseVerwalten(int button, int state, int x, int y);
void passiveVerwalten(int x, int y);

#endif // VERWALTUNG_H