# Basic Elements Reference

WinUI3 provides a set of basic UI elements for building console interfaces. Each element extends the `Control` class and implements the `Render` interface for drawing.

## Table of Contents

1. [Button](#button)
2. [CFButton](#cfbutton)
3. [FIButton](#fibutton)
4. [TextBox](#textbox)
5. [CFTextBox](#cftextbox)
6. [FITextBox](#fitextbox)
7. [CheckBox](#checkbox)
8. [Label](#label)
9. [Container](#container)

---

## Button

Base button class with visual states for default, hover, and focused.

**Header:** `BasicElements/Button.h`

**Inheritance:** `Control` + `Render`

**Constructor:**
```cpp
Button(SMALL_RECT r, const std::wstring t);
```
| Parameter | Type | Description |
|-----------|------|-------------|
| `r` | SMALL_RECT | Position and size |
| `t` | std::wstring | Button text |

**Properties:**
| Property | Type | Description |
|----------|------|-------------|
| `text` | std::wstring | Button label text |

**Visual States:**
- Default: Gray text
- Hovered: Blue background with bright text
- Focused: Green background with bright text

**Usage:**
```cpp
auto btn = std::make_shared<Button>(SMALL_RECT{10, 5, 30, 7}, L"Click Me");
btn->onClick = []() { /* handle click */ };
FocusManager::registerControl(btn);
```

---

## CFButton

Classic button with virtual `action()` method. Useful when you need to override the click behavior in a subclass.

**Header:** `BasicElements/CFButton.h`

**Inheritance:** `Button`

**Constructor:**
```cpp
CFButton(SMALL_RECT r, std::wstring t);
```

**Methods:**

```cpp
// Override to define button behavior
virtual void action() override;

// Mouse handler (inherited)
void onMouse(const MOUSE_EVENT_RECORD& mer) override;
```

**Usage:**
```cpp
// Define action in a cpp file
void CFButton::action() {
    MessageBoxW(NULL, L"Button clicked!", L"Success", MB_OK);
}

// Create and register
auto btn = std::make_shared<CFButton>(SMALL_RECT{10, 5, 30, 7}, L"Click Me");
FocusManager::registerControl(btn);
```

---

## FIButton

Functional button with `std::function` callback. More flexible than CFButton for lambda-based handlers.

**Header:** `BasicElements/FiButton.h`

**Inheritance:** `Button`

**Constructors:**
```cpp
// With callback
FIButton(SMALL_RECT r, std::wstring t, std::function<void()> onClick);

// Without callback (set later)
FIButton(SMALL_RECT r, std::wstring t);
```

**Properties:**
| Property | Type | Description |
|----------|------|-------------|
| `onClick` | std::function<void()> | Callback function called on click |

**Usage:**
```cpp
auto btn = std::make_shared<FIButton>(
    SMALL_RECT{10, 5, 30, 7}, 
    L"Login",
    []() {
        MessageBoxW(NULL, L"Logging in...", L"Info", MB_OK);
    }
);

// Or set callback later
btn->onClick = [this]() {
    this->validateForm();
};

FocusManager::registerControl(btn);
```

---

## TextBox

Text input field with keyboard input support. Handles character input, backspace, and focus management.

**Header:** `BasicElements/TextBox.h`

**Inheritance:** `Control` + `Render`

**Constructor:**
```cpp
TextBox(SMALL_RECT r, const std::wstring t);
```
| Parameter | Type | Description |
|-----------|------|-------------|
| `r` | SMALL_RECT | Position and size |
| `t` | std::wstring | Placeholder text |

**Properties:**
| Property | Type | Description |
|----------|------|-------------|
| `text` | std::wstring | Current input text |

**Visual States:**
- Default: Gray text
- Hovered: Blue background
- Focused: Green background with cursor

**Methods:**
```cpp
// Handle keyboard input
virtual void onKey(const KEY_EVENT_RECORD& ker) override;

// Subscribe/unsubscribe keyboard handlers
void subscribeKeyboard();
void unsubscribeKeyboard();

// Set focus (includes keyboard subscription)
void setFocus(bool f) override;
```

**Usage:**
```cpp
auto textBox = std::make_shared<TextBox>(SMALL_RECT{10, 5, 40, 7}, L"Enter text...");
FocusManager::registerControl(textBox);

// Get input value
std::wstring input = textBox->text;
```

---

## CFTextBox

Classic TextBox with virtual `onEnter()` method. Called when user presses Enter.

**Header:** `BasicElements/CFTextBox.h`

**Inheritance:** `TextBox`

**Constructor:**
```cpp
CFTextBox(SMALL_RECT r, std::wstring t);
```

**Methods:**

```cpp
// Called when Enter is pressed
virtual void onEnter();

// Keyboard handler
void onKey(const KEY_EVENT_RECORD& ker) override;
```

**Usage:**
```cpp
// Define handler in cpp file
void CFTextBox::onEnter() {
    if (text == L"password") {
        MessageBoxW(NULL, L"Password is valid!", L"Success", MB_OK);
    } else {
        MessageBoxW(NULL, L"Password is invalid!", L"Error", MB_ICONERROR);
    }
}

auto passwordBox = std::make_shared<CFTextBox>(
    SMALL_RECT{10, 5, 40, 7}, 
    L"Password"
);
FocusManager::registerControl(passwordBox);
```

---

## FITextBox

Functional TextBox with `std::function` callback for Enter key.

**Header:** `BasicElements/FiTextBox.h`

**Inheritance:** `TextBox`

**Constructors:**
```cpp
// With callback
FITextBox(SMALL_RECT r, std::wstring t, std::function<void(const std::wstring&)> onEnter);

// Without callback
FITextBox(SMALL_RECT r, std::wstring t);
```

**Properties:**
| Property | Type | Description |
|----------|------|-------------|
| `onEnter` | std::function<void(const std::wstring&)> | Callback with input text |

**Usage:**
```cpp
auto textBox = std::make_shared<FITextBox>(
    SMALL_RECT{10, 5, 40, 7},
    L"Enter name",
    [](const std::wstring& name) {
        MessageBoxW(NULL, (L"Hello, " + name + L"!").c_str(), L"Greeting", MB_OK);
    }
);
FocusManager::registerControl(textBox);
```

---

## CheckBox

Toggle checkbox with checked/unchecked states.

**Header:** `BasicElements/CheckBox.h`

**Inheritance:** `Control` + `Render`

**Constructor:**
```cpp
CheckBox(SMALL_RECT r, std::wstring t);
```

**Properties:**
| Property | Type | Description |
|----------|------|-------------|
| `checked` | bool | true if checked, false if unchecked |
| `text` | std::wstring | Label text |

**Visual:**
- Unchecked: `[ ] text`
- Checked: `[X] text`
- Focused: Green background
- Hovered: Blue background

**Methods:**
```cpp
// Toggle checked state
void action() override;

// Draw checkbox content
void drawContent();
```

**Usage:**
```cpp
auto checkbox = std::make_shared<CheckBox>(
    SMALL_RECT{10, 5, 40, 7},
    L"Remember Me"
);
FocusManager::registerControl(checkbox);

// Check state
if (checkbox->checked) {
    // Save preferences
}
```

---

## Label

Static text display element. Can display with or without border, centered or left-aligned.

**Header:** `BasicElements/Label.h`

**Inheritance:** `Control` + `Render`

**Constructors:**
```cpp
// Simple label
Label(SMALL_RECT r, const std::wstring t);

// With type flags
Label(SMALL_RECT r, const std::wstring t, uint8_t tp);
```

**Type Flags (combine with bitwise OR):**
| Flag | Value | Description |
|------|-------|-------------|
| `1` | 0x01 | Draw border |
| `2` | 0x02 | Center text (otherwise left-aligned) |

**Properties:**
| Property | Type | Description |
|----------|------|-------------|
| `text` | std::wstring | Display text |
| `type` | uint8_t | Display flags |

**Methods:**
```cpp
// Update text without full redraw
void updateText();
```

**Usage:**
```cpp
// Simple label with border
auto label1 = std::make_shared<Label>(
    SMALL_RECT{10, 2, 40, 4},
    L"Simple Label"
);

// Centered label with border
auto label2 = std::make_shared<Label>(
    SMALL_RECT{10, 6, 40, 8},
    L"Centered",
    3  // 1 (border) | 2 (centered)
);

// Label without border, left-aligned
auto label3 = std::make_shared<Label>(
    SMALL_RECT{10, 10, 40, 12},
    L"No Border",
    0
);

FocusManager::registerControl(label1);
FocusManager::registerControl(label2);
FocusManager::registerControl(label3);

// Update text dynamically
label1->text = L"New Text";
label1->updateText();
```

---

## Container

Layout container that arranges child controls in vertical or horizontal direction.

**Header:** `BasicElements/Container.h`

**Inheritance:** `Control` + `Render`

**Type Definitions:**
```cpp
enum LayoutDirection { Vertical = 0, Horizontal = 1 };
enum Alignment { Start = 0, Center = 1, End = 2 };
```

**Constructors:**
```cpp
// Empty container
Container(SMALL_RECT r, LayoutDirection d = Vertical);

// Container with children
Container(SMALL_RECT r, LayoutDirection d, std::vector<std::shared_ptr<Control>> c);
```

**Properties:**
| Property | Type | Description |
|----------|------|-------------|
| `controls` | std::vector<std::shared_ptr<Control>> | Child controls |
| `direction` | LayoutDirection | Vertical or Horizontal |
| `spacing` | short | Space between controls |
| `padding` | SMALL_RECT | Padding (Top, Left, Right, Bottom) |
| `alignment` | Alignment | Start, Center, or End |
| `hasbox` | bool | Draw border around container |

**Methods:**
```cpp
// Add child control
void addControl(const std::shared_ptr<Control>& ctrl);

// Remove child control
void removeControl(const std::shared_ptr<Control>& ctrl);

// Calculate and apply layout
void rearrangeControls();
```

**Usage:**
```cpp
// Create a vertical container
Container root({ 5, 5, 45, 20 }, Container::Vertical);
root.hasbox = true;
root.padding = { 1, 1, 1, 1 };

// Add controls
root.addControl(control1);
root.addControl(control2);
root.addControl(control3);

// Calculate layout
root.rearrangeControls();

// Draw (draws all children)
root.draw();

// Horizontal row example
auto row = std::make_shared<Container>(
    SMALL_RECT{0, 0, 40, 2}, 
    Container::Horizontal
);
row->addControl(btn1);
row->addControl(btn2);
row->addControl(btn3);
row->alignment = Container::Center;
row->rearrangeControls();
root.addControl(row);
root.rearrangeControls();
```

---

## Quick Reference Table

| Element | Purpose | Key Feature |
|---------|---------|-------------|
| `Button` | Base button | Visual states |
| `CFButton` | Click action | Virtual `action()` |
| `FIButton` | Click callback | `onClick` callback |
| `TextBox` | Text input | Keyboard handling |
| `CFTextBox` | Enter action | Virtual `onEnter()` |
| `FITextBox` | Enter callback | `onEnter` callback |
| `CheckBox` | Toggle | `checked` property |
| `Label` | Display | Type flags for style |
| `Container` | Layout | Auto-arrange children |

## Common Patterns

### Creating a Form
```cpp
auto loginBox = std::make_shared<TextBox>(SMALL_RECT{10, 4, 50, 6}, L"Login");
auto passwordBox = std::make_shared<TextBox>(SMALL_RECT{10, 8, 50, 10}, L"Password");
auto rememberCheck = std::make_shared<CheckBox>(SMALL_RECT{10, 12, 50, 14}, L"Remember Me");
auto submitBtn = std::make_shared<FIButton>(SMALL_RECT{20, 16, 40, 18}, L"Login");

submitBtn->onClick = [=]() {
    validateLogin(loginBox->text, passwordBox->text);
};

FocusManager::registerControl(loginBox);
FocusManager::registerControl(passwordBox);
FocusManager::registerControl(rememberCheck);
FocusManager::registerControl(submitBtn);
```

### Keyboard Navigation
```cpp
void KeyHandler(const KEY_EVENT_RECORD& ker) {
    if (ker.bKeyDown && ker.wVirtualKeyCode == VK_TAB) {
        FocusManager::nextFocus();  // Tab - next control
    } else if (ker.bKeyDown && ker.wVirtualKeyCode == VK_SPACE) {
        FocusManager::getFocused()->action();  // Space - activate
    }
}

eventManager.addHandler<KEY_EVENT_RECORD>(KeyHandler);
```