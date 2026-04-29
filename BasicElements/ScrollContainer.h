#pragma once
#include "Container.h"
#define DEMO

class ScrollContainer : public Container {
public:
    short scrollY = 0;
    short maxScroll = 0;

    ScrollContainer(SMALL_RECT r, LayoutDirection d = Vertical) : Container(r, d) {
        EventManager::getInstance().addHandler<MOUSE_EVENT_RECORD>([this](const MOUSE_EVENT_RECORD& mer) {
            this->onMouse(mer);
        });
    }

    void rearrangeControls() override {
        Container::rearrangeControls();
        if (controls.empty()) return;
        short contentBottom{0};
        for (const auto& ctrl : controls) {
            if (ctrl->rect.Bottom > contentBottom) contentBottom = ctrl->rect.Bottom;
        }        
        maxScroll = static_cast<short>((std::max)(0, (contentBottom - (rect.Top + padding.Top)) - (rect.Bottom - rect.Top - padding.Top - padding.Bottom)));
    }

    void onMouse(const MOUSE_EVENT_RECORD& mer) override {
        if (!isHovered(mer.dwMousePosition)) return;
        if (mer.dwEventFlags == MOUSE_WHEELED) {
            short scrollStep = (static_cast<short>(HIWORD(mer.dwButtonState)) > 0) ? 1 : -1;
            // std::cout << "scrollStep: " << scrollStep << "!controls.empty(): " << controls.empty() << std::endl;
            // std::cout << "controls[0]->rect.Top >= rect.Top + padding.Top: "  << (controls[0]->rect.Top >= rect.Top + padding.Top) << std::endl;
            // std::cout << "controls.back()->rect.Bottom <= rect.Bottom - padding.Bottom: " << (controls.back()->rect.Bottom <= rect.Bottom - padding.Bottom) << std::endl;
            if (scrollStep > 0 && !controls.empty() && controls[0]->rect.Top >= rect.Top + padding.Top) return;
            if (scrollStep < 0 && !controls.empty() && controls.back()->rect.Bottom <= rect.Bottom - padding.Bottom) return;
            for (auto& ctrl : controls) {
                ctrl->rect.Top += scrollStep;
                ctrl->rect.Bottom += scrollStep;
            }
            this->draw(); 
            return;
        }
    }

    void draw() override {
        Render::fillBox(rect, true);
        if (bordered) Render::DrawBox(rect);
        const short viewTop = rect.Top + padding.Top;
        const short viewBottom = rect.Bottom - padding.Bottom;
        for (auto& ctrl : controls) {
            if (ctrl->rect.Top >= viewTop && ctrl->rect.Bottom <= viewBottom) {
                ctrl->hidden = false; 
                ctrl->draw();
            } else {
                ctrl->hidden = true;  
            }
        }
    }

private:
    void drawScrollbar() {
        if (maxScroll <= 0) return;
        
        short x = rect.Right;
        short height = rect.Bottom - rect.Top - 1;
        if (height <= 0) return;

        // Рисуем "дорожку"
        for (short y = 0; y <= height; ++y) {
            Render::drawChar({x, (short)(rect.Top + y)}, L'░', 0x08);
        }

        // Рисуем "ползунок"
        short thumbPos = (scrollY * height) / (maxScroll + height);
        Render::drawChar({x, (short)(rect.Top + thumbPos)}, L'█', 0x07);
    }
};