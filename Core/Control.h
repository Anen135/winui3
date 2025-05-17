// Control.h
#pragma once
#include <windows.h>
#include <string>

// Forward declaration
class FocusManager;

class Control {
protected:
    bool hovered = false;
public:
    SMALL_RECT rect;
    bool focused = false;
    bool hidden = false;
    Control(SMALL_RECT r);

    virtual void draw() = 0;
    virtual void onMouse(const MOUSE_EVENT_RECORD& mer);
    virtual void onKey(const KEY_EVENT_RECORD& ker) {(void)ker;};
    virtual void action() {}
    virtual void focusChanged() {}
    virtual void setFocus(bool f);

    bool isHovered(const COORD& pos);
    bool hasFocus() const { return focused; }
};
