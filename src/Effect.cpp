#include <OUIF/Effect.h>

#include <OUIF/Renderer.h>
#include <OUIF/Widget.h>

#include <algorithm>

namespace ouif {

Rect Effect::expand_bounds(const EffectContext& context) const
{
    return context.bounds;
}

void Effect::pre_draw(const EffectContext& context)
{
    (void)context;
}

void Effect::post_draw(const EffectContext& context)
{
    (void)context;
}

BlurEffect::BlurEffect(float radius) noexcept
    : radius_(std::max(0.0f, radius))
{
}

void BlurEffect::set_radius(float radius) noexcept
{
    radius_ = std::max(0.0f, radius);
}

float BlurEffect::radius() const noexcept
{
    return radius_;
}

Rect BlurEffect::expand_bounds(const EffectContext& context) const
{
    return {
        context.bounds.x - radius_,
        context.bounds.y - radius_,
        context.bounds.width + radius_ * 2.0f,
        context.bounds.height + radius_ * 2.0f,
    };
}

void BlurEffect::pre_draw(const EffectContext& context)
{
    if (radius_ <= 0.0f) {
        return;
    }

    const auto style = context.widget.get_style();
    Color tint = style.background;
    tint.a = std::min(tint.a, 0.28f);
    context.renderer.draw_backdrop_blur(context.bounds, style.radius, radius_, tint);
}

void BlurEffect::post_draw(const EffectContext& context)
{
    (void)context;
}

} // namespace ouif
