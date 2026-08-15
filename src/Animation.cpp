#include <OUIF/Animation.h>

#include <algorithm>
#include <cmath>

namespace ouif {

namespace {

float clamp_unit(float value) noexcept
{
    return std::clamp(value, 0.0f, 1.0f);
}

float lerp(float from, float to, float progress) noexcept
{
    return from + (to - from) * progress;
}

Color lerp_color(Color from, Color to, float progress) noexcept
{
    return {
        lerp(from.r, to.r, progress),
        lerp(from.g, to.g, progress),
        lerp(from.b, to.b, progress),
        lerp(from.a, to.a, progress),
    };
}

Border lerp_border(Border from, Border to, float progress) noexcept
{
    return {
        lerp_color(from.color, to.color, progress),
        lerp(from.width, to.width, progress),
    };
}

BorderEdges lerp_edges(BorderEdges from, BorderEdges to, float progress) noexcept
{
    BorderEdges result;
    result.left = lerp_border(from.left, to.left, progress);
    result.top = lerp_border(from.top, to.top, progress);
    result.right = lerp_border(from.right, to.right, progress);
    result.bottom = lerp_border(from.bottom, to.bottom, progress);
    return result;
}

CornerRadius lerp_radius(CornerRadius from, CornerRadius to, float progress) noexcept
{
    return {
        lerp(from.top_left, to.top_left, progress),
        lerp(from.top_right, to.top_right, progress),
        lerp(from.bottom_right, to.bottom_right, progress),
        lerp(from.bottom_left, to.bottom_left, progress),
    };
}

bool has_property(StyleProperties properties, StyleProperty property) noexcept
{
    return (properties & style_property_mask(property)) != 0U;
}

bool near(float left, float right) noexcept
{
    return std::fabs(left - right) <= 0.0001f;
}

bool color_equals(Color left, Color right) noexcept
{
    return near(left.r, right.r) && near(left.g, right.g) && near(left.b, right.b) && near(left.a, right.a);
}

bool border_equals(Border left, Border right) noexcept
{
    return color_equals(left.color, right.color) && near(left.width, right.width);
}

bool edges_equal(BorderEdges left, BorderEdges right) noexcept
{
    return border_equals(left.left, right.left)
        && border_equals(left.top, right.top)
        && border_equals(left.right, right.right)
        && border_equals(left.bottom, right.bottom);
}

bool radius_equals(CornerRadius left, CornerRadius right) noexcept
{
    return near(left.top_left, right.top_left)
        && near(left.top_right, right.top_right)
        && near(left.bottom_right, right.bottom_right)
        && near(left.bottom_left, right.bottom_left);
}

} // namespace

float apply_easing(Easing easing, float progress) noexcept
{
    const float t = clamp_unit(progress);
    switch (easing) {
    case Easing::EaseIn:
        return t * t;
    case Easing::EaseOut:
        return 1.0f - (1.0f - t) * (1.0f - t);
    case Easing::EaseInOut:
        return t < 0.5f ? 2.0f * t * t : 1.0f - std::pow(-2.0f * t + 2.0f, 2.0f) * 0.5f;
    case Easing::Linear:
        return t;
    }
    return t;
}

Style interpolate_style(const Style& from, const Style& to, float progress) noexcept
{
    const float t = clamp_unit(progress);
    Style result;
    result.background = lerp_color(from.background, to.background, t);
    result.hovered = lerp_color(from.hovered, to.hovered, t);
    result.pressed = lerp_color(from.pressed, to.pressed, t);
    result.selected = lerp_color(from.selected, to.selected, t);
    result.focused = lerp_color(from.focused, to.focused, t);
    result.background_hovered = lerp_color(from.background_hovered, to.background_hovered, t);
    result.background_pressed = lerp_color(from.background_pressed, to.background_pressed, t);
    result.background_selected = lerp_color(from.background_selected, to.background_selected, t);
    result.foreground = lerp_color(from.foreground, to.foreground, t);
    result.border = lerp_border(from.border, to.border, t);
    result.border_selected = lerp_border(from.border_selected, to.border_selected, t);
    result.border_focused = lerp_border(from.border_focused, to.border_focused, t);
    result.borders = lerp_edges(from.borders, to.borders, t);
    result.borders_selected = lerp_edges(from.borders_selected, to.borders_selected, t);
    result.borders_focused = lerp_edges(from.borders_focused, to.borders_focused, t);
    result.border_width = lerp(from.border_width, to.border_width, t);
    result.border_width_selected = lerp(from.border_width_selected, to.border_width_selected, t);
    result.radius = lerp_radius(from.radius, to.radius, t);
    result.opacity = lerp(from.opacity, to.opacity, t);
    return result;
}

Style apply_animated_style(const Style& base, const Style& animated, StyleProperties properties) noexcept
{
    Style result = base;
    if (has_property(properties, StyleProperty::Background)) {
        result.background = animated.background;
    }
    if (has_property(properties, StyleProperty::StatefulBackgrounds)) {
        result.hovered = animated.hovered;
        result.pressed = animated.pressed;
        result.selected = animated.selected;
        result.focused = animated.focused;
        result.background_hovered = animated.background_hovered;
        result.background_pressed = animated.background_pressed;
        result.background_selected = animated.background_selected;
    }
    if (has_property(properties, StyleProperty::Foreground)) {
        result.foreground = animated.foreground;
    }
    if (has_property(properties, StyleProperty::Border)) {
        result.border = animated.border;
        result.border_width = animated.border_width;
    }
    if (has_property(properties, StyleProperty::StatefulBorders)) {
        result.border_selected = animated.border_selected;
        result.border_focused = animated.border_focused;
        result.border_width_selected = animated.border_width_selected;
    }
    if (has_property(properties, StyleProperty::BorderEdges)) {
        result.borders = animated.borders;
    }
    if (has_property(properties, StyleProperty::StatefulBorderEdges)) {
        result.borders_selected = animated.borders_selected;
        result.borders_focused = animated.borders_focused;
    }
    if (has_property(properties, StyleProperty::Radius)) {
        result.radius = animated.radius;
    }
    if (has_property(properties, StyleProperty::Opacity)) {
        result.opacity = animated.opacity;
    }
    return result;
}

bool style_equals(const Style& left, const Style& right) noexcept
{
    return color_equals(left.background, right.background)
        && color_equals(left.hovered, right.hovered)
        && color_equals(left.pressed, right.pressed)
        && color_equals(left.selected, right.selected)
        && color_equals(left.focused, right.focused)
        && color_equals(left.background_hovered, right.background_hovered)
        && color_equals(left.background_pressed, right.background_pressed)
        && color_equals(left.background_selected, right.background_selected)
        && color_equals(left.foreground, right.foreground)
        && border_equals(left.border, right.border)
        && border_equals(left.border_selected, right.border_selected)
        && border_equals(left.border_focused, right.border_focused)
        && edges_equal(left.borders, right.borders)
        && edges_equal(left.borders_selected, right.borders_selected)
        && edges_equal(left.borders_focused, right.borders_focused)
        && near(left.border_width, right.border_width)
        && near(left.border_width_selected, right.border_width_selected)
        && radius_equals(left.radius, right.radius)
        && near(left.opacity, right.opacity);
}

} // namespace ouif
