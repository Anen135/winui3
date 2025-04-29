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

// ------------------ Button ------------------
class Button : public Control, public Render {
public:
    std::wstring text;
    Button(SMALL_RECT r, const std::wstring t) : Control(r), text(t) {
        EventManager::getInstance().addHandler<MOUSE_EVENT_RECORD>([this](const MOUSE_EVENT_RECORD& mer) {
            this->onMouse(mer);
        });
    }
    void draw() override {
        Render::attr = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
        if (focused)
            attr = BACKGROUND_GREEN | FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
        if (hovered)
            attr = BACKGROUND_BLUE | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY;

        for (SHORT y = rect.Top; y <= rect.Bottom; y++) {
            for (SHORT x = rect.Left; x <= rect.Right; x++) {
                FillConsoleOutputAttribute(hout, attr, 1, { x, y }, &written);
                WriteConsoleOutputCharacterW(hout, L" ", 1, { x, y }, &written);
            }
        }
        DrawBox(rect);
        int textPos = (rect.Right - rect.Left + 1 - text.size()) / 2;
        WriteConsoleOutputCharacterW(hout, text.c_str(), text.size(), { (SHORT)(rect.Left + textPos), (SHORT)((rect.Top + rect.Bottom) / 2) }, &written);
    }

    void onMouse(const MOUSE_EVENT_RECORD& mer) override {
        Control::onMouse(mer);
        if ((mer.dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED)) {
            if (hovered) {
                FocusManager::focusControl(this);  
                setFocus(true);
                onClick();
            } else {
                setFocus(false);
            }
        }
    }
};


// ------------------ TextBox ------------------
class TextBox : public Control, public Render {
std::shared_ptr<std::function<void(const KEY_EVENT_RECORD&)>> handler;
bool redmode = false;
public:
    std::wstring text;
    TextBox(SMALL_RECT r, const std::wstring t) : Control(r), text(t) {
        EventManager::getInstance().addHandler<MOUSE_EVENT_RECORD>([this](const MOUSE_EVENT_RECORD& mer) {
            this->onMouse(mer);
        });
    }

    void draw() override {
        DWORD written;
        COORD pos;
        Render::attr = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;

        if (redmode)
            attr = BACKGROUND_RED | FOREGROUND_INTENSITY;
        else if (focused)
            attr = BACKGROUND_GREEN | FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
        else if (hovered)
            attr = BACKGROUND_BLUE | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
        
        for (SHORT y = rect.Top; y <= rect.Bottom; y++) {
            for (SHORT x = rect.Left; x <= rect.Right; x++) {
                pos = { x, y };
                FillConsoleOutputAttribute(hout, attr, 1, pos, &written);
                WriteConsoleOutputCharacterW(hout, L" ", 1, pos, &written);
            }
        }

        DrawBox(rect);
        pos = { (SHORT)(rect.Left + 1), (SHORT)((rect.Top + rect.Bottom) / 2) };
        WriteConsoleOutputCharacterW(hout, text.c_str(), text.size(), pos, &written);
    }

    void onMouse(const MOUSE_EVENT_RECORD& mer) override {
        Control::onMouse(mer);
        if ((mer.dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED)) {
            if (!hovered) {
                setFocus(false);
                EventManager::getInstance().removeHandler<KEY_EVENT_RECORD>(handler);
                return;
            } 
            FocusManager::focusControl(this);  
            if (focused) {
                setFocus(true);
                handler = EventManager::getInstance().addHandler<KEY_EVENT_RECORD>([this](const KEY_EVENT_RECORD & ker) {
                    this->onKey(ker);
                });
            }
        }
    }

    void subscribeKeyboard() {
        if (handler) return;
        handler = EventManager::getInstance().addHandler<KEY_EVENT_RECORD>(
            [this](const KEY_EVENT_RECORD& ker) {
                onKey(ker);
            }
        );
    }

    void unsubscribeKeyboard() {
        if (handler) {
            EventManager::getInstance().removeHandler<KEY_EVENT_RECORD>(handler);
            handler.reset();
        }
    }

    void onKey(const KEY_EVENT_RECORD& ker) override {
        if (ker.bKeyDown) {
            if (ker.uChar.UnicodeChar >= 32 && ker.uChar.UnicodeChar <= 126) {
                text.push_back(ker.uChar.UnicodeChar);
            }
            else if (ker.wVirtualKeyCode == VK_BACK && !text.empty()) {
                text.pop_back();
            }
            redmode = text == L"password";        
            draw();
        }
    }
};

// ------------------ CheckBox ------------------
class CheckBox : public Control, public Render {
public:
    bool checked = false;
    std::wstring text;
    CheckBox(SMALL_RECT r, std::wstring t) : Control(r), text(t) {
        EventManager::getInstance().addHandler<MOUSE_EVENT_RECORD>([this](const MOUSE_EVENT_RECORD& mer) {
            this->onMouse(mer);
        });
    }

    void drawContent() {
        std::wstring displayText = checked ? L"[X] " : L"[ ] ";
        displayText += text;
        WriteConsoleOutputCharacterW(hout, displayText.c_str(), displayText.size(), { (SHORT)(rect.Left + 1), (SHORT)((rect.Top + rect.Bottom) / 2) }, &written);
    }

    void draw() override {
        COORD coord;
        Render::attr = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;

        if (focused)
            attr = BACKGROUND_GREEN | FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
        if (hovered)
            attr = BACKGROUND_BLUE | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY;

        for (SHORT y = rect.Top; y <= rect.Bottom; y++) {
            for (SHORT x = rect.Left; x <= rect.Right; x++) {
                coord = { x, y };
                FillConsoleOutputAttribute(hout, attr, 1, coord, &written);
                WriteConsoleOutputCharacterW(hout, L" ", 1, coord, &written);
            }
        }
        drawContent();
    }

    void onMouse(const MOUSE_EVENT_RECORD& mer) override {
        Control::onMouse(mer);
        if (mer.dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED) {
            if (hovered) {
                FocusManager::focusControl(this);  
                checked = !checked;
                drawContent();
            } else {
                setFocus(false);
            }
        }
    }
};  

class LoginForm {
    std::shared_ptr<TextBox> loginBox;
    std::shared_ptr<TextBox> passwordBox;
    std::shared_ptr<CheckBox> rememberMeBox;
    std::shared_ptr<Button> loginButton;
public:
    void setup() {
        loginBox = std::make_shared<TextBox>(SMALL_RECT{ 10, 4, 50, 6 }, L"Login");
        passwordBox = std::make_shared<TextBox>(SMALL_RECT{ 10, 8, 50, 10 }, L"Password");
        rememberMeBox = std::make_shared<CheckBox>(SMALL_RECT{ 10, 12, 50, 14 }, L"Remember Me");
        loginButton = std::make_shared<Button>(SMALL_RECT{ 20, 16, 40, 18 }, L"Login");

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

void FocusChanger(const KEY_EVENT_RECORD& ker) {
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
    eventManager.addHandler<KEY_EVENT_RECORD>(FocusChanger);
    eventManager.start();

    InputState::setConsoleCursorPosition({ 0, 0 });
    std::cout << " [Press ESC to exit...] " << std::endl;

    while (!InputState::isKeyPressed(VK_ESCAPE)) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    return 0;
}
