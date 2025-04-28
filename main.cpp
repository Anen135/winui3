#include <windows.h>
#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <functional>
#include <map>
#include <typeindex>
#include <algorithm>
#include "EventManager.h"
#include "InputState.h"

// ------------------ Base Control ------------------
class Render {
public:
    WORD attr {FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE};
    HANDLE hout = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD written;

    void DrawBox(SMALL_RECT rect) {
        DWORD written;
        COORD pos;
        WCHAR hline = L'\u2500'; // ─
        WCHAR vline = L'\u2502'; // │
        WCHAR tl = L'\u250C';    // ┌
        WCHAR tr = L'\u2510';    // ┐
        WCHAR bl = L'\u2514';    // └
        WCHAR br = L'\u2518';    // ┘

        pos = { rect.Left, rect.Top };
        WriteConsoleOutputCharacterW(hout, &tl, 1, pos, &written);
        for (SHORT x = rect.Left + 1; x < rect.Right; x++) {
            pos = { x, rect.Top };
            WriteConsoleOutputCharacterW(hout, &hline, 1, pos, &written);
        }
        pos = { rect.Right, rect.Top };
        WriteConsoleOutputCharacterW(hout, &tr, 1, pos, &written);

        for (SHORT y = rect.Top + 1; y < rect.Bottom; y++) {
            pos = { rect.Left, y };
            WriteConsoleOutputCharacterW(hout, &vline, 1, pos, &written);
            pos = { rect.Right, y };
            WriteConsoleOutputCharacterW(hout, &vline, 1, pos, &written);
        }

        pos = { rect.Left, rect.Bottom };
        WriteConsoleOutputCharacterW(hout, &bl, 1, pos, &written);
        for (SHORT x = rect.Left + 1; x < rect.Right; x++) {
            pos = { x, rect.Bottom };
            WriteConsoleOutputCharacterW(hout, &hline, 1, pos, &written);
        }
        pos = { rect.Right, rect.Bottom };
        WriteConsoleOutputCharacterW(hout, &br, 1, pos, &written);
    }
};

class Control {
protected:
    SMALL_RECT rect;
    std::wstring text;
    bool focused = false;
    bool hovered = false;

public:
    Control(SMALL_RECT r, const std::wstring& t) : rect(r), text(t) {}

    virtual void draw() = 0;
    virtual void onMouse(const MOUSE_EVENT_RECORD& mer);
    virtual void onKey(const KEY_EVENT_RECORD& ker) {}

    bool isHovered(const COORD& pos);
    void setFocus(bool f);
    bool hasFocus() const { return focused; }
};

bool Control::isHovered(const COORD& pos) {
    return pos.X >= rect.Left && pos.X <= rect.Right &&
           pos.Y >= rect.Top && pos.Y <= rect.Bottom;
}

void Control::onMouse(const MOUSE_EVENT_RECORD& mer) {
    bool wasHovered = hovered;
    hovered = isHovered(mer.dwMousePosition);
    if (hovered != wasHovered) {
        draw();
    }
}

void Control::setFocus(bool f) {
    focused = f;
    draw();
}

// ------------------ Button ------------------
class Button : public Control, public Render {
public:
    Button(SMALL_RECT r, const std::wstring& t) : Control(r, t) {
        EventManager::getInstance().addHandler<MOUSE_EVENT_RECORD>([this](const MOUSE_EVENT_RECORD& mer) {
            this->onMouse(mer);
        });
    }
    void draw() override {
        Render::attr = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
        if (hovered)
            attr = BACKGROUND_BLUE | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
        if (focused)
            attr = BACKGROUND_GREEN | FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY;

        for (SHORT y = rect.Top; y <= rect.Bottom; y++) {
            for (SHORT x = rect.Left; x <= rect.Right; x++) {
                FillConsoleOutputAttribute(hout, attr, 1, { x, y }, &written);
                WriteConsoleOutputCharacterW(hout, L" ", 1, { x, y }, &written);
            }
        }
        DrawBox(rect);
        int textPos = (rect.Right - rect.Left + 1 - text.size()) / 2;
        WriteConsoleOutputCharacterW(hout, text.c_str(), text.size(), { (SHORT)(rect.Left + textPos), (SHORT)((rect.Top + rect.Bottom) / 2) }, &written);
    }

    void onMouse(const MOUSE_EVENT_RECORD& mer) override {
        Control::onMouse(mer);
        if ((mer.dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED)) {
            if (hovered) {
                setFocus(true);
                MessageBoxW(NULL, L"Button clicked!", L"Info", MB_OK);
            } else {
                setFocus(false);
            }
        }
    }
};

// ------------------ TextBox ------------------
class TextBox : public Control, public Render {
std::shared_ptr<std::function<void(const KEY_EVENT_RECORD&)>> handler;
public:
    TextBox(SMALL_RECT r, const std::wstring& t) : Control(r, t) {
        EventManager::getInstance().addHandler<MOUSE_EVENT_RECORD>([this](const MOUSE_EVENT_RECORD& mer) {
            this->onMouse(mer);
        });
    }

