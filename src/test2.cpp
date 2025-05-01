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
#include "../BasicElements/FiButton.h"
#include "../BasicElements/Label.h"

class CalculatorForm {
    std::shared_ptr<Label> display;
    std::vector<std::shared_ptr<Button>> buttons;

public:
    void setup() {
        display = std::make_shared<Label>(SMALL_RECT{10, 2, 53, 4}, L"0");

        std::vector<std::wstring> layout = {
            L"7", L"8", L"9", L"/",
            L"4", L"5", L"6", L"*",
            L"1", L"2", L"3", L"-",
            L"0", L".", L"=", L"+"
        };

        SHORT constexpr startX = 10;
        SHORT constexpr startY = 6;
        SHORT constexpr buttonWidth = 10;
        SHORT constexpr buttonHeight = 2;
        SHORT constexpr cols = 4;

        for (size_t i = 0; i < layout.size(); ++i) {
            SHORT row = i / cols;
            SHORT col = i % cols;
            SHORT left = startX + col * (buttonWidth + 1);
            SHORT top = startY + row * (buttonHeight + 1);
            SMALL_RECT r = { left, top, left + buttonWidth, top + buttonHeight };

            auto btn = std::make_shared<FiButton>(r, layout[i]);

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

        display->updateText();
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

    CalculatorForm calc;
    calc.setup();
    calc.draw();

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
