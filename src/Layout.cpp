#include <OUIF/Layout.h>

#include <algorithm>

namespace ouif {

void LinearLayout::set_alignment(Align alignment) noexcept
{
    alignment_ = alignment;
}

Align LinearLayout::alignment() const noexcept
{
    return alignment_;
}

void LinearLayout::set_gap(float gap) noexcept
{
    gap_ = std::max(0.0f, gap);
}

float LinearLayout::gap() const noexcept
{
    return gap_;
}

LinearLayout::LinearLayout(Direction direction)
    : direction_(direction)
{
}

void LinearLayout::on_layout(Rect content)
{
    auto& items = mutable_children();
    if (items.empty()) {
        return;
    }

    const bool row = direction_ == Direction::Row;
    float total_main = gap_ * static_cast<float>(items.size() - 1);
    float max_cross = 0.0f;

    for (const auto& child : items) {
        const auto child_bounds = child->bounds();
        const auto rules = child->layout_rules();
        const float main = row ? child_bounds.width : child_bounds.height;
        const float cross = row ? child_bounds.height : child_bounds.width;
        const float preferred_main = row ? rules.preferred_size.width : rules.preferred_size.height;
        const float preferred_cross = row ? rules.preferred_size.height : rules.preferred_size.width;

        total_main += main > 0.0f ? main : preferred_main;
        max_cross = std::max(max_cross, cross > 0.0f ? cross : preferred_cross);
    }

    const float available_main = row ? content.width : content.height;
    const float available_cross = row ? content.height : content.width;

    float cursor = row ? content.x : content.y;
    if (alignment_ == Align::Center) {
        cursor += (available_main - total_main) * 0.5f;
    } else if (alignment_ == Align::End) {
        cursor += available_main - total_main;
    }

    cursor = std::max(cursor, row ? content.x : content.y);

    for (const auto& child : items) {
        const auto child_bounds = child->bounds();
        const auto rules = child->layout_rules();
        const float preferred_width = child_bounds.width > 0.0f ? child_bounds.width : rules.preferred_size.width;
        const float preferred_height = child_bounds.height > 0.0f ? child_bounds.height : rules.preferred_size.height;
        const float width = rules.width == SizePolicy::Fill && !row ? available_cross : preferred_width;
        const float height = rules.height == SizePolicy::Fill && row ? available_cross : preferred_height;
        const float cross_start = row ? content.y + (available_cross - height) * 0.5f : content.x + (available_cross - width) * 0.5f;

        if (row) {
            child->set_bounds({ cursor, cross_start, width, height });
            cursor += width + gap_;
        } else {
            child->set_bounds({ cross_start, cursor, width, height });
            cursor += height + gap_;
        }
    }

    (void)max_cross;
}

RowLayout::RowLayout()
    : LinearLayout(Direction::Row)
{
}

ColLayout::ColLayout()
    : LinearLayout(Direction::Column)
{
}

} // namespace ouif
