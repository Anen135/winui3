# Architecture

WinUI3 follows a component-based architecture with a clear separation between controls, rendering, event handling, and focus management.

## Class Diagram

```
┌─────────────────────────────────────────────────────────────────┐
│                         Control (abstract)                      │
├─────────────────────────────────────────────────────────────────┤
│ - rect: SMALL_RECT                                              │
│ - focused: bool                                                 │
│ - hovered: bool                                                 │
│ - hidden: bool                                                  │
├─────────────────────────────────────────────────────────────────┤
│ + draw(): virtual void = 0                                      │
│ + onMouse(MOUSE_EVENT_RECORD): virtual void                     │
│ + onKey(KEY_EVENT_RECORD): virtual void                         │
│ + action(): virtual void                                        │
│ + setFocus(bool): virtual void                                  │
│ + isHovered(COORD): bool                                        │
└───────────────────────────┬─────────────────────────────────────┘
                            │
          ┌─────────────────┼─────────────────┐
          ▼                 ▼                 ▼
    ┌───────────┐    ┌───────────┐    ┌───────────┐
    │  Button   │    │  TextBox  │    │  Label    │
    └───────────┘    └───────────┘    └───────────┘
```

## Core Classes

### Control

Base class for all UI elements. Provides positioning, focus tracking, and hover detection.

**Header:** `Core/Control.h`

**Properties:**
| Property | Type | Description |
|----------|------|-------------|
| `rect` | SMALL_RECT | Position and size (Left, Top, Right, Bottom) |
| `focused` | bool | Whether the control has keyboard focus |
| `hovered` | bool | Whether the mouse is over the control |
| `hidden` | bool | Whether the control is visible |

**Methods:**

```cpp
// Constructor
Control(SMALL_RECT r);

// Draw the control (must be implemented by subclasses)
virtual void draw() = 0;

// Handle mouse events
virtual void onMouse(const MOUSE_EVENT_RECORD& mer);

// Handle keyboard events
virtual void onKey(const KEY_EVENT_RECORD& ker);

// Perform the control's action (e.g., button click)
virtual void action();

// Called when focus changes
virtual void focusChanged();

// Set focus state
virtual void setFocus(bool f);

// Check if position is hovered
bool isHovered(const COORD& pos);

// Check if control has focus
bool hasFocus() const;
```

---

### Render

Static utilities for console rendering. Provides box drawing, text rendering, and screen clearing.

**Header:** `Core/Render.h`

**Static Members:**
| Member | Type | Description |
|--------|------|-------------|
| `hout` | HANDLE | Console output handle |
| `attr` | WORD | Current text attribute (color) |
| `csbi` | CONSOLE_SCREEN_BUFFER_INFO | Console screen buffer info |

**Static Methods:**

```cpp
// Draw a box with Unicode border characters
void DrawBox(SMALL_RECT& rect);

// Fill a rectangular area with current attribute
void fillBox(SMALL_RECT& rect);

// Clear the entire screen
static void clearScreen();

// Update console buffer size
static void updateConsoleBufferSize(COORD dsize);

// Draw text centered in a rectangle
inline void drawTextCentered(const std::wstring& text, const SMALL_RECT& rect);

// Draw text left-aligned in a rectangle
inline void drawTextLeft(const std::wstring& text, const SMALL_RECT& rect);
```

**Color Attributes:**
Use Windows console attributes combined with bitwise OR:
- `FOREGROUND_RED`, `FOREGROUND_GREEN`, `FOREGROUND_BLUE`
- `FOREGROUND_INTENSITY`
- `BACKGROUND_RED`, `BACKGROUND_GREEN`, `BACKGROUND_BLUE`
- `BACKGROUND_INTENSITY`

Example:
```cpp
Render::attr = BACKGROUND_GREEN | FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
Render::fillBox(rect);
```

---

### FocusManager

Manages focus navigation between controls. Static singleton that tracks all registered controls.

**Header:** `Core/FocusManager.h`

**Static Methods:**

