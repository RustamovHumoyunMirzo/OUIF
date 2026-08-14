#pragma once

#include <OUIF/Event.h>
#include <OUIF/Export.h>
#include <OUIF/Geometry.h>
#include <OUIF/Style.h>

#include <memory>
#include <optional>
#include <cstdint>
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
};

struct Layout {
    Size preferred_size { 0.0f, 0.0f };
    Size min_size { 0.0f, 0.0f };
    Size max_size { 100000.0f, 100000.0f };
    Insets padding {};
    SizePolicy width = SizePolicy::Fill;
    SizePolicy height = SizePolicy::Fill;
};

class OUIF_API Widget {
public:
    Widget() = default;
    virtual ~Widget() = default;

    Widget(const Widget&) = delete;
    Widget& operator=(const Widget&) = delete;
    Widget(Widget&&) noexcept = default;
    Widget& operator=(Widget&&) noexcept = default;

    void set_bounds(Rect bounds) noexcept;
    [[nodiscard]] Rect bounds() const noexcept;
    void set_size(Size size) noexcept;

    void set_style(Style style) noexcept;
    [[nodiscard]] const Style& style() const noexcept;

    void set_layout(Layout layout) noexcept;
    [[nodiscard]] const Layout& layout_rules() const noexcept;
    void set_layout_policy(SizePolicy width, SizePolicy height) noexcept;

    void set_visible(bool visible) noexcept;
    [[nodiscard]] bool visible() const noexcept;

    void set_enabled(bool enabled) noexcept;
    [[nodiscard]] bool enabled() const noexcept;

    void add_child(std::unique_ptr<Widget> child);
    [[nodiscard]] const std::vector<std::unique_ptr<Widget>>& children() const noexcept;

    void set_state(WidgetState state, bool enabled) noexcept;
    void toggle_state(WidgetState state) noexcept;
    [[nodiscard]] bool has_state(WidgetState state) const noexcept;

    [[nodiscard]] bool hovered() const noexcept;
    [[nodiscard]] bool pressed() const noexcept;
    [[nodiscard]] bool hit_test(Point point) const noexcept;

    virtual void layout(Size available);
    virtual void render(Renderer& renderer);
    virtual bool event(const Event& event);

protected:
    [[nodiscard]] std::vector<std::unique_ptr<Widget>>& mutable_children() noexcept;
    virtual void draw(Renderer& renderer);
    virtual void on_layout(Rect content);
    virtual bool on_event(const Event& event);
    virtual void on_mouse_enter(const MouseEvent& event);
    virtual void on_mouse_leave(const MouseEvent& event);
    virtual bool on_mouse_move(const MouseEvent& event);
    virtual bool on_mouse_down(const MouseEvent& event);
    virtual bool on_mouse_up(const MouseEvent& event);
    virtual bool on_click(const MouseEvent& event);

private:
    [[nodiscard]] Point to_local(Point point) const noexcept;
    [[nodiscard]] std::optional<MouseEvent> mouse_event_from(const Event& event) const noexcept;

    Rect bounds_;
    Style style_;
    Layout layout_;
    std::vector<std::unique_ptr<Widget>> children_;
    bool visible_ = true;
    bool enabled_ = true;
    bool hovered_ = false;
    bool pressed_ = false;
    bool selected_ = false;
};

} // namespace ouif
