#include "Container.h"
#include "Control.h"
#include "Render.h"
#include "FocusManager.h"

class ScrollContainer : public Container {
public:
    short scrollOffset = 0;
    short contentLength = 0;

private:
    std::vector<SMALL_RECT> baseRects; // базовые rect детей без scroll

public:
    ScrollContainer(SMALL_RECT r, unsigned short d = Vertical)
        : Container(r, d) {}

    void captureBaseLayout() {
        baseRects.clear();
        baseRects.reserve(controls.size());

        for (auto& ctrl : controls) {
            baseRects.push_back(ctrl->rect);
        }
    }

    void updateContentLength() {
        contentLength = 0;

        if (baseRects.size() == controls.size() && !baseRects.empty()) {
            for (auto& r : baseRects) {
                contentLength += (direction == Vertical)
                    ? (r.Bottom - r.Top)
                    : (r.Right - r.Left);
            }
        } else {
            for (auto& ctrl : controls) {
                contentLength += (direction == Vertical)
                    ? (ctrl->rect.Bottom - ctrl->rect.Top)
                    : (ctrl->rect.Right - ctrl->rect.Left);
            }
        }

        contentLength += spacing * std::max<int>(0, static_cast<int>(controls.size()) - 1);
    }

    void applyScrollToChildren() {
        if (baseRects.size() != controls.size()) {
            captureBaseLayout();
        }

        for (size_t i = 0; i < controls.size(); ++i) {
            SMALL_RECT r = baseRects[i];

            if (direction == Vertical) {
                r.Top    -= scrollOffset;
                r.Bottom -= scrollOffset;
            } else {
                r.Left  -= scrollOffset;
                r.Right -= scrollOffset;
            }

            controls[i]->rect = r;
        }
    }

    void scroll(short delta) {
        updateContentLength();

        short visible = (direction == Vertical)
            ? (rect.Bottom - rect.Top - padding.Top - padding.Bottom)
            : (rect.Right - rect.Left - padding.Left - padding.Right);

        short maxScroll = std::max<short>(0, contentLength - visible);
        scrollOffset = std::clamp<short>(scrollOffset + delta, 0, maxScroll);

        applyScrollToChildren();
        draw();
    }

    void draw() override {
        Render::fillBox(rect);
        if (bordered) Render::DrawBox(rect);

        applyScrollToChildren();

        for (auto& ctrl : controls) {
            if (direction == Vertical) {
                if (ctrl->rect.Bottom < rect.Top + padding.Top) continue;
                if (ctrl->rect.Top > rect.Bottom - padding.Bottom) continue;
            } else {
                if (ctrl->rect.Right < rect.Left + padding.Left) continue;
                if (ctrl->rect.Left > rect.Right - padding.Right) continue;
            }

            ctrl->draw();
        }
    }
};