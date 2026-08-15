#pragma once

#include <OUIF/Color.h>
#include <OUIF/Export.h>
#include <OUIF/Geometry.h>

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <string_view>

namespace ouif {

class Window;

enum class WindowTheme : std::uint8_t {
    System,
    Light,
    Dark,
};

enum class WindowMaterial : std::uint8_t {
    System,
    Solid,
    Transparent,
};

enum class WindowMode : std::uint8_t {
    Windowed,
    Maximized,
    Fullscreen,
};

struct WindowConfig {
    std::string title = "OUIF Window";
    std::uint32_t width = 1280;
    std::uint32_t height = 720;
    std::optional<Point> position;
    bool visible = true;
    bool decorated = true;
    bool resizable = true;
    bool focused = true;
    bool always_on_top = false;
    bool transparent_framebuffer = false;
    WindowMode mode = WindowMode::Windowed;
    WindowTheme theme = WindowTheme::System;
    WindowMaterial material = WindowMaterial::System;
    Color background = Color::rgba(18, 20, 24, 255);
    Window* owner = nullptr;

    WindowConfig& with_title(std::string value)
    {
        title = std::move(value);
        return *this;
    }

    WindowConfig& with_size(std::uint32_t new_width, std::uint32_t new_height) noexcept
    {
        width = new_width;
        height = new_height;
        return *this;
    }

    WindowConfig& with_position(float x, float y) noexcept
    {
        position = Point { x, y };
        return *this;
    }

    WindowConfig& with_visible(bool value) noexcept
    {
        visible = value;
        return *this;
    }

    WindowConfig& with_decorated(bool value) noexcept
    {
        decorated = value;
        return *this;
    }

    WindowConfig& with_resizable(bool value) noexcept
    {
        resizable = value;
        return *this;
    }

    WindowConfig& with_always_on_top(bool value) noexcept
    {
        always_on_top = value;
        return *this;
    }

    WindowConfig& with_transparent_framebuffer(bool value) noexcept
    {
        transparent_framebuffer = value;
        return *this;
    }

    WindowConfig& with_mode(WindowMode value) noexcept
    {
        mode = value;
        return *this;
    }

    WindowConfig& with_theme(WindowTheme value) noexcept
    {
        theme = value;
        return *this;
    }

    WindowConfig& with_material(WindowMaterial value) noexcept
    {
        material = value;
        return *this;
    }

    WindowConfig& with_background(Color value) noexcept
    {
        background = value;
        return *this;
    }

    WindowConfig& with_owner(Window& value) noexcept
    {
        owner = &value;
        return *this;
    }
};

class OUIF_API Window {
public:
    struct Impl;

    Window() noexcept;
    ~Window();

    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;
    Window(Window&&) noexcept;
    Window& operator=(Window&&) noexcept;
    explicit Window(std::shared_ptr<Impl> impl) noexcept;

    [[nodiscard]] bool valid() const noexcept;
    [[nodiscard]] void* native_handle() const noexcept;
    [[nodiscard]] const WindowConfig& config() const noexcept;
    [[nodiscard]] std::string_view title() const noexcept;
    [[nodiscard]] Size size() const noexcept;
    [[nodiscard]] Point position() const noexcept;
    [[nodiscard]] bool visible() const noexcept;
    [[nodiscard]] bool should_close() const noexcept;
    [[nodiscard]] bool decorated() const noexcept;
    [[nodiscard]] bool resizable() const noexcept;
    [[nodiscard]] bool always_on_top() const noexcept;
    [[nodiscard]] WindowTheme theme() const noexcept;
    [[nodiscard]] WindowMaterial material() const noexcept;

    void set_title(std::string title);
    void set_size(std::uint32_t width, std::uint32_t height);
    void set_position(float x, float y);
    void show();
    void hide();
    void focus();
    void request_close();
    void set_decorated(bool decorated);
    void set_resizable(bool resizable);
    void set_always_on_top(bool always_on_top);
    void set_opacity(float opacity);
    void set_theme(WindowTheme theme) noexcept;
    void set_material(WindowMaterial material) noexcept;

private:
    friend class Application;

    std::shared_ptr<Impl> impl_;
};

struct DialogConfig {
    std::string title = "Dialog";
    std::uint32_t width = 420;
    std::uint32_t height = 220;
    bool modal = true;
    WindowTheme theme = WindowTheme::System;
    WindowMaterial material = WindowMaterial::System;
    Color background = Color::rgba(18, 20, 24, 255);
    Window* owner = nullptr;

    DialogConfig& with_title(std::string value)
    {
        title = std::move(value);
        return *this;
    }

    DialogConfig& with_size(std::uint32_t new_width, std::uint32_t new_height) noexcept
    {
        width = new_width;
        height = new_height;
        return *this;
    }

    DialogConfig& with_modal(bool value) noexcept
    {
        modal = value;
        return *this;
    }

    DialogConfig& with_theme(WindowTheme value) noexcept
    {
        theme = value;
        return *this;
    }

    DialogConfig& with_material(WindowMaterial value) noexcept
    {
        material = value;
        return *this;
    }

    DialogConfig& with_background(Color value) noexcept
    {
        background = value;
        return *this;
    }

    DialogConfig& with_owner(Window& value) noexcept
    {
        owner = &value;
        return *this;
    }
};

class DialogBuilder {
public:
    DialogBuilder& with_title(std::string title)
    {
        config_.title = std::move(title);
        return *this;
    }

    DialogBuilder& with_size(std::uint32_t width, std::uint32_t height) noexcept
    {
        config_.width = width;
        config_.height = height;
        return *this;
    }

    DialogBuilder& with_modal(bool modal) noexcept
    {
        config_.modal = modal;
        return *this;
    }

    DialogBuilder& with_theme(WindowTheme theme) noexcept
    {
        config_.theme = theme;
        return *this;
    }

    DialogBuilder& with_material(WindowMaterial material) noexcept
    {
        config_.material = material;
        return *this;
    }

    DialogBuilder& with_background(Color background) noexcept
    {
        config_.background = background;
        return *this;
    }

    DialogBuilder& with_owner(Window& owner) noexcept
    {
        config_.owner = &owner;
        return *this;
    }

    [[nodiscard]] const DialogConfig& config() const noexcept { return config_; }

private:
    DialogConfig config_ {};
};

} // namespace ouif
