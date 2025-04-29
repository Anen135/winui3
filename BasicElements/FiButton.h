#pragma once
#include "Button.h"
#include <functional>

class FiButton : public Button {
public:
    std::function<void()> onClick = nullptr;
    FiButton(SMALL_RECT r, std::wstring t, std::function<void()> onClick) : Button(r, t), onClick(onClick) {}
    FiButton(SMALL_RECT r, std::wstring t) : Button(r, t) {}    

    void onMouse(const MOUSE_EVENT_RECORD& mer) override {
        Control::onMouse(mer);
        if ((mer.dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED)) {
            if (hovered) {
                FocusManager::focusControl(this);  
                setFocus(true);
                onClick();
            } else {
                setFocus(false);
            }
        }
    }
};