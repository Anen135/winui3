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
        std::lock_guard lock(mutex); 
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
        std::vector<HandlerPtr<T>> handlersCopy;
        {
            std::lock_guard lock(mutex);
            handlersCopy = handlers; 
        }

        for (const auto& handler : handlersCopy) {
            (*handler)(event); 
        }
    }

};