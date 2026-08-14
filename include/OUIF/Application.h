#pragma once

#include <OUIF/Color.h>
#include <OUIF/Export.h>
#include <OUIF/Renderer.h>
#include <OUIF/Widget.h>
#include <OUIF/Xml.h>

#include <functional>
#include <memory>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <utility>

namespace ouif {

class XmlLoaderAccess;
using XmlWidgetFactory = std::function<std::unique_ptr<Widget>(const XmlElement&)>;

struct ApplicationConfig {
    std::string title = "OUIF Application";
    std::uint32_t width = 1280;
    std::uint32_t height = 720;
    void* native_window = nullptr;
    bool create_window = true;
    Color clear_color = Color::rgba(18, 20, 24, 255);
    RendererQualityConfig render_quality {};

    ApplicationConfig& with_title(std::string value)
    {
        title = std::move(value);
        return *this;
    }

    ApplicationConfig& with_size(std::uint32_t new_width, std::uint32_t new_height) noexcept
    {
        width = new_width;
        height = new_height;
        return *this;
    }

    ApplicationConfig& with_native_window(void* window) noexcept
    {
        native_window = window;
        create_window = false;
        return *this;
    }

    ApplicationConfig& with_clear_color(Color color) noexcept
    {
        clear_color = color;
        return *this;
    }

    ApplicationConfig& with_render_quality(RendererQuality quality) noexcept
    {
        render_quality.preset = quality;
        return *this;
    }

    ApplicationConfig& with_render_quality(RendererQualityConfig quality) noexcept
    {
        render_quality = quality;
        return *this;
    }
};

class OUIF_API Application {
public:
    explicit Application(ApplicationConfig config = {});
    ~Application();

    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;
    Application(Application&&) noexcept = delete;
    Application& operator=(Application&&) noexcept = delete;

    Widget& set_root(Widget& root);
    Widget& set_root(std::unique_ptr<Widget> root);

    template <typename T, typename... Args>
    T& set_root(Args&&... args)
    {
        static_assert(std::is_base_of_v<Widget, T>, "set_root<T> requires T to inherit ouif::Widget");
        auto root = std::make_unique<T>(std::forward<Args>(args)...);
        return static_cast<T&>(set_root(std::move(root)));
    }

    [[nodiscard]] Widget* root() noexcept;
    [[nodiscard]] const Widget* root() const noexcept;

    Application& register_xml_widget(std::string tag_name, XmlWidgetFactory factory);

    template <typename T>
    Application& register_xml_widget(std::string tag_name)
    {
        static_assert(std::is_base_of_v<Widget, T>, "register_xml_widget<T> requires T to inherit ouif::Widget");
        return register_xml_widget(std::move(tag_name), [](const XmlElement&) {
            return std::make_unique<T>();
        });
    }

    Widget& load_xml(std::string_view path);
    Widget& load_xml_string(std::string_view xml, std::string_view base_path = {});
    void load_stylesheet_file(std::string_view path);
    void join_stylesheet_file(std::string_view path);

    int run();
    void frame();
    bool dispatch_event(const Event& event);
    void request_exit() noexcept;

private:
    friend class XmlLoaderAccess;

    void initialize_renderer(void* native_window);

    ApplicationConfig config_;
    Renderer renderer_;
    std::unique_ptr<Widget> owned_root_;
    Widget* root_ = nullptr;
    std::unordered_map<std::string, XmlWidgetFactory> xml_factories_;
    bool running_ = false;
};

} // namespace ouif
