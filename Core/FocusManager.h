#pragma once
#include <vector>
#include <algorithm>
#include <memory>
#include <iostream>
#include <functional>
#include "Control.h"

class Control;
class FocusManager {
private:
    static inline std::vector<std::shared_ptr<Control>> controls;
    static inline int focusedIndex = -1;

public:
    static void registerControl(const std::shared_ptr<Control>& ctrl) {
        controls.push_back(ctrl);
    }

    static void clearControls() {
        controls.clear();
        focusedIndex = -1;
    }

    static void focusControl(Control* ctrl) {
        if (focusedIndex != -1) controls[focusedIndex]->setFocus(false);
        
        auto it = std::find_if(controls.begin(), controls.end(),
                               [ctrl](const std::shared_ptr<Control>& c) { return c.get() == ctrl; });
        if (it != controls.end()) {
            focusedIndex = std::distance(controls.begin(), it);
            controls[focusedIndex]->setFocus(true);
        }
    }

    static void nextFocus() {
        if (controls.empty()) return;
        if (focusedIndex != -1) controls[focusedIndex]->setFocus(false);

        focusedIndex = ((GetKeyState(VK_SHIFT) & 0x8000) != 0) ? ((focusedIndex + controls.size() - 1) % controls.size()) : ((focusedIndex + 1) % controls.size());
        controls[focusedIndex]->setFocus(true);
    }

    static void redrawAll() {
        for (auto& ctrl : controls) ctrl->draw();
    }

    static std::shared_ptr<Control> getFocused() {
        if (focusedIndex == -1) throw std::runtime_error("No focused control");
        return controls[focusedIndex];
    }

};
