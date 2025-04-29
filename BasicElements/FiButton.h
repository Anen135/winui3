#pragma once
#include "Button.h"
#include <functional>

class FiButton : public Button {
std::shared_ptr<std::function<void(const KEY_EVENT_RECORD&)>> handler;
public:
    std::function<void()> onClick = nullptr;
    FiButton(SMALL_RECT r, std::wstring t, std::function<void()> onClick) : Button(r, t), onClick(onClick) {}
    FiButton(SMALL_RECT r, std::wstring t) : Button(r, t) {}    

    void onMouse(const MOUSE_EVENT_RECORD& mer) override {
        Control::onMouse(mer);
        if ((mer.dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED)) {
            if (hovered) {
                FocusManager::focusControl(this);  
                onClick();
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
        if (ker.bKeyDown && ker.wVirtualKeyCode == VK_RETURN) {
            onClick();
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