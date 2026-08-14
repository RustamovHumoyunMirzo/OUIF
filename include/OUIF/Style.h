#pragma once

#include <OUIF/Color.h>

namespace ouif {

struct Style {
    Color background = Color::rgba(36, 39, 46, 255);
    Color background_hovered = Color::rgba(46, 52, 64, 255);
    Color background_pressed = Color::rgba(54, 63, 79, 255);
    Color background_selected = Color::rgba(54, 63, 79, 255);
    Color foreground = Color::rgba(242, 244, 248, 255);
    Color border = Color::rgba(88, 96, 112, 255);
    Color border_selected = Color::rgba(88, 96, 112, 255);
    float border_width = 0.0f;
    float border_width_selected = 0.0f;
    float radius = 0.0f;
    float opacity = 1.0f;

    constexpr Style& with_background(Color color) noexcept
    {
        background = color;
        return *this;
    }

    constexpr Style& with_background_hovered(Color color) noexcept
    {
        background_hovered = color;
        return *this;
    }

    constexpr Style& with_background_pressed(Color color) noexcept
    {
        background_pressed = color;
        return *this;
    }

    constexpr Style& with_background_selected(Color color) noexcept
    {
        background_selected = color;
        return *this;
    }

    constexpr Style& with_foreground(Color color) noexcept
    {
        foreground = color;
        return *this;
    }

    constexpr Style& with_border(Color color, float width) noexcept
    {
        border = color;
        border_width = width;
        return *this;
    }

    constexpr Style& with_border_selected(Color color, float width) noexcept
    {
        border_selected = color;
        border_width_selected = width;
        return *this;
    }

    constexpr Style& with_radius(float value) noexcept
    {
        radius = value;
        return *this;
    }

    constexpr Style& with_opacity(float value) noexcept
    {
        opacity = value;
        return *this;
    }
};

} // namespace ouif
