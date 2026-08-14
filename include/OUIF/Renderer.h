#pragma once

#include <OUIF/Color.h>
#include <OUIF/Export.h>
#include <OUIF/Geometry.h>
#include <OUIF/Style.h>

#include <cstdint>
#include <memory>

namespace ouif {

struct RendererConfig {
    void* native_window = nullptr;
    std::uint32_t width = 1280;
    std::uint32_t height = 720;
    const char* shader_directory = nullptr;
    bool vsync = true;
};

class OUIF_API Renderer {
public:
    Renderer();
    ~Renderer();

    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;
    Renderer(Renderer&&) noexcept;
    Renderer& operator=(Renderer&&) noexcept;

    void initialize(const RendererConfig& config);
    void shutdown() noexcept;
    void resize(std::uint32_t width, std::uint32_t height);
    void begin_frame(Color clear_color);
    void fill_rect(Rect rect, Color color);
    void fill_rounded_rect(Rect rect, CornerRadius radius, Color color);
    void stroke_rect(Rect rect, Color color, float width);
    void stroke_rounded_rect(Rect rect, CornerRadius radius, Color color, float width);
    void stroke_rounded_rect(Rect rect, CornerRadius radius, BorderEdges borders);
    void end_frame();

    [[nodiscard]] bool initialized() const noexcept;
    [[nodiscard]] Size size() const noexcept;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace ouif