    void draw() override {
        DWORD written;
        COORD pos;
        Render::attr = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;

        if (hovered)
            attr = BACKGROUND_BLUE | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
        if (focused)
            attr = BACKGROUND_GREEN | FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
        
        for (SHORT y = rect.Top; y <= rect.Bottom; y++) {
            for (SHORT x = rect.Left; x <= rect.Right; x++) {
                pos = { x, y };
                FillConsoleOutputAttribute(hout, attr, 1, pos, &written);
                WriteConsoleOutputCharacterW(hout, L" ", 1, pos, &written);
            }
        }

        DrawBox(rect);
        pos = { (SHORT)(rect.Left + 1), (SHORT)((rect.Top + rect.Bottom) / 2) };
        WriteConsoleOutputCharacterW(hout, text.c_str(), text.size(), pos, &written);
    }

    void onMouse(const MOUSE_EVENT_RECORD& mer) override {
        Control::onMouse(mer);
        if ((mer.dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED)) {
            if (!hovered) {
                setFocus(false);
                EventManager::getInstance().removeHandler<KEY_EVENT_RECORD>(handler);
                return;
            } 
            if (!focused) {
                setFocus(true);
                handler = EventManager::getInstance().addHandler<KEY_EVENT_RECORD>([this](const KEY_EVENT_RECORD & ker) {
                    this->onKey(ker);
                });
            }
        }
    }

    void onKey(const KEY_EVENT_RECORD& ker) override {
        if (ker.bKeyDown) {
            if (ker.uChar.UnicodeChar >= 32 && ker.uChar.UnicodeChar <= 126) {
                text.push_back(ker.uChar.UnicodeChar);
                draw();
            }
            else if (ker.wVirtualKeyCode == VK_BACK && !text.empty()) {
                text.pop_back();
                draw();
            }
        }
    }
};

// ------------------ CheckBox ------------------
class CheckBox : public Control, public Render {
private:
    bool checked = false;

public:
    CheckBox(SMALL_RECT r, const std::wstring& t) : Control(r, t) {
        EventManager::getInstance().addHandler<MOUSE_EVENT_RECORD>([this](const MOUSE_EVENT_RECORD& mer) {
            this->onMouse(mer);
        });
    }

    void draw() override {
        DWORD written;
        COORD coord;
        Render::attr = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;

        if (hovered)
            attr = BACKGROUND_BLUE | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
        if (focused)
            attr = BACKGROUND_GREEN | FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY;

        for (SHORT y = rect.Top; y <= rect.Bottom; y++) {
            for (SHORT x = rect.Left; x <= rect.Right; x++) {
                coord = { x, y };
                FillConsoleOutputAttribute(hout, attr, 1, coord, &written);
                WriteConsoleOutputCharacterW(hout, L" ", 1, coord, &written);
            }
        }

        std::wstring displayText = checked ? L"[X] " : L"[ ] ";
        displayText += text;

        coord = { (SHORT)(rect.Left + 1), (SHORT)((rect.Top + rect.Bottom) / 2) };
        WriteConsoleOutputCharacterW(hout, displayText.c_str(), displayText.size(), coord, &written);
    }

    void onMouse(const MOUSE_EVENT_RECORD& mer) override {
        Control::onMouse(mer);
        if (mer.dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED) {
            if (hovered) {
                checked = !checked;
                setFocus(true);
            } else {
                setFocus(false);
            }
        }
    }
};

// ------------------ Main ------------------
HANDLE hin, hout;
std::vector<std::shared_ptr<Control>> controls;
int focusedIndex = 0;

void RedrawAll() {
    for (auto& ctrl : controls) {
        ctrl->draw();
    }
}

void HandleKey(const KEY_EVENT_RECORD& ker) {
    if (ker.bKeyDown && ker.wVirtualKeyCode == VK_TAB) {
        controls[focusedIndex]->setFocus(false);
        focusedIndex = (focusedIndex + 1) % controls.size();
        controls[focusedIndex]->setFocus(true);
    }
}

int main() {
    hin = GetStdHandle(STD_INPUT_HANDLE);
    hout = GetStdHandle(STD_OUTPUT_HANDLE);

    controls.push_back(std::make_shared<Button>(SMALL_RECT{10, 2, 30, 4}, L"Click Me"));
    controls.push_back(std::make_shared<TextBox>(SMALL_RECT{10, 6, 40, 8}, L""));
    controls.push_back(std::make_shared<CheckBox>(SMALL_RECT{10, 10, 40, 12}, L"Checkbox"));

    controls[0]->setFocus(true);
    RedrawAll();

    DWORD mode;
    GetConsoleMode(hin, &mode);
    SetConsoleMode(hin, ENABLE_EXTENDED_FLAGS | ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT);

    auto& eventManager = EventManager::getInstance();
    eventManager.addHandler<KEY_EVENT_RECORD>(HandleKey);
    eventManager.start();

    std::cout << "Press ESC to exit..." << std::endl;
    while (!InputState::isKeyPressed(VK_ESCAPE)) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    return 0;
}
