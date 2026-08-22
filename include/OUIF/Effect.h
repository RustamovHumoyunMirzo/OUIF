#pragma once

#include <OUIF/Export.h>
#include <OUIF/Geometry.h>

#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace ouif {

class Renderer;
class Widget;

enum class EffectLayer {
    Layer,
    Backdrop,
};

struct EffectParameters {
    std::string name;
    std::vector<float> numbers;
    std::vector<std::string> args;
};

struct EffectContext {
    Renderer& renderer;
    Widget& widget;
    Rect bounds;
    EffectLayer layer = EffectLayer::Layer;
    const EffectParameters& parameters;
};

class OUIF_API Effect {
public:
    virtual ~Effect() = default;

    [[nodiscard]] virtual Rect expand_bounds(const EffectContext& context) const;
    virtual void pre_draw(const EffectContext& context);
    virtual void post_draw(const EffectContext& context);
};

using EffectFactory = std::function<std::shared_ptr<Effect>(const EffectParameters&)>;

class OUIF_API BlurEffect final : public Effect {
public:
    explicit BlurEffect(float radius = 8.0f) noexcept;

    void set_radius(float radius) noexcept;
    [[nodiscard]] float radius() const noexcept;

    [[nodiscard]] Rect expand_bounds(const EffectContext& context) const override;
    void pre_draw(const EffectContext& context) override;
    void post_draw(const EffectContext& context) override;

private:
    float radius_ = 8.0f;
};

} // namespace ouif
