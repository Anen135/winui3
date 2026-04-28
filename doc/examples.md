# Examples

This document describes the example applications included with WinUI3. Each demo showcases different features of the library.

## Examples Overview

| Demo | File | Description |
|------|------|-------------|
| Basic | `demo.cpp` | Basic controls (Button, TextBox, CheckBox) |
| Login Form | `demo1.cpp` | Login form with validation |
| Calculator | `demo2.cpp` | Calculator with arrow key navigation |
| File Explorer | `demo3.cpp` | File browser with mouse support |
| Container Layout | `demo4.cpp` | Container layout system |

---

## Demo 1: Basic Controls (`demo.cpp`)

**Location:** `src/demo.cpp`

Demonstrates the basic UI elements: CFButton, CFTextBox, and CheckBox.

### Features

- **CFButton**: Classic button with `action()` override
- **CFTextBox**: Text input with `onEnter()` validation
- **CheckBox**: Toggle checkbox
- Tab navigation between controls
- Space to activate focused control

### Key Code

```cpp
// Button action defined in cpp
void CFButton::action() {
    MessageBoxW(NULL, L"Hello, World!", L"Success", MB_OK);
}

// TextBox enter handler
void CFTextBox::onEnter() {
    if (text == L"password") 
        MessageBoxW(NULL, L"Password is valid!", L"Success", MB_OK);
    else 
        MessageBoxW(NULL, L"Password is invalid!", L"Error", MB_ICONERROR);
}

// Keyboard handler
void KeyHandler(const KEY_EVENT_RECORD& ker) {
    if (ker.bKeyDown && ker.wVirtualKeyCode == VK_TAB) 
        FocusManager::nextFocus();
    else if (ker.bKeyDown && ker.wVirtualKeyCode == VK_SPACE) 
        FocusManager::getFocused()->action();
}
```

### Controls Created

| Control | Position | Purpose |
|---------|----------|---------|
| CFButton | (10,2) - (30,4) | Click to show message |
| CFTextBox | (10,6) - (40,8) | Enter "password" |
| CheckBox | (10,10) - (40,12) | Toggle checkbox |

---

## Demo 2: Login Form (`demo1.cpp`)

**Location:** `src/demo1.cpp`

A complete login form with username, password, checkbox, and submit button using functional callbacks.

### Features

- **FIButton**: Button with `onClick` callback
- **TextBox**: Two text inputs
- **CheckBox**: "Remember Me" option
- Form validation on submit

### Key Code

```cpp
class LoginForm {
    std::shared_ptr<TextBox> loginBox;
    std::shared_ptr<TextBox> passwordBox;
    std::shared_ptr<CheckBox> rememberMeBox;
    std::shared_ptr<FIButton> loginButton;

public:
    void setup() {
        loginBox = std::make_shared<TextBox>(SMALL_RECT{ 10, 4, 50, 6 }, L"Login");
        passwordBox = std::make_shared<TextBox>(SMALL_RECT{ 10, 8, 50, 10 }, L"Password");
        rememberMeBox = std::make_shared<CheckBox>(SMALL_RECT{ 10, 12, 50, 14 }, L"Remember Me");
        loginButton = std::make_shared<FIButton>(SMALL_RECT{ 20, 16, 40, 18 }, L"Login");

        loginButton->onClick = [this]() {
            validate();
        };

        FocusManager::registerControl(loginBox);
        FocusManager::registerControl(passwordBox);
        FocusManager::registerControl(rememberMeBox);
        FocusManager::registerControl(loginButton);
    }

    void validate() {
        if (loginBox->text == L"admin" && passwordBox->text == L"1234") {
            MessageBoxW(NULL, 
                rememberMeBox->checked ? L"Welcome, admin! (Remembered)" : L"Welcome, admin!", 
                L"Success", MB_OK);
        } else {
            MessageBoxW(NULL, L"Invalid login or password.", L"Error", MB_ICONERROR);
        }
    }
};
```

### Credentials

| Field | Value |
|-------|-------|
| Login | `admin` |
| Password | `1234` |

---

## Demo 3: Calculator (`demo2.cpp`)

**Location:** `src/demo2.cpp`

A functioning calculator with number buttons, operators, and arrow key navigation.

### Features

- 16-button calculator layout (digits 0-9, operators +, -, *, /, =, .)
- Arrow key navigation between buttons
- Numpad and regular key support
- Expression evaluation
- Backspace functionality

### Layout

```
┌────────────────────────────────┐
│           [Display]            │
├────┬────┬────┬────┐
│ 7  │ 8  │ 9  │ /  │
├────┼────┼────┼────┤
│ 4  │ 5  │ 6  │ *  │
├────┼────┼────┼────┤
│ 1  │ 2  │ 3  │ -  │
├────┼────┼────┼────┤
│ 0  │ .  │ =  │ +  │
└────┴────┴────┴────┘
```

### Controls

```cpp
std::vector<std::wstring> layout = {
    L"7", L"8", L"9", L"/",
    L"4", L"5", L"6", L"*",
    L"1", L"2", L"3", L"-",
    L"0", L".", L"=", L"+"
};
```

