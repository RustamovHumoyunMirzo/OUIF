#pragma once

#include <OUIF/Export.h>
#include <OUIF/Style.h>

#include <cstdint>
#include <string>
#include <vector>

namespace ouif {

enum class Easing : std::uint8_t {
    Linear,
    EaseIn,
    EaseOut,
    EaseInOut,
};

enum class StyleProperty : std::uint32_t {
    None = 0,
    Background = 1U << 0U,
    StatefulBackgrounds = 1U << 1U,
    Foreground = 1U << 2U,
    Border = 1U << 3U,
    StatefulBorders = 1U << 4U,
    BorderEdges = 1U << 5U,
    StatefulBorderEdges = 1U << 6U,
    Radius = 1U << 7U,
    Opacity = 1U << 8U,
    All = 0xffffffffU,
};

using StyleProperties = std::uint32_t;

[[nodiscard]] constexpr StyleProperties style_property_mask(StyleProperty property) noexcept
{
    return static_cast<StyleProperties>(property);
}

struct StyleTransition {
    float duration = 0.0f;
    Easing easing = Easing::EaseOut;
    bool enabled = false;
};

struct StyleKeyframe {
    float offset = 0.0f;
    Style style {};
    StyleProperties properties = style_property_mask(StyleProperty::All);
};

struct StyleAnimation {
    std::string name;
    float duration = 1.0f;
    Easing easing = Easing::Linear;
    bool loop = false;
    std::vector<StyleKeyframe> keyframes;
};

OUIF_API float apply_easing(Easing easing, float progress) noexcept;
OUIF_API Style interpolate_style(const Style& from, const Style& to, float progress) noexcept;
OUIF_API Style apply_animated_style(const Style& base, const Style& animated, StyleProperties properties) noexcept;
OUIF_API bool style_equals(const Style& left, const Style& right) noexcept;

} // namespace ouif
