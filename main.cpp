#include <windows.h>
#include <commdlg.h> 

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

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

// ---------------------- Fenstergrößen ----------------------
int mainW = 854, mainH = 480;
int manaW = 0, manaH = 0;

// ---------------------- OpenGL Login-System ----------------------

// Login-Dialog Status
struct LoginDialog {
    bool active = false;
    bool loginSuccess = false;
    string username;
    string password;
    bool usernameActive = true; // Welches Feld ist aktiv
    bool showPassword = false;
    int cursorPos = 0;
    int cursorBlinkTimer = 0;
    bool cursorVisible = true;
};
LoginDialog loginDlg;

// Login-Credentials prüfen
bool checkCredentials(const string& username, const string& password) {
    ifstream file("credentials.txt");
    if (!file.is_open()) {
        return (username == "admin" && password == "1234");
    }

    string line;
    while (getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;

        size_t pos = line.find(':');
        if (pos != string::npos) {
            string fileUser = line.substr(0, pos);
            string filePass = line.substr(pos + 1);

            fileUser.erase(0, fileUser.find_first_not_of(" \t\r\n"));
            fileUser.erase(fileUser.find_last_not_of(" \t\r\n") + 1);
            filePass.erase(0, filePass.find_first_not_of(" \t\r\n"));
            filePass.erase(filePass.find_last_not_of(" \t\r\n") + 1);

            if (fileUser == username && filePass == password) {
                file.close();
                return true;
            }
        }
    }

    file.close();
    return false;
}

// Zeichne gefülltes Rechteck
void drawFilledRect(float x, float y, float w, float h, float r, float g, float b) {
    glColor3f(r, g, b);
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + w, y);
    glVertex2f(x + w, y + h);
    glVertex2f(x, y + h);
    glEnd();
}

// Zeichne Rechteck-Rahmen
void drawRect(float x, float y, float w, float h, float r, float g, float b, float thickness = 2.0f) {
    glColor3f(r, g, b);
    glLineWidth(thickness);
    glBegin(GL_LINE_LOOP);
    glVertex2f(x, y);
    glVertex2f(x + w, y);
    glVertex2f(x + w, y + h);
    glVertex2f(x, y + h);
    glEnd();
}

// Zeichne Login-Button
void drawLoginButton(float x, float y, float w, float h, const string& label, bool hover) {
    // Button-Hintergrund
    if (hover) {
        drawFilledRect(x, y, w, h, 0.3f, 0.5f, 0.8f); // Blau bei Hover
    }
    else {
        drawFilledRect(x, y, w, h, 0.2f, 0.4f, 0.7f); // Dunkelblau
    }

    // Rahmen
    drawRect(x, y, w, h, 1.0f, 1.0f, 1.0f, 1.5f);

    // Text
    glColor3f(1, 1, 1);
    void* font = GLUT_BITMAP_HELVETICA_18;
    int tw = glutBitmapLength(font, (const unsigned char*)label.c_str());
    float tx = x + (w - tw) / 2.0f;
    float ty = y + (h - 18) / 2.0f + 6;
    glRasterPos2f(tx, ty);
    for (char c : label) {
        glutBitmapCharacter(font, c);
    }
}

// Zeichne Textfeld
void drawTextField(float x, float y, float w, float h, const string& text, bool active, bool isPassword, bool showCursor) {
    // Hintergrund
    if (active) {
        drawFilledRect(x, y, w, h, 0.15f, 0.15f, 0.15f); // Dunkler wenn aktiv
    }
    else {
        drawFilledRect(x, y, w, h, 0.1f, 0.1f, 0.1f);
    }

    // Rahmen
    if (active) {
        drawRect(x, y, w, h, 0.3f, 0.6f, 1.0f, 2.5f); // Blauer Rahmen wenn aktiv
    }
    else {
        drawRect(x, y, w, h, 0.4f, 0.4f, 0.4f, 1.5f);
    }

    // Text anzeigen
    glColor3f(1, 1, 1);
    void* font = GLUT_BITMAP_HELVETICA_18;

    string displayText = text;
    if (isPassword && !text.empty()) {
        displayText = string(text.length(), '*'); // Passwort maskieren
    }

    glRasterPos2f(x + 10, y + h / 2 + 6);
    for (char c : displayText) {
        glutBitmapCharacter(font, c);
    }

    // Cursor zeichnen
    if (active && showCursor) {
        int tw = glutBitmapLength(font, (const unsigned char*)displayText.c_str());
        glColor3f(1, 1, 1);
        glBegin(GL_LINES);
        glVertex2f(x + 10 + tw, y + 8);
        glVertex2f(x + 10 + tw, y + h - 8);
        glEnd();
    }
}

