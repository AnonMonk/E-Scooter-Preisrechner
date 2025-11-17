#include "common.h"
#include "login.h"
#include "mainwindow.h"
#include "verwaltung.h"

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

    atexit(onExit);

    // Logo laden
    logoTexture = loadTexture("logo.png", logoW, logoH);
    if (!logoTexture) {
        // Nur Warnung - Programm läuft auch ohne Logo
        MessageBoxW(NULL, L"Logo konnte nicht geladen werden!", L"Warnung", MB_OK | MB_ICONWARNING);
    }

    glutDisplayFunc(displayMain);
    glutReshapeFunc(reshapeMain);
    glutMouseFunc(mouseMain);
    glutPassiveMotionFunc(passiveMain);
    glutKeyboardFunc(loginKeyboard);
    glutTimerFunc(100, tick, 0);
    relayoutMain();
    glutMainLoop();
    return 0;
}