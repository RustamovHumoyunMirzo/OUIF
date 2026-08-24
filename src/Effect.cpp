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
    const int passes = std::clamp(static_cast<int>(radius_ / 1.5f), 6, 24);
    for (int pass = passes; pass >= 1; --pass) {
        const float t = static_cast<float>(pass) / static_cast<float>(passes);
        const float spread = radius_ * t;
        Color bloom = style.background;
        bloom.r = std::min(1.0f, bloom.r + 0.08f);
        bloom.g = std::min(1.0f, bloom.g + 0.08f);
        bloom.b = std::min(1.0f, bloom.b + 0.08f);
        bloom.a = std::min(0.16f, 0.10f * (1.0f - t + 0.35f));

        context.renderer.fill_rounded_rect({ context.bounds.x - spread, context.bounds.y, context.bounds.width + spread * 2.0f, context.bounds.height }, style.radius, bloom);
        context.renderer.fill_rounded_rect({ context.bounds.x, context.bounds.y - spread, context.bounds.width, context.bounds.height + spread * 2.0f }, style.radius, bloom);
        context.renderer.fill_rounded_rect({ context.bounds.x - spread * 0.7f, context.bounds.y - spread * 0.7f, context.bounds.width + spread * 1.4f, context.bounds.height + spread * 1.4f }, style.radius, bloom);
    }
}

void BlurEffect::post_draw(const EffectContext& context)
{
    if (context.layer != EffectLayer::Layer || radius_ <= 0.0f) {
        return;
    }

    const auto style = context.widget.get_style();
    const int passes = std::clamp(static_cast<int>(radius_ / 1.5f), 6, 24);
    for (int pass = 1; pass <= passes; ++pass) {
        const float t = static_cast<float>(pass) / static_cast<float>(passes);
        Color color = style.background;
        color.r = std::min(1.0f, color.r + 0.12f);
        color.g = std::min(1.0f, color.g + 0.12f);
        color.b = std::min(1.0f, color.b + 0.12f);
        color.a = std::min(color.a, 0.075f * (1.0f - t + 0.25f));
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
