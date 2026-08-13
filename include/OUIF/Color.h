#pragma once

#include <cstdint>

namespace ouif {

struct Color {
    float r = 1.0f;
    float g = 1.0f;
    float b = 1.0f;
    float a = 1.0f;

    static constexpr Color rgba(std::uint8_t red, std::uint8_t green, std::uint8_t blue, std::uint8_t alpha) noexcept
    {
        return {
            static_cast<float>(red) / 255.0f,
            static_cast<float>(green) / 255.0f,
            static_cast<float>(blue) / 255.0f,
            static_cast<float>(alpha) / 255.0f,
        };
    }

    static constexpr Color rgb(std::uint8_t red, std::uint8_t green, std::uint8_t blue) noexcept
    {
        return rgba(red, green, blue, 255);
    }
};

} // namespace ouif