// Zeichne Login-Dialog
void drawLoginDialog() {
    int w = mainW;
    int h = mainH;

    // Halbtransparenter Hintergrund (Verdunkelung)
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0, 0, 0, 0.7f);
    glBegin(GL_QUADS);
    glVertex2f(0, 0);
    glVertex2f(w, 0);
    glVertex2f(w, h);
    glVertex2f(0, h);
    glEnd();
    glDisable(GL_BLEND);

    // Dialog-Box
    float dialogW = 400;
    float dialogH = 320;
    float dialogX = (w - dialogW) / 2;
    float dialogY = (h - dialogH) / 2;

    // Dialog-Hintergrund
    drawFilledRect(dialogX, dialogY, dialogW, dialogH, 0.05f, 0.05f, 0.05f);
    drawRect(dialogX, dialogY, dialogW, dialogH, 0.3f, 0.3f, 0.3f, 3.0f);

    // Titel
    glColor3f(1, 1, 1);
    void* fontTitle = GLUT_BITMAP_TIMES_ROMAN_24;
    string title = "Login - Verwaltung";
    int titleW = glutBitmapLength(fontTitle, (const unsigned char*)title.c_str());
    glRasterPos2f(dialogX + (dialogW - titleW) / 2, dialogY + dialogH - 40);
    for (char c : title) {
        glutBitmapCharacter(fontTitle, c);
    }

    // Untertitel
    glColor3f(0.7f, 0.7f, 0.7f);
    void* fontSub = GLUT_BITMAP_HELVETICA_12;
    string subtitle = "Bitte melden Sie sich an";
    int subW = glutBitmapLength(fontSub, (const unsigned char*)subtitle.c_str());
    glRasterPos2f(dialogX + (dialogW - subW) / 2, dialogY + dialogH - 65);
    for (char c : subtitle) {
        glutBitmapCharacter(fontSub, c);
    }

    // Label: Benutzername
    glColor3f(0.9f, 0.9f, 0.9f);
    void* font = GLUT_BITMAP_HELVETICA_18;
    glRasterPos2f(dialogX + 30, dialogY + dialogH - 100);
    string label1 = "Benutzername:";
    for (char c : label1) {
        glutBitmapCharacter(font, c);
    }

    // Textfeld: Benutzername
    drawTextField(dialogX + 30, dialogY + dialogH - 130, dialogW - 60, 35,
        loginDlg.username, loginDlg.usernameActive, false, loginDlg.cursorVisible);

    // Label: Passwort
    glColor3f(0.9f, 0.9f, 0.9f);
    glRasterPos2f(dialogX + 30, dialogY + dialogH - 170);
    string label2 = "Passwort:";
    for (char c : label2) {
        glutBitmapCharacter(font, c);
    }

    // Textfeld: Passwort
    drawTextField(dialogX + 30, dialogY + dialogH - 200, dialogW - 60, 35,
        loginDlg.password, !loginDlg.usernameActive, true, loginDlg.cursorVisible);

    // Hinweis
    glColor3f(0.6f, 0.6f, 0.6f);
    glRasterPos2f(dialogX + 30, dialogY + dialogH - 230);
    string hint = "Standard: admin / 1234";
    for (char c : hint) {
        glutBitmapCharacter(fontSub, c);
    }

    // Buttons
    float btnW = 150;
    float btnH = 40;
    float btnY = dialogY + 30;
    float btn1X = dialogX + 40;
    float btn2X = dialogX + dialogW - 40 - btnW;

    // Button: Login
    drawLoginButton(btn1X, btnY, btnW, btnH, "Login", false);

    // Button: Abbrechen
    drawLoginButton(btn2X, btnY, btnW, btnH, "Abbrechen", false);
}

