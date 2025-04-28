#pragma once
#include <windows.h>

class InputState {
public:
    static bool isKeyPressed(int vkey) {
        return (GetAsyncKeyState(vkey) & 0x8000) != 0;
    }

    static bool isMouseButtonPressed(DWORD button) {
        switch (button) {
            case VK_LBUTTON: return (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
            case VK_RBUTTON: return (GetAsyncKeyState(VK_RBUTTON) & 0x8000) != 0;
            case VK_MBUTTON: return (GetAsyncKeyState(VK_MBUTTON) & 0x8000) != 0;
            default: return false;
        }
    }

    static POINT getMouseScreenPosition() {
        POINT p;
        GetCursorPos(&p);
        return p;
    }

    static COORD getMouseConsolePosition() {
        POINT p = getMouseScreenPosition();
        HWND hwnd = GetConsoleWindow();
        ScreenToClient(hwnd, &p);
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
        return { (SHORT)(p.x / 8), (SHORT)(p.y / 16) }; // NOTE: font-size dependent
    }
};