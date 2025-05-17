#pragma once
#include <thread>
#include <atomic>
#include <vector>
#include <functional>
#include <Windows.h>
#include <shared_mutex>
#include <mutex>
#include <algorithm>
#include "Control.h"


template<typename T>
using HandlerPtr = std::shared_ptr<std::function<void(const T&)>>; // Это нужно не для контроля памяти, а для сравнения через ==.

class EventManager {
EventManager() = delete;
private:

    struct CleanupHelper {
        ~CleanupHelper() { 
            EventManager::stop(); 
            EventManager::clearAllHandlers();
        }
    };
    // Шаблонный класс для хранения обработчиков и их мьютекса
    template <typename T>
    class HandlerContainer {
    private:
        std::vector<HandlerPtr<T>> handlers;
        mutable std::shared_mutex mutex;

    public:
        // Добавление обработчика
        HandlerPtr<T> addHandler(std::function<void(const T&)> handler) {
            std::unique_lock lock(mutex); 
            auto handlerPtr = std::make_shared<std::function<void(const T&)>>(handler);
            handlers.push_back(handlerPtr);
            return handlerPtr; 
        }

        void clearHandlers() {
            std::unique_lock lock(mutex);
            handlers.clear();
        }

        // Удаление обработчика по указателю
        bool removeHandler(const HandlerPtr<T>& handlerPtr) {
            std::unique_lock lock(mutex); 
            auto it = std::find_if(handlers.begin(), handlers.end(), [&handlerPtr](const auto& ptr) {
                return ptr == handlerPtr; 
            });
            if (it != handlers.end()) {
                handlers.erase(it); 
                return true;
            }
            return false;
        }

        // Вызов всех обработчиков
        void invokeHandlers(const T& event) const {
            std::shared_lock lock(mutex);
            for (const auto& handler : handlers) {
                (*handler)(event); // Вызываем без захваченного мьютекса
            }
        }
    };

private:
    static std::thread eventThread;
    static std::atomic<bool> running;

    // Контейнеры для каждого типа событий winAPI, другие не нужны.
    static HandlerContainer<KEY_EVENT_RECORD> keyHandlers;
    static HandlerContainer<MOUSE_EVENT_RECORD> mouseHandlers;
    static HandlerContainer<FOCUS_EVENT_RECORD> focusHandlers;
    static HandlerContainer<MENU_EVENT_RECORD> menuHandlers;
    static HandlerContainer<WINDOW_BUFFER_SIZE_RECORD> windowBufferSizeHandlers;
    static HandlerContainer<INPUT_RECORD> inputHandlers; // Пользователь хочет получать все события

    // Основной цикл обработки событий
    static void eventLoop() {
        HANDLE hInput = GetStdHandle(STD_INPUT_HANDLE);
        if (hInput == INVALID_HANDLE_VALUE) {
            return;
        }

        while (running) {
            INPUT_RECORD inputRecords[128];
            DWORD eventsRead;

            if (!ReadConsoleInput(hInput, inputRecords, 128, &eventsRead)) { // Если больше 128 - нам это не нужно.
                std::cerr << "Error reading console input" << std::endl; // Ошибка - прерываем поток.
                EventManager::stop();
                return;
            }

            for (DWORD i = 0; i < eventsRead && running; ++i) {
                try {
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
                } catch (const std::exception& e) {
                    std::cerr << "Error: " << e.what() << std::endl;
                    EventManager::stop();
                    return; // Прерываем поток при ошибке.
                } catch (...) {
                    std::cerr << "Unknown error" << std::endl;
                    EventManager::stop();
                    return;
                }
            }
        }
    }

public:
    static void clearAllHandlers() {
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
    static void start() { 
        if (running) return; // Запрещаем повторное запуск
        running = true;
        eventThread = std::thread(EventManager::eventLoop);
    }

    // Остановка обработчика событий
    static void stop() { // мягко прерываем поток.
        running = false;
        if (eventThread.joinable()) {
            eventThread.join();
        }
    }
};

// Определение статических членов
std::thread EventManager::eventThread;
std::atomic<bool> EventManager::running = false;
EventManager::CleanupHelper EventManager::cleanupHelperInstance;
EventManager::HandlerContainer<KEY_EVENT_RECORD> EventManager::keyHandlers;
EventManager::HandlerContainer<MOUSE_EVENT_RECORD> EventManager::mouseHandlers;
EventManager::HandlerContainer<FOCUS_EVENT_RECORD> EventManager::focusHandlers;
EventManager::HandlerContainer<MENU_EVENT_RECORD> EventManager::menuHandlers;
EventManager::HandlerContainer<WINDOW_BUFFER_SIZE_RECORD> EventManager::windowBufferSizeHandlers;
EventManager::HandlerContainer<INPUT_RECORD> EventManager::inputHandlers;
