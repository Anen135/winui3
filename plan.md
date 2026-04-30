# Library Development Plan

## Project Overview
- **Name**: Windows Console UI Library (WinUI3)
- **Type**: C++ Header-only UI Library for Windows Console Applications
- **Target**: Windows Console Apps with GUI-like controls

## Current State
- Core framework: Control, EventManager, FocusManager, Render, InputState
- Basic Elements: Button, Container, TextBox, CheckBox, Label, ScrollContainer
- Build system: CMake with C++23
- Distribution: Modular + header-only option

---

## Phase 1: Architecture & Core Infrastructure

### 1.1 Module Organization
```
Core/
├── Control.h/cpp         - Base class (done)
├── EventManager.h/cpp   - Event system (done)
├── FocusManager.h/cpp  - Focus management (done)
├── InputState.h/cpp    - Input utilities (done)
├── Render.h/cpp        - Rendering (done)
├── Layout.h            - NEW: Layout system
├── Theme.h             - NEW: Theming support
├── Widget.h             - NEW: Widget base class
└── Application.h      - NEW: Main app class

BasicElements/
├── Button.h             (done)
├── Container.h         (done)
├── TextBox.h            (done)
├── CheckBox.h           (done)
├── Label.h             (done)
├── ScrollContainer.h  (done)
├── FiButton.h         - Future improvement
├── FiTextBox.h        - Future improvement
├── CFButton.h         - Future improvement
└── CFTextBox.h        - Future improvement
```

### 1.2 Core Classes to Implement
| Class | Purpose |
|-------|---------|
| `Widget` | Base for all UI elements with style support |
| `Layout` | Abstract layout engine for containers |
| `Theme` | Color schemes and styling |
| `Application` | Main loop and window management |

---

## Phase 2: Layout System

### 2.1 Layout Implementations
- `BoxLayout` - Simple box layout (vertical/horizontal)
- `GridLayout` - Grid-based layout
- `BorderLayout` - North/South/East/West/Center layout
- `FlowLayout` - Wrapping flow layout

### 2.2 Container Enhancements
- Better layout property management
- Minimum/maximum size constraints
- Preferred size calculation

---

## Phase 3: Additional UI Elements

### 3.1 Input Controls
- `TextBox` - Single-line text input
- `TextArea` - Multi-line text input
- `ComboBox` - Dropdown selection
- `Slider` - Numeric value selection
- `RadioButton` - Radio button group

### 3.2 Display Controls
- `ProgressBar` - Progress indicator
- `Image` - Console image rendering
- `ListView` - List display
- `TableView` - Table display

### 3.3 Dialogs
- `MessageBox` - System message box
- `FileDialog` - File selection dialog
- `ColorPicker` - Color selection (ANSI)

---

## Phase 4: Theming & Styling

### 4.1 Theme System
- Color scheme definitions
- Font style support (future)
- Component-specific styling
- Dark/Light mode support

### 4.2 Style Classes
- `Style` - Base style properties
- `ButtonStyle`, `ContainerStyle`, etc.
- `StyleSheet` - Collection of styles

---

## Phase 5: Accessibility & Events

### 5.1 Event Enhancements
- Custom event system
- Event bubbling/propagation
- Drag and drop events
- Keyboard shortcuts

### 5.2 Focus Navigation
- Tab order management
- Focus groups
- Keyboard navigation patterns

---

## Phase 6: Documentation & Examples

### 6.1 Documentation
- API reference
- Usage examples
- Migration guide

### 6.2 Demo Application
- Demo with all controls
- Showcase functionality

---

## Implementation Order

1. **Widget base class** - Refactor Control to Widget
2. **Layout system** - Implement BoxLayout, GridLayout
3. **Theme system** - Add styling support
4. **Additional controls** - TextArea, ComboBox, ListView
5. **Dialogs** - MessageBox, FileDialog
6. **Demo app** - Showcase all features

---

## Testing Strategy
- Build verification: `cmake --build .`
- Demo application testing
- Manual testing of all controls
- Focus, keyboard, mouse event verification