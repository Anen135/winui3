#pragma once
#include <windows.h>
class Render {
public:
    HANDLE hout = GetStdHandle(STD_OUTPUT_HANDLE);
    WORD attr {FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE};
    DWORD written;

    void DrawBox(SMALL_RECT rect) {
        // Unicode Box Drawing characters
        #define hline   L"\u2500"  // ─
        #define vline   L"\u2502"  // │
        #define tl L"\u250C" // ┌
        #define tr L"\u2510" // ┐
        #define bl L"\u2514" // └
        #define br L"\u2518" // ┘
        WriteConsoleOutputCharacterW(hout, tl, 1, { rect.Left, rect.Top }, &written);
        for (SHORT x = rect.Left + 1; x < rect.Right; x++) WriteConsoleOutputCharacterW(hout, hline, 1, { x, rect.Top }, &written);
        WriteConsoleOutputCharacterW(hout, tr, 1, { rect.Right, rect.Top }, &written);
        for (SHORT y = rect.Top + 1; y < rect.Bottom; y++) {
            WriteConsoleOutputCharacterW(hout, vline, 1, { rect.Left, y }, &written);
            WriteConsoleOutputCharacterW(hout, vline, 1, { rect.Right, y }, &written);
        }
        WriteConsoleOutputCharacterW(hout, bl, 1, { rect.Left, rect.Bottom }, &written);
        for (SHORT x = rect.Left + 1; x < rect.Right; x++) WriteConsoleOutputCharacterW(hout, hline, 1, { x, rect.Bottom }, &written);
        WriteConsoleOutputCharacterW(hout, br, 1, { rect.Right, rect.Bottom }, &written);
        #undef hline
        #undef vline
        #undef tl
        #undef tr
        #undef bl
        #undef br
    }

    void fillBox(SMALL_RECT rect) {
        for (SHORT y = rect.Top; y <= rect.Bottom; y++) {
            for (SHORT x = rect.Left; x <= rect.Right; x++) {
                FillConsoleOutputAttribute(hout, attr, 1, { x, y }, &written);
                WriteConsoleOutputCharacterW(hout, L" ", 1, { x, y }, &written);
            }
        }
    }
    // Расчёт центрирования текста
    inline void drawTextCentered(const std::wstring& text, const SMALL_RECT& rect) { WriteConsoleOutputCharacterW(hout, text.c_str(), text.size(), { static_cast<SHORT>(rect.Left + ((rect.Right - rect.Left + 1 - text.size()) / 2)), static_cast<SHORT>((rect.Top + rect.Bottom) / 2) }, &written); }
    inline void drawTextLeft(const std::wstring& text, const SMALL_RECT& rect) { WriteConsoleOutputCharacterW(hout, text.c_str(), text.size(), { static_cast<SHORT>(rect.Left + 1), static_cast<SHORT>((rect.Top + rect.Bottom) / 2) }, &written); }
};