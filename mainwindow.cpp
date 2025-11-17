#include "mainwindow.h"
#include "login.h"
#include "verwaltung.h"

// ---------------------- Hauptfenster-Buttons ----------------------
Button startBtn, stopBtn, manageBtn;

// ---------------------- Hilfsfunktionen ----------------------
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

void drawButton(const Button& b, void* font) {
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

    // Logo oben links
    if (logoTexture) {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, logoTexture);
        glColor3f(1, 1, 1);

        float x = 10.0f;
        float y = mainH - 10.0f;

        float minHeight = 480.0f;
        float maxHeight = 900.0f;
        float scale = 0.5f;
        float heightDiff = maxHeight - minHeight;

        if (heightDiff > 0.001f) {
            scale = 0.5f + 0.5f * ((mainH - minHeight) / heightDiff);
        }

        if (scale < 0.5f) scale = 0.5f;
        if (scale > 1.0f) scale = 1.0f;

        float w = logoW * scale;
        float h = logoH * scale;

        glBegin(GL_QUADS);
        glTexCoord2f(0, 1); glVertex2f(x, y - h);
        glTexCoord2f(1, 1); glVertex2f(x + w, y - h);
        glTexCoord2f(1, 0); glVertex2f(x + w, y);
        glTexCoord2f(0, 0); glVertex2f(x, y);
        glEnd();

        glDisable(GL_TEXTURE_2D);
    }

    // Zeit / Preisanzeige
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
    static double finalPrice = 0.0;
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

    // Hinweistext
    std::string hinweis1 = "Hinweis zum Abstellen des E-Scooters";
    std::string hinweis2 = "Bitte stelle den E-Scooter ordnungsgem?? und gem?? StVO ab.";
    std::string hinweis3 = "Blockiere keine Gehwege, Einfahrten, Radwege oder Rettungswege";
    std::string hinweis4 = "und stelle das Fahrzeug stabil und sicher ab.";
    std::string hinweis5 = "Du bist selbst daf?r verantwortlich, den Roller rechtm??ig zu parken.";
    std::string hinweis6 = "Der Betreiber ?bernimmt keine Haftung f?r Bu?gelder, Sch?den oder Kosten,";
    std::string hinweis7 = "die durch falsches Abstellen entstehen.";

    glColor3f(1.0f, 0.0f, 0.0f);
    int hw1 = textWidth(GLUT_BITMAP_HELVETICA_18, hinweis1);
    drawText((mainW - hw1) * 0.5f, mainH - 200.f, GLUT_BITMAP_HELVETICA_18, hinweis1);

    glColor3f(1.0f, 1.0f, 1.0f);
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
        if (running) {
            running = false; stopped = true;
            int totalSec = static_cast<int>(elapsedSecs);
            int mm = totalSec / 60, ss = totalSec % 60;
            double preis = grundgebuehr + ceil(elapsedSecs / 60.0) * preisProMinute;
            letzteFahrten.insert(letzteFahrten.begin(), { mm, ss, preis });
        }
    }
    else if (inside(manageBtn, x, y, mainH)) {
        // LOGIN ERFORDERLICH!
        if (!showLoginDialog()) {
            return;
        }

        // Login erfolgreich - ?ffne Verwaltung
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

void passiveMain(int x, int y) {
    bool oldStartHover = startBtn.hover;
    bool oldStopHover = stopBtn.hover;
    bool oldManageHover = manageBtn.hover;

    startBtn.hover = inside(startBtn, x, y, mainH);
    stopBtn.hover = inside(stopBtn, x, y, mainH);
    manageBtn.hover = inside(manageBtn, x, y, mainH);

    if (oldStartHover != startBtn.hover ||
        oldStopHover != stopBtn.hover ||
        oldManageHover != manageBtn.hover) {
        glutPostRedisplay();
    }
}

void reshapeMain(int w, int h) {
    const int MIN_WIDTH = 854;
    const int MIN_HEIGHT = 480;

    if (w < MIN_WIDTH || h < MIN_HEIGHT) {
        w = (w < MIN_WIDTH) ? MIN_WIDTH : w;
        h = (h < MIN_HEIGHT) ? MIN_HEIGHT : h;
        glutReshapeWindow(w, h);
        return;
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