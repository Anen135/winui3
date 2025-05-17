
#pragma once
#include <windows.h>
#include <string>
#include <vector>
#include <algorithm>
#include <memory>
#include <iostream>
#include <functional>
#include <thread>
#include <atomic>
#include <shared_mutex>
#include <mutex>

// Forward declaration
class FocusManager;

class Control {
protected:
    bool hovered = false;
public:
    SMALL_RECT rect;
    bool focused = false;
    bool hidden = false;
    Control(SMALL_RECT r);

    virtual void draw() = 0;
    virtual void onMouse(const MOUSE_EVENT_RECORD& mer);
    virtual void onKey(const KEY_EVENT_RECORD& ker) {}
    virtual void focusChanged() {}
    virtual void setFocus(bool f);

    bool isHovered(const COORD& pos);
    bool hasFocus() const { return focused; }
};

class Control;
class FocusManager {
private:
    static inline std::vector<std::shared_ptr<Control>> controls;
    static inline int focusedIndex = -1;

public:
    static void registerControl(const std::shared_ptr<Control>& ctrl) {
        controls.push_back(ctrl);
    }

    static void clearControls() {
        controls.clear();
        focusedIndex = -1;
    }

    static void focusControl(Control* ctrl) {
        if (focusedIndex != -1){
            controls[focusedIndex]->setFocus(false);
        }
        auto it = std::find_if(controls.begin(), controls.end(),
                               [ctrl](const std::shared_ptr<Control>& c) { return c.get() == ctrl; });
        if (it != controls.end()) {
            focusedIndex = std::distance(controls.begin(), it);
            controls[focusedIndex]->setFocus(true);
        }
    }

    static void nextFocus() {
        if (controls.empty()) return;
        if (focusedIndex != -1){
            controls[focusedIndex]->setFocus(false);
        }
        if ((GetKeyState(VK_SHIFT) & 0x8000) != 0) {
            focusedIndex = (focusedIndex + controls.size() - 1) % controls.size();
        } else {
            focusedIndex = (focusedIndex + 1) % controls.size();
        }
        controls[focusedIndex]->setFocus(true);
    }

    static void redrawAll() {
        for (auto& ctrl : controls) {
            ctrl->draw();
        }
    }

};


Control::Control(SMALL_RECT r)
    : rect(r) {}

bool Control::isHovered(const COORD& pos) {
    return pos.X >= rect.Left && pos.X <= rect.Right &&
           pos.Y >= rect.Top && pos.Y <= rect.Bottom;
}

void Control::onMouse(const MOUSE_EVENT_RECORD& mer) {
    bool wasHovered = hovered;
    hovered = isHovered(mer.dwMousePosition);
    if (hovered != wasHovered) {
        draw();
    }
}

void Control::setFocus(bool f) {
    if (focused == f) return;
    focused = f;
    draw();
}

template<typename T>
using HandlerPtr = std::shared_ptr<std::function<void(const T&)>>;

class EventManager {
private:
    std::thread eventThread;
    std::atomic<bool> running;

    // Шаблонный класс для хранения обработчиков и их мьютекса
    template <typename T>
    class HandlerContainer {
    private:
        std::vector<HandlerPtr<T>> handlers;
        mutable std::mutex mutex;

    public:
        // Добавление обработчика
        HandlerPtr<T> addHandler(std::function<void(const T&)> handler) {
            std::lock_guard lock(mutex); 
            auto handlerPtr = std::make_shared<std::function<void(const T&)>>(handler);
            handlers.push_back(handlerPtr);
            return handlerPtr; 
        }

        void clearHandlers() {
            std::lock_guard lock(mutex);
            handlers.clear();
        }

        // Удаление обработчика по указателю
        bool removeHandler(const HandlerPtr<T>& handlerPtr) {
            std::lock_guard lock(mutex); // Блокируем мьютекс для записи
            auto it = std::find_if(handlers.begin(), handlers.end(), [&handlerPtr](const auto& ptr) {
                return ptr == handlerPtr; // Сравниваем указатели
            });
            if (it != handlers.end()) {
                handlers.erase(it); // Удаляем обработчик
                return true; // Успешно удалено
            }
            return false; // Обработчик не найден
        }

