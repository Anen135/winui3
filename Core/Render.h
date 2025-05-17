#pragma once
#include <windows.h>
class Render {
public:
    inline static HANDLE hout { GetStdHandle(STD_OUTPUT_HANDLE) };
    WORD attr {FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE};
    inline static DWORD dump {0};
    static CONSOLE_SCREEN_BUFFER_INFO csbi;

    void DrawBox(SMALL_RECT& rect) {
        // Unicode Box Drawing characters
        #define hline   L"\u2500"  // ─
        #define vline   L"\u2502"  // │
        #define tl L"\u250C" // ┌
        #define tr L"\u2510" // ┐
        #define bl L"\u2514" // └
        #define br L"\u2518" // ┘
        WriteConsoleOutputCharacterW(hout, tl, 1, { rect.Left, rect.Top }, &dump);
        for (SHORT x = rect.Left + 1; x < rect.Right; x++) WriteConsoleOutputCharacterW(hout, hline, 1, { x, rect.Top }, &dump);
        WriteConsoleOutputCharacterW(hout, tr, 1, { rect.Right, rect.Top }, &dump);
        for (SHORT y = rect.Top + 1; y < rect.Bottom; y++) {
            WriteConsoleOutputCharacterW(hout, vline, 1, { rect.Left, y }, &dump);
            WriteConsoleOutputCharacterW(hout, vline, 1, { rect.Right, y }, &dump);
        }
        WriteConsoleOutputCharacterW(hout, bl, 1, { rect.Left, rect.Bottom }, &dump);
        for (SHORT x = rect.Left + 1; x < rect.Right; x++) WriteConsoleOutputCharacterW(hout, hline, 1, { x, rect.Bottom }, &dump);
        WriteConsoleOutputCharacterW(hout, br, 1, { rect.Right, rect.Bottom }, &dump);
        #undef hline
        #undef vline
        #undef tl
        #undef tr
        #undef bl
        #undef br
    }

    void fillBox(SMALL_RECT& rect) {
        for (SHORT y = rect.Top; y <= rect.Bottom; y++) {
            for (SHORT x = rect.Left; x <= rect.Right; x++) {
                FillConsoleOutputAttribute(hout, attr, 1, { x, y }, &dump);
                WriteConsoleOutputCharacterW(hout, L" ", 1, { x, y }, &dump);
            }
        }
    }

    static void clearScreen() {
        GetConsoleScreenBufferInfo(hout, &csbi);
        FillConsoleOutputAttribute(hout, csbi.wAttributes, csbi.dwSize.X * csbi.dwSize.Y, { 0, 0 }, &dump);
        FillConsoleOutputCharacter(hout, (TCHAR)' ', csbi.dwSize.X * csbi.dwSize.Y, { 0, 0 }, &dump);
        SetConsoleCursorPosition(hout, { 0, 0 });
    }

    static void updateConsoleBufferSize(COORD dsize) {
        GetConsoleScreenBufferInfo(hout, &csbi);
        SMALL_RECT sr = { 0, 0, static_cast<short>(csbi.dwSize.X + 1), static_cast<short>(csbi.dwSize.Y + 1) };
        SetConsoleScreenBufferSize(hout, dsize);
        SetConsoleWindowInfo(hout, true, &sr);
    }

  

    // Расчёт центрирования текста
    inline void drawTextCentered(const std::wstring& text, const SMALL_RECT& rect) { 
        if (static_cast<size_t>(rect.Right - rect.Left) < text.size()) WriteConsoleOutputCharacterW(hout, L"...", 3, { static_cast<SHORT>(rect.Left + (rect.Right - rect.Left + 1 - 3) / 2), static_cast<SHORT>((rect.Top + rect.Bottom) / 2) }, &dump);
        else WriteConsoleOutputCharacterW(hout, text.c_str(), text.size(), { static_cast<SHORT>(rect.Left + ((rect.Right - rect.Left + 1 - text.size()) / 2)), static_cast<SHORT>((rect.Top + rect.Bottom) / 2) }, &dump); 
    }
    inline void drawTextLeft(const std::wstring& text, const SMALL_RECT& rect) { 
        if (static_cast<size_t>(rect.Right - rect.Left) < text.size()) WriteConsoleOutputCharacterW(hout, L"...", 3, { static_cast<SHORT>(rect.Left + (rect.Right - rect.Left + 1 - 3) / 2), static_cast<SHORT>((rect.Top + rect.Bottom) / 2) }, &dump);
        else WriteConsoleOutputCharacterW(hout, text.c_str(), text.size(), { static_cast<SHORT>(rect.Left + 1), static_cast<SHORT>((rect.Top + rect.Bottom) / 2) }, &dump); 
    }
};

CONSOLE_SCREEN_BUFFER_INFO Render::csbi; 