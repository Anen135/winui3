#pragma once
#include <windows.h>
#include "../Core/Control.h"
#include "../Core/Render.h"
// ------------------ Button ------------------
class Button : public Control, public Render {
public:
    std::wstring text;
    Button(SMALL_RECT r, const std::wstring t) : Control(r), text(t) {
        EventManager::getInstance().addHandler<MOUSE_EVENT_RECORD>([this](const MOUSE_EVENT_RECORD& mer) {
            this->onMouse(mer);
        });
    }
    void draw() override {
                          Render::attr = FOREGROUND_RED   | FOREGROUND_GREEN | FOREGROUND_BLUE;
        if      (hovered) Render::attr = BACKGROUND_BLUE  | FOREGROUND_RED   | FOREGROUND_GREEN  | FOREGROUND_INTENSITY;
        if      (focused) Render::attr = BACKGROUND_GREEN | FOREGROUND_RED   | FOREGROUND_BLUE   | FOREGROUND_INTENSITY;
        Render::fillBox(rect);
        Render::DrawBox(rect);
        Render::drawTextCentered(text, rect);
    }

    virtual void onMouse(const MOUSE_EVENT_RECORD& mer) = 0;
};

