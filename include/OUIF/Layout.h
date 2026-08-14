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

    void set_gap(float gap) noexcept;
    [[nodiscard]] float gap() const noexcept;

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

} // namespace ouif
