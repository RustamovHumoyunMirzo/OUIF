#pragma once

#include <cstdint>
#include <initializer_list>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

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

        if (value.size() != 3 && value.size() != 4 && value.size() != 6 && value.size() != 8) {
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

        if (value.size() == 3 || value.size() == 4) {
            std::uint32_t expanded = 0;
            for (std::size_t index = 0; index < value.size(); ++index) {
                const auto nibble = (parsed >> ((value.size() - index - 1U) * 4U)) & 0xfU;
                expanded = (expanded << 8U) | (nibble << 4U) | nibble;
            }
            parsed = expanded;
        }

        if (value.size() == 4) {
            return hexa(parsed);
        }
        if (value.size() == 3) {
            return hex(parsed);
        }
        return value.size() == 6 ? hex(parsed) : hexa(parsed);
    }

    static std::optional<Color> named(std::string_view value) noexcept
    {
        if (value == "black") {
            return rgb(0, 0, 0);
        }
        if (value == "white") {
            return rgb(255, 255, 255);
        }
        if (value == "red") {
            return rgb(255, 0, 0);
        }
        if (value == "green") {
            return rgb(0, 128, 0);
        }
        if (value == "blue") {
            return rgb(0, 0, 255);
        }
        if (value == "transparent") {
            return rgba(0, 0, 0, 0);
        }
        return std::nullopt;
    }
};

struct GradientStop {
    float offset = 0.0f;
    Color color {};
};

struct Gradient {
    enum class Kind : std::uint8_t {
        Linear,
    };

    Kind kind = Kind::Linear;
    float angle_degrees = 0.0f;
    std::vector<GradientStop> stops;

    static Gradient Linear(float angle, std::initializer_list<GradientStop> gradient_stops)
    {
        Gradient gradient;
        gradient.kind = Kind::Linear;
        gradient.angle_degrees = angle;
        gradient.stops.assign(gradient_stops.begin(), gradient_stops.end());
        return gradient;
    }

    [[nodiscard]] bool empty() const noexcept
    {
        return stops.empty();
    }
};

} // namespace ouif
