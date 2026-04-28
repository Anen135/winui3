#pragma once
#include "Container.h"
#include <algorithm>

class ScrollContainer : public Container {
public:
    short scrollOffset = 0;
    short contentLength = 0;

private:
    std::vector<SMALL_RECT> baseRects;
    bool baseLayoutCaptured = false;

public:
    ScrollContainer(SMALL_RECT r, unsigned short d = Vertical)
        : Container(r, d) {
    }

    void captureBaseLayout() {
        if (baseLayoutCaptured) return;
        
        baseRects.clear();
        baseRects.reserve(controls.size());

        for (auto& ctrl : controls) {
            baseRects.push_back(ctrl->rect);
        }
        
        baseLayoutCaptured = true;
    }

    void updateContentLength() {
        if (!baseLayoutCaptured) {
            captureBaseLayout();
        }

        if (baseRects.empty()) {
            contentLength = 0;
            return;
        }

        if (direction == Vertical) {
            short total = 0;
            for (auto& r : baseRects) {
                total += (r.Bottom - r.Top);
            }
            total += spacing * static_cast<short>(baseRects.size() - 1);
            contentLength = total;
        } else {
            short maxWidth = 0;
            for (auto& r : baseRects) {
                maxWidth = std::max<short>(maxWidth, r.Right - r.Left);
            }
            contentLength = maxWidth + spacing * static_cast<short>(baseRects.size() - 1);
        }
    }

    void applyScrollToChildren() {
        if (!baseLayoutCaptured) return;
        if (baseRects.size() != controls.size()) return;

        for (size_t i = 0; i < controls.size(); ++i) {
            SMALL_RECT r = baseRects[i];

            if (direction == Vertical) {
                r.Top -= scrollOffset;
                r.Bottom -= scrollOffset;
            } else {
                r.Left -= scrollOffset;
                r.Right -= scrollOffset;
            }

            controls[i]->rect = r;
        }
    }

    short getVisibleLength() const {
        short borderOffset = bordered ? 1 : 0;
        if (direction == Vertical) {
            return rect.Bottom - rect.Top - padding.Top - padding.Bottom - borderOffset * 2;
        } else {
            return rect.Right - rect.Left - padding.Left - padding.Right - borderOffset * 2;
        }
    }

    void scroll(short delta) {
        updateContentLength();

        short visible = getVisibleLength();
        short maxScroll = std::max<short>(0, contentLength - visible);
        scrollOffset = std::clamp<short>(scrollOffset + delta, 0, maxScroll);

        applyScrollToChildren();
        draw();
    }

    void draw() override {
        Render::fillBox(rect, bordered);
        if (bordered) {
            Render::DrawBox(rect);
        }

        applyScrollToChildren();

        short visTop = rect.Top + padding.Top + bordered;
        short visBottom = rect.Bottom - padding.Bottom - bordered;
        short visLeft = rect.Left + padding.Left + bordered;
        short visRight = rect.Right - padding.Right - bordered;

        for (auto& ctrl : controls) {
            SMALL_RECT r = ctrl->rect;

            if (direction == Vertical) {
                if (r.Bottom <= visTop) continue;
                if (r.Top >= visBottom) continue;

                r.Top = (((r.Top) > (visTop)) ? (r.Top) : (visTop));
                r.Bottom = (((r.Bottom) < (visBottom)) ? (r.Bottom) : (visBottom));
            } else {
                if (r.Right <= visLeft) continue;
                if (r.Left >= visRight) continue;

                r.Left = (((r.Left) > (visLeft)) ? (r.Left) : (visLeft));
                r.Right = (((r.Right) < (visRight)) ? (r.Right) : (visRight));
            }

            SMALL_RECT old = ctrl->rect;
            ctrl->rect = r;

            ctrl->draw();

            ctrl->rect = old;
        }
    }
};