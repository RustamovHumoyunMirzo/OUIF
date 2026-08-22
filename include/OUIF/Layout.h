#pragma once

#include <OUIF/Widget.h>

namespace ouif {

enum class Align : std::uint8_t {
    Start,
    Center,
    End,
};

class OUIF_API LinearLayout : public Widget {
public:
    void set_alignment(Align alignment) noexcept;
    [[nodiscard]] Align alignment() const noexcept;

    void set_cross_alignment(Align alignment) noexcept;
    [[nodiscard]] Align cross_alignment() const noexcept;

    void set_gap(float gap) noexcept;
    [[nodiscard]] float gap() const noexcept;

    void set_gravity(Gravity gravity) noexcept;
    void set_gravity(HorizontalGravity horizontal, VerticalGravity vertical) noexcept;
    [[nodiscard]] Gravity gravity() const noexcept;

protected:
    enum class Direction : std::uint8_t {
        Row,
        Column,
    };

    explicit LinearLayout(Direction direction);
    void on_layout(Rect content) override;

private:
    Direction direction_;
    Align alignment_ = Align::Start;
    Align cross_alignment_ = Align::Center;
    float gap_ = 0.0f;
};

class OUIF_API RowLayout : public LinearLayout {
public:
    RowLayout();
};

class OUIF_API ColLayout : public LinearLayout {
public:
    ColLayout();
};

class OUIF_API ScrollLayout : public LinearLayout {
public:
    void set_scroll_offset(float offset) noexcept;
    [[nodiscard]] float scroll_offset() const noexcept;
    [[nodiscard]] float max_scroll_offset() const noexcept;
    [[nodiscard]] Size content_size() const noexcept;
    void set_scroll_step(float step) noexcept;
    [[nodiscard]] float scroll_step() const noexcept;
    void set_smooth_scroll_enabled(bool enabled) noexcept;
    [[nodiscard]] bool smooth_scroll_enabled() const noexcept;
    void set_scroll_smoothing(float smoothing) noexcept;
    [[nodiscard]] float scroll_smoothing() const noexcept;
    void jump_to_scroll_offset(float offset) noexcept;
    [[nodiscard]] bool scroll_animating() const noexcept;

    bool event(const Event& event) override;

protected:
    explicit ScrollLayout(Direction direction);
    void on_layout(Rect content) override;

private:
    Direction scroll_direction_;
    float scroll_offset_ = 0.0f;
    float target_scroll_offset_ = 0.0f;
    float max_scroll_offset_ = 0.0f;
    float scroll_step_ = 48.0f;
    float scroll_smoothing_ = 0.24f;
    Size content_size_ {};
    bool smooth_scroll_enabled_ = true;
};

class OUIF_API RowScroll : public ScrollLayout {
public:
    RowScroll();
};

class OUIF_API ColScroll : public ScrollLayout {
public:
    ColScroll();
};

} // namespace ouif
