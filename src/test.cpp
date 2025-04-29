#include <windows.h>
#include <iostream>
#include <memory>
#include "EventManager.h"
#include "InputState.h"
#include "FocusManager.h"
#include "Control.h"
#include "Render.h"
#include "../BasicElements/FiButton.h"
#include "../BasicElements/TextBox.h"
#include "../BasicElements/CheckBox.h"

// ------------------ Main ------------------
void FocusChanger(const KEY_EVENT_RECORD& ker) {
    if (ker.bKeyDown && ker.wVirtualKeyCode == VK_TAB) {
        FocusManager::nextFocus();
    }
}

void createMessageBox() {
    MessageBoxW(NULL, L"Hello, World!", L"Success", MB_OK);
}

    
int main() {
    HANDLE hin = GetStdHandle(STD_INPUT_HANDLE);
    FiButton btn(SMALL_RECT{10, 2, 30, 4}, L"Click Me", createMessageBox);
    FocusManager::registerControl(std::make_shared<FiButton>(btn));
    FocusManager::registerControl(std::make_shared<TextBox>(SMALL_RECT{10, 6, 40, 8}, L""));
    FocusManager::registerControl(std::make_shared<CheckBox>(SMALL_RECT{10, 10, 40, 12}, L"Checkbox"));
    FocusManager::nextFocus(); 
    FocusManager::redrawAll();

    DWORD mode;
    GetConsoleMode(hin, &mode);
    SetConsoleMode(hin, ENABLE_EXTENDED_FLAGS | ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT);

    auto& eventManager = EventManager::getInstance();
    eventManager.addHandler<KEY_EVENT_RECORD>(FocusChanger);
    eventManager.start();

    InputState::setConsoleCursorPosition({ 0, 0 });
    std::cout << " [Press ESC to exit...] " << std::endl;
    while (!InputState::isKeyPressed(VK_ESCAPE)) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    return 0;
}