// Keyboard Handler für Login
void loginKeyboard(unsigned char key, int x, int y) {
    if (!loginDlg.active) return;

    string& activeField = loginDlg.usernameActive ? loginDlg.username : loginDlg.password;

    if (key == 13 || key == 10) { // Enter
        // Prüfe Login
        if (checkCredentials(loginDlg.username, loginDlg.password)) {
            loginDlg.loginSuccess = true;
            loginDlg.active = false;
            MessageBoxW(NULL, L"Login erfolgreich!", L"Erfolg", MB_OK | MB_ICONINFORMATION);
        }
        else {
            MessageBoxW(NULL,
                L"Falscher Benutzername oder Passwort!\n\n"
                L"Standard-Login:\nBenutzername: admin\nPasswort: 1234",
                L"Login fehlgeschlagen", MB_OK | MB_ICONERROR);
            loginDlg.password.clear();
        }
    }
    else if (key == 9) { // Tab
        loginDlg.usernameActive = !loginDlg.usernameActive;
    }
    else if (key == 27) { // Escape
        loginDlg.active = false;
        loginDlg.loginSuccess = false;
    }
    else if (key == 8 || key == 127) { // Backspace
        if (!activeField.empty()) {
            activeField.pop_back();
        }
    }
    else if (key >= 32 && key <= 126) { // Druckbare Zeichen
        activeField += key;
    }

    glutPostRedisplay();
}

// Maus-Handler für Login
void loginMouse(int button, int state, int x, int y) {
    if (!loginDlg.active) return;
    if (button != GLUT_LEFT_BUTTON || state != GLUT_DOWN) return;

    int glY = mainH - y;

    float dialogW = 400;
    float dialogH = 320;
    float dialogX = (mainW - dialogW) / 2;
    float dialogY = (mainH - dialogH) / 2;

    // Prüfe Benutzername-Feld
    float field1X = dialogX + 30;
    float field1Y = dialogY + dialogH - 130;
    float field1W = dialogW - 60;
    float field1H = 35;

    if (x >= field1X && x <= field1X + field1W &&
        glY >= field1Y && glY <= field1Y + field1H) {
        loginDlg.usernameActive = true;
        glutPostRedisplay();
        return;
    }

    // Prüfe Passwort-Feld
    float field2Y = dialogY + dialogH - 200;
    if (x >= field1X && x <= field1X + field1W &&
        glY >= field2Y && glY <= field2Y + field1H) {
        loginDlg.usernameActive = false;
        glutPostRedisplay();
        return;
    }

    // Prüfe Buttons
    float btnW = 150;
    float btnH = 40;
    float btnY = dialogY + 30;
    float btn1X = dialogX + 40;
    float btn2X = dialogX + dialogW - 40 - btnW;

    // Login-Button
    if (x >= btn1X && x <= btn1X + btnW &&
        glY >= btnY && glY <= btnY + btnH) {
        if (checkCredentials(loginDlg.username, loginDlg.password)) {
            loginDlg.loginSuccess = true;
            loginDlg.active = false;
            MessageBoxW(NULL, L"Login erfolgreich!", L"Erfolg", MB_OK | MB_ICONINFORMATION);
        }
        else {
            MessageBoxW(NULL,
                L"Falscher Benutzername oder Passwort!\n\n"
                L"Standard-Login:\nBenutzername: admin\nPasswort: 1234",
                L"Login fehlgeschlagen", MB_OK | MB_ICONERROR);
            loginDlg.password.clear();
            glutPostRedisplay();
        }
        return;
    }

    // Abbrechen-Button
    if (x >= btn2X && x <= btn2X + btnW &&
        glY >= btnY && glY <= btnY + btnH) {
        loginDlg.active = false;
        loginDlg.loginSuccess = false;
        glutPostRedisplay();
        return;
    }
}

