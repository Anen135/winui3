#pragma once
#include <thread>
#include <atomic>
#include <vector>
#include <functional>
#include <Windows.h>
#include <shared_mutex>
#include <mutex>
#include <algorithm>

template<typename T>
using HandlerPtr = std::shared_ptr<std::function<void(const T&)>>; // Это нужно не для контроля памяти, а для сравнения через ==.

template <typename T>
class HandlerContainer {
private:
    std::vector<HandlerPtr<T>> handlers;
    mutable std::shared_mutex mutex;  // shared_mutex позволяет shared/unique lock-и

public:
    // Добавление обработчика (требует unique lock)
    HandlerPtr<T> addHandler(std::function<void(const T&)> handler) {
        std::unique_lock lock(mutex);  // unique_lock на запись
        auto handlerPtr = std::make_shared<std::function<void(const T&)>>(handler);
        handlers.push_back(handlerPtr);
        return handlerPtr;
    }

    // Очистка всех обработчиков (требует unique lock)
    void clearHandlers() {
        std::unique_lock lock(mutex);
        handlers.clear();
    }

    // Удаление обработчика по указателю (требует unique lock)
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

    // Вызов всех обработчиков (можно с shared_lock)
    void invokeHandlers(const T& event) const {
        std::vector<HandlerPtr<T>> handlersCopy;
        {
            std::shared_lock lock(mutex);  // shared_lock на чтение
            handlersCopy = handlers;  // копируем вектор под shared_lock
        }

        // Теперь вызываем без удержания мьютекса
        for (const auto& handler : handlersCopy) {
            (*handler)(event);
        }
    }
};
