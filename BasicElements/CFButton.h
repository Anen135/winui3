#pragma once
#include "Button.h"
#include <functional>

class CFButton : public Button {
public:
    CFButton(SMALL_RECT r, std::wstring t) : Button(r, t) {}
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

    virtual void action() override;
};