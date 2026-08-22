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
    if (context.layer != EffectLayer::Backdrop || radius_ <= 0.0f) {
        return;
    }

    const auto style = context.widget.get_style();
    const int passes = std::clamp(static_cast<int>(radius_ / 2.0f), 2, 12);
    for (int pass = passes; pass >= 1; --pass) {
        const float t = static_cast<float>(pass) / static_cast<float>(passes);
        Color color = style.background;
        color.a = std::min(color.a, 0.08f * (1.0f - t + 0.25f));
        const float spread = radius_ * t;
        context.renderer.fill_rounded_rect(
            {
                context.bounds.x - spread,
                context.bounds.y - spread,
                context.bounds.width + spread * 2.0f,
                context.bounds.height + spread * 2.0f,
            },
            style.radius,
            color
        );
    }
}

void BlurEffect::post_draw(const EffectContext& context)
{
    if (context.layer != EffectLayer::Layer || radius_ <= 0.0f) {
        return;
    }

    const auto style = context.widget.get_style();
    const int passes = std::clamp(static_cast<int>(radius_ / 2.0f), 2, 12);
    for (int pass = 1; pass <= passes; ++pass) {
        const float t = static_cast<float>(pass) / static_cast<float>(passes);
        Color color = style.background;
        color.a = std::min(color.a, 0.06f * (1.0f - t + 0.2f));
        const float spread = radius_ * t;
        context.renderer.stroke_rounded_rect(
            {
                context.bounds.x - spread,
                context.bounds.y - spread,
                context.bounds.width + spread * 2.0f,
                context.bounds.height + spread * 2.0f,
            },
            style.radius,
            color,
            std::max(1.0f, radius_ / static_cast<float>(passes))
        );
    }
}

} // namespace ouif
