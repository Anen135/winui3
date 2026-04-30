#pragma once
#include "EventManager.h"
#include "setup.h"
#include <memory>
#include <vector>
#include <functional>
#include <atomic>
#include <thread>
#include "..\Core\Control.h"
#include "..\Core\Render.h"

// ------------------ Application ------------------
// Main application class that manages the event loop and window
class Application {
public:
    using RunCallback = std::function<void()>;
    using ExitCallback = std::function<int()>;

private:
    static inline Application* instance = nullptr;

    std::wstring title;
    bool running = false;
    bool initialized = false;

    std::vector<std::shared_ptr<Control>> rootControls;
    std::shared_ptr<Control> focusedControl;

    RunCallback onStartup;
    RunCallback onShutdown;
    ExitCallback onExit;

    COORD consoleSize;

public:
    Application() = default;
    Application(const std::wstring& t) : title(t) {}
    ~Application() { stop(); }

    // Singleton access
    static Application* getInstance() { return instance; }
    static void setInstance(Application* app) { instance = app; }

    // Title
    const std::wstring& getTitle() const { return title; }
    void setTitle(const std::wstring& t) { title = t; }

    // Console size
    COORD getConsoleSize() const { return consoleSize; }
    void setConsoleSize(COORD size) { consoleSize = size; }

    // Initialization
    bool isInitialized() const { return initialized; }

    bool init() {
        if (initialized) return true;

        // Setup console
        #ifdef _WIN32
       #ifdef _WIN32
        coninit();
#endif
        #endif

        // Get initial console size
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        GetConsoleScreenBufferInfo(Render::hout, &csbi);
        consoleSize = csbi.dwSize;

        initialized = true;
        return true;
    }

    // Root controls management
    void addRootControl(const std::shared_ptr<Control>& ctrl) {
        rootControls.push_back(ctrl);
    }

    void removeRootControl(const std::shared_ptr<Control>& ctrl) {
        auto it = std::find(rootControls.begin(), rootControls.end(), ctrl);
        if (it != rootControls.end()) {
            rootControls.erase(it);
        }
    }

    const std::vector<std::shared_ptr<Control>>& getRootControls() const {
        return rootControls;
    }

    // Focus management
    void setFocusedControl(const std::shared_ptr<Control>& ctrl) {
        if (focusedControl) {
            focusedControl->setFocus(false);
        }
        focusedControl = ctrl;
        if (focusedControl) {
            focusedControl->setFocus(true);
        }
    }

    std::shared_ptr<Control> getFocusedControl() const {
        return focusedControl;
    }

    // Callbacks
    void onStartupCallback(RunCallback cb) { onStartup = cb; }
    void onShutdownCallback(RunCallback cb) { onShutdown = cb; }
    void onExitCallback(ExitCallback cb) { onExit = cb; }

    // Run application
    void run() {
        if (running) return;
        running = true;

        init();

        // Call startup callback
        if (onStartup) {
            onStartup();
        }

        // Start event manager
        auto& events = EventManager::getInstance();
        events.start();

        // Main render loop - simplified for now
        while (running) {
            // Draw all root controls
            for (auto& ctrl : rootControls) {
                if (!ctrl->hidden) {
                    ctrl->draw();
                }
            }

            // Check for exit condition
            if (onExit && onExit()) {
                running = false;
            }

            // Small delay to prevent busy loop
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }

        stop();
    }

    // Stop application
    void stop() {
        running = false;

        auto& events = EventManager::getInstance();
        events.stop();

        // Call shutdown callback
        if (onShutdown) {
            onShutdown();
        }
    }

    // Exit request
    void exit() {
        running = false;
    }

    bool isRunning() const { return running; }

    // Redraw all
    void redraw() {
        Render::clearScreen();
        for (auto& ctrl : rootControls) {
            if (!ctrl->hidden) {
                ctrl->draw();
            }
        }
    }
};