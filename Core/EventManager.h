#pragma once
#include <thread>
#include <atomic>
#include <vector>
#include <functional>
#include <Windows.h>
#include <algorithm>
#include "HandlerContainerShared.h"

template<typename T>
using HandlerPtr = std::shared_ptr<std::function<void(const T&)>>;

class EventManager {
private:
    std::thread eventThread;
    std::atomic<bool> running;
    static EventManager instance;


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
        if (hInput == INVALID_HANDLE_VALUE) return;

        // Цикл обработки событий
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
                }
                // inputHandlers.invokeHandlers(inputRecords[i]);
            }
        }
    }

public:
    // Получение экземпляра менеджера событий (Singleton)
    static EventManager& getInstance() {
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

EventManager EventManager::instance;