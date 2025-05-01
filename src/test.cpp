#include <windows.h>
#include <iostream>
#include <memory>
#include "EventManager.h"
#include "InputState.h"
#include "FocusManager.h"
#include "Control.h"
#include "Render.h"
#include "../BasicElements/FiButton.h"
#include "../BasicElements/FiTextBox.h"
#include "../BasicElements/CheckBox.h"

// ------------------ Main ------------------
void KeyHandler(const KEY_EVENT_RECORD& ker) {
    if (ker.bKeyDown && ker.wVirtualKeyCode == VK_TAB) {
        FocusManager::nextFocus();
    }
}

void createMessageBox() {
    MessageBoxW(NULL, L"Hello, World!", L"Success", MB_OK);
}

void validatePassword(const std::wstring& password) {
    if (password == L"password") MessageBoxW(NULL, L"Password is valid!", L"Success", MB_OK);
    else MessageBoxW(NULL, L"Password is invalid!", L"Error", MB_ICONERROR);
}


int main() {
    HANDLE hin = GetStdHandle(STD_INPUT_HANDLE);
    FocusManager::registerControl(std::make_shared<FiButton>(SMALL_RECT{10, 2, 30, 4}, L"Click Me", createMessageBox));
    FocusManager::registerControl(std::make_shared<FiTextBox>(SMALL_RECT{10, 6, 40, 8}, L"", validatePassword));
    FocusManager::registerControl(std::make_shared<CheckBox>(SMALL_RECT{10, 10, 40, 12}, L"Checkbox"));
    FocusManager::nextFocus(); 
    FocusManager::redrawAll();

    DWORD mode;
    GetConsoleMode(hin, &mode);
    SetConsoleMode(hin, ENABLE_EXTENDED_FLAGS | ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT);

    auto& eventManager = EventManager::getInstance();
    eventManager.addHandler<KEY_EVENT_RECORD>(KeyHandler);
    eventManager.start();

    InputState::setConsoleCursorPosition({ 0, 0 });
    std::cout << " [Press ESC to exit...] " << std::endl;
    while (!InputState::isKeyPressed(VK_ESCAPE)) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    return 0;
}