        // Вызов всех обработчиков
        void invokeHandlers(const T& event) const {
            std::vector<HandlerPtr<T>> handlersCopy;
            {
                std::lock_guard lock(mutex);
                handlersCopy = handlers; // Копируем все обработчики при захваченном мьютексе
            }

            for (const auto& handler : handlersCopy) {
                (*handler)(event); // Вызываем без захваченного мьютекса
            }
        }

    };



    // Контейнеры для каждого типа событий winAPI, другие не нужны.
    HandlerContainer<KEY_EVENT_RECORD> keyHandlers;
    HandlerContainer<MOUSE_EVENT_RECORD> mouseHandlers;
    HandlerContainer<FOCUS_EVENT_RECORD> focusHandlers;
    HandlerContainer<MENU_EVENT_RECORD> menuHandlers;
    HandlerContainer<WINDOW_BUFFER_SIZE_RECORD> windowBufferSizeHandlers;
    HandlerContainer<INPUT_RECORD> inputHandlers; // Пользователь хочет получать все события

    EventManager() : running(false) {}
    ~EventManager() { if (running) stop();}

    // Основной цикл обработки событий
        void eventLoop() {
        HANDLE hInput = GetStdHandle(STD_INPUT_HANDLE);
        if (hInput == INVALID_HANDLE_VALUE) {
            return;
        }

        while (running) {
            INPUT_RECORD inputRecords[128];
            DWORD eventsRead;

            if (!ReadConsoleInput(hInput, inputRecords, 128, &eventsRead)) { // Если больше 128 - нам это не нужно.
                std::cerr << "Error reading console input" << std::endl; // Ошибка - прерываем поток.
                stop();
                return;
            }

            for (DWORD i = 0; i < eventsRead && running; ++i) {
                switch (inputRecords[i].EventType) {
                    case KEY_EVENT:
                        keyHandlers.invokeHandlers(inputRecords[i].Event.KeyEvent);
                        break;
                    case MOUSE_EVENT:
                        mouseHandlers.invokeHandlers(inputRecords[i].Event.MouseEvent);
                        break;
                    case FOCUS_EVENT:
                        focusHandlers.invokeHandlers(inputRecords[i].Event.FocusEvent);
                        break;
                    case MENU_EVENT:
                        menuHandlers.invokeHandlers(inputRecords[i].Event.MenuEvent);
                        break;
                    case WINDOW_BUFFER_SIZE_EVENT:
                        windowBufferSizeHandlers.invokeHandlers(inputRecords[i].Event.WindowBufferSizeEvent);
                        break;
                    default:
                        inputHandlers.invokeHandlers(inputRecords[i]);
                        break;
                }
            }
        }
    }

public:
    // Получение экземпляра менеджера событий (Singleton)
    static EventManager& getInstance() {
        static EventManager instance;
        return instance;
    }

    void clearAllHandlers() {
        keyHandlers.clearHandlers();
        mouseHandlers.clearHandlers();
        focusHandlers.clearHandlers();
        menuHandlers.clearHandlers();
        windowBufferSizeHandlers.clearHandlers();
        inputHandlers.clearHandlers();
    }

    // Добавление обработчиков для каждого типа событий
    template <typename T>
    inline HandlerPtr<T> addHandler(std::function<void(const T&)> handler) {
        if constexpr (std::is_same_v<T, KEY_EVENT_RECORD>) {
            return keyHandlers.addHandler(handler);
        } else if constexpr (std::is_same_v<T, MOUSE_EVENT_RECORD>) {
            return mouseHandlers.addHandler(handler);
        } else if constexpr (std::is_same_v<T, FOCUS_EVENT_RECORD>) {
            return focusHandlers.addHandler(handler);
        } else if constexpr (std::is_same_v<T, MENU_EVENT_RECORD>) {
            return menuHandlers.addHandler(handler);
        } else if constexpr (std::is_same_v<T, WINDOW_BUFFER_SIZE_RECORD>) {
            return windowBufferSizeHandlers.addHandler(handler);
        } else if constexpr (std::is_same_v<T, INPUT_RECORD>) {
            return inputHandlers.addHandler(handler);
        }
    }

