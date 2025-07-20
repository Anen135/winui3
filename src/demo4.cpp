#define UNICODE
#include <windows.h>
#include <iostream>
#include <memory>
#include <vector>
#include "EventManager.h"
#include "InputState.h"
#include "FocusManager.h"
#include "Control.h"
#include "Render.h"
#include "../BasicElements/Container.h"

class CharacterElement : public Control, public Render {
public:
    wchar_t character;
    CharacterElement(SMALL_RECT r, wchar_t c) 
        : Control(r) { character = c; }
    bool hasbox {false};
void draw() override {
    if (hasbox) Render::DrawBox(rect);

    FillConsoleOutputAttribute(Render::hout, { FOREGROUND_GREEN | FOREGROUND_RED }, 1, { rect.Left, rect.Top }, &Render::dump);
    WriteConsoleOutputCharacterW(Render::hout, L"[", 1, { rect.Left, rect.Top }, &Render::dump);
    FillConsoleOutputAttribute(Render::hout, { FOREGROUND_RED }, 1, { short (rect.Left + 1), rect.Top }, &Render::dump);
    WriteConsoleOutputCharacterW(Render::hout, &character, 1, { short (rect.Left + 1), rect.Top }, &Render::dump);
    FillConsoleOutputAttribute(Render::hout, { FOREGROUND_GREEN | FOREGROUND_RED }, 1, { short (rect.Left + 2), rect.Top }, &Render::dump);
    WriteConsoleOutputCharacterW(Render::hout, L"]", 1, { short (rect.Left + 2), rect.Top }, &Render::dump);
}

};

// ------------------ Main ------------------

int main() {
    HANDLE hin = GetStdHandle(STD_INPUT_HANDLE);
    DWORD mode;
    GetConsoleMode(hin, &mode);
    SetConsoleMode(hin, ENABLE_EXTENDED_FLAGS | ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT);

    // Контейнер root
    Container root({ 5, 5, 45, 25 }, Container::Vertical);
    root.hasbox = true;
    root.padding = { 1, 1, 1, 1 };

    const int charWidth{3}, charHeight{2}, charsPerRow{6};

    // Сначала добавляем пустые строки в root
    for (unsigned short r{0}, chari{0}; r < 5; ++r) {
        auto elems = std::vector<std::shared_ptr<Control>> {
            std::make_shared<CharacterElement>(SMALL_RECT{ 0, 0, charWidth, charHeight }, L'A' + chari++),
            std::make_shared<CharacterElement>(SMALL_RECT{ 0, 0, charWidth, charHeight }, L'A' + chari++),
            std::make_shared<CharacterElement>(SMALL_RECT{ 0, 0, charWidth, charHeight }, L'A' + chari++),
            std::make_shared<CharacterElement>(SMALL_RECT{ 0, 0, charWidth, charHeight }, L'A' + chari++),
            std::make_shared<CharacterElement>(SMALL_RECT{ 0, 0, charWidth, charHeight }, L'A' + chari++),
            std::make_shared<CharacterElement>(SMALL_RECT{ 0, 0, charWidth, charHeight }, L'A' + chari++),
        };
        auto row = std::make_shared<Container>(SMALL_RECT{ 0, 0, charWidth * charsPerRow, charHeight }, Container::Horizontal, elems);
        row->alignment = Container::Center;
        row->padding.Top = 1;
        root.addControl(row);
        root.rearrangeControls();
        row->rearrangeControls();
    }


    // Отрисовка
    root.draw();

    InputState::setConsoleCursorPosition({ 0, 0 });
    std::wcout << L" [Press ESC to exit...] " << std::endl;

    while (!InputState::isKeyPressed(VK_ESCAPE)) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    SetConsoleMode(hin, mode);
    std::cout << "Program finished correctly" << std::endl;
    return 0;
}
