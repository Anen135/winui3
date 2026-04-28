# Getting Started

This guide will help you set up WinUI3 and create your first console UI application.

## Prerequisites

- Windows 10/11
- C++17 compatible compiler (Visual Studio, MinGW, etc.)
- Windows SDK

## Installation

WinUI3 is a header-only library. Simply include the required headers in your project:

```
winui3/
├── Core/
│   ├── Control.h
│   ├── Render.h
│   ├── EventManager.h
│   ├── FocusManager.h
│   └── InputState.h
├── BasicElements/
│   ├── Button.h
│   ├── TextBox.h
│   └── ...
└── src/
    └── your_app.cpp
```

### Include Paths

Add the following to your compiler include paths:
- `Core/` directory
- `BasicElements/` directory

## Minimal Application

Here's the simplest WinUI3 application:

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
#include "../BasicElements/Button.h"

// Keyboard handler for navigation
void KeyHandler(const KEY_EVENT_RECORD& ker) {
    if (ker.bKeyDown && ker.wVirtualKeyCode == VK_TAB) {
        FocusManager::nextFocus();  // Move to next control
    }
    else if (ker.bKeyDown && ker.wVirtualKeyCode == VK_SPACE) {
        FocusManager::getFocused()->action();  // Activate control
    }
}

int main() {
    HANDLE hin = GetStdHandle(STD_INPUT_HANDLE);

    // Create and register controls
    auto button = std::make_shared<Button>(
        SMALL_RECT{10, 5, 30, 7}, 
        L"Click Me"
    );
    
    // Override action (or use FIButton with callback)
    button->action = []() {
        MessageBoxW(NULL, L"Hello!", L"Clicked", MB_OK);
    };

    FocusManager::registerControl(button);
    FocusManager::nextFocus();  // Set initial focus
    FocusManager::redrawAll();  // Draw all controls

    // Configure console for input
    DWORD mode;
    GetConsoleMode(hin, &mode);
    SetConsoleMode(hin, ENABLE_EXTENDED_FLAGS | ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT);

    // Setup event handling
    auto& eventManager = EventManager::getInstance();
    eventManager.addHandler<KEY_EVENT_RECORD>(KeyHandler);
    eventManager.start();

    // Main loop - exit on ESC
    std::cout << " [Press ESC to exit...] " << std::endl;
    while (!InputState::isKeyPressed(VK_ESCAPE)) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    // Cleanup
    eventManager.stop();
    SetConsoleMode(hin, mode);
    return 0;
}
```

## Step-by-Step Setup

### 1. Console Configuration

WinUI3 requires specific console modes to receive mouse and keyboard events:

```cpp
HANDLE hin = GetStdHandle(STD_INPUT_HANDLE);
DWORD mode;
GetConsoleMode(hin, &mode);
SetConsoleMode(hin, ENABLE_EXTENDED_FLAGS | ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT);
```

Save the original mode to restore it on exit:
```cpp
SetConsoleMode(hin, mode);  // At program end
```

### 2. Creating Controls

All controls are created using `std::make_shared` and positioned with `SMALL_RECT`:

```cpp
// SMALL_RECT{left, top, right, bottom}
auto button = std::make_shared<FIButton>(
    SMALL_RECT{10, 5, 30, 7},  // Position: x=10, y=5, width=20, height=2
    L"Button Text"
);
```

**Coordinate System:**
- `Left`, `Right`: Horizontal positions (column)
- `Top`, `Bottom`: Vertical positions (row)

### 3. Registering Controls

Controls must be registered with FocusManager for keyboard navigation:

```cpp
FocusManager::registerControl(myButton);
FocusManager::registerControl(myTextBox);
FocusManager::registerControl(myCheckbox);
```

### 4. Setting Initial Focus

```cpp
FocusManager::nextFocus();  // Set focus to first registered control
FocusManager::redrawAll();  // Draw all controls
```

### 5. Event Handlers

Add handlers for keyboard events:

```cpp
auto& eventManager = EventManager::getInstance();

// Handle Tab for navigation
eventManager.addHandler<KEY_EVENT_RECORD>([](const KEY_EVENT_RECORD& ker) {
    if (ker.bKeyDown && ker.wVirtualKeyCode == VK_TAB) {
        FocusManager::nextFocus();
    }
});

