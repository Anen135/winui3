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
        short width = rect.Right - rect.Left;
        if (width <= 0) return;
        bool hasBorder = type & 1;
        short innerWidth = width - (hasBorder ? 2 : 0);
        if (innerWidth <= 0) return;
        std::wstring visible = text.substr(0, innerWidth);
        if (hasBorder) Render::DrawBox(rect);
        SMALL_RECT textRect = rect;
        if (hasBorder) {
            textRect.Left++;
            textRect.Right--;
        }
        if (type & 2) Render::drawTextCentered(visible, textRect);
        else Render::drawTextLeft(visible, textRect);
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
