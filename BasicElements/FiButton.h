#pragma once
#include "Button.h"
#include <functional>

class FIButton : public Button {
std::shared_ptr<std::function<void(const KEY_EVENT_RECORD&)>> handler;
public:
    std::function<void()> onClick = nullptr;
    FIButton(SMALL_RECT r, std::wstring t, std::function<void()> onClick) : Button(r, t), onClick(onClick) {}
    FIButton(SMALL_RECT r, std::wstring t) : Button(r, t) {}  
    FIButton(SMALL_RECT r, wchar_t t) : Button(r, std::wstring(1, t)) {}  

    void onMouse(const MOUSE_EVENT_RECORD& mer) override {
        Control::onMouse(mer);
        if ((mer.dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED)) {
            if (hovered) {
                FocusManager::focusControl(this);  
                action();
            } else {
                setFocus(false);
            }
        }
    }
    void action() override {
        if (onClick) onClick();
    }

};