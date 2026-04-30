#include "Widget.h"
#include <algorithm>

// ------------------ BoxLayout Implementation ------------------
void BoxLayout::arrange(Widget& parent) {
    (void)parent;
    // Arrangement logic would iterate over child widgets
}

short BoxLayout::getPreferredWidth() const {
    return 0;
}

short BoxLayout::getPreferredHeight() const {
    return 0;
}


// ------------------ BorderLayout Implementation ------------------
void BorderLayout::arrange(Widget& parent) {
    (void)parent;
}

short BorderLayout::getPreferredWidth() const {
    return westWidth + eastWidth + 10;
}

short BorderLayout::getPreferredHeight() const {
    return northHeight + southHeight + 5;
}


// ------------------ GridLayout Implementation ------------------
void GridLayout::arrange(Widget& parent) {
    (void)parent;
}

short GridLayout::getPreferredWidth() const {
    return cols * 10;
}

short GridLayout::getPreferredHeight() const {
    return rows * 3;
}