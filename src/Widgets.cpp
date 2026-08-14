#include <OUIF/Widgets.h>

#include <OUIF/Renderer.h>

#include <algorithm>

namespace ouif {

Spacer::Spacer(float flex) noexcept
{
    set_flex(flex);
}

Spacer::Spacer(Size size) noexcept
{
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

} // namespace ouif
