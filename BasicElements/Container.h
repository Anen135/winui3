#pragma once
#include <vector>
#include <memory>
#include "../Core/Control.h"
#include "../Core/Render.h"
#include <algorithm>

// ------------------ Container ------------------
class Container : public Control, public Render {
public:
    std::vector<std::shared_ptr<Control>> controls;
    bool hasbox = true;

    enum LayoutDirection { Vertical = 0, Horizontal = 1 };
    enum Alignment { Start = 0, Center = 1, End = 2 };

    unsigned short direction;
    short spacing = 1;
    SMALL_RECT padding = { 0, 0, 0, 0 };
    Alignment alignment = Start;

    Container(SMALL_RECT r, unsigned short d = Vertical)
        : Control(r), direction(d) {}
    Container(SMALL_RECT r, LayoutDirection d, std::vector<std::shared_ptr<Control>> c)
        : Control(r), direction(d), controls(c) {}

    void addControl(const std::shared_ptr<Control>& ctrl) {
        controls.push_back(ctrl);
    }

    void removeControl(const std::shared_ptr<Control>& ctrl) {
        controls.erase(std::remove(controls.begin(), controls.end(), ctrl), controls.end());
    }

    void rearrangeControls() {
        if (controls.empty()) return;

        short totalLength = 0;
        for (auto& ctrl : controls) {
            totalLength += (direction == Vertical)
                ? (ctrl->rect.Bottom - ctrl->rect.Top)
                : (ctrl->rect.Right - ctrl->rect.Left);
        }
        totalLength += spacing * (controls.size() - 1);

        short availableLength = (direction == Vertical)
            ? (rect.Bottom - rect.Top - padding.Top - padding.Bottom)
            : (rect.Right - rect.Left - padding.Left - padding.Right);

        short offset = paddingOffset(availableLength, totalLength);

        for (auto& ctrl : controls) {
            short w = ctrl->rect.Right - ctrl->rect.Left;
            short h = ctrl->rect.Bottom - ctrl->rect.Top;

            if (direction == Vertical) {
                ctrl->rect.Top = rect.Top + padding.Top + offset;
                ctrl->rect.Bottom = ctrl->rect.Top + h;
                ctrl->rect.Left = rect.Left + padding.Left;
                ctrl->rect.Right = rect.Right - padding.Right;
                offset += h + spacing;
            } else {
                ctrl->rect.Left = rect.Left + padding.Left + offset;
                ctrl->rect.Right = ctrl->rect.Left + w;
                ctrl->rect.Top = rect.Top + padding.Top;
                ctrl->rect.Bottom = rect.Bottom - padding.Bottom;
                offset += w + spacing;
            }
        }
    }

    short paddingOffset(short available, short content) const {
        switch (alignment) {
            case Start:  return 0;
            case Center: return (available - content) / 2;
            case End:    return (available - content);
            default:     return 0;
        }
    }

    void draw() override {
        Render::fillBox(rect);
        if (hasbox) Render::DrawBox(rect);
        for (auto& ctrl : controls) ctrl->draw();
    }
};
