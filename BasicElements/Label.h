#pragma once
#include <windows.h>
#include "../Core/Control.h"
#include "../Core/Render.h"
// ------------------ Label ------------------
class Label : public Control, public Render {
public:
    std::wstring text;
    uint8_t type {1};
    Label(SMALL_RECT r, const std::wstring t) : Control(r), text(t) {}
    Label(SMALL_RECT r, const std::wstring t, uint8_t tp) : Control(r), text(t), type(tp) {}
    void updateText() {
        Render::drawTextLeft(text, rect);
    }
    void draw() override {
        Render::fillBox(rect);
        if (type & 1) Render::DrawBox(rect);
        if  (type & 2) Render::drawTextCentered(text, rect);
        else Render::drawTextLeft(text, rect);
    }
    void onMouse(const MOUSE_EVENT_RECORD& mer) override {
        if (mer.dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED) {
            bool wasHovered = hovered;
            hovered = isHovered(mer.dwMousePosition);
            if (hovered == wasHovered) return;
            type |= hovered << 1;
            draw();
        }
    }
};
