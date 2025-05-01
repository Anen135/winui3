#include <windows.h>
#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <functional>
#include "EventManager.h"
#include "InputState.h"
#include "FocusManager.h"
#include "Control.h"
#include "Render.h"
#include "../BasicElements/FiButton.h"
#include "../BasicElements/TextBox.h"
#include "../BasicElements/CheckBox.h"

class LoginForm {
    std::shared_ptr<TextBox> loginBox;
    std::shared_ptr<TextBox> passwordBox;
    std::shared_ptr<CheckBox> rememberMeBox;
    std::shared_ptr<FiButton> loginButton;
public:
    void setup() {
        loginBox = std::make_shared<TextBox>(SMALL_RECT{ 10, 4, 50, 6 }, L"Login");
        passwordBox = std::make_shared<TextBox>(SMALL_RECT{ 10, 8, 50, 10 }, L"Password");
        rememberMeBox = std::make_shared<CheckBox>(SMALL_RECT{ 10, 12, 50, 14 }, L"Remember Me");
        loginButton = std::make_shared<FiButton>(SMALL_RECT{ 20, 16, 40, 18 }, L"Login");

        loginButton->onClick = [this]() {
            validate();
        };

        FocusManager::registerControl(loginBox);
        FocusManager::registerControl(passwordBox);
        FocusManager::registerControl(rememberMeBox);
        FocusManager::registerControl(loginButton);
    }

    void draw() {
        FocusManager::redrawAll();
    }

    void validate() {
        std::wstring login = loginBox->text;
        std::wstring password = passwordBox->text;

        if (login == L"admin" && password == L"1234") {
            MessageBoxW(NULL, rememberMeBox->checked ? L"Welcome, admin! (Remembered)" : L"Welcome, admin!", L"Success", MB_OK);
        } else {
            MessageBoxW(NULL, L"Invalid login or password.", L"Error", MB_ICONERROR);
        }
    }
};

void KeyHandler(const KEY_EVENT_RECORD& ker) {
    if (ker.bKeyDown && ker.wVirtualKeyCode == VK_TAB) {
        FocusManager::nextFocus();
    }
}

int main() {
    HANDLE hin = GetStdHandle(STD_INPUT_HANDLE);
    HANDLE hout = GetStdHandle(STD_OUTPUT_HANDLE);

    DWORD mode;
    GetConsoleMode(hin, &mode);
    SetConsoleMode(hin, ENABLE_EXTENDED_FLAGS | ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT);

    LoginForm form;
    form.setup();
    form.draw();

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
