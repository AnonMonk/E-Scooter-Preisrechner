#include "login.h"

// ---------------------- Login-Dialog Instanz ----------------------
LoginDialog loginDlg;

// ---------------------- Login-Credentials prüfen ----------------------
bool checkCredentials(const string& username, const string& password) {
    // Fest einprogrammierter Benutzer (ohne credentials.txt)
    if (username == "admin" && password == "1234") return true;

    // Optional: credentials.txt auslesen (falls vorhanden)
    ifstream file("credentials.txt");
    if (file.is_open()) {
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
    }

    return false;
}

// ---------------------- Zeichne Login-Button ----------------------
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

// ---------------------- Zeichne Textfeld ----------------------
void drawTextField(float x, float y, float w, float h, const string& text, bool active, bool isPassword, bool showCursor) {
    // Hintergrund - hellerer Grauton für bessere Lesbarkeit
    if (active) {
        drawFilledRect(x, y, w, h, 0.25f, 0.25f, 0.28f); // Heller wenn aktiv
    }
    else {
        drawFilledRect(x, y, w, h, 0.18f, 0.18f, 0.20f); // Mittelgrau
    }

    // Rahmen
    if (active) {
        drawRect(x, y, w, h, 0.4f, 0.7f, 1.0f, 2.5f); // Hellblauer Rahmen wenn aktiv
    }
    else {
        drawRect(x, y, w, h, 0.5f, 0.5f, 0.5f, 1.5f);
    }

    // Text anzeigen - helles Weiß, vertikal zentriert
    glColor3f(1.0f, 1.0f, 1.0f);
    void* font = GLUT_BITMAP_HELVETICA_18;

    string displayText = text;
    if (isPassword && !text.empty()) {
        displayText = string(text.length(), '*'); // Passwort maskieren
    }

    // Besser zentrierte vertikale Position
    glRasterPos2f(x + 10, y + h / 2 - 7);
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

// ---------------------- Zeichne Login-Dialog ----------------------
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

    // Dialog-Hintergrund - dunkler für besseren Kontrast
    drawFilledRect(dialogX, dialogY, dialogW, dialogH, 0.02f, 0.02f, 0.02f);
    drawRect(dialogX, dialogY, dialogW, dialogH, 0.5f, 0.5f, 0.5f, 3.0f);

    // Titel - helleres Weiß
    glColor3f(1.0f, 1.0f, 1.0f);
    void* fontTitle = GLUT_BITMAP_TIMES_ROMAN_24;
    string title = "Login - Verwaltung";
    int titleW = glutBitmapLength(fontTitle, (const unsigned char*)title.c_str());
    glRasterPos2f(dialogX + (dialogW - titleW) / 2, dialogY + dialogH - 40);
    for (char c : title) {
        glutBitmapCharacter(fontTitle, c);
    }

    // Untertitel - helleres Grau
    glColor3f(0.85f, 0.85f, 0.85f);
    void* fontSub = GLUT_BITMAP_HELVETICA_12;
    string subtitle = "Bitte melden Sie sich an";
    int subW = glutBitmapLength(fontSub, (const unsigned char*)subtitle.c_str());
    glRasterPos2f(dialogX + (dialogW - subW) / 2, dialogY + dialogH - 65);
    for (char c : subtitle) {
        glutBitmapCharacter(fontSub, c);
    }

    // Label: Benutzername - helleres Weiß
    glColor3f(1.0f, 1.0f, 1.0f);
    void* font = GLUT_BITMAP_HELVETICA_18;
    glRasterPos2f(dialogX + 30, dialogY + dialogH - 95);
    string label1 = "Benutzername:";
    for (char c : label1) {
        glutBitmapCharacter(font, c);
    }

    // Textfeld: Benutzername
    drawTextField(dialogX + 30, dialogY + dialogH - 135, dialogW - 60, 35,
        loginDlg.username, loginDlg.usernameActive, false, loginDlg.cursorVisible);

    // Label: Passwort - helleres Weiß
    glColor3f(1.0f, 1.0f, 1.0f);
    glRasterPos2f(dialogX + 30, dialogY + dialogH - 180);
    string label2 = "Passwort:";
    for (char c : label2) {
        glutBitmapCharacter(font, c);
    }

    // Textfeld: Passwort
    drawTextField(dialogX + 30, dialogY + dialogH - 220, dialogW - 60, 35,
        loginDlg.password, !loginDlg.usernameActive, true, loginDlg.cursorVisible);

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

// ---------------------- Keyboard Handler für Login ----------------------
void loginKeyboard(unsigned char key, int x, int y) {
    if (!loginDlg.active) return;

    string& activeField = loginDlg.usernameActive ? loginDlg.username : loginDlg.password;

    if (key == 13 || key == 10) { // Enter
        // Prüfe Login
        if (checkCredentials(loginDlg.username, loginDlg.password)) {
            loginDlg.loginSuccess = true;
            loginDlg.active = false;
            // Keine Erfolgsmeldung mehr
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

// ---------------------- Maus-Handler für Login ----------------------
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
    float field1Y = dialogY + dialogH - 135;
    float field1W = dialogW - 60;
    float field1H = 35;

    if (x >= field1X && x <= field1X + field1W &&
        glY >= field1Y && glY <= field1Y + field1H) {
        loginDlg.usernameActive = true;
        glutPostRedisplay();
        return;
    }

    // Prüfe Passwort-Feld
    float field2Y = dialogY + dialogH - 220;
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
            // Keine Erfolgsmeldung mehr
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

// ---------------------- Öffne Login-Dialog ----------------------
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