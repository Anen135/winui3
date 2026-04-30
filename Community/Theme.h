#pragma once
#include <windows.h>
#include <string>
#include <map>

// ------------------ Theme ------------------
// Color scheme and styling support
class Theme {
public:
    // Default ANSI color combinations
    static constexpr WORD DEFAULT      = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
    static constexpr WORD PRIMARY   = FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
    static constexpr WORD SUCCESS   = FOREGROUND_GREEN | FOREGROUND_INTENSITY;
    static constexpr WORD WARN      = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
    static constexpr WORD FAIL      = FOREGROUND_RED | FOREGROUND_INTENSITY;
    static constexpr WORD INFO     = FOREGROUND_BLUE | FOREGROUND_INTENSITY;

    // White is all foreground colors
    static constexpr WORD WHITE     = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
    static constexpr WORD BLACK    = 0;

    // Background combinations
    static constexpr WORD BG_PRIMARY = BACKGROUND_BLUE | WHITE;
    static constexpr WORD BG_SUCCESS = BACKGROUND_GREEN | WHITE;
    static constexpr WORD BG_WARN    = BACKGROUND_RED | BACKGROUND_BLUE | WHITE;
    static constexpr WORD BG_FAIL   = BACKGROUND_RED | WHITE;
    static constexpr WORD BG_INFO = BACKGROUND_BLUE | WHITE;

    // State colors
    static constexpr WORD HOVER     = BACKGROUND_BLUE | WHITE;
    static constexpr WORD FOCUSED   = BACKGROUND_BLUE | FOREGROUND_GREEN | WHITE | BACKGROUND_INTENSITY;
    static constexpr WORD DISABLED  = FOREGROUND_INTENSITY;
    static constexpr WORD SELECTED  = BACKGROUND_BLUE | WHITE;

    // Button specific
    static constexpr WORD BTN_NORMAL  = WHITE;
    static constexpr WORD BTN_HOVER   = BACKGROUND_BLUE | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
    static constexpr WORD BTN_PRESSED = BACKGROUND_BLUE | WHITE;
    static constexpr WORD BTN_FOCUSED = BACKGROUND_GREEN | FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY;

    // Container specific
    static constexpr WORD CONTAINER_BG  = WHITE;
    static constexpr WORD BORDER    = FOREGROUND_BLUE | FOREGROUND_GREEN;

    // Input specific
    static constexpr WORD INPUT_BG   = BLACK | WHITE;
    static constexpr WORD INPUT_TEXT = WHITE;
    static constexpr WORD INPUT_CURSOR = FOREGROUND_BLUE | FOREGROUND_INTENSITY;

private:
    std::wstring name;
    std::map<std::wstring, WORD> customColors;

public:
    Theme() = default;
    Theme(const std::wstring& n) : name(n) {}

    const std::wstring& getName() const { return name; }
    void setName(const std::wstring& n) { name = n; }

    // Custom color registration
    void setColor(const std::wstring& key, WORD color) {
        customColors[key] = color;
    }

    WORD getColor(const std::wstring& key, WORD fallback = DEFAULT) const {
        auto it = customColors.find(key);
        return it != customColors.end() ? it->second : fallback;
    }

    // Predefined themes
    static Theme dark() {
        Theme t(L"dark");
        t.setColor(L"primary", PRIMARY);
        t.setColor(L"background", BLACK);
        t.setColor(L"foreground", DEFAULT);
        return t;
    }

    static Theme light() {
        Theme t(L"light");
        t.setColor(L"primary", PRIMARY);
        t.setColor(L"background", WHITE | BACKGROUND_BLUE);
        t.setColor(L"foreground", DEFAULT);
        return t;
    }

    static Theme& defaultTheme() {
        static Theme t(L"default");
        return t;
    }
};


// ------------------ Style ------------------
// Style properties for individual widgets
class Style {
public:
    WORD normal = Theme::DEFAULT;
    WORD hover = Theme::HOVER;
    WORD focused = Theme::FOCUSED;
    WORD disabled = Theme::DISABLED;
    WORD pressed = Theme::BTN_PRESSED;

    wchar_t fillChar = L' ';
    short borderWidth = 1;

    bool bordered = true;
    bool shadowed = false;

    // Text alignment
    enum TextAlign { Left, Center, Right };
    TextAlign align = Left;

    Style() = default;

    Style(WORD n, WORD h, WORD f, WORD d)
        : normal(n), hover(h), focused(f), disabled(d) {}

    // Get attribute for state
    WORD getForState(bool en, bool ho, bool fo) const {
        if (!en) return disabled;
        if (fo) return focused;
        if (ho) return hover;
        return normal;
    }
};


// ------------------ ButtonStyle ------------------
class ButtonStyle : public Style {
public:
    bool rounded = false;

    ButtonStyle() {
        normal = Theme::BTN_NORMAL;
        hover = Theme::BTN_HOVER;
        focused = Theme::BTN_FOCUSED;
        pressed = Theme::BTN_PRESSED;
    }
};


// ------------------ ContainerStyle ------------------
class ContainerStyle : public Style {
public:
    bool showBorder = true;
    bool showHeader = false;
    short padding = 1;

    ContainerStyle() {
        normal = Theme::CONTAINER_BG;
        bordered = true;
    }
};


// ------------------ InputStyle ------------------
class InputStyle : public Style {
public:
    bool showCursor = true;
    bool passwordMode = false;
    short maxLength = 256;

    InputStyle() {
        normal = Theme::INPUT_BG;
        hover = Theme::INPUT_BG;
        focused = Theme::INPUT_BG;
    }
};