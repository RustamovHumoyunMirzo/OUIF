#pragma once

#include <OUIF/Color.h>

namespace ouif {

struct Style {
    Color background = Color::rgba(36, 39, 46, 255);
    Color background_hovered = Color::rgba(46, 52, 64, 255);
    Color background_pressed = Color::rgba(54, 63, 79, 255);
    Color foreground = Color::rgba(242, 244, 248, 255);
    Color border = Color::rgba(88, 96, 112, 255);
    float border_width = 0.0f;
    float radius = 0.0f;
    float opacity = 1.0f;
};

} // namespace ouif