```cpp
// Register a control for focus management
static void registerControl(const std::shared_ptr<Control>& ctrl);

// Clear all registered controls
static void clearControls();

// Set focus to a specific control
static void focusControl(Control* ctrl);

// Move focus to next control (Tab)
static void nextFocus();

// Move focus to previous control (Shift+Tab)
static void prevFocus();

// Redraw all registered controls
static void redrawAll();

// Get the currently focused control
static std::shared_ptr<Control> getFocused();
```

**Usage:**
```cpp
// Register controls
FocusManager::registerControl(myButton);
FocusManager::registerControl(myTextBox);

// Navigate focus
FocusManager::nextFocus();   // Tab key
FocusManager::prevFocus();   // Shift+Tab

// Get focused control
auto focused = FocusManager::getFocused();
focused->action();  // Execute action
```

---

### EventManager

Singleton event processor that handles console input events in a separate thread.

**Header:** `Core/EventManager.h`

**Template Type:** `HandlerPtr<T> = std::shared_ptr<std::function<void(const T&)>>`

**Methods:**

```cpp
// Get singleton instance
static EventManager& getInstance();

// Add event handler - returns handler pointer for removal
template<typename T>
HandlerPtr<T> addHandler(std::function<void(const T&)> handler);

// Remove event handler
template<typename T>
bool removeHandler(const HandlerPtr<T>& handlerPtr);

// Clear all handlers of a specific type
template<typename T>
void clearAllHandlers();

// Start event processing thread
void start();

// Stop event processing thread
void stop();
```

**Supported Event Types:**
- `KEY_EVENT_RECORD` - Keyboard events
- `MOUSE_EVENT_RECORD` - Mouse events
- `FOCUS_EVENT_RECORD` - Focus events
- `MENU_EVENT_RECORD` - Menu events
- `WINDOW_BUFFER_SIZE_RECORD` - Window resize events

**Usage:**
```cpp
auto& eventManager = EventManager::getInstance();

// Add keyboard handler
auto keyHandler = eventManager.addHandler<KEY_EVENT_RECORD>([](const KEY_EVENT_RECORD& ker) {
    if (ker.bKeyDown && ker.wVirtualKeyCode == VK_TAB) {
        FocusManager::nextFocus();
    }
});

// Add mouse handler
auto mouseHandler = eventManager.addHandler<MOUSE_EVENT_RECORD>([](const MOUSE_EVENT_RECORD& mer) {
    // Handle mouse events
});

// Start processing
eventManager.start();

// Later: remove handler
eventManager.removeHandler(keyHandler);
```

---

### InputState

Utility class for checking keyboard state.

**Header:** `Core/InputState.h`

**Methods:**

```cpp
// Check if a virtual key is pressed
static bool isKeyPressed(int virtualKeyCode);

// Set console cursor position
static void setConsoleCursorPosition(COORD pos);
```

**Usage:**
```cpp
// Check if ESC is pressed
if (InputState::isKeyPressed(VK_ESCAPE)) {
    // Exit application
}

// Move cursor
InputState::setConsoleCursorPosition({0, 0});
```

---

## Event Flow

```
┌──────────────┐     ┌───────────────┐     ┌────────────────┐
│  Console     │────▶│  EventManager │────▶│  Handlers      │
│  Input       │     │  (thread)     │     │  (callbacks)   │
└──────────────┘     └───────────────┘     └────────────────┘
                                                      │
                                                      ▼
                                             ┌────────────────┐
                                             │  FocusManager  │
                                             │  updates       │
                                             └────────────────┘
                                                      │
                                                      ▼
                                             ┌────────────────┐
                                             │  Control::draw │
                                             │  rerenders     │
                                             └────────────────┘
```

## Initialization Sequence

1. Create control instances with positions
2. Register controls with `FocusManager::registerControl()`
3. Set initial focus with `FocusManager::nextFocus()` or `focusControl()`
4. Redraw all controls with `FocusManager::redrawAll()`
5. Configure console mode for input
6. Add event handlers
7. Start event manager
8. Enter main loop