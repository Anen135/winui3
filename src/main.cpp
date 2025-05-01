// Console File Explorer

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
#include "Render.h"
#include "Label.h"

namespace fs = std::filesystem;
std::wstring currentPath = fs::absolute(L".").wstring();
void loadDirectory(const std::wstring& path);
HANDLE hin, hout;
class FileButton;
std::vector<std::shared_ptr<FileButton>> allButtons;
int currentPage = 0;
int maxButtonsPerPage = 0;
constexpr SHORT buttonHeight = 3;
std::shared_ptr<Label> currentPathLabel = std::make_shared<Label>(SMALL_RECT{60, 2, 110, 5}, L"0");
std::shared_ptr<Label> helpLabel = std::make_shared<Label>(SMALL_RECT{60, 7, 110, 10}, L"[Press ESC to exit...]", 3);
std::shared_ptr<Label> pageLabel = std::make_shared<Label>(SMALL_RECT{60, 10, 110, 13}, L"0", 3);
std::shared_ptr<Label> pageHelpLabel = std::make_shared<Label>(SMALL_RECT{60, 13, 110, 15}, L"[Use F9/F10 to change page]", 2);

class FileButton : public Control, public Render, public std::enable_shared_from_this<FileButton>  {
HandlerPtr<MOUSE_EVENT_RECORD> handler;
public:
    std::wstring name;
    uint8_t type = 0;

    void initHandlers() {
        auto self = shared_from_this();
        if (handler) EventManager::getInstance().removeHandler<MOUSE_EVENT_RECORD>(handler);
        handler = EventManager::getInstance().addHandler<MOUSE_EVENT_RECORD>([self](const MOUSE_EVENT_RECORD& mer) {
            self->onMouse(mer);
        });
    }

    void initBackHandler() {
        auto self = shared_from_this();
        if (handler) EventManager::getInstance().removeHandler<MOUSE_EVENT_RECORD>(handler);
        handler = EventManager::getInstance().addHandler<MOUSE_EVENT_RECORD>([self](const MOUSE_EVENT_RECORD& mer) {
            self->onBack(mer);
        });
    }

    ~FileButton() {
        EventManager::getInstance().removeHandler<MOUSE_EVENT_RECORD>(handler);
    }

    FileButton(SMALL_RECT r, const std::wstring& n, uint8_t t = 0)
    : Control(r), name(n), type(t) {}


    void draw() override {
        if (focused)      Render::attr = BACKGROUND_GREEN   | FOREGROUND_RED    | FOREGROUND_BLUE  | FOREGROUND_INTENSITY;
        else if (hovered) Render::attr = BACKGROUND_BLUE    | FOREGROUND_RED    | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
        else              Render::attr = FOREGROUND_RED     | FOREGROUND_GREEN  | FOREGROUND_BLUE;
        Render::fillBox(rect);
        Render::DrawBox(rect);
        Render::drawTextLeft(name, rect);
    }

    void onMouse(const MOUSE_EVENT_RECORD& mer) override {
        Control::onMouse(mer);
        if (hovered && (mer.dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED)) {
            FocusManager::focusControl(this);
            if (type == 2) {
                currentPath = fs::absolute(fs::path(currentPath).parent_path()).wstring();
                loadDirectory(currentPath);
            } else if (type) {
                currentPath = fs::absolute(fs::path(currentPath) / name).wstring();
                loadDirectory(currentPath);
            } else {
                ShellExecuteW(NULL, L"open", (fs::path(currentPath) / name).c_str(), NULL, NULL, SW_SHOW);
            }
        }
    }

    void onBack(const MOUSE_EVENT_RECORD& mer) {
        Control::onMouse(mer);
        if (hovered && (mer.dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED)) {
            FocusManager::focusControl(this);
            
        }
    }
};

void redrawCurrentPage() {
    // Считаем размер консоли
    GetConsoleScreenBufferInfo(hout, &Render::csbi);
    maxButtonsPerPage = (Render::csbi.dwSize.Y - 5) / buttonHeight;

    Render::clearScreen();
    FocusManager::clearControls();  
    EventManager::getInstance().clearHandlers<MOUSE_EVENT_RECORD>(); 

    int start = currentPage * maxButtonsPerPage;
    int end = std::min(start + maxButtonsPerPage, (int)allButtons.size());

    SHORT top = 2;
    for (int i = start; i < end; ++i) {
        allButtons[i]->rect = SMALL_RECT{5, top, 50, top + 2};
        allButtons[i]->initHandlers();  
        FocusManager::registerControl(allButtons[i]);  
        top += buttonHeight;
    }
    currentPathLabel->text = currentPath;
    pageLabel->text = std::to_wstring(currentPage) + L" / " + std::to_wstring(allButtons.size() / maxButtonsPerPage);
    FocusManager::registerControl(currentPathLabel);
    FocusManager::registerControl(pageHelpLabel);
    FocusManager::registerControl(helpLabel);
    FocusManager::registerControl(pageLabel);
    FocusManager::nextFocus();  // Переключить фокус на первую кнопку
    FocusManager::redrawAll();  // Перерисовать все элементы управления
}  

void loadDirectory(const std::wstring& path) {
    if (!fs::is_directory(path)) return;

    // Создаем кнопки
    allButtons.clear();  // Очистка всех старых кнопок
    if (fs::path(path) != fs::path(path).root_path()) allButtons.push_back(std::make_shared<FileButton>(SMALL_RECT{5, 0, 50, 0}, fs::path(path).parent_path().filename().wstring(), 2));
    for (const auto& entry : fs::directory_iterator(path)) allButtons.push_back(std::make_shared<FileButton>(SMALL_RECT{5, 0, 50, 0}, entry.path().filename().wstring(), entry.is_directory()));
    
    currentPage = 0;     // Сброс текущей страницы
    redrawCurrentPage();  // Обновить экран после загрузки новых кнопок
}

void KeyHandler(const KEY_EVENT_RECORD& ker) {
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

void WindowHandler(const WINDOW_BUFFER_SIZE_RECORD& wbsr) {
    CONSOLE_SCREEN_BUFFER_INFO csbi = Render::csbi;
    GetConsoleScreenBufferInfo(hout, &Render::csbi);
    if (csbi.dwSize.X == Render::csbi.dwSize.X || csbi.dwSize.Y == Render::csbi.dwSize.Y) return;
    Render::csbi = csbi;
    redrawCurrentPage();
}

int main() {
    hin  = GetStdHandle(STD_INPUT_HANDLE);
    hout = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode;

    GetConsoleMode(hin, &mode);
    SetConsoleMode(hin, ENABLE_EXTENDED_FLAGS | ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT | ENABLE_PROCESSED_INPUT);

    auto& eventManager = EventManager::getInstance();
    eventManager.addHandler<KEY_EVENT_RECORD>(KeyHandler);
    eventManager.addHandler<WINDOW_BUFFER_SIZE_RECORD>(WindowHandler);

    loadDirectory(currentPath);

    eventManager.start();

    while (!InputState::isKeyPressed(VK_ESCAPE)) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    return 0;
}