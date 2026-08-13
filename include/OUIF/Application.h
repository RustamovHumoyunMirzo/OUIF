#pragma once

#include <OUIF/Color.h>
#include <OUIF/Export.h>
#include <OUIF/Renderer.h>
#include <OUIF/Widget.h>

#include <memory>
#include <string>

namespace ouif {

struct ApplicationConfig {
    std::string title = "OUIF Application";
    std::uint32_t width = 1280;
    std::uint32_t height = 720;
    void* native_window = nullptr;
    bool create_window = true;
    Color clear_color = Color::rgba(18, 20, 24, 255);
};

class OUIF_API Application {
public:
    explicit Application(ApplicationConfig config = {});
    ~Application();

    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;
    Application(Application&&) noexcept;
    Application& operator=(Application&&) noexcept;

    void set_root(std::unique_ptr<Widget> root);
    [[nodiscard]] Widget* root() noexcept;
    [[nodiscard]] const Widget* root() const noexcept;

    int run();
    void frame();
    bool dispatch_event(const Event& event);
    void request_exit() noexcept;

private:
    void initialize_renderer(void* native_window);

    ApplicationConfig config_;
    Renderer renderer_;
    std::unique_ptr<Widget> root_;
    bool running_ = false;
};

} // namespace ouif
