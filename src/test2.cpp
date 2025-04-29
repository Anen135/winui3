#include <windows.h>
#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <functional>
#include <stack>
#include <cctype>
#include "EventManager.h"
#include "InputState.h"
#include "FocusManager.h"
#include "Control.h"


// ------------------ Base Control ------------------

class Render {
public:
    WORD attr {FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE};
    HANDLE hout = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD written;

    void DrawBox(SMALL_RECT rect) {
        DWORD written;
        COORD pos;
        WCHAR hline = L'\u2500'; // ─
        WCHAR vline = L'\u2502'; // │
        WCHAR tl = L'\u250C';    // ┌
        WCHAR tr = L'\u2510';    // ┐
        WCHAR bl = L'\u2514';    // └
        WCHAR br = L'\u2518';    // ┘

        pos = { rect.Left, rect.Top };
        WriteConsoleOutputCharacterW(hout, &tl, 1, pos, &written);
        for (SHORT x = rect.Left + 1; x < rect.Right; x++) {
            pos = { x, rect.Top };
            WriteConsoleOutputCharacterW(hout, &hline, 1, pos, &written);
        }
        pos = { rect.Right, rect.Top };
        WriteConsoleOutputCharacterW(hout, &tr, 1, pos, &written);

        for (SHORT y = rect.Top + 1; y < rect.Bottom; y++) {
            pos = { rect.Left, y };
            WriteConsoleOutputCharacterW(hout, &vline, 1, pos, &written);
            pos = { rect.Right, y };
            WriteConsoleOutputCharacterW(hout, &vline, 1, pos, &written);
        }

        pos = { rect.Left, rect.Bottom };
        WriteConsoleOutputCharacterW(hout, &bl, 1, pos, &written);
        for (SHORT x = rect.Left + 1; x < rect.Right; x++) {
            pos = { x, rect.Bottom };
            WriteConsoleOutputCharacterW(hout, &hline, 1, pos, &written);
        }
        pos = { rect.Right, rect.Bottom };
        WriteConsoleOutputCharacterW(hout, &br, 1, pos, &written);
    }
};

// ------------------ Button ------------------
class Button : public Control, public Render {
public:
    std::function<void()> onClick;
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


class CalculatorForm {
    std::shared_ptr<TextBox> display;
    std::vector<std::shared_ptr<Button>> buttons;

public:
    void setup() {
        display = std::make_shared<TextBox>(SMALL_RECT{10, 2, 53, 4}, L"");
        display->text = L"0";

        std::vector<std::wstring> layout = {
            L"7", L"8", L"9", L"/",
            L"4", L"5", L"6", L"*",
            L"1", L"2", L"3", L"-",
            L"0", L".", L"=", L"+"
        };

        int startX = 10;
        int startY = 6;
        int buttonWidth = 10;
        int buttonHeight = 2;
        int cols = 4;

        for (size_t i = 0; i < layout.size(); ++i) {
            int row = i / cols;
            int col = i % cols;
            SHORT left = startX + col * (buttonWidth + 1);
            SHORT top = startY + row * (buttonHeight + 1);
            SMALL_RECT r = { left, top, left + buttonWidth, top + buttonHeight };

            auto btn = std::make_shared<Button>(r, layout[i]);

            btn->onClick = [this, text = layout[i]]() {
                this->onButtonClick(text);
            };

            buttons.push_back(btn);
            FocusManager::registerControl(btn);
        }

        FocusManager::registerControl(display);
    }

    void draw() {
        FocusManager::redrawAll();
    }

private:
    void onButtonClick(const std::wstring& value) {
        if (value == L"=") {
            calculate();
            return;
        }

        if (display->text == L"0")
            display->text = value;
        else
            display->text += value;

        display->draw();
    }

    void calculate() {
        try {
            double result = evaluate(display->text);
            display->text = std::to_wstring(result);

            // Убираем лишние нули
            while (!display->text.empty() && (display->text.back() == L'0' || display->text.back() == L'.')) {
                if (display->text.back() == L'.') {
                    display->text.pop_back();
                    break;
                }
                display->text.pop_back();
            }

            display->draw();
        } catch (...) {
            display->text = L"Error";
            display->draw();
        }
    }

    double evaluate(const std::wstring& expr) {
        std::string s(expr.begin(), expr.end());
        size_t idx = 0;
        return parseExpression(s, idx);
    }

    double parseExpression(const std::string& s, size_t& idx) {
        double result = parseTerm(s, idx);
        while (idx < s.size()) {
            if (s[idx] == '+') {
                ++idx;
                result += parseTerm(s, idx);
            } else if (s[idx] == '-') {
                ++idx;
                result -= parseTerm(s, idx);
            } else {
                break;
            }
        }
        return result;
    }

    double parseTerm(const std::string& s, size_t& idx) {
        double result = parseFactor(s, idx);
        while (idx < s.size()) {
            if (s[idx] == '*') {
                ++idx;
                result *= parseFactor(s, idx);
            } else if (s[idx] == '/') {
                ++idx;
                result /= parseFactor(s, idx);
            } else {
                break;
            }
        }
        return result;
    }

    double parseFactor(const std::string& s, size_t& idx) {
        double result = 0;
        bool negative = false;

        if (s[idx] == '-') {
            negative = true;
            ++idx;
        }

        while (idx < s.size() && (isdigit(s[idx]) || s[idx] == '.')) {
            std::string number;
            while (idx < s.size() && (isdigit(s[idx]) || s[idx] == '.')) {
                number += s[idx++];
            }
            result = std::stod(number);
        }

        return negative ? -result : result;
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

    CalculatorForm calc;
    calc.setup();
    calc.draw();

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
