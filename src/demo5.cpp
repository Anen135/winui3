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
#define VERSION "1.5"


class CharacterElement : public Control, public Render {
public:
    wchar_t character;
    CharacterElement(SMALL_RECT r, wchar_t c) : Control(r) { character = c; }
    bool bordered {false};
    void draw() override {
        if (bordered) Render::DrawBox(rect);

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

    ScrollContainer root({ 5, 5, 45, 25 }, Container::Vertical);
    root.padding = { 1, 1, 1, 1 };

    auto l = std::make_shared<Label>(SMALL_RECT{ 0, 0, 20, 3 }, L"Scroll the container with mouse wheel", 3);
    root.addControl(l);
    root.rearrangeControls();

    auto& eventManager = EventManager::getInstance();
    eventManager.addHandler<MOUSE_EVENT_RECORD>([&root, &l](const MOUSE_EVENT_RECORD& mer) {
        if (mer.dwEventFlags == MOUSE_MOVED) return;
        if (mer.dwEventFlags == MOUSE_WHEELED) {
            SHORT delta = HIWORD(mer.dwButtonState);
            short steps = static_cast<short>(delta / WHEEL_DELTA);
            l->text = L"Scrolled " + std::to_wstring(steps) + L" steps (total offset: " + std::to_wstring(root.scrollOffset) + L")";
            l->updateText();
            root.scroll(steps);
            return;
        }

        if (mer.dwEventFlags == 3) return;
        else if (mer.dwEventFlags == 2) {
            COORD mousePos = InputState::getMouseConsolePosition();
            if (root.isHovered(mousePos)) {
                std::cout << "Mouse double-clicked at (" << mousePos.X << ", " << mousePos.Y << ")\n";
            }
        } else if (mer.dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED) {
            COORD mousePos = InputState::getMouseConsolePosition();
            if (root.isHovered(mousePos)) {
                std::cout << "Mouse clicked at (" << mousePos.X << ", " << mousePos.Y << ")\n";
            }
        }
    });

    eventManager.start();

    const int charWidth{3}, charHeight{2}, charsPerRow{6};
    for (unsigned short r{0}, chari{0}; r < 10; r++) {
        auto elems = std::vector<std::shared_ptr<Control>> {
            std::make_shared<CharacterElement>(SMALL_RECT{ 0, 0, charWidth, charHeight }, L'A' + chari++),
            std::make_shared<CharacterElement>(SMALL_RECT{ 0, 0, charWidth, charHeight }, L'A' + chari++),
            std::make_shared<CharacterElement>(SMALL_RECT{ 0, 0, charWidth, charHeight }, L'A' + chari++),
            std::make_shared<CharacterElement>(SMALL_RECT{ 0, 0, charWidth, charHeight }, L'A' + chari++),
            std::make_shared<CharacterElement>(SMALL_RECT{ 0, 0, charWidth, charHeight }, L'A' + chari++),
            std::make_shared<CharacterElement>(SMALL_RECT{ 0, 0, charWidth, charHeight }, L'A' + chari++),
        };

        auto row = std::make_shared<Container>(
            SMALL_RECT{ 0, 0, charWidth * charsPerRow, charHeight },
            Container::Horizontal,
            elems,
            Container::Center
        );

        row->padding.Top = 1;
        root.addControl(row);
        root.rearrangeControls();
        row->rearrangeControls();
    }

    root.captureBaseLayout();   // вот это важно: база фиксируется ПОСЛЕ сборки всех детей
    root.draw();

    InputState::setConsoleCursorPosition({0, 0});
    std::cout << " [Press ESC to exit...] " << std::endl;
    std::cout << " [Version " << VERSION << "] " << std::endl;

    while (!InputState::isKeyPressed(VK_ESCAPE)) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    SetConsoleMode(hin, mode);
    std::cout << "Program finished correctly." << std::endl;
    return 0;
}