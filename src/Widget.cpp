#include <OUIF/Widget.h>

#include <OUIF/Renderer.h>

#include <algorithm>
#include <stdexcept>

namespace ouif {

Widget::~Widget()
{
    detach_from_parent();

    for (auto* child : children_) {
        if (child != nullptr && child->parent_ == this) {
            child->parent_ = nullptr;
        }
    }
    children_.clear();
    owned_children_.clear();
}

void Widget::set_bounds(Rect bounds) noexcept
{
    bounds_ = bounds;
}

Rect Widget::bounds() const noexcept
{
    return bounds_;
}

void Widget::set_size(Size size) noexcept
{
    bounds_.width = size.width;
    bounds_.height = size.height;
    layout_.preferred_size = size;
    layout_.width = SizePolicy::Fixed;
    layout_.height = SizePolicy::Fixed;
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

void Widget::set_layout_policy(SizePolicy width, SizePolicy height) noexcept
{
    layout_.width = width;
    layout_.height = height;
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

Widget& Widget::add_child(Widget& child)
{
    if (&child == this) {
        throw std::invalid_argument("A widget cannot be added as its own child");
    }

    if (child.parent_ == this) {
        return child;
    }

    if (child.parent_ != nullptr) {
        if (child.parent_->owns_child(child)) {
            throw std::invalid_argument("Cannot reparent a widget owned by another parent");
        }
        child.parent_->detach_child(child, false);
    }

    children_.erase(std::remove(children_.begin(), children_.end(), &child), children_.end());
    child.parent_ = this;
    children_.push_back(&child);
    return child;
}

Widget& Widget::add_child(std::unique_ptr<Widget> child)
{
    if (!child) {
        throw std::invalid_argument("Cannot add a null widget child");
    }

    auto& reference = *child;
    if (&reference == this) {
        throw std::invalid_argument("A widget cannot be added as its own child");
    }

    if (reference.parent_ != nullptr) {
        reference.parent_->detach_child(reference, false);
    }

    children_.erase(std::remove(children_.begin(), children_.end(), &reference), children_.end());
    reference.parent_ = this;
    owned_children_.push_back(std::move(child));
    children_.push_back(&reference);
    return reference;
}

bool Widget::remove_child(Widget& child) noexcept
{
    return detach_child(child, true);
}

void Widget::clear_children() noexcept
{
    for (auto* child : children_) {
        if (child != nullptr && child->parent_ == this) {
            child->parent_ = nullptr;
        }
    }

    children_.clear();
    owned_children_.clear();
}

const std::vector<Widget*>& Widget::children() const noexcept
{
    return children_;
}

Widget* Widget::parent() noexcept
{
    return parent_;
}

const Widget* Widget::parent() const noexcept
{
    return parent_;
}

void Widget::set_state(WidgetState state, bool enabled) noexcept
{
    if (state == WidgetState::Selected) {
        selected_ = enabled;
    }
}

void Widget::toggle_state(WidgetState state) noexcept
{
    set_state(state, !has_state(state));
}

bool Widget::has_state(WidgetState state) const noexcept
{
    if (state == WidgetState::Selected) {
        return selected_;
    }
    return false;
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
    for (auto* child : children_) {
        if (child->bounds().width == 0.0f && child->bounds().height == 0.0f) {
            child->set_bounds(content);
        }
        const auto child_bounds = child->bounds();
        child->layout({ child_bounds.width, child_bounds.height });
    }
}

void Widget::render(Renderer& renderer)
{
    if (!visible_) {
        return;
    }

    draw(renderer);

    for (auto* child : children_) {
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

std::vector<Widget*>& Widget::mutable_children() noexcept
{
    return children_;
}

void Widget::draw(Renderer& renderer)
{
    if (style_.opacity <= 0.0f) {
        return;
    }

    const Color background = selected_ ? style_.selected : (pressed_ ? style_.pressed : (hovered_ ? style_.hovered : style_.background));
    renderer.fill_rect(bounds_, background);
    const Border border = selected_ && style_.border_selected.width > 0.0f ? style_.border_selected : style_.border;
    if (border.width > 0.0f) {
        renderer.stroke_rect(bounds_, border.color, border.width);
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

void Widget::detach_from_parent() noexcept
{
    if (parent_ != nullptr) {
        auto* parent = parent_;
        parent_ = nullptr;
        parent->detach_child(*this, false);
    }
}

bool Widget::detach_child(Widget& child, bool destroy_owned) noexcept
{
    bool removed = false;
    auto child_it = std::remove(children_.begin(), children_.end(), &child);
    if (child_it != children_.end()) {
        children_.erase(child_it, children_.end());
        removed = true;
    }

    if (child.parent_ == this) {
        child.parent_ = nullptr;
        removed = true;
    }

    auto owned_it = std::find_if(owned_children_.begin(), owned_children_.end(), [&child](const auto& owned) {
        return owned.get() == &child;
    });

    if (owned_it != owned_children_.end()) {
        removed = true;
        if (destroy_owned) {
            (*owned_it)->parent_ = nullptr;
            owned_children_.erase(owned_it);
        }
    }

    return removed;
}

bool Widget::owns_child(const Widget& child) const noexcept
{
    return std::any_of(owned_children_.begin(), owned_children_.end(), [&child](const auto& owned) {
        return owned.get() == &child;
    });
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
