#pragma once
#include "TextBox.h"
#include "functional"

class CFTextBox : public TextBox {
public:
    CFTextBox(SMALL_RECT r, std::wstring t) : TextBox(r, t) {}
    void onKey(const KEY_EVENT_RECORD& ker) override {
        if (ker.bKeyDown) {
            if ((ker.uChar.UnicodeChar >= 32 && ker.uChar.UnicodeChar <= 126) ||        
                    (ker.uChar.UnicodeChar >= 1040 && ker.uChar.UnicodeChar <= 1103)) {
                text.push_back(ker.uChar.UnicodeChar);
            }
            else if (ker.wVirtualKeyCode == VK_BACK && !text.empty()) {
                text.pop_back();
            } else if (ker.wVirtualKeyCode == VK_RETURN) {
                onEnter();
            }
            draw();
        }
    }
    void onEnter();
};