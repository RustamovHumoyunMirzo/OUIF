#include <OUIF/Widget.h>

#include <OUIF/Renderer.h>

#include <algorithm>

namespace ouif {

void Widget::set_bounds(Rect bounds) noexcept
{
    bounds_ = bounds;
}

Rect Widget::bounds() const noexcept
{
    return bounds_;
}

void Widget::set_style(Style style) noexcept
{
    style_ = style;
}

const Style& Widget::style() const noexcept
{
    return style_;
}

void Widget::set_layout(Layout layout) noexcept
{
    layout_ = layout;
}

const Layout& Widget::layout_rules() const noexcept
{
    return layout_;
}

void Widget::set_visible(bool visible) noexcept
{
    visible_ = visible;
}

bool Widget::visible() const noexcept
{
    return visible_;
}

void Widget::set_enabled(bool enabled) noexcept
{
    enabled_ = enabled;
}

bool Widget::enabled() const noexcept
{
    return enabled_;
}

void Widget::add_child(std::unique_ptr<Widget> child)
{
    children_.push_back(std::move(child));
}

const std::vector<std::unique_ptr<Widget>>& Widget::children() const noexcept
{
    return children_;
}

bool Widget::hovered() const noexcept
{
    return hovered_;
}

bool Widget::pressed() const noexcept
{
    return pressed_;
}

bool Widget::hit_test(Point point) const noexcept
{
    return visible_ && enabled_ && bounds_.contains(point);
}

void Widget::layout(Size available)
{
    const auto clamp_width = [this](float value) {
        return std::clamp(value, layout_.min_size.width, layout_.max_size.width);
    };
    const auto clamp_height = [this](float value) {
        return std::clamp(value, layout_.min_size.height, layout_.max_size.height);
    };

    if (bounds_.width == 0.0f || layout_.width == SizePolicy::Fill) {
        bounds_.width = clamp_width(available.width);
    } else if (layout_.width == SizePolicy::Fixed) {
        bounds_.width = clamp_width(layout_.preferred_size.width);
    }

    if (bounds_.height == 0.0f || layout_.height == SizePolicy::Fill) {
        bounds_.height = clamp_height(available.height);
    } else if (layout_.height == SizePolicy::Fixed) {
        bounds_.height = clamp_height(layout_.preferred_size.height);
    }

    const Rect content = bounds_.inset(layout_.padding);
    on_layout(content);
    for (const auto& child : children_) {
        if (child->bounds().width == 0.0f && child->bounds().height == 0.0f) {
            child->set_bounds(content);
        }
        child->layout({ content.width, content.height });
    }
}

void Widget::render(Renderer& renderer)
{
    if (!visible_) {
        return;
    }

    draw(renderer);

    for (const auto& child : children_) {
        child->render(renderer);
    }
}

bool Widget::event(const Event& event)
{
    if (!visible_ || !enabled_) {
        return false;
    }

    const auto mouse = mouse_event_from(event);

    for (auto it = children_.rbegin(); it != children_.rend(); ++it) {
        if ((*it)->event(event)) {
            return true;
        }
    }

    if (!mouse.has_value()) {
        return on_event(event);
    }

    const bool inside = hit_test(mouse->position);
    if (mouse->type == MouseEventType::Move) {
        if (inside && !hovered_) {
            hovered_ = true;
            MouseEvent enter = *mouse;
            enter.type = MouseEventType::Enter;
            on_mouse_enter(enter);
        } else if (!inside && hovered_) {
            hovered_ = false;
            pressed_ = false;
            MouseEvent leave = *mouse;
            leave.type = MouseEventType::Leave;
            on_mouse_leave(leave);
        }

        return inside && on_mouse_move(*mouse);
    }

    if (!inside && !pressed_) {
        return false;
    }

    if (mouse->type == MouseEventType::Down) {
        pressed_ = inside;
        return inside && on_mouse_down(*mouse);
    }

    if (mouse->type == MouseEventType::Up) {
        const bool clicked = pressed_ && inside;
        pressed_ = false;
        const bool handled = on_mouse_up(*mouse);
        if (clicked) {
            MouseEvent click = *mouse;
            click.type = MouseEventType::Click;
            return on_click(click) || handled;
        }
        return handled;
    }

    return on_event(event);
}

void Widget::draw(Renderer& renderer)
{
    if (style_.opacity <= 0.0f) {
        return;
    }

    const Color background = pressed_ ? style_.background_pressed : (hovered_ ? style_.background_hovered : style_.background);
    renderer.fill_rect(bounds_, background);
    if (style_.border_width > 0.0f) {
        renderer.stroke_rect(bounds_, style_.border, style_.border_width);
    }
}

void Widget::on_layout(Rect content)
{
    (void)content;
}

bool Widget::on_event(const Event& event)
{
    (void)event;
    return false;
}

void Widget::on_mouse_enter(const MouseEvent& event)
{
    (void)event;
}

void Widget::on_mouse_leave(const MouseEvent& event)
{
    (void)event;
}

bool Widget::on_mouse_move(const MouseEvent& event)
{
    (void)event;
    return false;
}

bool Widget::on_mouse_down(const MouseEvent& event)
{
    (void)event;
    return true;
}

bool Widget::on_mouse_up(const MouseEvent& event)
{
    (void)event;
    return true;
}

bool Widget::on_click(const MouseEvent& event)
{
    (void)event;
    return false;
}

Point Widget::to_local(Point point) const noexcept
{
    return { point.x - bounds_.x, point.y - bounds_.y };
}

std::optional<MouseEvent> Widget::mouse_event_from(const Event& event) const noexcept
{
    if (const auto* move = std::get_if<MouseMoveEvent>(&event)) {
        return MouseEvent { MouseEventType::Move, move->position, to_local(move->position), MouseButton::Left };
    }

    if (const auto* button = std::get_if<MouseButtonEvent>(&event)) {
        return MouseEvent {
            button->pressed ? MouseEventType::Down : MouseEventType::Up,
            button->position,
            to_local(button->position),
            button->button,
        };
    }

    if (const auto* mouse = std::get_if<MouseEvent>(&event)) {
        MouseEvent local = *mouse;
        local.local_position = to_local(mouse->position);
        return local;
    }

    return std::nullopt;
}

} // namespace ouif