// Öffne Login-Dialog
bool showLoginDialog() {
    loginDlg.active = true;
    loginDlg.loginSuccess = false;
    loginDlg.username.clear();
    loginDlg.password.clear();
    loginDlg.usernameActive = true;
    loginDlg.cursorBlinkTimer = 0;
    loginDlg.cursorVisible = true;

    glutPostRedisplay();

    // Warte auf Login oder Abbruch
    while (loginDlg.active) {
        glutMainLoopEvent();

        // Cursor-Blinken
        loginDlg.cursorBlinkTimer++;
        if (loginDlg.cursorBlinkTimer > 30) {
            loginDlg.cursorVisible = !loginDlg.cursorVisible;
            loginDlg.cursorBlinkTimer = 0;
            glutPostRedisplay();
        }

        Sleep(16); // ~60 FPS
    }

    glutPostRedisplay();
    return loginDlg.loginSuccess;
}

// ---------------------- Logo-Variablen ----------------------
GLuint logoTexture = 0;
int logoW = 0, logoH = 0;

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

// ---------------------- Fahrten-Daten ----------------------
struct Fahrt {
    int minuten;
    int sekunden;
    double preis;
};
vector<Fahrt> letzteFahrten;

// ---------------------- Tarif ----------------------
const double grundgebuehr = 1.00;
const double preisProMinute = 0.25;

// ---------------------- Fenstergrößen & IDs ----------------------
int mainWinId = -1;
int verwaltenWinId = -1;

// ---------------------- Buttons ----------------------
struct Button {
    float x, y, w, h;
    string label;
    bool hover = false;
};
Button startBtn, stopBtn, manageBtn;
Button exportBtn, closeBtn;

// ---------------------- Zeit-/Preis-Status ----------------------
steady_clock::time_point startTime;
bool running = false;
bool stopped = false;
double elapsedSecs = 0.0;
double finalPrice = 0.0;

// ---------------------- Text & Buttons ----------------------
void drawText(float x, float y, void* font, const string& s) {
    glRasterPos2f(x, y);
    for (unsigned char c : s) glutBitmapCharacter(font, c);
}
int textWidth(void* font, const string& s) {
    return glutBitmapLength(font, reinterpret_cast<const unsigned char*>(s.c_str()));
}
bool inside(const Button& b, int x, int y, int winH) {
    int glY = winH - y;
    return x >= b.x && x <= b.x + b.w && glY >= b.y && glY <= b.y + b.h;
}
void drawButton(const Button& b, void* font = GLUT_BITMAP_HELVETICA_18) {
    glColor3f(b.hover ? 0.35f : 0.25f, b.hover ? 0.35f : 0.25f, b.hover ? 0.35f : 0.25f);
    glBegin(GL_QUADS);
    glVertex2f(b.x, b.y);
    glVertex2f(b.x + b.w, b.y);
    glVertex2f(b.x + b.w, b.y + b.h);
    glVertex2f(b.x, b.y + b.h);
    glEnd();
    glColor3f(1, 1, 1);
    int tw = textWidth(font, b.label);
    drawText(b.x + (b.w - tw) * 0.5f, b.y + (b.h - 18) * 0.5f + 6, font, b.label);
}

// ---------------------- Layout Hauptfenster ----------------------
void relayoutMain() {
    const float btnW = 150.f, btnH = 48.f, gap = 30.f;

    // Zwei Buttons zentriert am unteren Rand
    float totalW = 2.f * btnW + gap;
    float startX = (mainW - totalW) * 0.5f;
    float y = 24.f;

    startBtn = { startX, y, btnW, btnH, "Start" };
    stopBtn = { startX + btnW + gap, y, btnW, btnH, "Stop" };

    // "Verwalten" oben rechts
    const float mW = 130.f, mH = 36.f, margin = 12.f;
    manageBtn = { static_cast<float>(mainW - margin - mW),
                  static_cast<float>(mainH - margin - mH),
                  mW, mH, "Verwalten" };
}

