#pragma once

namespace ouif {

struct Point {
    float x = 0.0f;
    float y = 0.0f;
};

struct Size {
    float width = 0.0f;
    float height = 0.0f;
};

struct Insets {
    float left = 0.0f;
    float top = 0.0f;
    float right = 0.0f;
    float bottom = 0.0f;
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
