#include <OUIF/Widgets.h>

#include <OUIF/Renderer.h>

#include <algorithm>
#include <utility>

namespace ouif {

Spacer::Spacer() noexcept
{
    set_accepts_children(false);
}

Spacer::Spacer(float flex) noexcept
{
    set_accepts_children(false);
    set_flex(flex);
}

Spacer::Spacer(Size size) noexcept
{
    set_accepts_children(false);
    set_size(size);
}

bool Spacer::event(const Event& event)
{
    (void)event;
    return false;
}

void Spacer::draw(Renderer& renderer)
{
    (void)renderer;
}

Divider::Divider(Orientation orientation, float thickness)
    : orientation_(orientation)
    , thickness_(std::max(0.0f, thickness))
{
    set_accepts_children(false);
    apply_axis_size();
}

void Divider::set_orientation(Orientation orientation) noexcept
{
    orientation_ = orientation;
    apply_axis_size();
}

Orientation Divider::orientation() const noexcept
{
    return orientation_;
}

void Divider::set_thickness(float thickness) noexcept
{
    thickness_ = std::max(0.0f, thickness);
    apply_axis_size();
}

float Divider::thickness() const noexcept
{
    return thickness_;
}

void Divider::set_color(Color color) noexcept
{
    set_background(color);
}

Color Divider::color() const noexcept
{
    return get_background();
}

bool Divider::event(const Event& event)
{
    (void)event;
    return false;
}

void Divider::draw(Renderer& renderer)
{
    renderer.fill_rect(bounds(), color());
}

void Divider::apply_axis_size() noexcept
{
    if (orientation_ == Orientation::Horizontal) {
        set_layout_policy(SizePolicy::Fill, SizePolicy::Fixed);
        set_size({ 0.0f, thickness_ });
    } else {
        set_layout_policy(SizePolicy::Fixed, SizePolicy::Fill);
        set_size({ thickness_, 0.0f });
    }
}

Label::Label()
{
    set_accepts_children(false);
    set_size({ 160.0f, 32.0f });
}

Label::Label(std::string text)
    : Label()
{
    set_text(std::move(text));
}

void Label::set_text(std::string text)
{
    text_ = std::move(text);
}

std::string_view Label::text() const noexcept
{
    return text_;
}

std::string_view Label::get_text() const noexcept
{
    return text();
}

void Label::set_text_style(TextStyle style) noexcept
{
    text_style_ = std::move(style);
    has_text_color_ = true;
}

void Label::set_text_style(InheritTag) noexcept
{
    if (auto* parent_label = dynamic_cast<const Label*>(parent())) {
        set_text_style(parent_label->get_text_style());
    } else if (parent() != nullptr) {
        text_style_.color = parent()->get_foreground();
        has_text_color_ = true;
    }
}

const TextStyle& Label::text_style() const noexcept
{
    return text_style_;
}

const TextStyle& Label::get_text_style() const noexcept
{
    return text_style();
}

void Label::set_font_family(std::string family)
{
    text_style_.font_family = std::move(family);
}

void Label::set_font_family(InheritTag)
{
    if (auto* parent_label = dynamic_cast<const Label*>(parent())) {
        set_font_family(std::string(parent_label->font_family()));
    }
}

std::string_view Label::font_family() const noexcept
{
    return text_style_.font_family;
}

void Label::set_font_size(float size) noexcept
{
    text_style_.font_size = std::max(1.0f, size);
}

void Label::set_font_size(InheritTag) noexcept
{
    if (auto* parent_label = dynamic_cast<const Label*>(parent())) {
        set_font_size(parent_label->font_size());
    }
}

float Label::font_size() const noexcept
{
    return text_style_.font_size;
}

void Label::set_text_color(Color color) noexcept
{
    text_style_.color = color;
    has_text_color_ = true;
}

void Label::set_text_color(InheritTag) noexcept
{
    if (auto* parent_label = dynamic_cast<const Label*>(parent())) {
        set_text_color(parent_label->text_color());
    } else if (parent() != nullptr) {
        set_text_color(parent()->get_foreground());
    }
}

Color Label::text_color() const noexcept
{
    return has_text_color_ ? text_style_.color : get_style().foreground;
}

void Label::set_text_align(TextAlign align) noexcept
{
    text_style_.align = align;
}

void Label::set_text_align(InheritTag) noexcept
{
    if (auto* parent_label = dynamic_cast<const Label*>(parent())) {
        set_text_align(parent_label->text_align());
    }
}

TextAlign Label::text_align() const noexcept
{
    return text_style_.align;
}

void Label::set_text_overflow(TextOverflow overflow) noexcept
{
    text_style_.overflow = overflow;
}

void Label::set_text_overflow(InheritTag) noexcept
{
    if (auto* parent_label = dynamic_cast<const Label*>(parent())) {
        set_text_overflow(parent_label->text_overflow());
    }
}

TextOverflow Label::text_overflow() const noexcept
{
    return text_style_.overflow;
}

bool Label::event(const Event& event)
{
    return Widget::event(event);
}

void Label::draw(Renderer& renderer)
{
    Widget::draw(renderer);
    auto style = text_style_;
    style.color = text_color();
    renderer.draw_text(text_, bounds().inset(layout_rules().padding), style);
}

} // namespace ouif