// ---------------------- Anzeige Hauptfenster ----------------------
void displayMain() {
    glClear(GL_COLOR_BUFFER_BIT);
    glColor3f(1, 1, 1);

    // --- Dynamisch skaliertes Logo oben links ---
    if (logoTexture) {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, logoTexture);
        glColor3f(1, 1, 1);

        // --- Position oben links ---
        float x = 10.0f;
        float y = mainH - 10.0f;

        // --- Dynamische Skalierung mit Division-by-Zero Schutz ---
        float minHeight = 480.0f;
        float maxHeight = 900.0f;

        float scale = 0.5f;
        float heightDiff = maxHeight - minHeight;

        // FIX 2: Division durch Null vermeiden
        if (heightDiff > 0.001f) {
            scale = 0.5f + 0.5f * ((mainH - minHeight) / heightDiff);
        }

        if (scale < 0.5f) scale = 0.5f;
        if (scale > 1.0f) scale = 1.0f;

        // --- Größe berechnen ---
        float w = logoW * scale;
        float h = logoH * scale;

        // --- Zeichnen ---
        glBegin(GL_QUADS);
        glTexCoord2f(0, 1); glVertex2f(x, y - h);
        glTexCoord2f(1, 1); glVertex2f(x + w, y - h);
        glTexCoord2f(1, 0); glVertex2f(x + w, y);
        glTexCoord2f(0, 0); glVertex2f(x, y);
        glEnd();

        glDisable(GL_TEXTURE_2D);
    }

    // --- Zeit / Preisanzeige ---
    if (running) {
        auto now = steady_clock::now();
        elapsedSecs = duration<double>(now - startTime).count();
        stopped = false;
    }

    int totalSec = static_cast<int>(elapsedSecs);
    int mm = totalSec / 60;
    int ss = totalSec % 60;
    double minGerundet = ceil(elapsedSecs / 60.0);
    double aktuellerPreis = grundgebuehr + minGerundet * preisProMinute;
    if (stopped) finalPrice = aktuellerPreis;

    void* fontBig = GLUT_BITMAP_TIMES_ROMAN_24;
    void* fontMed = GLUT_BITMAP_HELVETICA_18;

    ostringstream tss;
    tss << "Zeit: " << (mm < 10 ? "0" : "") << mm << ":" << (ss < 10 ? "0" : "") << ss;
    int tw = textWidth(fontBig, tss.str());
    drawText((mainW - tw) * 0.5f, mainH - 110.f, fontBig, tss.str());

    ostringstream pss;
    pss << std::fixed << std::setprecision(2)
        << (stopped ? "Endpreis: " : "Aktueller Preis: ")
        << (stopped ? finalPrice : aktuellerPreis) << " EUR";
    int pw = textWidth(fontMed, pss.str());
    drawText((mainW - pw) * 0.5f, mainH - 150.f, fontMed, pss.str());

    // --- Hinweistext unter der Preisangabe ---
    std::string hinweis1 = "Hinweis zum Abstellen des E-Scooters";
    std::string hinweis2 = "Bitte stelle den E-Scooter ordnungsgemäß und gemäß StVO ab.";
    std::string hinweis3 = "Blockiere keine Gehwege, Einfahrten, Radwege oder Rettungswege";
    std::string hinweis4 = "und stelle das Fahrzeug stabil und sicher ab.";
    std::string hinweis5 = "Du bist selbst dafür verantwortlich, den Roller rechtmäßig zu parken.";
    std::string hinweis6 = "Der Betreiber übernimmt keine Haftung für Bußgelder, Schäden oder Kosten,";
    std::string hinweis7 = "die durch falsches Abstellen entstehen.";

    // Überschrift etwas größer und rot
    glColor3f(1.0f, 0.0f, 0.0f);

    int hw1 = textWidth(GLUT_BITMAP_HELVETICA_18, hinweis1);
    drawText((mainW - hw1) * 0.5f, mainH - 200.f, GLUT_BITMAP_HELVETICA_18, hinweis1);

    glColor3f(1.0f, 1.0f, 1.0f);

    // Fließtext kleiner
    int yBase = mainH - 230.f;
    int lineSpacing = 22;

    drawText((mainW - textWidth(GLUT_BITMAP_HELVETICA_12, hinweis2)) * 0.5f, yBase - 0 * lineSpacing, GLUT_BITMAP_HELVETICA_12, hinweis2);
    drawText((mainW - textWidth(GLUT_BITMAP_HELVETICA_12, hinweis3)) * 0.5f, yBase - 1 * lineSpacing, GLUT_BITMAP_HELVETICA_12, hinweis3);
    drawText((mainW - textWidth(GLUT_BITMAP_HELVETICA_12, hinweis4)) * 0.5f, yBase - 2 * lineSpacing, GLUT_BITMAP_HELVETICA_12, hinweis4);
    drawText((mainW - textWidth(GLUT_BITMAP_HELVETICA_12, hinweis5)) * 0.5f, yBase - 3 * lineSpacing, GLUT_BITMAP_HELVETICA_12, hinweis5);
    drawText((mainW - textWidth(GLUT_BITMAP_HELVETICA_12, hinweis6)) * 0.5f, yBase - 4 * lineSpacing, GLUT_BITMAP_HELVETICA_12, hinweis6);
    drawText((mainW - textWidth(GLUT_BITMAP_HELVETICA_12, hinweis7)) * 0.5f, yBase - 5 * lineSpacing, GLUT_BITMAP_HELVETICA_12, hinweis7);


    drawButton(startBtn);
    drawButton(stopBtn);
    drawButton(manageBtn);

    // Login-Dialog zeichnen (wenn aktiv)
    if (loginDlg.active) {
        drawLoginDialog();
    }

    glutSwapBuffers();
}

