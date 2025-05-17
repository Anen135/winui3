
#pragma once
#include <windows.h>
#include "../Core/Control.h"
#include "../Core/Render.h"
// ------------------ CheckBox ------------------
class CheckBox : public Control, public Render {
public:
    bool checked = false;
    std::wstring text;
    CheckBox(SMALL_RECT r, std::wstring t) : Control(r), text(t) {
        EventManager::getInstance().addHandler<MOUSE_EVENT_RECORD>([this](const MOUSE_EVENT_RECORD& mer) {
            this->onMouse(mer);
        });
    }

    void drawContent() {
        std::wstring displayText = checked ? L"[X] " : L"[ ] ";
        displayText += text;
        WriteConsoleOutputCharacterW(hout, displayText.c_str(), displayText.size(), { (SHORT)(rect.Left + 1), (SHORT)((rect.Top + rect.Bottom) / 2) }, &dump);
    }

    void draw() override {
        Render::     attr = FOREGROUND_RED   | FOREGROUND_GREEN | FOREGROUND_BLUE;
        if (focused) attr = BACKGROUND_GREEN | FOREGROUND_RED   | FOREGROUND_BLUE  | FOREGROUND_INTENSITY;
        if (hovered) attr = BACKGROUND_BLUE  | FOREGROUND_RED   | FOREGROUND_GREEN | FOREGROUND_INTENSITY;

        Render::fillBox(rect);
        drawContent();
    }

    void onMouse(const MOUSE_EVENT_RECORD& mer) override {
        Control::onMouse(mer);
        if (mer.dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED) {
            if (hovered) {
                FocusManager::focusControl(this);  
                action();
            } else {
                setFocus(false);
            }
        }
    }

    void action() override {
        checked = !checked;
        drawContent();
    }
};  