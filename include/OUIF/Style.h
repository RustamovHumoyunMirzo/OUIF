#pragma once

#include <OUIF/Color.h>
#include <OUIF/Geometry.h>

namespace ouif {

struct Border {
    Color color = Color::rgba(88, 96, 112, 255);
    float width = 0.0f;

    constexpr Border() noexcept = default;

    constexpr Border(Color border_color, float border_width) noexcept
        : color(border_color)
        , width(border_width)
    {
    }
};

struct BorderEdges {
    Border left {};
    Border top {};
    Border right {};
    Border bottom {};

    constexpr BorderEdges() noexcept = default;

    constexpr BorderEdges(Border all) noexcept
        : left(all)
        , top(all)
        , right(all)
        , bottom(all)
    {
    }

    [[nodiscard]] constexpr bool empty() const noexcept
    {
        return left.width <= 0.0f && top.width <= 0.0f && right.width <= 0.0f && bottom.width <= 0.0f;
    }

    [[nodiscard]] constexpr bool uniform() const noexcept
    {
        return left.width == top.width
            && left.width == right.width
            && left.width == bottom.width
            && left.color.r == top.color.r
            && left.color.g == top.color.g
            && left.color.b == top.color.b
            && left.color.a == top.color.a
            && left.color.r == right.color.r
            && left.color.g == right.color.g
            && left.color.b == right.color.b
            && left.color.a == right.color.a
            && left.color.r == bottom.color.r
            && left.color.g == bottom.color.g
            && left.color.b == bottom.color.b
            && left.color.a == bottom.color.a;
    }
};

struct Style {
    Color background = Color::rgba(36, 39, 46, 255);
    Color hovered = Color::rgba(46, 52, 64, 255);
    Color pressed = Color::rgba(54, 63, 79, 255);
    Color selected = Color::rgba(54, 63, 79, 255);
    Color focused = Color::rgba(46, 52, 64, 255);
    Color background_hovered = Color::rgba(46, 52, 64, 255);
    Color background_pressed = Color::rgba(54, 63, 79, 255);
    Color background_selected = Color::rgba(54, 63, 79, 255);
    Color foreground = Color::rgba(242, 244, 248, 255);
    Border border {};
    Border border_selected {};
    Border border_focused {};
    BorderEdges borders {};
    BorderEdges borders_selected {};
    BorderEdges borders_focused {};
    float border_width = 0.0f;
    float border_width_selected = 0.0f;
    CornerRadius radius {};
    float opacity = 1.0f;

    constexpr Style& with_background(Color color) noexcept
    {
        background = color;
        return *this;
    }

    constexpr Style& with_background_hovered(Color color) noexcept
    {
        hovered = color;
        background_hovered = color;
        return *this;
    }

    constexpr Style& with_background_pressed(Color color) noexcept
    {
        pressed = color;
        background_pressed = color;
        return *this;
    }

    constexpr Style& with_background_selected(Color color) noexcept
    {
        selected = color;
        background_selected = color;
        return *this;
    }

    constexpr Style& with_background_focused(Color color) noexcept
    {
        focused = color;
        return *this;
    }

    constexpr Style& with_foreground(Color color) noexcept
    {
        foreground = color;
        return *this;
    }

    constexpr Style& with_border(Color color, float width) noexcept
    {
        border = { color, width };
        borders = BorderEdges(border);
        border_width = width;
        return *this;
    }

    constexpr Style& with_border_selected(Color color, float width) noexcept
    {
        border_selected = { color, width };
        borders_selected = BorderEdges(border_selected);
        border_width_selected = width;
        return *this;
    }

    constexpr Style& with_border_focused(Color color, float width) noexcept
    {
        border_focused = { color, width };
        borders_focused = BorderEdges(border_focused);
        return *this;
    }

    constexpr Style& with_border_left(Color color, float width) noexcept
    {
        borders.left = { color, width };
        return *this;
    }

    constexpr Style& with_border_top(Color color, float width) noexcept
    {
        borders.top = { color, width };
        return *this;
    }

    constexpr Style& with_border_right(Color color, float width) noexcept
    {
        borders.right = { color, width };
        return *this;
    }

    constexpr Style& with_border_bottom(Color color, float width) noexcept
    {
        borders.bottom = { color, width };
        return *this;
    }

    constexpr Style& with_border_left_selected(Color color, float width) noexcept
    {
        borders_selected.left = { color, width };
        return *this;
    }

    constexpr Style& with_border_top_selected(Color color, float width) noexcept
    {
        borders_selected.top = { color, width };
        return *this;
    }

    constexpr Style& with_border_right_selected(Color color, float width) noexcept
    {
        borders_selected.right = { color, width };
        return *this;
    }

    constexpr Style& with_border_bottom_selected(Color color, float width) noexcept
    {
        borders_selected.bottom = { color, width };
        return *this;
    }

    constexpr Style& with_border_left_focused(Color color, float width) noexcept
    {
        borders_focused.left = { color, width };
        return *this;
    }

    constexpr Style& with_border_top_focused(Color color, float width) noexcept
    {
        borders_focused.top = { color, width };
        return *this;
    }

    constexpr Style& with_border_right_focused(Color color, float width) noexcept
    {
        borders_focused.right = { color, width };
        return *this;
    }

    constexpr Style& with_border_bottom_focused(Color color, float width) noexcept
    {
        borders_focused.bottom = { color, width };
        return *this;
    }

    constexpr Style& with_radius(float value) noexcept
    {
        radius = CornerRadius(value);
        return *this;
    }

    constexpr Style& with_radius(CornerRadius value) noexcept
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