// ---------------------- Vorab-Prototypen ----------------------
void mouseMain(int, int, int, int);
void passiveMain(int, int);
void reshapeMain(int, int);
void displayVerwalten();
void reshapeVerwalten(int, int);
void mouseVerwalten(int, int, int, int);
void passiveVerwalten(int, int);
void relayoutVerwalten();
void exportCSV();

// ---------------------- Maus & Hover Hauptfenster ----------------------
void mouseMain(int button, int state, int x, int y) {
    // Wenn Login-Dialog aktiv ist, leite an Login-Handler weiter
    if (loginDlg.active) {
        loginMouse(button, state, x, y);
        return;
    }

    if (button != GLUT_LEFT_BUTTON || state != GLUT_DOWN) return;

    if (inside(startBtn, x, y, mainH)) {
        startTime = steady_clock::now(); running = true; stopped = false;
    }
    else if (inside(stopBtn, x, y, mainH)) {
        // FIX 4: Nur speichern wenn eine Fahrt läuft
        if (running) {
            running = false; stopped = true;
            int totalSec = static_cast<int>(elapsedSecs);
            int mm = totalSec / 60, ss = totalSec % 60;
            double preis = grundgebuehr + ceil(elapsedSecs / 60.0) * preisProMinute;
            letzteFahrten.insert(letzteFahrten.begin(), { mm, ss, preis });
            if (letzteFahrten.size() > 10) letzteFahrten.pop_back();
        }
    }
    else if (inside(manageBtn, x, y, mainH)) {
        // LOGIN ERFORDERLICH!
        if (!showLoginDialog()) {
            // Login fehlgeschlagen
            return;
        }

        // Login erfolgreich - Öffne Verwaltung
        if (verwaltenWinId == -1) {
            verwaltenWinId = glutCreateWindow("Verwalten");
            glutReshapeWindow(mainW, mainH);
            glClearColor(0.08f, 0.08f, 0.08f, 1.0f);
            glutDisplayFunc(displayVerwalten);
            glutReshapeFunc(reshapeVerwalten);
            glutMouseFunc(mouseVerwalten);
            glutPassiveMotionFunc(passiveVerwalten);
            relayoutVerwalten();
        }
        else {
            glutSetWindow(verwaltenWinId);
        }
        glutPostRedisplay();
    }
}

