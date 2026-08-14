#pragma once

#include <cstdint>
#include <optional>
#include <string_view>

namespace ouif {

struct Color {
    float r = 1.0f;
    float g = 1.0f;
    float b = 1.0f;
    float a = 1.0f;

    constexpr Color() noexcept = default;

    constexpr Color(float red, float green, float blue, float alpha = 1.0f) noexcept
        : r(red)
        , g(green)
        , b(blue)
        , a(alpha)
    {
    }

    Color(std::string_view hex_value) noexcept
    {
        if (auto parsed = from_hex(hex_value)) {
            *this = *parsed;
        }
    }

    Color(const char* hex_value) noexcept
        : Color(std::string_view(hex_value != nullptr ? hex_value : ""))
    {
    }

    static constexpr Color rgba(std::uint8_t red, std::uint8_t green, std::uint8_t blue, std::uint8_t alpha) noexcept
    {
        return Color {
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

    static constexpr Color hex(std::uint32_t rgb) noexcept
    {
        return rgba(
            static_cast<std::uint8_t>((rgb >> 16U) & 0xffU),
            static_cast<std::uint8_t>((rgb >> 8U) & 0xffU),
            static_cast<std::uint8_t>(rgb & 0xffU),
            255
        );
    }

    static constexpr Color hexa(std::uint32_t rgba_value) noexcept
    {
        return rgba(
            static_cast<std::uint8_t>((rgba_value >> 24U) & 0xffU),
            static_cast<std::uint8_t>((rgba_value >> 16U) & 0xffU),
            static_cast<std::uint8_t>((rgba_value >> 8U) & 0xffU),
            static_cast<std::uint8_t>(rgba_value & 0xffU)
        );
    }

    static std::optional<Color> from_hex(std::string_view value) noexcept
    {
        if (!value.empty() && value.front() == '#') {
            value.remove_prefix(1);
        }

        if (value.size() != 6 && value.size() != 8) {
            return std::nullopt;
        }

        std::uint32_t parsed = 0;
        for (const char character : value) {
            parsed <<= 4U;
            if (character >= '0' && character <= '9') {
                parsed |= static_cast<std::uint32_t>(character - '0');
            } else if (character >= 'a' && character <= 'f') {
                parsed |= static_cast<std::uint32_t>(character - 'a' + 10);
            } else if (character >= 'A' && character <= 'F') {
                parsed |= static_cast<std::uint32_t>(character - 'A' + 10);
            } else {
                return std::nullopt;
            }
        }

        return value.size() == 6 ? hex(parsed) : hexa(parsed);
    }
};

} // namespace ouif