// Handle Enter/Space for activation
eventManager.addHandler<KEY_EVENT_RECORD>([](const KEY_EVENT_RECORD& ker) {
    if (ker.bKeyDown && ker.wVirtualKeyCode == VK_SPACE) {
        FocusManager::getFocused()->action();
    }
});

// Start event processing
eventManager.start();
```

### 6. Main Loop

Keep the application running while checking for exit conditions:

```cpp
while (!InputState::isKeyPressed(VK_ESCAPE)) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}
```

## Complete Example: Login Form

Here's a more complete example showing a login form:

```cpp
#include <windows.h>
#include <iostream>
#include <memory>
#include "EventManager.h"
#include "InputState.h"
#include "FocusManager.h"
#include "Control.h"
#include "Render.h"
#include "../BasicElements/FiButton.h"
#include "../BasicElements/TextBox.h"
#include "../BasicElements/CheckBox.h"

class LoginForm {
    std::shared_ptr<TextBox> loginBox;
    std::shared_ptr<TextBox> passwordBox;
    std::shared_ptr<CheckBox> rememberMeBox;
    std::shared_ptr<FIButton> loginButton;

public:
    void setup() {
        // Create controls
        loginBox = std::make_shared<TextBox>(SMALL_RECT{10, 4, 50, 6}, L"Login");
        passwordBox = std::make_shared<TextBox>(SMALL_RECT{10, 8, 50, 10}, L"Password");
        rememberMeBox = std::make_shared<CheckBox>(SMALL_RECT{10, 12, 50, 14}, L"Remember Me");
        loginButton = std::make_shared<FIButton>(SMALL_RECT{20, 16, 40, 18}, L"Login");

        // Set up button callback
        loginButton->onClick = [this]() {
            validate();
        };

        // Register controls
        FocusManager::registerControl(loginBox);
        FocusManager::registerControl(passwordBox);
        FocusManager::registerControl(rememberMeBox);
        FocusManager::registerControl(loginButton);
    }

    void draw() {
        FocusManager::redrawAll();
    }

    void validate() {
        std::wstring login = loginBox->text;
        std::wstring password = passwordBox->text;

        if (login == L"admin" && password == L"1234") {
            MessageBoxW(NULL, 
                rememberMeBox->checked ? L"Welcome, admin! (Remembered)" : L"Welcome, admin!", 
                L"Success", MB_OK);
        } else {
            MessageBoxW(NULL, L"Invalid login or password.", L"Error", MB_ICONERROR);
        }
    }
};

int main() {
    HANDLE hin = GetStdHandle(STD_INPUT_HANDLE);
    
    // Configure console
    DWORD mode;
    GetConsoleMode(hin, &mode);
    SetConsoleMode(hin, ENABLE_EXTENDED_FLAGS | ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT);

    // Setup form
    LoginForm form;
    form.setup();
    form.draw();

    // Event handlers
    auto& eventManager = EventManager::getInstance();
    eventManager.addHandler<KEY_EVENT_RECORD>([](const KEY_EVENT_RECORD& ker) {
        if (ker.bKeyDown && ker.wVirtualKeyCode == VK_TAB) {
            FocusManager::nextFocus();
        }
    });
    eventManager.start();

    // Main loop
    while (!InputState::isKeyPressed(VK_ESCAPE)) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    SetConsoleMode(hin, mode);
    return 0;
}
```

## Building

### Visual Studio

1. Create a new C++ Console Application
2. Add include paths: `Core;BasicElements`
3. Add `#define UNICODE` at the top
4. Link against `kernel32.lib` (automatic with Windows.h)

### CMake

```cmake
cmake_minimum_required(VERSION 3.15)
project(MyApp)

set(CMAKE_CXX_STANDARD 17)

include_directories(${CMAKE_SOURCE_DIR}/Core 
                    ${CMAKE_SOURCE_DIR}/BasicElements)

add_executable(MyApp main.cpp)
```

### Compile

```bash
cl /EHsc /W4 main.cpp
```

## Keyboard Shortcuts

| Key | Action |
|-----|--------|
| Tab | Next control |
| Shift+Tab | Previous control |
| Space | Activate (button click, toggle) |
| Enter | Submit (in TextBox) |
| Backspace | Delete character (in TextBox) |
| ESC | Exit application |

## Next Steps

- [Architecture](architecture.md) - Learn about core classes
- [Basic Elements](basic-elements.md) - Explore all UI components
- [Examples](examples.md) - See working demo applications