#pragma once
#include "Button.h"
#include <functional>

class CFButton : public Button {
public:
    CFButton(SMALL_RECT r, std::wstring t) : Button(r, t) {}
    void onMouse(const MOUSE_EVENT_RECORD& mer) override {
        Control::onMouse(mer);
        if (!(mer.dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED)) return;
        if (mer.dwEventFlags != 0 && mer.dwEventFlags != 2) return;
        if (!hovered) { setFocus(false); return; }
        FocusManager::focusControl(this);
        action();
    }

    virtual void action() override;
};