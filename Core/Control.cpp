#include "Control.h"
#include "FocusManager.h"

Control::Control(SMALL_RECT r)
    : rect(r) {}

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
    if (focused == f) return;
    focused = f;
    draw();
}