#pragma once

#include <OUIF/Event.h>
#include <OUIF/Export.h>
#include <OUIF/Geometry.h>
#include <OUIF/Style.h>

#include <memory>
#include <optional>
#include <cstdint>
#include <type_traits>
#include <utility>
#include <vector>

namespace ouif {

class Renderer;

enum class SizePolicy : std::uint8_t {
    Fixed,
    Fill,
    Content,
};

enum class WidgetState : std::uint8_t {
    Selected,
    Focused,
};

struct Layout {
    Size preferred_size { 0.0f, 0.0f };
    Size min_size { 0.0f, 0.0f };
    Size max_size { 100000.0f, 100000.0f };
    Insets margin {};
    Insets padding {};
    float flex = 0.0f;
    SizePolicy width = SizePolicy::Fill;
    SizePolicy height = SizePolicy::Fill;
};

class OUIF_API Widget {
public:
    Widget() = default;
    virtual ~Widget();

    Widget(const Widget&) = delete;
    Widget& operator=(const Widget&) = delete;
    Widget(Widget&&) noexcept = delete;
    Widget& operator=(Widget&&) noexcept = delete;

    void set_bounds(Rect bounds) noexcept;
    [[nodiscard]] Rect bounds() const noexcept;
    void set_size(Size size) noexcept;

    void set_style(Style style) noexcept;
    [[nodiscard]] const Style& style() const noexcept;

    void set_layout(Layout layout) noexcept;
    [[nodiscard]] const Layout& layout_rules() const noexcept;
    void set_layout_policy(SizePolicy width, SizePolicy height) noexcept;
    void set_margin(Insets margin) noexcept;
    void set_padding(Insets padding) noexcept;
    void set_flex(float flex) noexcept;

    void set_visible(bool visible) noexcept;
    [[nodiscard]] bool visible() const noexcept;

    void set_enabled(bool enabled) noexcept;
    [[nodiscard]] bool enabled() const noexcept;

    Widget& add_child(Widget& child);
    Widget& add_child(std::unique_ptr<Widget> child);
    bool remove_child(Widget& child) noexcept;
    void clear_children() noexcept;

    template <typename... Widgets>
        requires(sizeof...(Widgets) > 0)
    Widget& children(Widgets&... widgets)
    {
        (add_child(widgets), ...);
        return *this;
    }

    template <typename T, typename... Args>
    T& add_child(Args&&... args)
    {
        static_assert(std::is_base_of_v<Widget, T>, "add_child<T> requires T to inherit ouif::Widget");
        auto child = std::make_unique<T>(std::forward<Args>(args)...);
        return static_cast<T&>(add_child(std::move(child)));
    }

    [[nodiscard]] const std::vector<Widget*>& children() const noexcept;
    [[nodiscard]] Widget* parent() noexcept;
    [[nodiscard]] const Widget* parent() const noexcept;

    void set_state(WidgetState state, bool enabled) noexcept;
    void toggle_state(WidgetState state) noexcept;
    [[nodiscard]] bool has_state(WidgetState state) const noexcept;

    [[nodiscard]] bool hovered() const noexcept;
    [[nodiscard]] bool pressed() const noexcept;
    [[nodiscard]] bool focused() const noexcept;
    void focus() noexcept;
    void blur() noexcept;
    [[nodiscard]] bool hit_test(Point point) const noexcept;

    virtual void layout(Size available);
    virtual void render(Renderer& renderer);
    virtual bool event(const Event& event);

protected:
    [[nodiscard]] std::vector<Widget*>& mutable_children() noexcept;
    virtual void draw(Renderer& renderer);
    virtual void on_layout(Rect content);
    virtual bool on_event(const Event& event);
    virtual void on_mouse_enter(const MouseEvent& event);
    virtual void on_mouse_leave(const MouseEvent& event);
    virtual void on_focus();
    virtual void on_blur();
    virtual bool on_mouse_move(const MouseEvent& event);
    virtual bool on_mouse_down(const MouseEvent& event);
    virtual bool on_mouse_up(const MouseEvent& event);
    virtual bool on_click(const MouseEvent& event);

private:
    void detach_from_parent() noexcept;
    bool detach_child(Widget& child, bool destroy_owned) noexcept;
    [[nodiscard]] bool owns_child(const Widget& child) const noexcept;

    [[nodiscard]] Point to_local(Point point) const noexcept;
    [[nodiscard]] std::optional<MouseEvent> mouse_event_from(const Event& event) const noexcept;

    Widget* parent_ = nullptr;
    Rect bounds_;
    Style style_;
    Layout layout_;
    std::vector<std::unique_ptr<Widget>> owned_children_;
    std::vector<Widget*> children_;
    bool visible_ = true;
    bool enabled_ = true;
    bool hovered_ = false;
    bool pressed_ = false;
    bool selected_ = false;
    bool focused_ = false;

    static Widget* focused_widget_;
};

} // namespace ouif
