
#pragma once
#include <windows.h>
#include "../Core/Control.h"
#include "../Core/Render.h"
// ------------------ TextBox ------------------
class TextBox : public Control, public Render {
std::shared_ptr<std::function<void(const KEY_EVENT_RECORD&)>> handler;
bool redmode = false;
public:
    std::wstring text;
    TextBox(SMALL_RECT r, const std::wstring t) : Control(r), text(t) {
        EventManager::getInstance().addHandler<MOUSE_EVENT_RECORD>([this](const MOUSE_EVENT_RECORD& mer) {
            this->onMouse(mer);
        });
    }

    void draw() override {
      /*if      ( all) */ Render::attr = FOREGROUND_RED     | FOREGROUND_GREEN                  | FOREGROUND_BLUE;
        if      (redmode) Render::attr = BACKGROUND_RED     | FOREGROUND_INTENSITY;
        else if (focused) Render::attr = BACKGROUND_GREEN   | FOREGROUND_RED | FOREGROUND_BLUE  | FOREGROUND_INTENSITY;
        else if (hovered) Render::attr = BACKGROUND_BLUE    | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY;

        Render::fillBox(rect);
        Render::DrawBox(rect);
        Render::drawTextLeft(text, rect);
    }

    void onMouse(const MOUSE_EVENT_RECORD& mer) override {
        Control::onMouse(mer);
        if ((mer.dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED)) {
            if (!hovered) {
                setFocus(false);
                unsubscribeKeyboard();
                return;
            } 
            FocusManager::focusControl(this);  
            if (focused) {
                setFocus(true);
                subscribeKeyboard();
            }
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

    void onKey(const KEY_EVENT_RECORD& ker) override {
        if (ker.bKeyDown) {
            if (ker.uChar.UnicodeChar >= 32 && ker.uChar.UnicodeChar <= 126) {
                text.push_back(ker.uChar.UnicodeChar);
            }
            else if (ker.wVirtualKeyCode == VK_BACK && !text.empty()) {
                text.pop_back();
            }
            redmode = text == L"password";        
            draw();
        }
    }
};
