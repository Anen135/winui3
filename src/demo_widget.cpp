#define UNICODE
#include <windows.h>
#include <iostream>
#include <memory>
#include "EventManager.h"
#include "InputState.h"
#include "Render.h"
#include "Control.h"
#include "../Core/FocusManager.h"
#include "../Community/Widget.h"
#include "../Community/Theme.h"
#include "../BasicElements/Container.h"
#include "../BasicElements/Label.h"


// Custom button using Widget + Render inheritance
class ThemedButton : public Widget, public Render {
public:
    std::wstring text;
    bool bordered = true;

    ThemedButton(SMALL_RECT r, const std::wstring& t)
        : Widget(r, t), text(t) {
        normalAttr = Theme::BTN_NORMAL;
        hoverAttr = Theme::BTN_HOVER;
        focusedAttr = Theme::BTN_FOCUSED;
        disabledAttr = Theme::DISABLED;

        EventManager::getInstance().addHandler<MOUSE_EVENT_RECORD>([this](const MOUSE_EVENT_RECORD& mer) {
            if (enabled){
                Control::onMouse(mer);
                if (!(mer.dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED)) return;
                if (mer.dwEventFlags != 0 && mer.dwEventFlags != 2) return;
                if (!hovered) { setFocus(false); return; }
                FocusManager::focusControl(this);
                action();        
            }
        });
    }

    void draw() override {
        if (!visible) return;

        attr = enabled 
            ? (focused ? focusedAttr 
              : hovered ? hoverAttr 
              : normalAttr)
            : disabledAttr;

        fillBox(rect);
        if (bordered) DrawBox(rect);
        drawTextCentered(text, rect);
    }

    void action() override {
        // Toggle on click
        text = (text == L"Clicked!") ? L"Click Me!" : L"Clicked!";
        draw();
    }
};


int main() {
    // Setup console mode
    HANDLE hin = GetStdHandle(STD_INPUT_HANDLE);
    DWORD mode;
    GetConsoleMode(hin, &mode);
    SetConsoleMode(hin, ENABLE_EXTENDED_FLAGS | ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT);

    // Create root container
    Container root({ 5, 3, 50, 20 }, Container::Vertical);
    root.spacing = 2;
    root.addControl(std::make_shared<Label>(SMALL_RECT{ 0, 0, 40, 1 }, L"WinUI3 Widget Demo", 0));

    // Add themed buttons using Widget
    auto btn1 = std::make_shared<ThemedButton>(SMALL_RECT{ 0, 0, 15, 3 }, L"Click Me!");
    auto btn2 = std::make_shared<ThemedButton>(SMALL_RECT{ 0, 0, 15, 3 }, L"Toggle!");
    auto btn3 = std::make_shared<ThemedButton>(SMALL_RECT{ 0, 0, 15, 3 }, L"Disabled");

    // Disable third button - Widget property
    btn3->enabled = false;

    root.addControl(btn1);
    root.addControl(btn2);
    root.addControl(btn3);

    // Show Widget utilities
    auto infoLabel = std::make_shared<Label>(SMALL_RECT{ 0, 0, 40, 3 }, L"");
    infoLabel->text = L"ID: " + btn1->id + L" Visible: " + (btn1->visible ? L"true" : L"false")
        + L" Enabled: " + (btn1->enabled ? L"true" : L"false");
    root.addControl(infoLabel);

    // Layout and draw
    root.rearrangeControls();
    root.draw();

    std::cout << " [Mouse: Hover/Click | ESC: Exit]" << std::endl;

    // Start event manager
    EventManager::getInstance().start();

    // Main loop
    while (!InputState::isKeyPressed(VK_ESCAPE)) {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    SetConsoleMode(hin, mode);
    return 0;
}