
#pragma once
#include <windows.h>
#include "../Core/Control.h"
#include "../Core/Render.h"
// ------------------ CheckBox ------------------
class CheckBox : public Control, public Render {
std::shared_ptr<std::function<void(const KEY_EVENT_RECORD&)>> handler;
public:
    bool checked = false;
    std::wstring text;
    CheckBox(SMALL_RECT r, std::wstring t) : Control(r), text(t) {
        EventManager::getInstance().addHandler<MOUSE_EVENT_RECORD>([this](const MOUSE_EVENT_RECORD& mer) {
            this->onMouse(mer);
        });
    }

    void drawContent() {
        std::wstring displayText = checked ? L"[X] " : L"[ ] ";
        displayText += text;
        WriteConsoleOutputCharacterW(hout, displayText.c_str(), displayText.size(), { (SHORT)(rect.Left + 1), (SHORT)((rect.Top + rect.Bottom) / 2) }, &written);
    }

    void draw() override {
        Render::     attr = FOREGROUND_RED   | FOREGROUND_GREEN | FOREGROUND_BLUE;
        if (focused) attr = BACKGROUND_GREEN | FOREGROUND_RED   | FOREGROUND_BLUE  | FOREGROUND_INTENSITY;
        if (hovered) attr = BACKGROUND_BLUE  | FOREGROUND_RED   | FOREGROUND_GREEN | FOREGROUND_INTENSITY;

        Render::fillBox(rect);
        drawContent();
    }

    void onMouse(const MOUSE_EVENT_RECORD& mer) override {
        Control::onMouse(mer);
        if (mer.dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED) {
            if (hovered) {
                FocusManager::focusControl(this);  
                checked = !checked;
                drawContent();
            } else {
                setFocus(false);
            }
        }
    }

    void setFocus(bool f) override {
        Control::setFocus(f);
        if (f) {
            subscribeKeyboard();
        } else {
            unsubscribeKeyboard();
        }   
    }

    void onKey(const KEY_EVENT_RECORD& ker) {
        if (ker.bKeyDown && ker.wVirtualKeyCode == VK_SPACE) {
            checked = !checked;
            drawContent();
        }
    }

    void subscribeKeyboard() {
        if (handler) return;
        handler = EventManager::getInstance().addHandler<KEY_EVENT_RECORD>([this](const KEY_EVENT_RECORD & ker) {
            this->onKey(ker);
        });
    }

    void unsubscribeKeyboard() {
        if (!handler) return;
        EventManager::getInstance().removeHandler<KEY_EVENT_RECORD>(handler);
        handler.reset();
    }
};  