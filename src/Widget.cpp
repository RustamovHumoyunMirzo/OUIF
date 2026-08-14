#include <OUIF/Widget.h>

#include <OUIF/Renderer.h>

#include <algorithm>
#include <cctype>
#include <cstring>
#include <functional>
#include <stdexcept>
#include <string>
#include <string_view>

#if OUIF_WITH_KATANA
#include <katana.h>
#endif

namespace ouif {

Widget* Widget::focused_widget_ = nullptr;

namespace {

#if OUIF_WITH_KATANA
struct KatanaOutputDeleter {
    void operator()(KatanaOutput* output) const noexcept
    {
        if (output != nullptr) {
            katana_destroy_output(output);
        }
    }
};

using KatanaOutputPtr = std::unique_ptr<KatanaOutput, KatanaOutputDeleter>;

enum class CssState {
    Base,
    Hover,
    Pressed,
    Selected,
    Focus,
};

std::string_view text_or_empty(const char* text) noexcept
{
    return text != nullptr ? std::string_view(text) : std::string_view();
}

std::string lower_copy(std::string_view value)
{
    std::string copy(value);
    std::transform(copy.begin(), copy.end(), copy.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return copy;
}

bool equals_ignore_case(std::string_view left, std::string_view right)
{
    return lower_copy(left) == lower_copy(right);
}

std::optional<Color> color_from_value(const KatanaValue& value)
{
    if (value.unit == KATANA_VALUE_PARSER_HEXCOLOR || value.unit == KATANA_VALUE_RGBCOLOR) {
        return Color::from_hex(text_or_empty(value.string));
    }

    if (value.unit == KATANA_VALUE_IDENT || value.unit == KATANA_VALUE_STRING) {
        return Color::from_hex(text_or_empty(value.string));
    }

    return std::nullopt;
}

std::optional<float> pixels_from_value(const KatanaValue& value)
{
    if (value.unit == KATANA_VALUE_PARSER_INTEGER) {
        return static_cast<float>(value.iValue);
    }
    if (value.unit == KATANA_VALUE_NUMBER || value.unit == KATANA_VALUE_PX) {
        return static_cast<float>(value.fValue);
    }

    return std::nullopt;
}

Length length_from_value(const KatanaValue& value)
{
    const auto number = static_cast<float>(value.fValue);
    switch (value.unit) {
    case KATANA_VALUE_PARSER_INTEGER:
        return Length::px(static_cast<float>(value.iValue));
    case KATANA_VALUE_NUMBER:
    case KATANA_VALUE_PX:
        return Length::px(number);
    case KATANA_VALUE_PERCENTAGE:
        return Length::percent(number);
    case KATANA_VALUE_VW:
        return Length::vw(number);
    case KATANA_VALUE_VH:
        return Length::vh(number);
    default:
        return {};
    }
}

KatanaValue* value_at(KatanaArray* values, unsigned int index)
{
    if (values == nullptr || index >= values->length) {
        return nullptr;
    }
    return static_cast<KatanaValue*>(values->data[index]);
}

void apply_background(Style& style, CssState state, Color color)
{
    switch (state) {
    case CssState::Hover:
        style.with_background_hovered(color);
        break;
    case CssState::Pressed:
        style.with_background_pressed(color);
        break;
    case CssState::Selected:
        style.with_background_selected(color);
        break;
    case CssState::Focus:
        style.with_background_focused(color);
        break;
    case CssState::Base:
        style.with_background(color);
        break;
    }
}

void apply_border(Style& style, CssState state, Color color, float width)
{
    if (state == CssState::Selected) {
        style.with_border_selected(color, width);
    } else if (state == CssState::Focus) {
        style.with_border_focused(color, width);
    } else {
        style.with_border(color, width);
    }
}

void apply_declaration(Widget& widget, Style& style, KatanaDeclaration& declaration, CssState state, bool& touched_style)
{
    const auto property = lower_copy(text_or_empty(declaration.property));
    auto* first = value_at(declaration.values, 0);
    if (first == nullptr) {
        return;
    }

    if (property == "background" || property == "background-color" || property == "with-background") {
        if (auto color = color_from_value(*first)) {
            apply_background(style, state, *color);
            touched_style = true;
        }
        return;
    }

    if (property == "background-hovered" || property == "with-background-hovered") {
        if (auto color = color_from_value(*first)) {
            style.with_background_hovered(*color);
            touched_style = true;
        }
        return;
    }

    if (property == "background-pressed" || property == "with-background-pressed") {
        if (auto color = color_from_value(*first)) {
            style.with_background_pressed(*color);
            touched_style = true;
        }
        return;
    }

    if (property == "background-selected" || property == "with-background-selected") {
        if (auto color = color_from_value(*first)) {
            style.with_background_selected(*color);
            touched_style = true;
        }
        return;
    }

    if (property == "background-focused" || property == "with-background-focused") {
        if (auto color = color_from_value(*first)) {
            style.with_background_focused(*color);
            touched_style = true;
        }
        return;
    }

    if (property == "foreground" || property == "color" || property == "with-foreground") {
        if (auto color = color_from_value(*first)) {
            style.with_foreground(*color);
            touched_style = true;
        }
        return;
    }

    if (property == "border" || property == "border-selected" || property == "border-focused") {
        std::optional<Color> border_color;
        std::optional<float> border_width;
        for (unsigned int index = 0; declaration.values != nullptr && index < declaration.values->length; ++index) {
            auto* value = value_at(declaration.values, index);
            if (value == nullptr) {
                continue;
            }
            if (!border_color) {
                border_color = color_from_value(*value);
            }
            if (!border_width) {
                border_width = pixels_from_value(*value);
            }
        }
        if (border_color && border_width) {
            const CssState target = property == "border-selected" ? CssState::Selected : (property == "border-focused" ? CssState::Focus : state);
            apply_border(style, target, *border_color, *border_width);
            touched_style = true;
        }
        return;
    }

    if (property == "border-width") {
        if (auto width = pixels_from_value(*first)) {
            style.border.width = *width;
            style.border_width = *width;
            touched_style = true;
        }
        return;
    }

    if (property == "radius" || property == "border-radius" || property == "with-radius") {
        if (auto radius = pixels_from_value(*first)) {
            style.with_radius(*radius);
            touched_style = true;
        }
        return;
    }

    if (property == "radius-top-left" || property == "border-top-left-radius") {
        if (auto radius = pixels_from_value(*first)) {
            style.radius.top_left = *radius;
            touched_style = true;
        }
        return;
    }

    if (property == "radius-top-right" || property == "border-top-right-radius") {
        if (auto radius = pixels_from_value(*first)) {
            style.radius.top_right = *radius;
            touched_style = true;
        }
        return;
    }

    if (property == "radius-bottom-right" || property == "border-bottom-right-radius") {
        if (auto radius = pixels_from_value(*first)) {
            style.radius.bottom_right = *radius;
            touched_style = true;
        }
        return;
    }

    if (property == "radius-bottom-left" || property == "border-bottom-left-radius") {
        if (auto radius = pixels_from_value(*first)) {
            style.radius.bottom_left = *radius;
            touched_style = true;
        }
        return;
    }

    if (property == "opacity") {
        if (auto opacity = pixels_from_value(*first)) {
            style.with_opacity(std::clamp(*opacity, 0.0f, 1.0f));
            touched_style = true;
        }
        return;
    }

    if (property == "width") {
        widget.set_width(length_from_value(*first));
        return;
    }

    if (property == "height") {
        widget.set_height(length_from_value(*first));
        return;
    }

    if (property == "flex") {
        if (auto flex = pixels_from_value(*first)) {
            widget.set_flex(*flex);
        }
        return;
    }

    if (property == "margin" || property == "padding") {
        float parts[4] {};
        unsigned int count = 0;
        for (; declaration.values != nullptr && count < declaration.values->length && count < 4; ++count) {
            auto* value = value_at(declaration.values, count);
            parts[count] = value != nullptr ? pixels_from_value(*value).value_or(0.0f) : 0.0f;
        }
        Insets insets;
        if (count == 1) {
            insets = Insets(parts[0]);
        } else if (count == 2) {
            insets = Insets(parts[1], parts[0]);
        } else if (count == 3) {
            insets = Insets(parts[1], parts[0], parts[1], parts[2]);
        } else if (count >= 4) {
            insets = Insets(parts[3], parts[0], parts[1], parts[2]);
        }

        if (property == "margin") {
            widget.set_margin(insets);
        } else {
            widget.set_padding(insets);
        }
        return;
    }
}

bool selector_matches(Widget& widget, KatanaSelector* selector, CssState& state)
{
    for (auto* current = selector; current != nullptr; current = current->tagHistory) {
        if (current->match == KatanaSelectorMatchTag) {
            const auto tag = current->tag != nullptr ? text_or_empty(current->tag->local) : std::string_view();
            if (!tag.empty() && tag != "*" && !equals_ignore_case(tag, widget.type_name()) && !equals_ignore_case(tag, "Widget")) {
                return false;
            }
        } else if (current->match == KatanaSelectorMatchId) {
            if (current->data == nullptr || !equals_ignore_case(text_or_empty(current->data->value), widget.name())) {
                return false;
            }
        } else if (current->match == KatanaSelectorMatchClass) {
            if (current->data == nullptr || !widget.has_class(text_or_empty(current->data->value))) {
                return false;
            }
        } else if (current->match == KatanaSelectorMatchPseudoClass) {
            switch (current->pseudo) {
            case KatanaPseudoHover:
                state = CssState::Hover;
                break;
            case KatanaPseudoActive:
                state = CssState::Pressed;
                break;
            case KatanaPseudoChecked:
                state = CssState::Selected;
                break;
            case KatanaPseudoFocus:
                state = CssState::Focus;
                break;
            default:
                if (current->data != nullptr && equals_ignore_case(text_or_empty(current->data->value), "selected")) {
                    state = CssState::Selected;
                } else if (current->data != nullptr && equals_ignore_case(text_or_empty(current->data->value), "pressed")) {
                    state = CssState::Pressed;
                } else {
                    return false;
                }
                break;
            }
        }

        if (current == current->tagHistory) {
            break;
        }
    }

    return true;
}

bool apply_stylesheet_rule(Widget& widget, KatanaStyleRule& rule, Style& css_style)
{
    bool touched_style = false;
    bool matched = false;
    CssState state = CssState::Base;

    for (unsigned int selector_index = 0; rule.selectors != nullptr && selector_index < rule.selectors->length; ++selector_index) {
        CssState selector_state = CssState::Base;
        if (selector_matches(widget, static_cast<KatanaSelector*>(rule.selectors->data[selector_index]), selector_state)) {
            state = selector_state;
            matched = true;
            break;
        }
    }

    if (!matched || rule.declarations == nullptr) {
        return false;
    }

    for (unsigned int declaration_index = 0; declaration_index < rule.declarations->length; ++declaration_index) {
        apply_declaration(widget, css_style, *static_cast<KatanaDeclaration*>(rule.declarations->data[declaration_index]), state, touched_style);
    }

    return touched_style;
}
#endif

float resolve_length(Length length, Size available, bool horizontal)
{
    return length.automatic() ? 0.0f : length.resolve(available, horizontal);
}

} // namespace

Widget::~Widget()
{
    if (focused_widget_ == this) {
        focused_widget_ = nullptr;
    }

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
    if (size.width > 0.0f) {
        bounds_.width = size.width;
        layout_.preferred_size.width = size.width;
        layout_.width = SizePolicy::Fixed;
    }

    if (size.height > 0.0f) {
        bounds_.height = size.height;
        layout_.preferred_size.height = size.height;
        layout_.height = SizePolicy::Fixed;
    }

    layout_.preferred_size = size;
}

void Widget::set_width(Length width) noexcept
{
    layout_.width_value = width;
    if (width.automatic()) {
        return;
    }
    layout_.width = SizePolicy::Fixed;
    if (width.unit == LengthUnit::Px) {
        bounds_.width = width.value;
        layout_.preferred_size.width = width.value;
    }
}

void Widget::set_height(Length height) noexcept
{
    layout_.height_value = height;
    if (height.automatic()) {
        return;
    }
    layout_.height = SizePolicy::Fixed;
    if (height.unit == LengthUnit::Px) {
        bounds_.height = height.value;
        layout_.preferred_size.height = height.value;
    }
}

void Widget::set_size(Length width, Length height) noexcept
{
    set_width(width);
    set_height(height);
}

void Widget::set_style(Style style) noexcept
{
    inline_style_ = style;
    has_inline_style_ = true;
    recompute_style();
}

const Style& Widget::style() const noexcept
{
    return style_;
}

const Style& Widget::get_style() const noexcept
{
    return style();
}

void Widget::set_background(Color color) noexcept
{
    auto next = style_;
    next.with_background(color);
    set_style(next);
}

Color Widget::get_background() const noexcept
{
    return style_.background;
}

void Widget::set_background_hovered(Color color) noexcept
{
    auto next = style_;
    next.with_background_hovered(color);
    set_style(next);
}

Color Widget::get_background_hovered() const noexcept
{
    return style_.hovered;
}

void Widget::set_background_pressed(Color color) noexcept
{
    auto next = style_;
    next.with_background_pressed(color);
    set_style(next);
}

Color Widget::get_background_pressed() const noexcept
{
    return style_.pressed;
}

void Widget::set_background_selected(Color color) noexcept
{
    auto next = style_;
    next.with_background_selected(color);
    set_style(next);
}

Color Widget::get_background_selected() const noexcept
{
    return style_.selected;
}

void Widget::set_background_focused(Color color) noexcept
{
    auto next = style_;
    next.with_background_focused(color);
    set_style(next);
}

Color Widget::get_background_focused() const noexcept
{
    return style_.focused;
}

void Widget::set_foreground(Color color) noexcept
{
    auto next = style_;
    next.with_foreground(color);
    set_style(next);
}

Color Widget::get_foreground() const noexcept
{
    return style_.foreground;
}

void Widget::set_border(Color color, float width) noexcept
{
    auto next = style_;
    next.with_border(color, width);
    set_style(next);
}

Border Widget::get_border() const noexcept
{
    return style_.border;
}

void Widget::set_border_selected(Color color, float width) noexcept
{
    auto next = style_;
    next.with_border_selected(color, width);
    set_style(next);
}

Border Widget::get_border_selected() const noexcept
{
    return style_.border_selected;
}

void Widget::set_border_focused(Color color, float width) noexcept
{
    auto next = style_;
    next.with_border_focused(color, width);
    set_style(next);
}

Border Widget::get_border_focused() const noexcept
{
    return style_.border_focused;
}

void Widget::set_radius(float radius) noexcept
{
    auto next = style_;
    next.with_radius(radius);
    set_style(next);
}

void Widget::set_radius(CornerRadius radius) noexcept
{
    auto next = style_;
    next.with_radius(radius);
    set_style(next);
}

CornerRadius Widget::get_radius() const noexcept
{
    return style_.radius;
}

void Widget::set_opacity(float opacity) noexcept
{
    auto next = style_;
    next.with_opacity(std::clamp(opacity, 0.0f, 1.0f));
    set_style(next);
}

float Widget::get_opacity() const noexcept
{
    return style_.opacity;
}

void Widget::set_stylesheet(std::string stylesheet)
{
    stylesheet_ = std::move(stylesheet);
    apply_stylesheet_to_tree();
}

void Widget::join_stylesheet(std::string_view stylesheet)
{
    if (!stylesheet_.empty() && !stylesheet.empty()) {
        stylesheet_.push_back('\n');
    }
    stylesheet_.append(stylesheet);
    apply_stylesheet_to_tree();
}

std::string_view Widget::get_stylesheet() const noexcept
{
    return stylesheet_;
}

void Widget::set_name(std::string name)
{
    name_ = std::move(name);
    if (!stylesheet_.empty()) {
        apply_stylesheet_to_tree();
    } else if (parent_ != nullptr) {
        parent_->apply_stylesheet_to_tree();
    }
}

std::string_view Widget::name() const noexcept
{
    return name_;
}

std::string_view Widget::get_name() const noexcept
{
    return name();
}

void Widget::set_type_name(std::string type_name)
{
    type_name_ = std::move(type_name);
    if (type_name_.empty()) {
        type_name_ = "Widget";
    }
    if (!stylesheet_.empty()) {
        apply_stylesheet_to_tree();
    } else if (parent_ != nullptr) {
        parent_->apply_stylesheet_to_tree();
    }
}

std::string_view Widget::type_name() const noexcept
{
    return type_name_;
}

Widget& Widget::add_class(std::string class_name)
{
    if (!class_name.empty() && !has_class(class_name)) {
        classes_.push_back(std::move(class_name));
        if (!stylesheet_.empty()) {
            apply_stylesheet_to_tree();
        } else if (parent_ != nullptr) {
            parent_->apply_stylesheet_to_tree();
        }
    }
    return *this;
}

bool Widget::remove_class(std::string_view class_name)
{
    const auto before = classes_.size();
    classes_.erase(std::remove_if(classes_.begin(), classes_.end(), [class_name](const auto& current) {
        return current == class_name;
    }), classes_.end());
    const bool removed = classes_.size() != before;
    if (removed) {
        if (!stylesheet_.empty()) {
            apply_stylesheet_to_tree();
        } else if (parent_ != nullptr) {
            parent_->apply_stylesheet_to_tree();
        }
    }
    return removed;
}

bool Widget::has_class(std::string_view class_name) const noexcept
{
    return std::any_of(classes_.begin(), classes_.end(), [class_name](const auto& current) {
        return current == class_name;
    });
}

const std::vector<std::string>& Widget::classes() const noexcept
{
    return classes_;
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

void Widget::set_margin(Insets margin) noexcept
{
    layout_.margin = margin;
}

Insets Widget::get_margin() const noexcept
{
    return layout_.margin;
}

void Widget::set_padding(Insets padding) noexcept
{
    layout_.padding = padding;
}

Insets Widget::get_padding() const noexcept
{
    return layout_.padding;
}

void Widget::set_flex(float flex) noexcept
{
    layout_.flex = std::max(0.0f, flex);
}

float Widget::get_flex() const noexcept
{
    return layout_.flex;
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
    apply_stylesheet_to_tree();
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
    apply_stylesheet_to_tree();
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
    } else if (state == WidgetState::Focused) {
        if (enabled) {
            focus();
        } else {
            blur();
        }
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
    if (state == WidgetState::Focused) {
        return focused_;
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

bool Widget::focused() const noexcept
{
    return focused_;
}

void Widget::focus() noexcept
{
    if (focused_widget_ == this) {
        return;
    }

    if (focused_widget_ != nullptr) {
        focused_widget_->blur();
    }

    focused_widget_ = this;
    focused_ = true;
    on_focus();
}

void Widget::blur() noexcept
{
    if (!focused_) {
        return;
    }

    focused_ = false;
    if (focused_widget_ == this) {
        focused_widget_ = nullptr;
    }
    on_blur();
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

    if (!layout_.width_value.automatic()) {
        if (parent_ == nullptr || bounds_.width == 0.0f) {
            bounds_.width = clamp_width(resolve_length(layout_.width_value, available, true));
        }
    } else if (bounds_.width == 0.0f || layout_.width == SizePolicy::Fill) {
        bounds_.width = clamp_width(available.width);
    } else if (layout_.width == SizePolicy::Fixed) {
        bounds_.width = clamp_width(layout_.preferred_size.width);
    }

    if (!layout_.height_value.automatic()) {
        if (parent_ == nullptr || bounds_.height == 0.0f) {
            bounds_.height = clamp_height(resolve_length(layout_.height_value, available, false));
        }
    } else if (bounds_.height == 0.0f || layout_.height == SizePolicy::Fill) {
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
        if (inside) {
            focus();
        }
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

    const Color background = selected_ ? style_.selected : (focused_ ? style_.focused : (pressed_ ? style_.pressed : (hovered_ ? style_.hovered : style_.background)));
    renderer.fill_rounded_rect(bounds_, style_.radius, background);
    const Border border = selected_ && style_.border_selected.width > 0.0f
        ? style_.border_selected
        : (focused_ && style_.border_focused.width > 0.0f ? style_.border_focused : style_.border);
    if (border.width > 0.0f) {
        renderer.stroke_rounded_rect(bounds_, style_.radius, border.color, border.width);
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

void Widget::on_focus()
{
}

void Widget::on_blur()
{
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

void Widget::recompute_style() noexcept
{
    style_ = has_stylesheet_style_ ? stylesheet_style_ : Style {};
    if (has_inline_style_) {
        style_ = inline_style_;
    }
}

void Widget::apply_stylesheet_to_tree()
{
#if OUIF_WITH_KATANA
    if (stylesheet_.empty()) {
        has_stylesheet_style_ = false;
        stylesheet_style_ = Style {};
        recompute_style();
        for (auto* child : children_) {
            child->apply_stylesheet_to_tree();
        }
        return;
    }

    KatanaOutputPtr output(katana_parse(stylesheet_.c_str(), stylesheet_.size(), KatanaParserModeStylesheet));
    if (!output || output->errors.length > 0 || output->stylesheet == nullptr) {
        return;
    }

    const auto apply_to = [&](Widget& widget) {
        Style css_style {};
        bool touched = false;
        for (unsigned int rule_index = 0; rule_index < output->stylesheet->rules.length; ++rule_index) {
            auto* base_rule = static_cast<KatanaRule*>(output->stylesheet->rules.data[rule_index]);
            if (base_rule == nullptr || base_rule->type != KatanaRuleStyle) {
                continue;
            }
            touched = apply_stylesheet_rule(widget, *reinterpret_cast<KatanaStyleRule*>(base_rule), css_style) || touched;
        }
        widget.stylesheet_style_ = css_style;
        widget.has_stylesheet_style_ = touched;
        widget.recompute_style();
    };

    const std::function<void(Widget&)> apply_recursive = [&](Widget& widget) {
        apply_to(widget);
        for (auto* child : widget.children_) {
            if (child != nullptr) {
                apply_recursive(*child);
            }
        }
    };

    apply_recursive(*this);
#else
    (void)this;
#endif
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
