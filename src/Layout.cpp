#include <OUIF/Layout.h>

#include <algorithm>
#include <cmath>

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

float main_size_for(const Widget& child, Size available, bool row)
{
    return row ? fixed_width_for(child, available) : fixed_height_for(child, available);
}

float cross_size_for(const Widget& child, Size available, bool row)
{
    return row ? fixed_height_for(child, available) : fixed_width_for(child, available);
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

ScrollLayout::ScrollLayout(Direction direction)
    : LinearLayout(direction)
    , scroll_direction_(direction)
{
}

void ScrollLayout::set_scroll_offset(float offset) noexcept
{
    target_scroll_offset_ = std::clamp(offset, 0.0f, max_scroll_offset_);
    if (!smooth_scroll_enabled_) {
        scroll_offset_ = target_scroll_offset_;
    }
}

float ScrollLayout::scroll_offset() const noexcept
{
    return target_scroll_offset_;
}

float ScrollLayout::max_scroll_offset() const noexcept
{
    return max_scroll_offset_;
}

Size ScrollLayout::content_size() const noexcept
{
    return content_size_;
}

void ScrollLayout::set_scroll_step(float step) noexcept
{
    scroll_step_ = std::max(1.0f, step);
}

float ScrollLayout::scroll_step() const noexcept
{
    return scroll_step_;
}

void ScrollLayout::set_smooth_scroll_enabled(bool enabled) noexcept
{
    smooth_scroll_enabled_ = enabled;
    if (!smooth_scroll_enabled_) {
        scroll_offset_ = target_scroll_offset_;
    }
}

bool ScrollLayout::smooth_scroll_enabled() const noexcept
{
    return smooth_scroll_enabled_;
}

void ScrollLayout::set_scroll_smoothing(float smoothing) noexcept
{
    scroll_smoothing_ = std::clamp(smoothing, 0.01f, 1.0f);
}

float ScrollLayout::scroll_smoothing() const noexcept
{
    return scroll_smoothing_;
}

void ScrollLayout::jump_to_scroll_offset(float offset) noexcept
{
    target_scroll_offset_ = std::clamp(offset, 0.0f, max_scroll_offset_);
    scroll_offset_ = target_scroll_offset_;
}

bool ScrollLayout::scroll_animating() const noexcept
{
    return std::abs(target_scroll_offset_ - scroll_offset_) > 0.1f;
}

bool ScrollLayout::event(const Event& event)
{
    if (const auto* wheel = std::get_if<MouseWheelEvent>(&event)) {
        if (hit_test(wheel->position)) {
            const bool row = scroll_direction_ == Direction::Row;
            const float wheel_delta = row && wheel->delta_x != 0.0f ? wheel->delta_x : wheel->delta_y;
            const float previous = target_scroll_offset_;
            set_scroll_offset(target_scroll_offset_ - wheel_delta * scroll_step_);
            if (target_scroll_offset_ != previous) {
                layout({ bounds().width, bounds().height });
                return true;
            }
        }
    }

    return LinearLayout::event(event);
}

void ScrollLayout::on_layout(Rect content)
{
    auto& items = mutable_children();
    const bool row = scroll_direction_ == Direction::Row;
    const float available_main = row ? content.width : content.height;
    const float available_cross = row ? content.height : content.width;
    const Size available { content.width, content.height };
    float total_main = items.empty() ? 0.0f : gap() * static_cast<float>(items.size() - 1);
    float max_cross = 0.0f;

    for (const auto& child : items) {
        const auto rules = child->layout_rules();
        const float margin_main = row ? rules.margin.left + rules.margin.right : rules.margin.top + rules.margin.bottom;
        const float margin_cross = row ? rules.margin.top + rules.margin.bottom : rules.margin.left + rules.margin.right;
        total_main += main_size_for(*child, available, row) + margin_main;
        max_cross = std::max(max_cross, cross_size_for(*child, available, row) + margin_cross);
    }

    content_size_ = row ? Size { total_main, max_cross } : Size { max_cross, total_main };
    max_scroll_offset_ = std::max(0.0f, total_main - available_main);
    target_scroll_offset_ = std::clamp(target_scroll_offset_, 0.0f, max_scroll_offset_);
    scroll_offset_ = std::clamp(scroll_offset_, 0.0f, max_scroll_offset_);
    if (smooth_scroll_enabled_) {
        const float delta = target_scroll_offset_ - scroll_offset_;
        if (std::abs(delta) <= 0.1f) {
            scroll_offset_ = target_scroll_offset_;
        } else {
            scroll_offset_ += delta * scroll_smoothing_;
        }
    } else {
        scroll_offset_ = target_scroll_offset_;
    }

    float cursor = (row ? content.x : content.y) - scroll_offset_;
    if (max_scroll_offset_ <= 0.0f && alignment() == Align::Center) {
        cursor += (available_main - total_main) * 0.5f;
    } else if (max_scroll_offset_ <= 0.0f && alignment() == Align::End) {
        cursor += available_main - total_main;
    }

    for (const auto& child : items) {
        const auto rules = child->layout_rules();
        const bool fills_cross = row ? rules.height == SizePolicy::Fill : rules.width == SizePolicy::Fill;
        const float margin_before = row ? rules.margin.left : rules.margin.top;
        const float margin_after = row ? rules.margin.right : rules.margin.bottom;
        const float margin_cross_before = row ? rules.margin.top : rules.margin.left;
        const float margin_cross_after = row ? rules.margin.bottom : rules.margin.right;
        const float main = main_size_for(*child, available, row);
        const float preferred_cross = cross_size_for(*child, available, row);
        const float cross_space = std::max(0.0f, available_cross - margin_cross_before - margin_cross_after);
        const float cross = fills_cross ? cross_space : preferred_cross;
        float cross_start = row ? content.y + margin_cross_before : content.x + margin_cross_before;
        if (!fills_cross && cross_alignment() == Align::Center) {
            cross_start = (row ? content.y : content.x) + margin_cross_before + (cross_space - cross) * 0.5f;
        } else if (!fills_cross && cross_alignment() == Align::End) {
            cross_start = (row ? content.y : content.x) + available_cross - margin_cross_after - cross;
        }

        cursor += margin_before;
        if (row) {
            child->set_bounds({ cursor, cross_start, main, cross });
            cursor += main + margin_after + gap();
        } else {
            child->set_bounds({ cross_start, cursor, cross, main });
            cursor += main + margin_after + gap();
        }
    }
}

RowScroll::RowScroll()
    : ScrollLayout(Direction::Row)
{
}

ColScroll::ColScroll()
    : ScrollLayout(Direction::Column)
{
}

} // namespace ouif
