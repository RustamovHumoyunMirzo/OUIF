#pragma once

#include <OUIF/Widget.h>

namespace ouif {

enum class Orientation : std::uint8_t {
    Horizontal,
    Vertical,
};

class OUIF_API Spacer : public Widget {
public:
    Spacer() = default;
    explicit Spacer(float flex) noexcept;
    explicit Spacer(Size size) noexcept;

    bool event(const Event& event) override;

protected:
    void draw(Renderer& renderer) override;
};

class OUIF_API Divider : public Widget {
public:
    explicit Divider(Orientation orientation = Orientation::Horizontal, float thickness = 1.0f);

    void set_orientation(Orientation orientation) noexcept;
    [[nodiscard]] Orientation orientation() const noexcept;
    void set_thickness(float thickness) noexcept;
    [[nodiscard]] float thickness() const noexcept;
    void set_color(Color color) noexcept;
    [[nodiscard]] Color color() const noexcept;

    bool event(const Event& event) override;

protected:
    void draw(Renderer& renderer) override;

private:
    void apply_axis_size() noexcept;

    Orientation orientation_ = Orientation::Horizontal;
    float thickness_ = 1.0f;
};

} // namespace ouif
