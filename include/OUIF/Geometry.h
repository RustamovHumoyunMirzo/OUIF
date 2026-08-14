#pragma once

#include <algorithm>

namespace ouif {

struct Point {
    float x = 0.0f;
    float y = 0.0f;
};

struct Size {
    float width = 0.0f;
    float height = 0.0f;
};

enum class LengthUnit {
    Auto,
    Px,
    Percent,
    ViewWidth,
    ViewHeight,
};

struct Length {
    float value = 0.0f;
    LengthUnit unit = LengthUnit::Auto;

    static constexpr Length auto_value() noexcept { return {}; }
    static constexpr Length px(float value) noexcept { return { value, LengthUnit::Px }; }
    static constexpr Length percent(float value) noexcept { return { value, LengthUnit::Percent }; }
    static constexpr Length vw(float value) noexcept { return { value, LengthUnit::ViewWidth }; }
    static constexpr Length vh(float value) noexcept { return { value, LengthUnit::ViewHeight }; }

    [[nodiscard]] constexpr bool automatic() const noexcept { return unit == LengthUnit::Auto; }

    [[nodiscard]] constexpr float resolve(Size available, bool horizontal) const noexcept
    {
        switch (unit) {
        case LengthUnit::Px:
            return value;
        case LengthUnit::Percent:
            return (horizontal ? available.width : available.height) * (value / 100.0f);
        case LengthUnit::ViewWidth:
            return available.width * (value / 100.0f);
        case LengthUnit::ViewHeight:
            return available.height * (value / 100.0f);
        case LengthUnit::Auto:
        default:
            return 0.0f;
        }
    }
};

struct Insets {
    float left = 0.0f;
    float top = 0.0f;
    float right = 0.0f;
    float bottom = 0.0f;

    constexpr Insets() noexcept = default;

    constexpr Insets(float all) noexcept
        : left(all)
        , top(all)
        , right(all)
        , bottom(all)
    {
    }

    constexpr Insets(float horizontal, float vertical) noexcept
        : left(horizontal)
        , top(vertical)
        , right(horizontal)
        , bottom(vertical)
    {
    }

    constexpr Insets(float left_value, float top_value, float right_value, float bottom_value) noexcept
        : left(left_value)
        , top(top_value)
        , right(right_value)
        , bottom(bottom_value)
    {
    }
};

struct CornerRadius {
    float top_left = 0.0f;
    float top_right = 0.0f;
    float bottom_right = 0.0f;
    float bottom_left = 0.0f;

    constexpr CornerRadius() noexcept = default;

    constexpr CornerRadius(float all) noexcept
        : top_left(all)
        , top_right(all)
        , bottom_right(all)
        , bottom_left(all)
    {
    }

    constexpr CornerRadius(float top_left_value, float top_right_value, float bottom_right_value, float bottom_left_value) noexcept
        : top_left(top_left_value)
        , top_right(top_right_value)
        , bottom_right(bottom_right_value)
        , bottom_left(bottom_left_value)
    {
    }
};

struct Rect {
    float x = 0.0f;
    float y = 0.0f;
    float width = 0.0f;
    float height = 0.0f;

    [[nodiscard]] bool contains(Point point) const noexcept
    {
        return point.x >= x && point.y >= y && point.x <= x + width && point.y <= y + height;
    }

    [[nodiscard]] Rect inset(Insets insets) const noexcept
    {
        return {
            x + insets.left,
            y + insets.top,
            width - insets.left - insets.right,
            height - insets.top - insets.bottom,
        };
    }
};

} // namespace ouif