    template <typename T>
    inline bool removeHandler(const HandlerPtr<T>& handlerPtr) {
        if constexpr (std::is_same_v<T, KEY_EVENT_RECORD>) {
            return keyHandlers.removeHandler(handlerPtr);
        } else if constexpr (std::is_same_v<T, MOUSE_EVENT_RECORD>) {
            return mouseHandlers.removeHandler(handlerPtr);
        } else if constexpr (std::is_same_v<T, FOCUS_EVENT_RECORD>) {
            return focusHandlers.removeHandler(handlerPtr);
        } else if constexpr (std::is_same_v<T, MENU_EVENT_RECORD>) {
            return menuHandlers.removeHandler(handlerPtr);
        } else if constexpr (std::is_same_v<T, WINDOW_BUFFER_SIZE_RECORD>) {
            return windowBufferSizeHandlers.removeHandler(handlerPtr);
        } else if constexpr (std::is_same_v<T, INPUT_RECORD>) {
            return inputHandlers.removeHandler(handlerPtr);
        }
    }

    template <typename T>
    inline void clearAllHandlers() {
        if constexpr (std::is_same_v<T, KEY_EVENT_RECORD>) {
            keyHandlers.clearHandlers();
        } else if constexpr (std::is_same_v<T, MOUSE_EVENT_RECORD>) {
             mouseHandlers.clearHandlers();
        } else if constexpr (std::is_same_v<T, FOCUS_EVENT_RECORD>) {
             focusHandlers.clearHandlers();
        } else if constexpr (std::is_same_v<T, MENU_EVENT_RECORD>) {
             menuHandlers.clearHandlers();
        } else if constexpr (std::is_same_v<T, WINDOW_BUFFER_SIZE_RECORD>) {
             windowBufferSizeHandlers.clearHandlers();
        } else if constexpr (std::is_same_v<T, INPUT_RECORD>) {
             inputHandlers.clearHandlers();
        }
    }

    // Запуск обработчика событий
    void start() { // Разрешаем повторный запуск.
        running = true;
        eventThread = std::thread([this]() { this->eventLoop(); });
    }

    // Остановка обработчика событий
    void stop() { // мягко прерываем поток.
        running = false;
        if (eventThread.joinable()) {
            eventThread.join();
        }
    }
};

class FocusManager {
private:
    static inline std::vector<std::shared_ptr<Control>> controls;
    static inline int focusedIndex = -1;

public:
    static void registerControl(const std::shared_ptr<Control>& ctrl) {
        controls.push_back(ctrl);
    }

    static void clearControls() {
        controls.clear();
        focusedIndex = -1;
    }

    static void focusControl(Control* ctrl) {
        if (focusedIndex != -1){
            controls[focusedIndex]->setFocus(false);
        }
        auto it = std::find_if(controls.begin(), controls.end(),
                               [ctrl](const std::shared_ptr<Control>& c) { return c.get() == ctrl; });
        if (it != controls.end()) {
            focusedIndex = std::distance(controls.begin(), it);
            controls[focusedIndex]->setFocus(true);
        }
    }

    static void nextFocus() {
        if (controls.empty()) return;
        if (focusedIndex != -1){
            controls[focusedIndex]->setFocus(false);
        }
        if ((GetKeyState(VK_SHIFT) & 0x8000) != 0) {
            focusedIndex = (focusedIndex + controls.size() - 1) % controls.size();
        } else {
            focusedIndex = (focusedIndex + 1) % controls.size();
        }
        controls[focusedIndex]->setFocus(true);
    }

    static void redrawAll() {
        for (auto& ctrl : controls) {
            ctrl->draw();
        }
    }

};


class InputState {
public:
    static bool isKeyPressed(int vkey) {
        return (GetAsyncKeyState(vkey) & 0x8000) != 0;
    }

    static bool isMouseButtonPressed(DWORD button) {
        switch (button) {
            case VK_LBUTTON: return (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
            case VK_RBUTTON: return (GetAsyncKeyState(VK_RBUTTON) & 0x8000) != 0;
            case VK_MBUTTON: return (GetAsyncKeyState(VK_MBUTTON) & 0x8000) != 0;
            default: return false;
        }
    }

    static POINT getMouseScreenPosition() {
        POINT p;
        GetCursorPos(&p);
        return p;
    }

    static COORD getMouseConsolePosition() {
        POINT p = getMouseScreenPosition();
        HWND hwnd = GetConsoleWindow();
        ScreenToClient(hwnd, &p);
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
        return { (SHORT)(p.x / 8), (SHORT)(p.y / 16) }; // NOTE: font-size dependent
    }

    static inline void setConsoleCursorPosition(COORD pos) { SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos); }
};

class Render {
public:
    inline static HANDLE hout { GetStdHandle(STD_OUTPUT_HANDLE) };
    WORD attr {FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE};
    inline static DWORD dump {0};
    static CONSOLE_SCREEN_BUFFER_INFO csbi;

