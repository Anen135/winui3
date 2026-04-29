#pragma once
#include "Container.h"

class ScrollContainer : public Container {
public:
    short scrollY = 0;
    short maxScroll = 0;

    ScrollContainer(SMALL_RECT r, LayoutDirection d = Vertical) : Container(r, d) {
        // Выключаем стандартный авто-выравниватель по центру, 
        // так как скролл обычно подразумевает Start-выравнивание
        alignment = Start;
    }

    void rearrangeControls() override {
        // Сначала вызываем базовую логику распределения
        Container::rearrangeControls();

        // Рассчитываем максимальный скролл
        if (controls.empty()) return;
        
        short contentBottom = 0;
        for (const auto& ctrl : controls) {
            if (ctrl->rect.Bottom > contentBottom) contentBottom = ctrl->rect.Bottom;
        }

        short visibleHeight = rect.Bottom - rect.Top - padding.Top - padding.Bottom;
        short totalHeight = contentBottom - (rect.Top + padding.Top);
        
        maxScroll = (std::max)(0, totalHeight - visibleHeight);
    }

    void onMouse(const MOUSE_EVENT_RECORD& mer) override {
        // 1. Обработка колесика мыши
        if (mer.dwEventFlags == MOUSE_WHEELED) {
            short delta = (short)HIWORD(mer.dwButtonState);
            if (delta > 0) scrollY = (std::max)(0, scrollY - 1);
            else scrollY = (std::min)((int)maxScroll, scrollY + 1);
            return; 
        }

        // 2. Корректировка координат для дочерних элементов
        // Создаем копию события с измененными координатами
        MOUSE_EVENT_RECORD adjustedMer = mer;
        adjustedMer.dwMousePosition.Y += scrollY; 

        for (auto& ctrl : controls) {
            ctrl->onMouse(adjustedMer);
        }
    }

    void draw() override {
        Render::fillBox(rect);
        if (bordered) Render::DrawBox(rect);

        // Включаем Clipping (нужно реализовать в Render или здесь)
        // Идея: рисовать только то, что попадает в rect
        for (auto& ctrl : controls) {
            // Временный сдвиг для отрисовки
            SMALL_RECT originalRect = ctrl->rect;
            
            ctrl->rect.Top -= scrollY;
            ctrl->rect.Bottom -= scrollY;

            // Простая проверка на видимость (AABB)
            if (ctrl->rect.Bottom >= rect.Top + padding.Top && 
                ctrl->rect.Top <= rect.Bottom - padding.Bottom) {
                
                // Тут кроется нюанс: если элемент отрисуется частично 
                // вне границ, он затрет рамку или соседние UI блоки.
                // Рекомендуется в Render::draw реализовать проверку границ rect контейнера.
                ctrl->draw();
            }

            ctrl->rect = originalRect; // Возвращаем координаты
        }

        // Опционально: отрисовка полосы прокрутки (Scrollbar)
        drawScrollbar();
    }

private:
    void drawScrollbar() {
        if (maxScroll <= 0) return;
        
        short x = rect.Right;
        short height = rect.Bottom - rect.Top - 1;
        if (height <= 0) return;

        // Рисуем "дорожку"
        for (short y = 0; y <= height; ++y) {
            Render::putChar({x, (short)(rect.Top + y)}, L'░', 0x08);
        }

        // Рисуем "ползунок"
        short thumbPos = (scrollY * height) / (maxScroll + height);
        Render::putChar({x, (short)(rect.Top + thumbPos)}, L'█', 0x07);
    }
};