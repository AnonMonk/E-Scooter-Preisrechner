#ifndef LOGIN_H
#define LOGIN_H

#include "common.h"

// ---------------------- Login-Dialog Status ----------------------
struct LoginDialog {
    bool active = false;
    bool loginSuccess = false;
    string username;
    string password;
    bool usernameActive = true;
    bool showPassword = false;
    int cursorPos = 0;
    int cursorBlinkTimer = 0;
    bool cursorVisible = true;
};

extern LoginDialog loginDlg;

// ---------------------- Login-Funktionen ----------------------
bool checkCredentials(const string& username, const string& password);
void drawLoginButton(float x, float y, float w, float h, const string& label, bool hover);
void drawTextField(float x, float y, float w, float h, const string& text, bool active, bool isPassword, bool showCursor);
void drawLoginDialog();
void loginKeyboard(unsigned char key, int x, int y);
void loginMouse(int button, int state, int x, int y);
bool showLoginDialog();

#endif // LOGIN_H