### Keyboard Support

| Key | Action |
|-----|--------|
| Arrow Keys | Navigate between buttons |
| Numpad 0-9 | Input digits |
| Numpad operators | +, -, *, / |
| Enter | Calculate (=) |
| Backspace | Delete last character |

### Custom Focus Movement

```cpp
void moveFocus(int dx, int dy) {
    auto focused = FocusManager::getFocused();
    auto it = std::find_if(buttons.begin(), buttons.end(),
        [focused](const std::shared_ptr<Button>& btn) {
            return btn.get() == focused.get();
        });
    
    int index = std::distance(buttons.begin(), it);
    int newRow = (index / cols + dy + rows) % rows;
    int newCol = (index % cols + dx + cols) % cols;
    int newIndex = newRow * cols + newCol;
    
    FocusManager::focusControl(buttons[newIndex].get());
}
```

---

## Demo 4: File Explorer (`demo3.cpp`)

**Location:** `src/demo3.cpp`

A file browser that displays files and folders in the current directory. Supports mouse clicks.

### Features

- Browse filesystem directories
- Mouse interaction (click to open)
- Navigate into folders
- Go to parent directory
- Pagination for large directories

### Custom FileButton

```cpp
class FileButton : public Control, public Render, 
                   public std::enable_shared_from_this<FileButton> {
public:
    std::wstring name;
    uint8_t type = 0;  // 0 = file, 1 = folder, 2 = parent

    void action() override {
        switch (type) {
            case 0:  // Open file
                ShellExecuteW(NULL, L"open", path.c_str(), NULL, NULL, SW_SHOW);
                break;
            case 1:  // Enter folder
                currentPath = fs::absolute(fs::path(currentPath) / name).wstring();
                loadDirectory(currentPath);
                break;
            case 2:  // Parent directory
                currentPath = fs::path(currentPath).parent_path().wstring();
                loadDirectory(currentPath);
                break;
        }
    }
};
```

### Key Bindings

| Key | Action |
|-----|--------|
| Tab | Next file/folder |
| Space | Open selected item |
| F9 | Previous page |
| F10 | Next page |
| Up/Down | Navigate |
| Mouse Click | Open item |

### Pagination

The explorer automatically calculates how many files fit on screen and provides pagination:

```cpp
void redrawCurrentPage() {
    maxButtonsPerPage = (Render::csbi.dwSize.Y - 5) / buttonHeight;
    // ... load current page
}
```

---

## Demo 5: Container Layout (`demo4.cpp`)

**Location:** `src/demo4.cpp`

Demonstrates the Container layout system for arranging multiple controls.

### Features

- Vertical container as root
- Horizontal containers for rows
- Padding and spacing
- Center alignment
- Custom CharacterElement for display

### Layout Structure

```cpp
// Root container (vertical)
Container root({ 5, 5, 45, 25 }, Container::Vertical);
root.hasbox = true;
root.padding = { 1, 1, 1, 1 };

// Add rows
for (unsigned short r = 0; r < 5; ++r) {
    // Horizontal row
    auto row = std::make_shared<Container>(
        SMALL_RECT{ 0, 0, charWidth * charsPerRow, charHeight }, 
        Container::Horizontal, 
        elements
    );
    row->alignment = Container::Center;
    root.addControl(row);
    root.rearrangeControls();
}
```

### Container Options

**Direction:**
- `Container::Vertical` - Stack controls vertically
- `Container::Horizontal` - Arrange controls horizontally

**Alignment:**
- `Container::Start` - Align to start (left/top)
- `Container::Center` - Center align
- `Container::End` - Align to end (right/bottom)

### Custom Element

```cpp
class CharacterElement : public Control, public Render {
public:
    wchar_t character;
    CharacterElement(SMALL_RECT r, wchar_t c) : Control(r), character(c) {}
    
    void draw() override {
        // Custom drawing: [A]
        WriteConsoleOutputCharacterW(hout, L"[", 1, { rect.Left, rect.Top }, &dump);
        WriteConsoleOutputCharacterW(hout, &character, 1, { short(rect.Left + 1), rect.Top }, &dump);
        WriteConsoleOutputCharacterW(hout, L"]", 1, { short(rect.Left + 2), rect.Top }, &dump);
    }
};
```

---

## Building and Running Examples

### Using CMake

```bash
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022"
cmake --build . --config Release
```

### Using Visual Studio

1. Open the solution
2. Set demo project as startup project
3. Build and run (Ctrl+F5)

### Using Command Line

```bash
cl /EHsc /W4 /I..\Core /I..\BasicElements demo.cpp /link kernel32.lib
```

---

## Summary

| Demo | Key Concepts |
|------|--------------|
| demo.cpp | CFButton, CFTextBox, CheckBox, Tab navigation |
| demo1.cpp | FIButton with callbacks, form validation |
| demo2.cpp | Calculator UI, custom focus movement, keyboard input |
| demo3.cpp | Custom controls, mouse events, filesystem, pagination |
| demo4.cpp | Container layout, nested containers, alignment |

Explore these demos to understand how to build various types of console UI applications with WinUI3.