// FIX 1: glutPostRedisplay() hinzugefügt für sofortigen Hover-Effekt
void passiveMain(int x, int y) {
    bool oldStartHover = startBtn.hover;
    bool oldStopHover = stopBtn.hover;
    bool oldManageHover = manageBtn.hover;

    startBtn.hover = inside(startBtn, x, y, mainH);
    stopBtn.hover = inside(stopBtn, x, y, mainH);
    manageBtn.hover = inside(manageBtn, x, y, mainH);

    // Nur neu zeichnen wenn sich Hover-Status geändert hat
    if (oldStartHover != startBtn.hover ||
        oldStopHover != stopBtn.hover ||
        oldManageHover != manageBtn.hover) {
        glutPostRedisplay();
    }
}

void reshapeMain(int w, int h) {
    // Mindestgröße: 854x480
    const int MIN_WIDTH = 854;
    const int MIN_HEIGHT = 480;

    // Wenn Fenster zu klein, auf Mindestgröße setzen
    if (w < MIN_WIDTH || h < MIN_HEIGHT) {
        w = (w < MIN_WIDTH) ? MIN_WIDTH : w;
        h = (h < MIN_HEIGHT) ? MIN_HEIGHT : h;
        glutReshapeWindow(w, h);
        return; // Beende Funktion, wird nochmal aufgerufen
    }

    mainW = w; mainH = h;
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, w, 0, h);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    relayoutMain();
}


// ---------------------- CSV Export mit Speicher-Dialog ----------------------
void exportCSV() {
    wchar_t filename[MAX_PATH] = L"fahrten.csv";

    OPENFILENAMEW ofn{};
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.lpstrFilter = L"CSV-Dateien (*.csv)\0*.csv\0Alle Dateien (*.*)\0*.*\0";
    ofn.lpstrFile = filename;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrDefExt = L"csv";
    ofn.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;

    if (!GetSaveFileNameW(&ofn)) return; // Abbruch

    std::wstring ws(ofn.lpstrFile);
    std::string  path(ws.begin(), ws.end());

    std::ofstream file(path);
    if (!file.is_open()) {
        MessageBoxW(NULL, L"Fehler beim Öffnen der Datei!", L"Fehler",
            MB_OK | MB_ICONERROR);
        return;
    }

    file << "Dauer (mm:ss);Preis (EUR)\n";
    for (const auto& f : letzteFahrten) {
        std::ostringstream ss;
        ss << std::fixed << std::setprecision(2) << f.preis;
        std::string preis = ss.str();
        std::replace(preis.begin(), preis.end(), '.', ','); // Punkt → Komma

        file << f.minuten << ":" << (f.sekunden < 10 ? "0" : "") << f.sekunden
            << ";" << preis << "\n";
    }
    file.close();

    std::wstring msg = L"Datei gespeichert unter:\n\n" + ws;
    MessageBoxW(NULL, msg.c_str(), L"Export erfolgreich",
        MB_OK | MB_ICONINFORMATION);
}

// ---------------------- Verwalten-Fenster ----------------------
void relayoutVerwalten() {
    float bw = 200.f, bh = 40.f;

    // "Schliessen" oben rechts
    closeBtn = { (float)(manaW - bw - 30), (float)(manaH - bh - 20), bw, bh, "Schliessen" };

    // "CSV exportieren" mittig unten unter der Tabelle
    exportBtn = { (float)(manaW - bw) / 2.0f, 80.f, bw, bh, "CSV exportieren" };
}

