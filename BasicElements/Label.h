#pragma once
#include <windows.h>
#include "../Core/Control.h"
#include "../Core/Render.h"
// ------------------ Label ------------------
class Label : public Control, public Render {
bool hasBox {true};
public:
    std::wstring text;
    Label(SMALL_RECT r, const std::wstring t) : Control(r), text(t) {
        EventManager::getInstance().addHandler<MOUSE_EVENT_RECORD>([this](const MOUSE_EVENT_RECORD& mer) {
            this->onMouse(mer);
        });
    }
    void updateText() {
        Render::drawTextLeft(text, rect);
    }
    void draw() override {
        Render::fillBox(rect);
        if (hasBox) Render::DrawBox(rect);
        Render::drawTextLeft(text, rect);
    }
    void onMouse(const MOUSE_EVENT_RECORD& mer) override {
        if (mer.dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED) {
            bool wasHovered = hovered;
            hovered = isHovered(mer.dwMousePosition);
            if (hovered == wasHovered) return;
            hasBox = wasHovered;
            draw();
        }
    }
};
