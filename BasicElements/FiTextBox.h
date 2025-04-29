#pragma once
#include "TextBox.h"
#include "functional"

class FiTextBox : public TextBox {
public:
    std::function<void(const std::wstring& password)> onEnter = nullptr;
    FiTextBox(SMALL_RECT r, std::wstring t, std::function<void(const std::wstring& password)> onEnter) : TextBox(r, t), onEnter(onEnter) {}
    FiTextBox(SMALL_RECT r, std::wstring t) : TextBox(r, t) {}
    void onKey(const KEY_EVENT_RECORD& ker) override {
        if (ker.bKeyDown) {
            if (ker.uChar.UnicodeChar >= 32 && ker.uChar.UnicodeChar <= 126) {
                text.push_back(ker.uChar.UnicodeChar);
            }
            else if (ker.wVirtualKeyCode == VK_BACK && !text.empty()) {
                text.pop_back();
            } else if (ker.wVirtualKeyCode == VK_RETURN) {
                onEnter(text);
            }
            draw();
        }
    }
};