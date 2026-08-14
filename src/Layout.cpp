#include <OUIF/Layout.h>

#include <algorithm>

namespace ouif {
namespace {

float fixed_width_for(const Widget& child, Size available)
{
    const auto rules = child.layout_rules();
    if (!rules.width_value.automatic()) {
        return rules.width_value.resolve(available, true);
    }
    const auto child_bounds = child.bounds();
    return child_bounds.width > 0.0f && rules.width != SizePolicy::Fill ? child_bounds.width : rules.preferred_size.width;
}

float fixed_height_for(const Widget& child, Size available)
{
    const auto rules = child.layout_rules();
    if (!rules.height_value.automatic()) {
        return rules.height_value.resolve(available, false);
    }
    const auto child_bounds = child.bounds();
    return child_bounds.height > 0.0f && rules.height != SizePolicy::Fill ? child_bounds.height : rules.preferred_size.height;
}

} // namespace

void LinearLayout::set_alignment(Align alignment) noexcept
{
    alignment_ = alignment;
}

Align LinearLayout::alignment() const noexcept
{
    return alignment_;
}

void LinearLayout::set_cross_alignment(Align alignment) noexcept
{
    cross_alignment_ = alignment;
}

Align LinearLayout::cross_alignment() const noexcept
{
    return cross_alignment_;
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
    const float available_main = row ? content.width : content.height;
    const float available_cross = row ? content.height : content.width;
    float total_main = gap_ * static_cast<float>(items.size() - 1);
    float total_flex = 0.0f;

    for (const auto& child : items) {
        const auto child_bounds = child->bounds();
        const auto rules = child->layout_rules();
        const bool fills_main = rules.flex > 0.0f || (row ? rules.width == SizePolicy::Fill : rules.height == SizePolicy::Fill);
        const float main = row ? child_bounds.width : child_bounds.height;
        const float preferred_main = row ? fixed_width_for(*child, { content.width, content.height }) : fixed_height_for(*child, { content.width, content.height });
        const float margin_main = row ? rules.margin.left + rules.margin.right : rules.margin.top + rules.margin.bottom;

        if (fills_main) {
            total_flex += rules.flex > 0.0f ? rules.flex : 1.0f;
            total_main += margin_main;
        } else {
            total_main += (main > 0.0f ? main : preferred_main) + margin_main;
        }
    }

    const float remaining = std::max(0.0f, available_main - total_main);

    float cursor = row ? content.x : content.y;
    if (total_flex == 0.0f && alignment_ == Align::Center) {
        cursor += (available_main - total_main) * 0.5f;
    } else if (total_flex == 0.0f && alignment_ == Align::End) {
        cursor += available_main - total_main;
    }

    cursor = std::max(cursor, row ? content.x : content.y);

    for (const auto& child : items) {
        const auto rules = child->layout_rules();
        const bool fills_main = rules.flex > 0.0f || (row ? rules.width == SizePolicy::Fill : rules.height == SizePolicy::Fill);
        const bool fills_cross = row ? rules.height == SizePolicy::Fill : rules.width == SizePolicy::Fill;
        const float flex = rules.flex > 0.0f ? rules.flex : (fills_main ? 1.0f : 0.0f);
        const float margin_before = row ? rules.margin.left : rules.margin.top;
        const float margin_after = row ? rules.margin.right : rules.margin.bottom;
        const float margin_cross_before = row ? rules.margin.top : rules.margin.left;
        const float margin_cross_after = row ? rules.margin.bottom : rules.margin.right;
        const float preferred_width = fixed_width_for(*child, { content.width, content.height });
        const float preferred_height = fixed_height_for(*child, { content.width, content.height });
        const float weighted_main = total_flex > 0.0f && fills_main ? remaining * (flex / total_flex) : 0.0f;
        const float cross_space = std::max(0.0f, available_cross - margin_cross_before - margin_cross_after);
        const float width = row
            ? (fills_main ? weighted_main : preferred_width)
            : (fills_cross ? cross_space : preferred_width);
        const float height = row
            ? (fills_cross ? cross_space : preferred_height)
            : (fills_main ? weighted_main : preferred_height);
        float cross_start = row ? content.y + margin_cross_before : content.x + margin_cross_before;
        if (!fills_cross && cross_alignment_ == Align::Center) {
            cross_start = (row ? content.y : content.x) + margin_cross_before + (cross_space - (row ? height : width)) * 0.5f;
        } else if (!fills_cross && cross_alignment_ == Align::End) {
            cross_start = (row ? content.y : content.x) + available_cross - margin_cross_after - (row ? height : width);
        }

        cursor += margin_before;
        if (row) {
            child->set_bounds({ cursor, cross_start, width, height });
            cursor += width + margin_after + gap_;
        } else {
            child->set_bounds({ cross_start, cursor, width, height });
            cursor += height + margin_after + gap_;
        }
    }
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
