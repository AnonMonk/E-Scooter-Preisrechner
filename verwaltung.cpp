#include "verwaltung.h"
#include "mainwindow.h"

// ---------------------- Verwaltungs-Buttons ----------------------
Button exportBtn, closeBtn;

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

    if (!GetSaveFileNameW(&ofn)) return;

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
        std::replace(preis.begin(), preis.end(), '.', ',');

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

    closeBtn = { (float)(manaW - bw - 30), (float)(manaH - bh - 20), bw, bh, "Schliessen" };
    exportBtn = { (float)(manaW - bw) / 2.0f, 80.f, bw, bh, "CSV exportieren" };
}

void displayVerwalten() {
    glClear(GL_COLOR_BUFFER_BIT);
    glColor3f(1, 1, 1);

    std::string title = "Verwaltung";
    int titleW = textWidth(GLUT_BITMAP_TIMES_ROMAN_24, title);
    drawText((manaW - titleW) / 2.0f, manaH - 60.f, GLUT_BITMAP_TIMES_ROMAN_24, title);

    // Zeige Anzahl der Fahrten
    std::ostringstream subtitleStream;
    if (letzteFahrten.size() <= 10) {
        subtitleStream << "Alle Fahrten (" << letzteFahrten.size() << " Gesamt)";
    }
    else {
        subtitleStream << "Die letzten 10 Fahrten (" << letzteFahrten.size() << " Gesamt)";
    }
    std::string subtitle = subtitleStream.str();
    int subW = textWidth(GLUT_BITMAP_HELVETICA_18, subtitle);
    drawText((manaW - subW) / 2.0f, manaH - 100.f, GLUT_BITMAP_HELVETICA_18, subtitle);

    // Zeige nur die ersten 10 Fahrten (neueste zuerst)
    int startY = manaH - 140;
    size_t maxDisplay = (letzteFahrten.size() < 10) ? letzteFahrten.size() : 10;
    for (size_t i = 0; i < maxDisplay; ++i) {
        const auto& f = letzteFahrten[i];
        std::ostringstream line;
        line << i + 1 << ". "
            << f.minuten << ":" << (f.sekunden < 10 ? "0" : "") << f.sekunden
            << "  -  " << std::fixed << std::setprecision(2) << f.preis << " EUR";
        drawText(60.f, (float)(startY - (int)(i * 25)), GLUT_BITMAP_HELVETICA_12, line.str());
    }

    drawButton(closeBtn, GLUT_BITMAP_HELVETICA_12);
    drawButton(exportBtn, GLUT_BITMAP_HELVETICA_12);

    glutSwapBuffers();
}

void reshapeVerwalten(int w, int h) {
    const int MIN_WIDTH = 854;
    const int MIN_HEIGHT = 480;

    if (w < MIN_WIDTH || h < MIN_HEIGHT) {
        w = (w < MIN_WIDTH) ? MIN_WIDTH : w;
        h = (h < MIN_HEIGHT) ? MIN_HEIGHT : h;
        glutReshapeWindow(w, h);
        return;
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

void passiveVerwalten(int x, int y) {
    bool oldExportHover = exportBtn.hover;
    bool oldCloseHover = closeBtn.hover;

    exportBtn.hover = inside(exportBtn, x, y, manaH);
    closeBtn.hover = inside(closeBtn, x, y, manaH);

    if (oldExportHover != exportBtn.hover ||
        oldCloseHover != closeBtn.hover) {
        glutPostRedisplay();
    }
}