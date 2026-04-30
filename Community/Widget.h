#pragma once
#include <string>
#include <functional>
#include "..\Core\Control.h"
#include <vector>

// ------------------ Widget ------------------
// Base class for all UI elements with styling and identification
class Widget : public Control {
public:
    std::wstring id;
    std::wstring name;
    bool enabled = true;
    bool visible = true;
    bool dirty = true;  // Mark for redraw

    // Style properties
    WORD normalAttr = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
    WORD hoverAttr = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
    WORD focusedAttr = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
    WORD disabledAttr = FOREGROUND_INTENSITY;
    WORD background = 0;
    wchar_t fillChar = L' ';

    Widget() = default;
    Widget(SMALL_RECT r, const std::wstring& n = L"") : Control(r), id(n), name(n) {}
    Widget(SMALL_RECT r, const std::wstring& n, const std::wstring& i) : Control(r), id(i), name(n) {}

    // Get current attribute based on state
    WORD getAttribute() const {
        if (!enabled) return disabledAttr;
        if (!visible) return 0;
        if (focused) return focusedAttr;
        if (hovered) return hoverAttr;
        return normalAttr;
    }

    // Mark widget as needing redraw
    void markDirty() { dirty = true; }
    void clearDirty() { dirty = false; }

    // Visibility control
    void show() { visible = true; }
    void hide() { visible = false; markDirty(); }

    // Enable/Disable
    void enable() { enabled = true; }
    void disable() { enabled = false; markDirty(); }

    // Base draw that respects visibility
    void draw() override {
        if (!visible) return;
        clearDirty();
    }

    // Position utilities
    short width() const { return rect.Right - rect.Left; }
    short height() const { return rect.Bottom - rect.Top; }
    void setSize(short w, short h) {
        rect.Right = rect.Left + w;
        rect.Bottom = rect.Top + h;
        markDirty();
    }

    void setPosition(short x, short y) {
        short w = width();
        short h = height();
        rect.Left = x;
        rect.Top = y;
        rect.Right = x + w;
        rect.Bottom = y + h;
        markDirty();
    }
};


// ------------------ Layout Interface ------------------
class Layout {
public:
    virtual ~Layout() = default;
    virtual void arrange(Widget& parent) = 0;
    virtual short getPreferredWidth() const = 0;
    virtual short getPreferredHeight() const = 0;
};


// ------------------ BoxLayout ------------------
class BoxLayout : public Layout {
public:
    enum Direction { Horizontal, Vertical };
    enum Alignment { Start, Center, End, Stretch };

    Direction direction = Vertical;
    Alignment alignment = Start;
    short spacing = 1;

    BoxLayout() = default;
    BoxLayout(Direction d, Alignment a = Start, short s = 1)
        : direction(d), alignment(a), spacing(s) {}

    void arrange(Widget& parent) override;
    short getPreferredWidth() const override;
    short getPreferredHeight() const override;
};


// ------------------ BorderLayout ------------------
class BorderLayout : public Layout {
public:
    short northHeight = 0;
    short southHeight = 0;
    short westWidth = 0;
    short eastWidth = 0;

    BorderLayout() = default;
    BorderLayout(short n, short s, short w, short e)
        : northHeight(n), southHeight(s), westWidth(w), eastWidth(e) {}

    void arrange(Widget& parent) override;
    short getPreferredWidth() const override;
    short getPreferredHeight() const override;
};


// ------------------ GridLayout ------------------
class GridLayout : public Layout {
public:
    short rows = 1;
    short cols = 1;
    short hSpacing = 1;
    short vSpacing = 1;

    GridLayout() = default;
    GridLayout(short r, short c, short h = 1, short v = 1)
        : rows(r), cols(c), hSpacing(h), vSpacing(v) {}

    void arrange(Widget& parent) override;
    short getPreferredWidth() const override;
    short getPreferredHeight() const override;
};