void displayVerwalten() {
    glClear(GL_COLOR_BUFFER_BIT);
    glColor3f(1, 1, 1);

    // Titel "Verwaltung" zentriert oben
    std::string title = "Verwaltung";
    int titleW = textWidth(GLUT_BITMAP_TIMES_ROMAN_24, title);
    drawText((manaW - titleW) / 2.0f, manaH - 60.f, GLUT_BITMAP_TIMES_ROMAN_24, title);

    // Untertitel
    std::string subtitle = "Die letzten 10 Fahrten";
    int subW = textWidth(GLUT_BITMAP_HELVETICA_18, subtitle);
    drawText((manaW - subW) / 2.0f, manaH - 100.f, GLUT_BITMAP_HELVETICA_18, subtitle);

    // Tabelle linksbündig
    int startY = manaH - 140;
    for (size_t i = 0; i < letzteFahrten.size(); ++i) {
        const auto& f = letzteFahrten[i];
        std::ostringstream line;
        line << i + 1 << ". "
            << f.minuten << ":" << (f.sekunden < 10 ? "0" : "") << f.sekunden
            << "  -  " << std::fixed << std::setprecision(2) << f.preis << " EUR";
        drawText(60.f, (float)(startY - (int)(i * 25)), GLUT_BITMAP_HELVETICA_12, line.str());
    }

    // Buttons
    drawButton(closeBtn, GLUT_BITMAP_HELVETICA_12); // oben rechts
    drawButton(exportBtn, GLUT_BITMAP_HELVETICA_12); // mittig unten

    glutSwapBuffers();
}

void reshapeVerwalten(int w, int h) {
    // Mindestgröße: 854x480
    const int MIN_WIDTH = 854;
    const int MIN_HEIGHT = 480;

    // Wenn Fenster zu klein, auf Mindestgröße setzen
    if (w < MIN_WIDTH || h < MIN_HEIGHT) {
        w = (w < MIN_WIDTH) ? MIN_WIDTH : w;
        h = (h < MIN_HEIGHT) ? MIN_HEIGHT : h;
        glutReshapeWindow(w, h);
        return; // Beende Funktion, wird nochmal aufgerufen
    }

    manaW = w; manaH = h;
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, w, 0, h);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    relayoutVerwalten();
}

void mouseVerwalten(int button, int state, int x, int y) {
    if (button != GLUT_LEFT_BUTTON || state != GLUT_DOWN) return;

    if (inside(exportBtn, x, y, manaH)) {
        exportCSV();
    }
    else if (inside(closeBtn, x, y, manaH)) {
        glutDestroyWindow(verwaltenWinId);
        verwaltenWinId = -1;
        if (mainWinId > 0) {
            glutSetWindow(mainWinId);
            glutPostRedisplay();
        }
    }
}

// FIX 1: glutPostRedisplay() hinzugefügt für sofortigen Hover-Effekt
void passiveVerwalten(int x, int y) {
    bool oldExportHover = exportBtn.hover;
    bool oldCloseHover = closeBtn.hover;

    exportBtn.hover = inside(exportBtn, x, y, manaH);
    closeBtn.hover = inside(closeBtn, x, y, manaH);

    // Nur neu zeichnen wenn sich Hover-Status geändert hat
    if (oldExportHover != exportBtn.hover ||
        oldCloseHover != closeBtn.hover) {
        glutPostRedisplay();
    }
}

// ---------------------- Timer ----------------------
void tick(int) {
    glutPostRedisplay();
    if (verwaltenWinId != -1) {
        glutSetWindow(verwaltenWinId);
        glutPostRedisplay();
        glutSetWindow(mainWinId);
    }
    glutTimerFunc(100, tick, 0);
}

// ---------------------- Cleanup beim Beenden ----------------------
// FIX 3: Ressourcen werden beim Beenden freigegeben
void onExit() {
    cleanupResources();
}

// ---------------------- main ----------------------
int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(mainW, mainH);
    mainWinId = glutCreateWindow("E-Scooter Fahrpreisrechner");
    glClearColor(0, 0, 0, 1);

    // FIX 3: Cleanup-Funktion registrieren
    atexit(onExit);

    // Logo laden
    logoTexture = loadTexture("logo.png", logoW, logoH);
    if (!logoTexture) {
        MessageBoxW(NULL, L"Logo konnte nicht geladen werden!", L"Fehler", MB_OK | MB_ICONERROR);
    }

    glutDisplayFunc(displayMain);
    glutReshapeFunc(reshapeMain);
    glutMouseFunc(mouseMain);
    glutPassiveMotionFunc(passiveMain);
    glutKeyboardFunc(loginKeyboard); // Keyboard-Handler für Login
    glutTimerFunc(100, tick, 0);
    relayoutMain();
    glutMainLoop();
    return 0;
}