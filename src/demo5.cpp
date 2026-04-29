#define UNICODE
#include <windows.h>
#include <iostream>
#include <memory>
#include "EventManager.h"
#include "InputState.h"
#include "FocusManager.h"
#include "Control.h"
#include "Render.h"
#include "../BasicElements/ScrollContainer.h"
#include "../BasicElements/Container.h"
#include "../BasicElements/Label.h"
#include "../BasicElements/CFButton.h"
#define VERSION "1.7"
#define DISPLAY_SETUP {\
    InputState::setConsoleCursorPosition({0, 0}); \
    std::cout << " [Wheel: Scroll | ESC: Exit] " << std::endl;\
    std::cout << " [Version " << VERSION << "] " << std::endl;\
}\

#define ROOT_SETUP(root) { \
    root.padding = { 1, 1, 1, 1 };\
    root.spacing = 1;\
    root.addControl(std::make_shared<Label>(SMALL_RECT{ 0, 0, 30, 1 }, L"--- WTF UI Scroll Test ---", 0));\
    for (int i = 0; i < 20; i++) root.addControl(std::make_shared<CharacterElement>( SMALL_RECT{ 0, 0, 3, 3 }, L'A' + (i % 26) ));\
    root.rearrangeControls();\
    root.draw();\
}\


class CharacterElement : public CFButton {
public:
    wchar_t character;
    CharacterElement(SMALL_RECT r, wchar_t c) : CFButton(r, L""), character(c) {
        text = std::wstring(1, character);
    }
    bool bordered {true};
    void draw() override {
        CFButton::draw();
        if (bordered) Render::DrawBox(rect);
        FillConsoleOutputAttribute(Render::hout, { FOREGROUND_GREEN | FOREGROUND_RED }, 1, { rect.Left, rect.Top }, &Render::dump);
        WriteConsoleOutputCharacterW(Render::hout, L"[", 1, { rect.Left, rect.Top }, &Render::dump);
        FillConsoleOutputAttribute(Render::hout, { FOREGROUND_RED }, 1, { short (rect.Left + 1), rect.Top }, &Render::dump);
        WriteConsoleOutputCharacterW(Render::hout, &character, 1, { short (rect.Left + 1), rect.Top }, &Render::dump);
        FillConsoleOutputAttribute(Render::hout, { FOREGROUND_GREEN | FOREGROUND_RED }, 1, { short (rect.Left + 2), rect.Top }, &Render::dump);
        WriteConsoleOutputCharacterW(Render::hout, L"]", 1, { short (rect.Left + 2), rect.Top }, &Render::dump);
    }
    void action() override {
        character = (character - L'A' + 1) % 26 + L'A';
        draw();
    }
};

// ------------------ Main ------------------
int main() {
    HANDLE hin = GetStdHandle(STD_INPUT_HANDLE);
    DWORD mode;
    GetConsoleMode(hin, &mode);
    SetConsoleMode(hin, ENABLE_EXTENDED_FLAGS | ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT);

    ScrollContainer root({ 5, 5, 45, 25 }, Container::Vertical);
    ROOT_SETUP(root)
    DISPLAY_SETUP

    EventManager::getInstance().start();
    while (!InputState::isKeyPressed(VK_ESCAPE)) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    SetConsoleMode(hin, mode);
    return 0;
}