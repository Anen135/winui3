// Console File Explorer (исправленная версия)

#include <windows.h>
#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <functional>
#include <map>
#include <typeindex>
#include <filesystem>
#include <algorithm>
#include <thread>
#include <shellapi.h>

#include "EventManager.h"
#include "InputState.h"
#include "FocusManager.h"
#include "Control.h"

namespace fs = std::filesystem;

class Render {
public:
    WORD attr {FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE};
    HANDLE hout;
    DWORD written;

    Render() : hout(GetStdHandle(STD_OUTPUT_HANDLE)) {}

    void DrawBox(SMALL_RECT rect) {
        COORD pos;
        WCHAR hline = L'─';
        WCHAR vline = L'│';
        WCHAR tl = L'┌';
        WCHAR tr = L'┐';
        WCHAR bl = L'└';
        WCHAR br = L'┘';

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

class FileButton : public Control, public Render, public std::enable_shared_from_this<FileButton>  {
public:
    std::wstring name;
    bool isDir;

    void initHandlers() {
        auto self = shared_from_this();
        EventManager::getInstance().addHandler<MOUSE_EVENT_RECORD>([self](const MOUSE_EVENT_RECORD& mer) {
            self->onMouse(mer);
        });
    }


    FileButton(SMALL_RECT r, const std::wstring& n, bool dir)
    : Control(r), name(n), isDir(dir) {}


    void draw() override {
        if (focused)
            attr = BACKGROUND_GREEN | FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
        else if (hovered)
            attr = BACKGROUND_BLUE | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
        else
            attr = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;

        for (SHORT y = rect.Top; y <= rect.Bottom; y++) {
            for (SHORT x = rect.Left; x <= rect.Right; x++) {
                FillConsoleOutputAttribute(hout, attr, 1, {x, y}, &written);
                WriteConsoleOutputCharacterW(hout, L" ", 1, {x, y}, &written);
            }
        }
        DrawBox(rect);

        COORD pos = { (SHORT)(rect.Left + 1), (SHORT)((rect.Top + rect.Bottom) / 2) };
        for (size_t i = 0; i < name.size() && pos.X <= rect.Right - 1; ++i, ++pos.X) {
            WriteConsoleOutputCharacterW(hout, &name[i], 1, pos, &written);
        }
    }

    void onMouse(const MOUSE_EVENT_RECORD& mer) override;
};

HANDLE hin, hout;
std::wstring currentPath = fs::absolute(L".").wstring();


void loadDirectory(const std::wstring& path);

void FileButton::onMouse(const MOUSE_EVENT_RECORD& mer) {
    Control::onMouse(mer);
    if (mer.dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED) {
        if (hovered) {
            FocusManager::focusControl(this);
            if (name == L"..") {
                currentPath = fs::absolute(fs::path(currentPath).parent_path()).wstring();
                loadDirectory(currentPath);
            } else if (isDir) {
                currentPath = fs::absolute(fs::path(currentPath) / name).wstring();
                loadDirectory(currentPath);
            } else {
                ShellExecuteW(NULL, L"open", (fs::path(currentPath) / name).c_str(), NULL, NULL, SW_SHOW);
                // !!! currentPath не меняем для файлов !!!
            }
        }
    }
}

void clearScreen() {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    DWORD count;
    DWORD cellCount;
    COORD homeCoords = { 0, 0 };

    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hConsole == INVALID_HANDLE_VALUE) return;

    if (!GetConsoleScreenBufferInfo(hConsole, &csbi)) return;
    cellCount = csbi.dwSize.X * csbi.dwSize.Y;

    FillConsoleOutputCharacter(hConsole, (TCHAR)' ', cellCount, homeCoords, &count);
    FillConsoleOutputAttribute(hConsole, csbi.wAttributes, cellCount, homeCoords, &count);
    SetConsoleCursorPosition(hConsole, homeCoords);
}

// Новые глобальные переменные для управления страницами
std::vector<std::shared_ptr<FileButton>> allButtons;
int currentPage = 0;
int maxButtonsPerPage = 0;

void redrawCurrentPage();

void FocusChanger(const KEY_EVENT_RECORD& ker) {
    if (ker.bKeyDown) {
        if (ker.wVirtualKeyCode == VK_TAB) {
            FocusManager::nextFocus();
        } else if (ker.wVirtualKeyCode == VK_F9) { // PageUp
            if (currentPage > 0) {
                currentPage--;
                redrawCurrentPage();
            }
        } else if (ker.wVirtualKeyCode == VK_F10) { // PageDown
            int maxPage = (allButtons.size() + maxButtonsPerPage - 1) / maxButtonsPerPage - 1;
            if (currentPage < maxPage) {
                currentPage++;
                redrawCurrentPage();
            }
        }
    }
}


void loadDirectory(const std::wstring& path) {
    if (!fs::is_directory(path)) return;

    clearScreen();
    FocusManager::clearControls(); // Сброс фокуса перед добавлением новых кнопок
    EventManager::getInstance().clearHandlers<MOUSE_EVENT_RECORD>(); // Очистка старых обработчиков

    // Считаем размер консоли
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(hout, &csbi);
    SHORT screenHeight = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
    SHORT availableHeight = screenHeight - 5; // небольшой отступ
    SHORT buttonHeight = 3;
    maxButtonsPerPage = availableHeight / buttonHeight;

    allButtons.clear();  // Очистка всех старых кнопок
    currentPage = 0;     // Сброс текущей страницы

    // Создаем кнопки
    if (fs::path(path) != fs::path(path).root_path()) {
        auto btn = std::make_shared<FileButton>(SMALL_RECT{5, 0, 50, 0}, L"..", true);
        btn->initHandlers();
        allButtons.push_back(btn);
    }

    for (const auto& entry : fs::directory_iterator(path)) {
        std::wstring name = entry.path().filename().wstring();
        bool isDir = entry.is_directory();
        auto btn = std::make_shared<FileButton>(SMALL_RECT{5, 0, 50, 0}, name, isDir);
        btn->initHandlers();
        allButtons.push_back(btn);
    }

    redrawCurrentPage();  // Обновить экран после загрузки новых кнопок
}


void redrawCurrentPage() {
    clearScreen();
    FocusManager::clearControls();  // Очистить все элементы управления (кнопки)

    // Очистка старых обработчиков
    EventManager::getInstance().clearHandlers<MOUSE_EVENT_RECORD>();

    int start = currentPage * maxButtonsPerPage;
    int end = std::min(start + maxButtonsPerPage, (int)allButtons.size());

    SHORT top = 2;
    for (int i = start; i < end; ++i) {
        allButtons[i]->rect = SMALL_RECT{5, top, 50, top + 2};
        allButtons[i]->initHandlers();  // Регистрация обработчиков для текущей страницы
        FocusManager::registerControl(allButtons[i]);  // Зарегистрировать кнопку в менеджере фокуса
        top += 3;
    }

    FocusManager::nextFocus();  // Переключить фокус на первую кнопку
    FocusManager::redrawAll();  // Перерисовать все элементы управления
}   

int main() {
    hin = GetStdHandle(STD_INPUT_HANDLE);
    hout = GetStdHandle(STD_OUTPUT_HANDLE);

    DWORD mode;
    GetConsoleMode(hin, &mode);
    SetConsoleMode(hin, ENABLE_EXTENDED_FLAGS | ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT | ENABLE_PROCESSED_INPUT);

    auto& eventManager = EventManager::getInstance();
    eventManager.addHandler<KEY_EVENT_RECORD>(FocusChanger);

    loadDirectory(currentPath);

    std::thread eventThread([&eventManager]() {
        eventManager.start();
    });

    InputState::setConsoleCursorPosition({0, 0});
    std::wcout << L" [Press ESC to exit...] " << std::endl;

    while (!InputState::isKeyPressed(VK_ESCAPE)) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    // Завершение программы
    eventManager.stop();
    eventThread.join();
    EventManager::getInstance().clearHandlers<MOUSE_EVENT_RECORD>(); 
    return 0;
}