    void DrawBox(SMALL_RECT& rect) {
        // Unicode Box Drawing characters
        #define hline   L"\u2500"  // ─
        #define vline   L"\u2502"  // │
        #define tl L"\u250C" // ┌
        #define tr L"\u2510" // ┐
        #define bl L"\u2514" // └
        #define br L"\u2518" // ┘
        WriteConsoleOutputCharacterW(hout, tl, 1, { rect.Left, rect.Top }, &dump);
        for (SHORT x = rect.Left + 1; x < rect.Right; x++) WriteConsoleOutputCharacterW(hout, hline, 1, { x, rect.Top }, &dump);
        WriteConsoleOutputCharacterW(hout, tr, 1, { rect.Right, rect.Top }, &dump);
        for (SHORT y = rect.Top + 1; y < rect.Bottom; y++) {
            WriteConsoleOutputCharacterW(hout, vline, 1, { rect.Left, y }, &dump);
            WriteConsoleOutputCharacterW(hout, vline, 1, { rect.Right, y }, &dump);
        }
        WriteConsoleOutputCharacterW(hout, bl, 1, { rect.Left, rect.Bottom }, &dump);
        for (SHORT x = rect.Left + 1; x < rect.Right; x++) WriteConsoleOutputCharacterW(hout, hline, 1, { x, rect.Bottom }, &dump);
        WriteConsoleOutputCharacterW(hout, br, 1, { rect.Right, rect.Bottom }, &dump);
        #undef hline
        #undef vline
        #undef tl
        #undef tr
        #undef bl
        #undef br
    }

    void fillBox(SMALL_RECT& rect) {
        for (SHORT y = rect.Top; y <= rect.Bottom; y++) {
            for (SHORT x = rect.Left; x <= rect.Right; x++) {
                FillConsoleOutputAttribute(hout, attr, 1, { x, y }, &dump);
                WriteConsoleOutputCharacterW(hout, L" ", 1, { x, y }, &dump);
            }
        }
    }

    static void clearScreen() {
        GetConsoleScreenBufferInfo(hout, &csbi);
        FillConsoleOutputAttribute(hout, csbi.wAttributes, csbi.dwSize.X * csbi.dwSize.Y, { 0, 0 }, &dump);
        FillConsoleOutputCharacter(hout, (TCHAR)' ', csbi.dwSize.X * csbi.dwSize.Y, { 0, 0 }, &dump);
        SetConsoleCursorPosition(hout, { 0, 0 });
    }

    static void updateConsoleBufferSize(COORD dsize) {
        GetConsoleScreenBufferInfo(hout, &csbi);
        SMALL_RECT sr = { 0, 0, csbi.dwSize.X + 1, csbi.dwSize.Y + 1 };
        SetConsoleScreenBufferSize(hout, dsize);
        SetConsoleWindowInfo(hout, true, &sr);
    }

  

    // Расчёт центрирования текста
    inline void drawTextCentered(const std::wstring& text, const SMALL_RECT& rect) { 
        if (rect.Right - rect.Left < text.size()) WriteConsoleOutputCharacterW(hout, L"...", 3, { static_cast<SHORT>(rect.Left + (rect.Right - rect.Left + 1 - 3) / 2), static_cast<SHORT>((rect.Top + rect.Bottom) / 2) }, &dump);
        else WriteConsoleOutputCharacterW(hout, text.c_str(), text.size(), { static_cast<SHORT>(rect.Left + ((rect.Right - rect.Left + 1 - text.size()) / 2)), static_cast<SHORT>((rect.Top + rect.Bottom) / 2) }, &dump); 
    }
    inline void drawTextLeft(const std::wstring& text, const SMALL_RECT& rect) { 
        if (rect.Right - rect.Left < text.size()) WriteConsoleOutputCharacterW(hout, L"...", 3, { static_cast<SHORT>(rect.Left + (rect.Right - rect.Left + 1 - 3) / 2), static_cast<SHORT>((rect.Top + rect.Bottom) / 2) }, &dump);
        else WriteConsoleOutputCharacterW(hout, text.c_str(), text.size(), { static_cast<SHORT>(rect.Left + 1), static_cast<SHORT>((rect.Top + rect.Bottom) / 2) }, &dump); 
    }
};

CONSOLE_SCREEN_BUFFER_INFO Render::csbi; 