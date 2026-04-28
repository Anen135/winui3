# WinUI3 - Console UI Library for Windows

A lightweight C++ library for building console-based user interfaces on Windows. WinUI3 provides a component-based system for creating interactive UI elements with focus management, event handling, and rendering utilities.

## Features

- **Component-based UI** - Base Control class with position, focus, and hover states
- **Focus Management** - Tab/Shift+Tab navigation between controls
- **Event System** - Flexible event handlers for keyboard, mouse, and window events
- **Rendering** - Box drawing, text rendering, screen clearing utilities
- **Basic Elements** - Button, TextBox, CheckBox, Label, Container

## Quick Start

```cpp
#define UNICODE
#include <windows.h>
#include <iostream>
#include <memory>
#include "EventManager.h"
#include "InputState.h"
#include "FocusManager.h"
#include "Control.h"
#include "Render.h"
#include "../BasicElements/CFButton.h"

void KeyHandler(const KEY_EVENT_RECORD& ker) {
    if (ker.bKeyDown && ker.wVirtualKeyCode == VK_TAB) 
        FocusManager::nextFocus();
    else if (ker.bKeyDown && ker.wVirtualKeyCode == VK_SPACE) 
        FocusManager::getFocused()->action();
}

void CFButton::action() {
    MessageBoxW(NULL, L"Hello, World!", L"Success", MB_OK);
}

int main() {
    HANDLE hin = GetStdHandle(STD_INPUT_HANDLE);
    
    // Register controls
    FocusManager::registerControl(std::make_shared<CFButton>(
        SMALL_RECT{10, 2, 30, 4}, L"Click Me"));
    FocusManager::nextFocus();
    FocusManager::redrawAll();

    // Setup console mode
    DWORD mode;
    GetConsoleMode(hin, &mode);
    SetConsoleMode(hin, ENABLE_EXTENDED_FLAGS | ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT);

    // Start event loop
    auto& eventManager = EventManager::getInstance();
    eventManager.addHandler<KEY_EVENT_RECORD>(KeyHandler);
    eventManager.start();

    // Main loop - exit on ESC
    while (!InputState::isKeyPressed(VK_ESCAPE)) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    SetConsoleMode(hin, mode);
    return 0;
}
```

## Project Structure

```
winui3/
├── Core/               # Core framework classes
│   ├── Control.h       # Base control class
│   ├── Render.h        # Rendering utilities
│   ├── EventManager.h  # Event processing
│   ├── FocusManager.h  # Focus navigation
│   └── InputState.h    # Keyboard state
├── BasicElements/      # UI components
│   ├── Button.h        # Base button
│   ├── TextBox.h       # Text input
│   ├── CheckBox.h      # Toggle checkbox
│   ├── Label.h         # Text display
│   └── Container.h     # Layout container
├── src/                # Example applications
│   ├── demo.cpp        # Basic example
│   ├── demo1.cpp       # Login form
│   ├── demo2.cpp       # Calculator
│   ├── demo3.cpp       # File explorer
│   └── demo4.cpp       # Container layout
└── doc/                # Documentation
```

## Navigation

- [Getting Started](getting-started.md) - Setup and configuration
- [Architecture](architecture.md) - Core classes and relationships
- [Basic Elements](basic-elements.md) - UI component reference
- [Examples](examples.md) - Demo application walkthroughs

## Requirements

- Windows SDK
- C++17 compatible compiler
- CMake or direct include setup