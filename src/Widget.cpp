#include <OUIF/Widget.h>

#include <OUIF/Renderer.h>
#include <OUIF/Widgets.h>

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <iterator>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>

#if OUIF_WITH_KATANA
#include <katana.h>
#endif

namespace ouif {

Widget* Widget::focused_widget_ = nullptr;

namespace {

#if OUIF_WITH_KATANA
struct KatanaOutputDeleter {
    void operator()(KatanaOutput* output) const noexcept
    {
        if (output != nullptr) {
            katana_destroy_output(output);
        }
    }
};

using KatanaOutputPtr = std::unique_ptr<KatanaOutput, KatanaOutputDeleter>;

enum class CssState {
    Base,
    Hover,
    Pressed,
    Selected,
    Focus,
};

struct CssMotionDeclaration {
    StyleTransition transition {};
    bool has_transition = false;
    std::string animation_name;
    float animation_duration = 1.0f;
    Easing animation_easing = Easing::Linear;
    bool animation_loop = false;
    bool has_animation = false;
};

using ParsedKeyframes = std::unordered_map<std::string, std::vector<StyleKeyframe>>;

std::string_view text_or_empty(const char* text) noexcept
{
    return text != nullptr ? std::string_view(text) : std::string_view();
}

std::string lower_copy(std::string_view value)
{
    std::string copy(value);
    std::transform(copy.begin(), copy.end(), copy.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return copy;
}

bool equals_ignore_case(std::string_view left, std::string_view right)
{
    return lower_copy(left) == lower_copy(right);
}

std::optional<Color> color_from_value(const KatanaValue& value)
{
    if (value.unit == KATANA_VALUE_PARSER_HEXCOLOR || value.unit == KATANA_VALUE_RGBCOLOR) {
        return Color::from_hex(text_or_empty(value.string));
    }

    if (value.unit == KATANA_VALUE_IDENT || value.unit == KATANA_VALUE_STRING) {
        return Color::from_hex(text_or_empty(value.string));
    }

    return std::nullopt;
}

std::optional<float> pixels_from_value(const KatanaValue& value)
{
    if (value.unit == KATANA_VALUE_PARSER_INTEGER) {
        return static_cast<float>(value.iValue);
    }
    if (value.unit == KATANA_VALUE_NUMBER || value.unit == KATANA_VALUE_PX || value.unit == KATANA_VALUE_DIMENSION) {
        return static_cast<float>(value.fValue);
    }

    return std::nullopt;
}

std::optional<float> seconds_from_value(const KatanaValue& value)
{
    if (value.unit == KATANA_VALUE_MS) {
        return static_cast<float>(value.fValue) / 1000.0f;
    }
    if (value.unit == KATANA_VALUE_S) {
        return static_cast<float>(value.fValue);
    }
    if (value.unit == KATANA_VALUE_NUMBER || value.unit == KATANA_VALUE_PARSER_INTEGER) {
        return pixels_from_value(value);
    }
    return std::nullopt;
}

std::optional<Easing> easing_from_text(std::string_view value)
{
    const auto lower = lower_copy(value);
    if (lower == "linear") {
        return Easing::Linear;
    }
    if (lower == "ease-in") {
        return Easing::EaseIn;
    }
    if (lower == "ease-out" || lower == "ease") {
        return Easing::EaseOut;
    }
    if (lower == "ease-in-out") {
        return Easing::EaseInOut;
    }
    return std::nullopt;
}

std::optional<std::string> ident_from_value(const KatanaValue& value)
{
    if (value.unit == KATANA_VALUE_IDENT || value.unit == KATANA_VALUE_STRING || value.unit == KATANA_VALUE_PARSER_IDENTIFIER) {
        const auto text = text_or_empty(value.string);
        if (!text.empty()) {
            return std::string(text);
        }
    }
    return std::nullopt;
}

Length length_from_value(const KatanaValue& value)
{
    const auto number = static_cast<float>(value.fValue);
    switch (value.unit) {
    case KATANA_VALUE_PARSER_INTEGER:
        return Length::px(static_cast<float>(value.iValue));
    case KATANA_VALUE_NUMBER:
    case KATANA_VALUE_PX:
        return Length::px(number);
    case KATANA_VALUE_PERCENTAGE:
        return Length::percent(number);
    case KATANA_VALUE_VW:
        return Length::vw(number);
    case KATANA_VALUE_VH:
        return Length::vh(number);
    default:
        return {};
    }
}

KatanaValue* value_at(KatanaArray* values, unsigned int index)
{
    if (values == nullptr || index >= values->length) {
        return nullptr;
    }
    return static_cast<KatanaValue*>(values->data[index]);
}

void collect_values(KatanaValue& value, std::vector<KatanaValue*>& values)
{
    if (value.unit == KATANA_VALUE_PARSER_LIST && value.list != nullptr) {
        for (unsigned int index = 0; index < value.list->length; ++index) {
            auto* nested = value_at(value.list, index);
            if (nested != nullptr) {
                collect_values(*nested, values);
            }
        }
        return;
    }

    values.push_back(&value);
}

std::vector<KatanaValue*> flattened_values(KatanaArray* source)
{
    std::vector<KatanaValue*> values;
    for (unsigned int index = 0; source != nullptr && index < source->length; ++index) {
        auto* value = value_at(source, index);
        if (value != nullptr) {
            collect_values(*value, values);
        }
    }
    return values;
}

std::vector<KatanaValue*> function_args(const KatanaValue& value)
{
    std::vector<KatanaValue*> values;
    if (value.unit != KATANA_VALUE_PARSER_FUNCTION || value.function == nullptr) {
        return values;
    }
    return flattened_values(value.function->args);
}

void apply_transform_value(Transform& transform, const KatanaValue& value)
{
    if (value.unit != KATANA_VALUE_PARSER_FUNCTION || value.function == nullptr) {
        return;
    }

    const auto name = lower_copy(text_or_empty(value.function->name));
    const auto args = function_args(value);
    if (name == "translate" || name == "translate3d") {
        if (!args.empty()) {
            transform.translate_x = pixels_from_value(*args[0]).value_or(transform.translate_x);
        }
        if (args.size() > 1) {
            transform.translate_y = pixels_from_value(*args[1]).value_or(transform.translate_y);
        }
        return;
    }
    if (name == "translatex") {
        if (!args.empty()) {
            transform.translate_x = pixels_from_value(*args[0]).value_or(transform.translate_x);
        }
        return;
    }
    if (name == "translatey") {
        if (!args.empty()) {
            transform.translate_y = pixels_from_value(*args[0]).value_or(transform.translate_y);
        }
        return;
    }
    if (name == "scale" || name == "scale3d") {
        if (!args.empty()) {
            transform.scale_x = pixels_from_value(*args[0]).value_or(transform.scale_x);
            transform.scale_y = args.size() > 1 ? pixels_from_value(*args[1]).value_or(transform.scale_x) : transform.scale_x;
        }
        return;
    }
    if (name == "scalex") {
        if (!args.empty()) {
            transform.scale_x = pixels_from_value(*args[0]).value_or(transform.scale_x);
        }
        return;
    }
    if (name == "scaley") {
        if (!args.empty()) {
            transform.scale_y = pixels_from_value(*args[0]).value_or(transform.scale_y);
        }
        return;
    }
    if (name == "rotate" || name == "rotatez") {
        if (!args.empty()) {
            transform.rotation_degrees = pixels_from_value(*args[0]).value_or(transform.rotation_degrees);
        }
    }
}

std::vector<float> numbers_from_text(std::string_view text)
{
    std::vector<float> values;
    const char* cursor = text.data();
    const char* end = text.data() + text.size();
    while (cursor < end) {
        char* parsed_end = nullptr;
        const float value = std::strtof(cursor, &parsed_end);
        if (parsed_end != cursor) {
            values.push_back(value);
            cursor = parsed_end;
        } else {
            ++cursor;
        }
    }
    return values;
}

void apply_transform_text(Transform& transform, std::string_view text)
{
    const auto lower = lower_copy(text);
    std::size_t cursor = 0;
    while (cursor < lower.size()) {
        const auto open = lower.find('(', cursor);
        if (open == std::string::npos) {
            break;
        }
        const auto close = lower.find(')', open + 1);
        if (close == std::string::npos) {
            break;
        }

        std::size_t name_start = open;
        while (name_start > 0 && (std::isalpha(static_cast<unsigned char>(lower[name_start - 1])) != 0 || lower[name_start - 1] == '-')) {
            --name_start;
        }

        const auto name = std::string_view(lower).substr(name_start, open - name_start);
        const auto values = numbers_from_text(std::string_view(lower).substr(open + 1, close - open - 1));
        if (name == "translate" || name == "translate3d") {
            if (!values.empty()) {
                transform.translate_x = values[0];
            }
            if (values.size() > 1) {
                transform.translate_y = values[1];
            }
        } else if (name == "translatex") {
            if (!values.empty()) {
                transform.translate_x = values[0];
            }
        } else if (name == "translatey") {
            if (!values.empty()) {
                transform.translate_y = values[0];
            }
        } else if (name == "scale" || name == "scale3d") {
            if (!values.empty()) {
                transform.scale_x = values[0];
                transform.scale_y = values.size() > 1 ? values[1] : values[0];
            }
        } else if (name == "scalex") {
            if (!values.empty()) {
                transform.scale_x = values[0];
            }
        } else if (name == "scaley") {
            if (!values.empty()) {
                transform.scale_y = values[0];
            }
        } else if (name == "rotate" || name == "rotatez") {
            if (!values.empty()) {
                transform.rotation_degrees = values[0];
            }
        }

        cursor = close + 1;
    }
}

void apply_background(Style& style, CssState state, Color color)
{
    switch (state) {
    case CssState::Hover:
        style.with_background_hovered(color);
        break;
    case CssState::Pressed:
        style.with_background_pressed(color);
        break;
    case CssState::Selected:
        style.with_background_selected(color);
        break;
    case CssState::Focus:
        style.with_background_focused(color);
        break;
    case CssState::Base:
        style.with_background(color);
        break;
    }
}

void apply_border(Style& style, CssState state, Color color, float width)
{
    if (state == CssState::Selected) {
        style.with_border_selected(color, width);
    } else if (state == CssState::Focus) {
        style.with_border_focused(color, width);
    } else {
        style.with_border(color, width);
    }
}

void apply_border_side(Style& style, CssState state, std::string_view side, Color color, float width)
{
    BorderEdges* edges = &style.borders;
    if (state == CssState::Selected) {
        edges = &style.borders_selected;
    } else if (state == CssState::Focus) {
        edges = &style.borders_focused;
    }

    if (side == "left") {
        edges->left = { color, width };
    } else if (side == "top") {
        edges->top = { color, width };
    } else if (side == "right") {
        edges->right = { color, width };
    } else if (side == "bottom") {
        edges->bottom = { color, width };
    }
}

void mark_property(StyleProperties* properties, StyleProperty property) noexcept
{
    if (properties != nullptr) {
        *properties |= style_property_mask(property);
    }
}

void apply_declaration(
    Widget& widget,
    Style& style,
    KatanaDeclaration& declaration,
    CssState state,
    bool& touched_style,
    bool allow_widget_properties = true,
    StyleProperties* touched_properties = nullptr,
    CssMotionDeclaration* motion = nullptr
)
{
    const auto property = lower_copy(text_or_empty(declaration.property));
    auto* first = value_at(declaration.values, 0);
    if (first == nullptr) {
        return;
    }

    if (property == "background" || property == "background-color" || property == "with-background") {
        if (auto color = color_from_value(*first)) {
            apply_background(style, state, *color);
            mark_property(touched_properties, state == CssState::Base ? StyleProperty::Background : StyleProperty::StatefulBackgrounds);
            touched_style = true;
        }
        return;
    }

    if (property == "background-hovered" || property == "hover-background" || property == "with-background-hovered") {
        if (auto color = color_from_value(*first)) {
            style.with_background_hovered(*color);
            mark_property(touched_properties, StyleProperty::StatefulBackgrounds);
            touched_style = true;
        }
        return;
    }

    if (property == "background-pressed" || property == "pressed-background" || property == "with-background-pressed") {
        if (auto color = color_from_value(*first)) {
            style.with_background_pressed(*color);
            mark_property(touched_properties, StyleProperty::StatefulBackgrounds);
            touched_style = true;
        }
        return;
    }

    if (property == "background-selected" || property == "selected-background" || property == "with-background-selected") {
        if (auto color = color_from_value(*first)) {
            style.with_background_selected(*color);
            mark_property(touched_properties, StyleProperty::StatefulBackgrounds);
            touched_style = true;
        }
        return;
    }

    if (property == "background-focused" || property == "focused-background" || property == "with-background-focused") {
        if (auto color = color_from_value(*first)) {
            style.with_background_focused(*color);
            mark_property(touched_properties, StyleProperty::StatefulBackgrounds);
            touched_style = true;
        }
        return;
    }

    if (property == "foreground" || property == "color" || property == "with-foreground") {
        if (auto color = color_from_value(*first)) {
            style.with_foreground(*color);
            mark_property(touched_properties, StyleProperty::Foreground);
            touched_style = true;
        }
        return;
    }

    if (property == "border" || property == "border-selected" || property == "border-focused") {
        std::optional<Color> border_color;
        std::optional<float> border_width;
        for (auto* value : flattened_values(declaration.values)) {
            if (value == nullptr) {
                continue;
            }
            if (!border_color) {
                border_color = color_from_value(*value);
            }
            if (!border_width) {
                border_width = pixels_from_value(*value);
            }
        }
        if (border_color && border_width) {
            const CssState target = property == "border-selected" ? CssState::Selected : (property == "border-focused" ? CssState::Focus : state);
            apply_border(style, target, *border_color, *border_width);
            mark_property(touched_properties, target == CssState::Base ? StyleProperty::Border : StyleProperty::StatefulBorders);
            mark_property(touched_properties, target == CssState::Base ? StyleProperty::BorderEdges : StyleProperty::StatefulBorderEdges);
            touched_style = true;
        }
        return;
    }

    if (property == "border-left" || property == "border-top" || property == "border-right" || property == "border-bottom") {
        std::optional<Color> border_color;
        std::optional<float> border_width;
        for (auto* value : flattened_values(declaration.values)) {
            if (value == nullptr) {
                continue;
            }
            if (!border_color) {
                border_color = color_from_value(*value);
            }
            if (!border_width) {
                border_width = pixels_from_value(*value);
            }
        }
        if (border_color && border_width) {
            apply_border_side(style, state, std::string_view(property).substr(7), *border_color, *border_width);
            mark_property(touched_properties, state == CssState::Base ? StyleProperty::BorderEdges : StyleProperty::StatefulBorderEdges);
            touched_style = true;
        }
        return;
    }

    if (property == "border-width") {
        if (auto width = pixels_from_value(*first)) {
            style.border.width = *width;
            style.borders.left.width = *width;
            style.borders.top.width = *width;
            style.borders.right.width = *width;
            style.borders.bottom.width = *width;
            style.border_width = *width;
            mark_property(touched_properties, StyleProperty::Border);
            mark_property(touched_properties, StyleProperty::BorderEdges);
            touched_style = true;
        }
        return;
    }

    if (property == "border-left-width" || property == "border-top-width" || property == "border-right-width" || property == "border-bottom-width") {
        if (auto width = pixels_from_value(*first)) {
            apply_border_side(style, state, std::string_view(property).substr(7, property.find("-width") - 7), style.border.color, *width);
            mark_property(touched_properties, state == CssState::Base ? StyleProperty::BorderEdges : StyleProperty::StatefulBorderEdges);
            touched_style = true;
        }
        return;
    }

    if (property == "radius" || property == "border-radius" || property == "with-radius") {
        if (auto radius = pixels_from_value(*first)) {
            style.with_radius(*radius);
            mark_property(touched_properties, StyleProperty::Radius);
            touched_style = true;
        }
        return;
    }

    if (property == "radius-top-left" || property == "border-top-left-radius") {
        if (auto radius = pixels_from_value(*first)) {
            style.radius.top_left = *radius;
            mark_property(touched_properties, StyleProperty::Radius);
            touched_style = true;
        }
        return;
    }

    if (property == "radius-top-right" || property == "border-top-right-radius") {
        if (auto radius = pixels_from_value(*first)) {
            style.radius.top_right = *radius;
            mark_property(touched_properties, StyleProperty::Radius);
            touched_style = true;
        }
        return;
    }

    if (property == "radius-bottom-right" || property == "border-bottom-right-radius") {
        if (auto radius = pixels_from_value(*first)) {
            style.radius.bottom_right = *radius;
            mark_property(touched_properties, StyleProperty::Radius);
            touched_style = true;
        }
        return;
    }

    if (property == "radius-bottom-left" || property == "border-bottom-left-radius") {
        if (auto radius = pixels_from_value(*first)) {
            style.radius.bottom_left = *radius;
            mark_property(touched_properties, StyleProperty::Radius);
            touched_style = true;
        }
        return;
    }

    if (property == "opacity") {
        if (auto opacity = pixels_from_value(*first)) {
            style.with_opacity(std::clamp(*opacity, 0.0f, 1.0f));
            mark_property(touched_properties, StyleProperty::Opacity);
            touched_style = true;
        }
        return;
    }

    if (property == "transition") {
        if (motion == nullptr) {
            return;
        }
        motion->has_transition = true;
        motion->transition.enabled = true;
        for (auto* value : flattened_values(declaration.values)) {
            if (value == nullptr) {
                continue;
            }
            if (auto seconds = seconds_from_value(*value)) {
                motion->transition.duration = std::max(0.0f, *seconds);
                continue;
            }
            if (auto ident = ident_from_value(*value)) {
                if (auto easing = easing_from_text(*ident)) {
                    motion->transition.easing = *easing;
                }
            }
        }
        return;
    }

    if (property == "transition-duration") {
        if (motion != nullptr) {
            motion->has_transition = true;
            motion->transition.enabled = true;
            motion->transition.duration = std::max(0.0f, seconds_from_value(*first).value_or(motion->transition.duration));
        }
        return;
    }

    if (property == "transition-timing-function") {
        if (motion != nullptr) {
            motion->has_transition = true;
            motion->transition.enabled = true;
            if (auto ident = ident_from_value(*first)) {
                if (auto easing = easing_from_text(*ident)) {
                    motion->transition.easing = *easing;
                }
            }
        }
        return;
    }

    if (property == "animation") {
        if (motion == nullptr) {
            return;
        }
        motion->has_animation = true;
        for (auto* value : flattened_values(declaration.values)) {
            if (value == nullptr) {
                continue;
            }
            if (auto seconds = seconds_from_value(*value)) {
                motion->animation_duration = std::max(0.0f, *seconds);
                continue;
            }
            if (auto ident = ident_from_value(*value)) {
                const auto lower = lower_copy(*ident);
                if (auto easing = easing_from_text(lower)) {
                    motion->animation_easing = *easing;
                } else if (lower == "infinite") {
                    motion->animation_loop = true;
                } else if (lower != "normal" && lower != "none") {
                    motion->animation_name = std::move(*ident);
                }
            }
        }
        return;
    }

    if (property == "animation-name") {
        if (motion != nullptr) {
            motion->has_animation = true;
            motion->animation_name = ident_from_value(*first).value_or("");
        }
        return;
    }

    if (property == "animation-duration") {
        if (motion != nullptr) {
            motion->has_animation = true;
            motion->animation_duration = std::max(0.0f, seconds_from_value(*first).value_or(motion->animation_duration));
        }
        return;
    }

    if (property == "animation-timing-function") {
        if (motion != nullptr) {
            motion->has_animation = true;
            if (auto ident = ident_from_value(*first)) {
                if (auto easing = easing_from_text(*ident)) {
                    motion->animation_easing = *easing;
                }
            }
        }
        return;
    }

    if (property == "animation-iteration-count") {
        if (motion != nullptr) {
            motion->has_animation = true;
            if (auto ident = ident_from_value(*first); ident && equals_ignore_case(*ident, "infinite")) {
                motion->animation_loop = true;
            } else if (auto count = pixels_from_value(*first)) {
                motion->animation_loop = *count != 1.0f;
            }
        }
        return;
    }

    if (property == "transform") {
        Transform transform = widget.get_transform();
        for (auto* value : flattened_values(declaration.values)) {
            if (value != nullptr) {
                apply_transform_value(transform, *value);
            }
        }
        const auto raw = text_or_empty(declaration.raw);
        if (!raw.empty()) {
            apply_transform_text(transform, raw);
        } else if (declaration.string != nullptr) {
            apply_transform_text(transform, text_or_empty(declaration.string));
        }
        widget.set_transform(transform);
        return;
    }

    if (property == "translate" || property == "translation") {
        Transform transform = widget.get_transform();
        transform.translate_x = pixels_from_value(*first).value_or(transform.translate_x);
        if (auto* second = value_at(declaration.values, 1)) {
            transform.translate_y = pixels_from_value(*second).value_or(transform.translate_y);
        }
        widget.set_transform(transform);
        return;
    }

    if (property == "translate-x") {
        Transform transform = widget.get_transform();
        transform.translate_x = pixels_from_value(*first).value_or(transform.translate_x);
        widget.set_transform(transform);
        return;
    }

    if (property == "translate-y") {
        Transform transform = widget.get_transform();
        transform.translate_y = pixels_from_value(*first).value_or(transform.translate_y);
        widget.set_transform(transform);
        return;
    }

    if (property == "scale") {
        Transform transform = widget.get_transform();
        transform.scale_x = pixels_from_value(*first).value_or(transform.scale_x);
        if (auto* second = value_at(declaration.values, 1)) {
            transform.scale_y = pixels_from_value(*second).value_or(transform.scale_x);
        } else {
            transform.scale_y = transform.scale_x;
        }
        widget.set_transform(transform);
        return;
    }

    if (property == "scale-x") {
        Transform transform = widget.get_transform();
        transform.scale_x = pixels_from_value(*first).value_or(transform.scale_x);
        widget.set_transform(transform);
        return;
    }

    if (property == "scale-y") {
        Transform transform = widget.get_transform();
        transform.scale_y = pixels_from_value(*first).value_or(transform.scale_y);
        widget.set_transform(transform);
        return;
    }

    if (property == "rotate" || property == "rotation") {
        Transform transform = widget.get_transform();
        transform.rotation_degrees = pixels_from_value(*first).value_or(transform.rotation_degrees);
        widget.set_transform(transform);
        return;
    }

    if (property == "transform-origin") {
        Transform transform = widget.get_transform();
        transform.origin_x = pixels_from_value(*first).value_or(transform.origin_x);
        if (auto* second = value_at(declaration.values, 1)) {
            transform.origin_y = pixels_from_value(*second).value_or(transform.origin_y);
        }
        widget.set_transform(transform);
        return;
    }

    if (auto* label = dynamic_cast<Label*>(&widget)) {
        if (property == "font-size") {
            if (auto size = pixels_from_value(*first)) {
                label->set_font_size(*size);
            }
            return;
        }
        if (property == "font-family") {
            if (auto family = ident_from_value(*first)) {
                label->set_font_family(std::move(*family));
            }
            return;
        }
        if (property == "text-color") {
            if (auto color = color_from_value(*first)) {
                label->set_text_color(*color);
            }
            return;
        }
        if (property == "text-align") {
            if (auto align = ident_from_value(*first)) {
                const auto lower = lower_copy(*align);
                if (lower == "center") {
                    label->set_text_align(TextAlign::Center);
                } else if (lower == "right" || lower == "end") {
                    label->set_text_align(TextAlign::End);
                } else {
                    label->set_text_align(TextAlign::Start);
                }
            }
            return;
        }
        if (property == "text-overflow") {
            if (auto overflow = ident_from_value(*first)) {
                label->set_text_overflow(equals_ignore_case(*overflow, "wrap") ? TextOverflow::Wrap : TextOverflow::Clip);
            }
            return;
        }
    }

    if (!allow_widget_properties) {
        return;
    }

    if (property == "width") {
        widget.set_width(length_from_value(*first));
        return;
    }

    if (property == "height") {
        widget.set_height(length_from_value(*first));
        return;
    }

    if (property == "flex") {
        if (auto flex = pixels_from_value(*first)) {
            widget.set_flex(*flex);
        }
        return;
    }

    if (property == "clip-content" || property == "clip_content") {
        const auto value = (first->unit == KATANA_VALUE_IDENT || first->unit == KATANA_VALUE_STRING || first->unit == KATANA_VALUE_PARSER_IDENTIFIER)
            ? lower_copy(text_or_empty(first->string))
            : std::string {};
        if (value == "false" || value == "0" || value == "no" || value == "off" || value == "visible") {
            widget.set_clip_content(false);
        } else if (value == "true" || value == "1" || value == "yes" || value == "on" || value == "hidden") {
            widget.set_clip_content(true);
        } else if (auto numeric = pixels_from_value(*first)) {
            widget.set_clip_content(*numeric != 0.0f);
        }
        return;
    }

    if (property == "margin" || property == "padding") {
        float parts[4] {};
        unsigned int count = 0;
        for (; declaration.values != nullptr && count < declaration.values->length && count < 4; ++count) {
            auto* value = value_at(declaration.values, count);
            parts[count] = value != nullptr ? pixels_from_value(*value).value_or(0.0f) : 0.0f;
        }
        Insets insets;
        if (count == 1) {
            insets = Insets(parts[0]);
        } else if (count == 2) {
            insets = Insets(parts[1], parts[0]);
        } else if (count == 3) {
            insets = Insets(parts[1], parts[0], parts[1], parts[2]);
        } else if (count >= 4) {
            insets = Insets(parts[3], parts[0], parts[1], parts[2]);
        }

        if (property == "margin") {
            widget.set_margin(insets);
        } else {
            widget.set_padding(insets);
        }
        return;
    }
}

bool selector_matches(Widget& widget, KatanaSelector* selector, CssState& state)
{
    for (auto* current = selector; current != nullptr; current = current->tagHistory) {
        if (current->match == KatanaSelectorMatchTag) {
            const auto tag = current->tag != nullptr ? text_or_empty(current->tag->local) : std::string_view();
            if (!tag.empty() && tag != "*" && !equals_ignore_case(tag, widget.type_name()) && !equals_ignore_case(tag, "Widget")) {
                return false;
            }
        } else if (current->match == KatanaSelectorMatchId) {
            if (current->data == nullptr || !equals_ignore_case(text_or_empty(current->data->value), widget.name())) {
                return false;
            }
        } else if (current->match == KatanaSelectorMatchClass) {
            if (current->data == nullptr || !widget.has_class(text_or_empty(current->data->value))) {
                return false;
            }
        } else if (current->match == KatanaSelectorMatchPseudoClass) {
            switch (current->pseudo) {
            case KatanaPseudoHover:
                state = CssState::Hover;
                break;
            case KatanaPseudoActive:
                state = CssState::Pressed;
                break;
            case KatanaPseudoChecked:
                state = CssState::Selected;
                break;
            case KatanaPseudoFocus:
                state = CssState::Focus;
                break;
            default:
                if (current->data != nullptr && equals_ignore_case(text_or_empty(current->data->value), "selected")) {
                    state = CssState::Selected;
                } else if (current->data != nullptr && equals_ignore_case(text_or_empty(current->data->value), "pressed")) {
                    state = CssState::Pressed;
                } else {
                    return false;
                }
                break;
            }
        }

        if (current == current->tagHistory) {
            break;
        }
    }

    return true;
}

std::optional<float> offset_from_keyframe_selector(const KatanaValue& value)
{
    if (auto ident = ident_from_value(value)) {
        if (equals_ignore_case(*ident, "from")) {
            return 0.0f;
        }
        if (equals_ignore_case(*ident, "to")) {
            return 1.0f;
        }
    }
    if (value.unit == KATANA_VALUE_PERCENTAGE) {
        return std::clamp(static_cast<float>(value.fValue) / 100.0f, 0.0f, 1.0f);
    }
    return std::nullopt;
}

ParsedKeyframes collect_keyframes(KatanaStylesheet& stylesheet, Widget& parser_widget)
{
    ParsedKeyframes keyframes;
    for (unsigned int rule_index = 0; rule_index < stylesheet.rules.length; ++rule_index) {
        auto* base_rule = static_cast<KatanaRule*>(stylesheet.rules.data[rule_index]);
        if (base_rule == nullptr || base_rule->type != KatanaRuleKeyframes) {
            continue;
        }

        auto* keyframe_rule = reinterpret_cast<KatanaKeyframesRule*>(base_rule);
        std::vector<StyleKeyframe> frames;
        for (unsigned int frame_index = 0; keyframe_rule->keyframes != nullptr && frame_index < keyframe_rule->keyframes->length; ++frame_index) {
            auto* frame = static_cast<KatanaKeyframe*>(keyframe_rule->keyframes->data[frame_index]);
            if (frame == nullptr) {
                continue;
            }

            Style frame_style {};
            StyleProperties properties = style_property_mask(StyleProperty::None);
            bool touched = false;
            for (unsigned int declaration_index = 0; frame->declarations != nullptr && declaration_index < frame->declarations->length; ++declaration_index) {
                apply_declaration(
                    parser_widget,
                    frame_style,
                    *static_cast<KatanaDeclaration*>(frame->declarations->data[declaration_index]),
                    CssState::Base,
                    touched,
                    false,
                    &properties
                );
            }

            if (!touched) {
                continue;
            }

            bool pushed = false;
            for (unsigned int selector_index = 0; frame->selectors != nullptr && selector_index < frame->selectors->length; ++selector_index) {
                auto* selector = static_cast<KatanaValue*>(frame->selectors->data[selector_index]);
                if (selector == nullptr) {
                    continue;
                }
                if (auto offset = offset_from_keyframe_selector(*selector)) {
                    frames.push_back({ *offset, frame_style, properties });
                    pushed = true;
                }
            }
            if (!pushed) {
                const float offset = keyframe_rule->keyframes->length <= 1
                    ? 1.0f
                    : static_cast<float>(frame_index) / static_cast<float>(keyframe_rule->keyframes->length - 1U);
                frames.push_back({ offset, frame_style, properties });
            }
        }

        std::sort(frames.begin(), frames.end(), [](const auto& left, const auto& right) {
            return left.offset < right.offset;
        });
        if (!frames.empty()) {
            keyframes[std::string(text_or_empty(keyframe_rule->name))] = std::move(frames);
        }
    }
    return keyframes;
}

void apply_motion_declaration(Widget& widget, const CssMotionDeclaration& motion, const ParsedKeyframes& keyframes)
{
    if (motion.has_transition) {
        widget.set_transition(motion.transition);
    }

    if (!motion.has_animation) {
        return;
    }

    const auto found = keyframes.find(motion.animation_name);
    if (motion.animation_name.empty() || found == keyframes.end()) {
        widget.clear_animation();
        return;
    }

    widget.set_animation({
        .name = motion.animation_name,
        .duration = std::max(0.0001f, motion.animation_duration),
        .easing = motion.animation_easing,
        .loop = motion.animation_loop,
        .keyframes = found->second,
    });
}

bool apply_stylesheet_rule(Widget& widget, KatanaStyleRule& rule, Style& css_style, const ParsedKeyframes& keyframes)
{
    bool touched_style = false;
    bool matched = false;
    CssState state = CssState::Base;
    CssMotionDeclaration motion;

    for (unsigned int selector_index = 0; rule.selectors != nullptr && selector_index < rule.selectors->length; ++selector_index) {
        CssState selector_state = CssState::Base;
        if (selector_matches(widget, static_cast<KatanaSelector*>(rule.selectors->data[selector_index]), selector_state)) {
            state = selector_state;
            matched = true;
            break;
        }
    }

    if (!matched || rule.declarations == nullptr) {
        return false;
    }

    for (unsigned int declaration_index = 0; declaration_index < rule.declarations->length; ++declaration_index) {
        apply_declaration(
            widget,
            css_style,
            *static_cast<KatanaDeclaration*>(rule.declarations->data[declaration_index]),
            state,
            touched_style,
            true,
            nullptr,
            &motion
        );
    }

    apply_motion_declaration(widget, motion, keyframes);
    return touched_style;
}
#endif

float resolve_length(Length length, Size available, bool horizontal)
{
    return length.automatic() ? 0.0f : length.resolve(available, horizontal);
}

float normalized_animation_progress(const StyleAnimation& animation, float elapsed) noexcept
{
    if (animation.duration <= 0.0f) {
        return 1.0f;
    }

    if (animation.loop) {
        return std::fmod(std::max(0.0f, elapsed), animation.duration) / animation.duration;
    }

    return std::clamp(elapsed / animation.duration, 0.0f, 1.0f);
}

BorderEdges normalized_edges(BorderEdges edges, Border fallback)
{
    if (edges.empty() && fallback.width > 0.0f) {
        return BorderEdges(fallback);
    }
    return edges;
}

BorderEdges active_border_edges(const Style& style, bool selected, bool focused)
{
    if (selected) {
        return normalized_edges(style.borders_selected, style.border_selected);
    }
    if (focused) {
        return normalized_edges(style.borders_focused, style.border_focused);
    }
    return normalized_edges(style.borders, style.border);
}

float clamp_hit_radius(float radius, Rect rect) noexcept
{
    return std::clamp(radius, 0.0f, std::min(rect.width, rect.height) * 0.5f);
}

bool point_in_corner(Point point, Point center, float radius) noexcept
{
    if (radius <= 0.0f) {
        return true;
    }

    const float dx = point.x - center.x;
    const float dy = point.y - center.y;
    return dx * dx + dy * dy <= radius * radius;
}

bool rounded_rect_contains(Rect rect, CornerRadius radius, Point point) noexcept
{
    if (!rect.contains(point)) {
        return false;
    }

    const float top_left = clamp_hit_radius(radius.top_left, rect);
    const float top_right = clamp_hit_radius(radius.top_right, rect);
    const float bottom_right = clamp_hit_radius(radius.bottom_right, rect);
    const float bottom_left = clamp_hit_radius(radius.bottom_left, rect);

    if (top_left > 0.0f && point.x < rect.x + top_left && point.y < rect.y + top_left) {
        return point_in_corner(point, { rect.x + top_left, rect.y + top_left }, top_left);
    }
    if (top_right > 0.0f && point.x > rect.x + rect.width - top_right && point.y < rect.y + top_right) {
        return point_in_corner(point, { rect.x + rect.width - top_right, rect.y + top_right }, top_right);
    }
    if (bottom_right > 0.0f && point.x > rect.x + rect.width - bottom_right && point.y > rect.y + rect.height - bottom_right) {
        return point_in_corner(point, { rect.x + rect.width - bottom_right, rect.y + rect.height - bottom_right }, bottom_right);
    }
    if (bottom_left > 0.0f && point.x < rect.x + bottom_left && point.y > rect.y + rect.height - bottom_left) {
        return point_in_corner(point, { rect.x + bottom_left, rect.y + rect.height - bottom_left }, bottom_left);
    }

    return true;
}

} // namespace

Widget::~Widget()
{
    if (focused_widget_ == this) {
        focused_widget_ = nullptr;
    }

    detach_from_parent();

    for (auto* child : children_) {
        if (child != nullptr && child->parent_ == this) {
            child->parent_ = nullptr;
        }
    }
    children_.clear();
    owned_children_.clear();
}

void Widget::set_bounds(Rect bounds) noexcept
{
    bounds_ = bounds;
}

Rect Widget::bounds() const noexcept
{
    return bounds_;
}

void Widget::set_size(Size size) noexcept
{
    if (size.width > 0.0f) {
        bounds_.width = size.width;
        layout_.preferred_size.width = size.width;
        layout_.width = SizePolicy::Fixed;
    }

    if (size.height > 0.0f) {
        bounds_.height = size.height;
        layout_.preferred_size.height = size.height;
        layout_.height = SizePolicy::Fixed;
    }

    layout_.preferred_size = size;
}

void Widget::set_width(Length width) noexcept
{
    layout_.width_value = width;
    if (width.automatic()) {
        return;
    }
    layout_.width = SizePolicy::Fixed;
    if (width.unit == LengthUnit::Px) {
        bounds_.width = width.value;
        layout_.preferred_size.width = width.value;
    }
}

void Widget::set_height(Length height) noexcept
{
    layout_.height_value = height;
    if (height.automatic()) {
        return;
    }
    layout_.height = SizePolicy::Fixed;
    if (height.unit == LengthUnit::Px) {
        bounds_.height = height.value;
        layout_.preferred_size.height = height.value;
    }
}

void Widget::set_size(Length width, Length height) noexcept
{
    set_width(width);
    set_height(height);
}

void Widget::set_style(Style style) noexcept
{
    inline_style_ = style;
    has_inline_style_ = true;
    recompute_style();
}

const Style& Widget::style() const noexcept
{
    return style_;
}

const Style& Widget::get_style() const noexcept
{
    return style();
}

void Widget::set_background(Color color) noexcept
{
    auto next = style_;
    next.with_background(color);
    set_style(next);
}

Color Widget::get_background() const noexcept
{
    return style_.background;
}

void Widget::set_background_hovered(Color color) noexcept
{
    auto next = style_;
    next.with_background_hovered(color);
    set_style(next);
}

Color Widget::get_background_hovered() const noexcept
{
    return style_.hovered;
}

void Widget::set_background_pressed(Color color) noexcept
{
    auto next = style_;
    next.with_background_pressed(color);
    set_style(next);
}

Color Widget::get_background_pressed() const noexcept
{
    return style_.pressed;
}

void Widget::set_background_selected(Color color) noexcept
{
    auto next = style_;
    next.with_background_selected(color);
    set_style(next);
}

Color Widget::get_background_selected() const noexcept
{
    return style_.selected;
}

void Widget::set_background_focused(Color color) noexcept
{
    auto next = style_;
    next.with_background_focused(color);
    set_style(next);
}

Color Widget::get_background_focused() const noexcept
{
    return style_.focused;
}

void Widget::set_foreground(Color color) noexcept
{
    auto next = style_;
    next.with_foreground(color);
    set_style(next);
}

Color Widget::get_foreground() const noexcept
{
    return style_.foreground;
}

void Widget::set_border(Color color, float width) noexcept
{
    auto next = style_;
    next.with_border(color, width);
    set_style(next);
}

Border Widget::get_border() const noexcept
{
    return style_.border;
}

void Widget::set_border_left(Color color, float width) noexcept
{
    auto next = style_;
    next.with_border_left(color, width);
    set_style(next);
}

Border Widget::get_border_left() const noexcept
{
    return active_border_edges(style_, selected_, focused_).left;
}

void Widget::set_border_top(Color color, float width) noexcept
{
    auto next = style_;
    next.with_border_top(color, width);
    set_style(next);
}

Border Widget::get_border_top() const noexcept
{
    return active_border_edges(style_, selected_, focused_).top;
}

void Widget::set_border_right(Color color, float width) noexcept
{
    auto next = style_;
    next.with_border_right(color, width);
    set_style(next);
}

Border Widget::get_border_right() const noexcept
{
    return active_border_edges(style_, selected_, focused_).right;
}

void Widget::set_border_bottom(Color color, float width) noexcept
{
    auto next = style_;
    next.with_border_bottom(color, width);
    set_style(next);
}

Border Widget::get_border_bottom() const noexcept
{
    return active_border_edges(style_, selected_, focused_).bottom;
}

void Widget::set_border_selected(Color color, float width) noexcept
{
    auto next = style_;
    next.with_border_selected(color, width);
    set_style(next);
}

Border Widget::get_border_selected() const noexcept
{
    return style_.border_selected;
}

void Widget::set_border_focused(Color color, float width) noexcept
{
    auto next = style_;
    next.with_border_focused(color, width);
    set_style(next);
}

Border Widget::get_border_focused() const noexcept
{
    return style_.border_focused;
}

void Widget::set_radius(float radius) noexcept
{
    auto next = style_;
    next.with_radius(radius);
    set_style(next);
}

void Widget::set_radius(CornerRadius radius) noexcept
{
    auto next = style_;
    next.with_radius(radius);
    set_style(next);
}

CornerRadius Widget::get_radius() const noexcept
{
    return style_.radius;
}

void Widget::set_opacity(float opacity) noexcept
{
    auto next = style_;
    next.with_opacity(std::clamp(opacity, 0.0f, 1.0f));
    set_style(next);
}

float Widget::get_opacity() const noexcept
{
    return style_.opacity;
}

void Widget::set_transition(StyleTransition transition) noexcept
{
    transition.duration = std::max(0.0f, transition.duration);
    transition.enabled = transition.enabled && transition.duration > 0.0f;
    transition_ = transition;
}

void Widget::set_transition(float duration, Easing easing) noexcept
{
    set_transition({ std::max(0.0f, duration), easing, duration > 0.0f });
}

void Widget::clear_transition() noexcept
{
    transition_ = {};
    transition_active_ = false;
    transition_elapsed_ = 0.0f;
    style_ = target_style_;
}

const StyleTransition& Widget::transition() const noexcept
{
    return transition_;
}

const StyleTransition& Widget::get_transition() const noexcept
{
    return transition();
}

void Widget::set_animation(StyleAnimation animation)
{
    std::sort(animation.keyframes.begin(), animation.keyframes.end(), [](const auto& left, const auto& right) {
        return left.offset < right.offset;
    });
    animation.duration = std::max(0.0001f, animation.duration);
    animation_ = std::move(animation);
    animation_elapsed_ = 0.0f;
}

void Widget::clear_animation() noexcept
{
    animation_.reset();
    animation_elapsed_ = 0.0f;
    style_ = transition_active_ ? style_ : target_style_;
}

const std::optional<StyleAnimation>& Widget::animation() const noexcept
{
    return animation_;
}

const std::optional<StyleAnimation>& Widget::get_animation() const noexcept
{
    return animation();
}

bool Widget::animation_running() const noexcept
{
    return animation_.has_value();
}

void Widget::set_stylesheet(std::string stylesheet)
{
    stylesheet_ = std::move(stylesheet);
    apply_stylesheet_to_tree();
}

void Widget::join_stylesheet(std::string_view stylesheet)
{
    if (!stylesheet_.empty() && !stylesheet.empty()) {
        stylesheet_.push_back('\n');
    }
    stylesheet_.append(stylesheet);
    apply_stylesheet_to_tree();
}

std::string_view Widget::get_stylesheet() const noexcept
{
    return stylesheet_;
}

void Widget::set_name(std::string name)
{
    name_ = std::move(name);
    if (!stylesheet_.empty()) {
        apply_stylesheet_to_tree();
    } else if (parent_ != nullptr) {
        parent_->apply_stylesheet_to_tree();
    }
}

std::string_view Widget::name() const noexcept
{
    return name_;
}

std::string_view Widget::get_name() const noexcept
{
    return name();
}

void Widget::set_type_name(std::string type_name)
{
    type_name_ = std::move(type_name);
    if (type_name_.empty()) {
        type_name_ = "Widget";
    }
    if (!stylesheet_.empty()) {
        apply_stylesheet_to_tree();
    } else if (parent_ != nullptr) {
        parent_->apply_stylesheet_to_tree();
    }
}

std::string_view Widget::type_name() const noexcept
{
    return type_name_;
}

Widget& Widget::add_class(std::string class_name)
{
    if (!class_name.empty() && !has_class(class_name)) {
        classes_.push_back(std::move(class_name));
        if (!stylesheet_.empty()) {
            apply_stylesheet_to_tree();
        } else if (parent_ != nullptr) {
            parent_->apply_stylesheet_to_tree();
        }
    }
    return *this;
}

bool Widget::remove_class(std::string_view class_name)
{
    const auto before = classes_.size();
    classes_.erase(std::remove_if(classes_.begin(), classes_.end(), [class_name](const auto& current) {
        return current == class_name;
    }), classes_.end());
    const bool removed = classes_.size() != before;
    if (removed) {
        if (!stylesheet_.empty()) {
            apply_stylesheet_to_tree();
        } else if (parent_ != nullptr) {
            parent_->apply_stylesheet_to_tree();
        }
    }
    return removed;
}

bool Widget::has_class(std::string_view class_name) const noexcept
{
    return std::any_of(classes_.begin(), classes_.end(), [class_name](const auto& current) {
        return current == class_name;
    });
}

const std::vector<std::string>& Widget::classes() const noexcept
{
    return classes_;
}

void Widget::set_layout(Layout layout) noexcept
{
    layout_ = layout;
}

const Layout& Widget::layout_rules() const noexcept
{
    return layout_;
}

void Widget::set_layout_policy(SizePolicy width, SizePolicy height) noexcept
{
    layout_.width = width;
    layout_.height = height;
}

void Widget::set_margin(Insets margin) noexcept
{
    layout_.margin = margin;
}

Insets Widget::get_margin() const noexcept
{
    return layout_.margin;
}

void Widget::set_padding(Insets padding) noexcept
{
    layout_.padding = padding;
}

Insets Widget::get_padding() const noexcept
{
    return layout_.padding;
}

void Widget::set_flex(float flex) noexcept
{
    layout_.flex = std::max(0.0f, flex);
}

float Widget::get_flex() const noexcept
{
    return layout_.flex;
}

void Widget::set_transform(Transform transform) noexcept
{
    transform_ = transform;
}

const Transform& Widget::transform() const noexcept
{
    return transform_;
}

const Transform& Widget::get_transform() const noexcept
{
    return transform();
}

void Widget::set_translation(float x, float y) noexcept
{
    transform_.translate_x = x;
    transform_.translate_y = y;
}

void Widget::set_scale(float scale) noexcept
{
    set_scale(scale, scale);
}

void Widget::set_scale(float x, float y) noexcept
{
    transform_.scale_x = x;
    transform_.scale_y = y;
}

void Widget::set_rotation(float degrees) noexcept
{
    transform_.rotation_degrees = degrees;
}

void Widget::set_transform_origin(float x, float y) noexcept
{
    transform_.origin_x = x;
    transform_.origin_y = y;
}

void Widget::set_visible(bool visible) noexcept
{
    visible_ = visible;
    if (!visible_ && focused_) {
        blur();
    }
}

bool Widget::visible() const noexcept
{
    return visible_;
}

void Widget::set_enabled(bool enabled) noexcept
{
    enabled_ = enabled;
    if (!enabled_ && focused_) {
        blur();
    }
}

bool Widget::enabled() const noexcept
{
    return enabled_;
}

void Widget::set_clip_content(bool clip) noexcept
{
    clip_content_ = clip;
}

bool Widget::clip_content() const noexcept
{
    return clip_content_;
}

void Widget::set_focusable(bool focusable) noexcept
{
    focusable_ = focusable;
    if (!focusable_ && focused_) {
        blur();
    }
}

bool Widget::focusable() const noexcept
{
    return focusable_;
}

bool Widget::can_focus() const noexcept
{
    return visible_ && enabled_ && focusable_;
}

void Widget::set_keyboard_activation_enabled(bool enabled) noexcept
{
    keyboard_activation_enabled_ = enabled;
    if (enabled) {
        focusable_ = true;
    }
}

bool Widget::keyboard_activation_enabled() const noexcept
{
    return keyboard_activation_enabled_;
}

void Widget::set_accessibility_role(AccessibilityRole role) noexcept
{
    accessibility_.role = role;
}

AccessibilityRole Widget::accessibility_role() const noexcept
{
    return accessibility_.role;
}

void Widget::set_accessibility_label(std::string label)
{
    accessibility_.label = std::move(label);
}

std::string_view Widget::accessibility_label() const noexcept
{
    return accessibility_.label;
}

void Widget::set_accessibility_description(std::string description)
{
    accessibility_.description = std::move(description);
}

std::string_view Widget::accessibility_description() const noexcept
{
    return accessibility_.description;
}

void Widget::set_accessibility(AccessibilityInfo info)
{
    accessibility_ = std::move(info);
}

const AccessibilityInfo& Widget::accessibility() const noexcept
{
    return accessibility_;
}

bool Widget::focus_next(bool reverse) noexcept
{
    std::vector<Widget*> focusable_widgets;
    collect_focusable_widgets(focusable_widgets);
    if (focusable_widgets.empty()) {
        return false;
    }

    auto current = std::find(focusable_widgets.begin(), focusable_widgets.end(), focused_widget_);
    if (current == focusable_widgets.end()) {
        (reverse ? focusable_widgets.back() : focusable_widgets.front())->focus();
        return true;
    }

    if (reverse) {
        if (current == focusable_widgets.begin()) {
            focusable_widgets.back()->focus();
        } else {
            (*std::prev(current))->focus();
        }
    } else {
        ++current;
        if (current == focusable_widgets.end()) {
            focusable_widgets.front()->focus();
        } else {
            (*current)->focus();
        }
    }
    return true;
}

Widget& Widget::add_child(Widget& child)
{
    if (!accepts_children_) {
        throw std::invalid_argument("This widget does not accept child widgets");
    }

    if (&child == this) {
        throw std::invalid_argument("A widget cannot be added as its own child");
    }

    if (child.parent_ == this) {
        return child;
    }

    if (child.parent_ != nullptr) {
        if (child.parent_->owns_child(child)) {
            throw std::invalid_argument("Cannot reparent a widget owned by another parent");
        }
        child.parent_->detach_child(child, false);
    }

    children_.erase(std::remove(children_.begin(), children_.end(), &child), children_.end());
    child.parent_ = this;
    children_.push_back(&child);
    apply_stylesheet_to_tree();
    return child;
}

Widget& Widget::add_child(std::unique_ptr<Widget> child)
{
    if (!accepts_children_) {
        throw std::invalid_argument("This widget does not accept child widgets");
    }

    if (!child) {
        throw std::invalid_argument("Cannot add a null widget child");
    }

    auto& reference = *child;
    if (&reference == this) {
        throw std::invalid_argument("A widget cannot be added as its own child");
    }

    if (reference.parent_ != nullptr) {
        reference.parent_->detach_child(reference, false);
    }

    children_.erase(std::remove(children_.begin(), children_.end(), &reference), children_.end());
    reference.parent_ = this;
    owned_children_.push_back(std::move(child));
    children_.push_back(&reference);
    apply_stylesheet_to_tree();
    return reference;
}

bool Widget::remove_child(Widget& child) noexcept
{
    return detach_child(child, true);
}

void Widget::clear_children() noexcept
{
    for (auto* child : children_) {
        if (child != nullptr && child->parent_ == this) {
            child->parent_ = nullptr;
        }
    }

    children_.clear();
    owned_children_.clear();
}

void Widget::set_accepts_children(bool accepts) noexcept
{
    accepts_children_ = accepts;
    if (!accepts_children_) {
        clear_children();
    }
}

bool Widget::accepts_children() const noexcept
{
    return accepts_children_;
}

const std::vector<Widget*>& Widget::children() const noexcept
{
    return children_;
}

Widget* Widget::parent() noexcept
{
    return parent_;
}

const Widget* Widget::parent() const noexcept
{
    return parent_;
}

void Widget::set_state(WidgetState state, bool enabled) noexcept
{
    if (state == WidgetState::Selected) {
        selected_ = enabled;
    } else if (state == WidgetState::Focused) {
        if (enabled) {
            focus();
        } else {
            blur();
        }
    }
}

void Widget::toggle_state(WidgetState state) noexcept
{
    set_state(state, !has_state(state));
}

bool Widget::has_state(WidgetState state) const noexcept
{
    if (state == WidgetState::Selected) {
        return selected_;
    }
    if (state == WidgetState::Focused) {
        return focused_;
    }
    return false;
}

bool Widget::hovered() const noexcept
{
    return hovered_;
}

bool Widget::pressed() const noexcept
{
    return pressed_;
}

bool Widget::focused() const noexcept
{
    return focused_;
}

void Widget::focus() noexcept
{
    if (focused_widget_ == this) {
        return;
    }

    if (focused_widget_ != nullptr) {
        focused_widget_->blur();
    }

    focused_widget_ = this;
    focused_ = true;
    on_focus();
}

void Widget::blur() noexcept
{
    if (!focused_) {
        return;
    }

    focused_ = false;
    if (focused_widget_ == this) {
        focused_widget_ = nullptr;
    }
    on_blur();
}

bool Widget::hit_test(Point point) const noexcept
{
    return visible_ && enabled_ && rounded_rect_contains(bounds_, style_.radius, point);
}

void Widget::layout(Size available)
{
    advance_style_motion(1.0f / 60.0f);

    const auto clamp_width = [this](float value) {
        return std::clamp(value, layout_.min_size.width, layout_.max_size.width);
    };
    const auto clamp_height = [this](float value) {
        return std::clamp(value, layout_.min_size.height, layout_.max_size.height);
    };

    if (!layout_.width_value.automatic()) {
        if (parent_ == nullptr || bounds_.width == 0.0f) {
            bounds_.width = clamp_width(resolve_length(layout_.width_value, available, true));
        }
    } else if (bounds_.width == 0.0f || layout_.width == SizePolicy::Fill) {
        bounds_.width = clamp_width(available.width);
    } else if (layout_.width == SizePolicy::Fixed) {
        bounds_.width = clamp_width(layout_.preferred_size.width);
    }

    if (!layout_.height_value.automatic()) {
        if (parent_ == nullptr || bounds_.height == 0.0f) {
            bounds_.height = clamp_height(resolve_length(layout_.height_value, available, false));
        }
    } else if (bounds_.height == 0.0f || layout_.height == SizePolicy::Fill) {
        bounds_.height = clamp_height(available.height);
    } else if (layout_.height == SizePolicy::Fixed) {
        bounds_.height = clamp_height(layout_.preferred_size.height);
    }

    const Rect content = bounds_.inset(layout_.padding);
    on_layout(content);
    for (auto* child : children_) {
        if (child->bounds().width == 0.0f && child->bounds().height == 0.0f) {
            child->set_bounds(content);
        }
        const auto child_bounds = child->bounds();
        child->layout({ child_bounds.width, child_bounds.height });
    }
}

void Widget::render(Renderer& renderer)
{
    if (!visible_) {
        return;
    }

    renderer.push_transform(bounds_, transform_);
    draw(renderer);

    if (clip_content_) {
        renderer.push_clip(bounds_);
    }
    for (auto* child : children_) {
        child->render(renderer);
    }
    if (clip_content_) {
        renderer.pop_clip();
    }
    renderer.pop_transform();
}

bool Widget::event(const Event& event)
{
    if (!visible_ || !enabled_) {
        return false;
    }

    if (const auto* key = std::get_if<KeyEvent>(&event)) {
        return handle_key_event(*key);
    }

    const auto mouse = mouse_event_from(event);
    if (mouse.has_value() && mouse->type == MouseEventType::Leave) {
        clear_mouse_state(mouse->position);
        return on_event(event);
    }

    const auto* wheel = std::get_if<MouseWheelEvent>(&event);
    if ((mouse.has_value() || wheel != nullptr) && clip_content_) {
        const Point position = mouse.has_value() ? mouse->position : wheel->position;
        if (!hit_test(position)) {
            clear_mouse_state(position);
            return false;
        }
    }

    for (auto it = children_.rbegin(); it != children_.rend(); ++it) {
        if ((*it)->event(event)) {
            return true;
        }
    }

    if (!mouse.has_value()) {
        return on_event(event);
    }

    const bool inside = hit_test(mouse->position);
    if (mouse->type == MouseEventType::Move) {
        if (inside && !hovered_) {
            hovered_ = true;
            MouseEvent enter = *mouse;
            enter.type = MouseEventType::Enter;
            on_mouse_enter(enter);
        } else if (!inside && hovered_) {
            hovered_ = false;
            pressed_ = false;
            MouseEvent leave = *mouse;
            leave.type = MouseEventType::Leave;
            on_mouse_leave(leave);
        }

        return inside && on_mouse_move(*mouse);
    }

    if (!inside && !pressed_) {
        return false;
    }

    if (mouse->type == MouseEventType::Down) {
        pressed_ = inside;
        if (inside && can_focus()) {
            focus();
        }
        return inside && on_mouse_down(*mouse);
    }

    if (mouse->type == MouseEventType::Up) {
        const bool clicked = pressed_ && inside;
        pressed_ = false;
        const bool handled = on_mouse_up(*mouse);
        if (clicked) {
            MouseEvent click = *mouse;
            click.type = MouseEventType::Click;
            return on_click(click) || handled;
        }
        return handled;
    }

    return on_event(event);
}

std::vector<Widget*>& Widget::mutable_children() noexcept
{
    return children_;
}

void Widget::draw(Renderer& renderer)
{
    if (style_.opacity <= 0.0f) {
        return;
    }

    const Color background = selected_ ? style_.selected : (focused_ ? style_.focused : (pressed_ ? style_.pressed : (hovered_ ? style_.hovered : style_.background)));
    renderer.fill_rounded_rect(bounds_, style_.radius, background);
    const auto borders = active_border_edges(style_, selected_, focused_);
    if (borders.empty()) {
        return;
    }

    renderer.stroke_rounded_rect(bounds_, style_.radius, borders);
}

void Widget::on_layout(Rect content)
{
    (void)content;
}

bool Widget::on_event(const Event& event)
{
    (void)event;
    return false;
}

void Widget::on_mouse_enter(const MouseEvent& event)
{
    (void)event;
}

void Widget::on_mouse_leave(const MouseEvent& event)
{
    (void)event;
}

void Widget::on_focus()
{
}

void Widget::on_blur()
{
}

bool Widget::on_mouse_move(const MouseEvent& event)
{
    (void)event;
    return false;
}

bool Widget::on_mouse_down(const MouseEvent& event)
{
    (void)event;
    return true;
}

bool Widget::on_mouse_up(const MouseEvent& event)
{
    (void)event;
    return true;
}

bool Widget::on_click(const MouseEvent& event)
{
    (void)event;
    return false;
}

bool Widget::on_key_down(const KeyEvent& event)
{
    (void)event;
    return false;
}

bool Widget::on_key_up(const KeyEvent& event)
{
    (void)event;
    return false;
}

bool Widget::on_keyboard_activate(const KeyEvent& event)
{
    (void)event;
    const Point center {
        bounds_.x + bounds_.width * 0.5f,
        bounds_.y + bounds_.height * 0.5f,
    };
    return on_click(MouseEvent {
        MouseEventType::Click,
        center,
        { bounds_.width * 0.5f, bounds_.height * 0.5f },
        MouseButton::Left,
    });
}

void Widget::detach_from_parent() noexcept
{
    if (parent_ != nullptr) {
        auto* parent = parent_;
        parent_ = nullptr;
        parent->detach_child(*this, false);
    }
}

bool Widget::detach_child(Widget& child, bool destroy_owned) noexcept
{
    bool removed = false;
    auto child_it = std::remove(children_.begin(), children_.end(), &child);
    if (child_it != children_.end()) {
        children_.erase(child_it, children_.end());
        removed = true;
    }

    if (child.parent_ == this) {
        child.parent_ = nullptr;
        removed = true;
    }

    auto owned_it = std::find_if(owned_children_.begin(), owned_children_.end(), [&child](const auto& owned) {
        return owned.get() == &child;
    });

    if (owned_it != owned_children_.end()) {
        removed = true;
        if (destroy_owned) {
            (*owned_it)->parent_ = nullptr;
            owned_children_.erase(owned_it);
        }
    }

    return removed;
}

bool Widget::owns_child(const Widget& child) const noexcept
{
    return std::any_of(owned_children_.begin(), owned_children_.end(), [&child](const auto& owned) {
        return owned.get() == &child;
    });
}

void Widget::recompute_style() noexcept
{
    Style next = has_stylesheet_style_ ? stylesheet_style_ : Style {};
    if (has_inline_style_) {
        next = inline_style_;
    }

    if (!has_computed_style_) {
        style_ = next;
        target_style_ = next;
        has_computed_style_ = true;
        return;
    }

    if (style_equals(target_style_, next)) {
        return;
    }

    target_style_ = next;
    if (transition_.enabled && transition_.duration > 0.0f) {
        transition_from_style_ = style_;
        transition_to_style_ = next;
        transition_elapsed_ = 0.0f;
        transition_active_ = true;
    } else {
        style_ = next;
        transition_active_ = false;
        transition_elapsed_ = 0.0f;
    }
}

Style Widget::sample_animation_style(const Style& base, float progress) const noexcept
{
    if (!animation_ || animation_->keyframes.empty()) {
        return base;
    }

    const auto& frames = animation_->keyframes;
    const float eased = apply_easing(animation_->easing, progress);
    const auto upper = std::find_if(frames.begin(), frames.end(), [eased](const auto& frame) {
        return frame.offset >= eased;
    });

    if (upper == frames.begin()) {
        return apply_animated_style(base, upper->style, upper->properties);
    }

    if (upper == frames.end()) {
        return apply_animated_style(base, frames.back().style, frames.back().properties);
    }

    const auto& to = *upper;
    const auto& from = *(upper - 1);
    const float span = std::max(0.0001f, to.offset - from.offset);
    const float local = std::clamp((eased - from.offset) / span, 0.0f, 1.0f);
    const Style from_style = apply_animated_style(base, from.style, from.properties);
    const Style to_style = apply_animated_style(base, to.style, to.properties);
    const Style sampled = interpolate_style(from_style, to_style, local);
    return apply_animated_style(base, sampled, from.properties | to.properties);
}

void Widget::advance_style_motion(float dt) noexcept
{
    Style base = target_style_;
    if (transition_active_) {
        transition_elapsed_ += std::max(0.0f, dt);
        const float progress = transition_.duration <= 0.0f ? 1.0f : std::clamp(transition_elapsed_ / transition_.duration, 0.0f, 1.0f);
        base = interpolate_style(transition_from_style_, transition_to_style_, apply_easing(transition_.easing, progress));
        if (progress >= 1.0f) {
            transition_active_ = false;
            base = target_style_;
        }
    }

    if (animation_) {
        animation_elapsed_ += std::max(0.0f, dt);
        const float progress = normalized_animation_progress(*animation_, animation_elapsed_);
        style_ = sample_animation_style(base, progress);
        if (!animation_->loop && animation_elapsed_ >= animation_->duration) {
            animation_elapsed_ = animation_->duration;
        }
        return;
    }

    style_ = base;
}

void Widget::apply_stylesheet_to_tree()
{
#if OUIF_WITH_KATANA
    if (stylesheet_.empty()) {
        has_stylesheet_style_ = false;
        stylesheet_style_ = Style {};
        recompute_style();
        for (auto* child : children_) {
            child->apply_stylesheet_to_tree();
        }
        return;
    }

    KatanaOutputPtr output(katana_parse(stylesheet_.c_str(), stylesheet_.size(), KatanaParserModeStylesheet));
    if (!output || output->errors.length > 0 || output->stylesheet == nullptr) {
        return;
    }

    const auto keyframes = collect_keyframes(*output->stylesheet, *this);

    const auto apply_to = [&](Widget& widget) {
        Style css_style {};
        bool touched = false;
        for (unsigned int rule_index = 0; rule_index < output->stylesheet->rules.length; ++rule_index) {
            auto* base_rule = static_cast<KatanaRule*>(output->stylesheet->rules.data[rule_index]);
            if (base_rule == nullptr || base_rule->type != KatanaRuleStyle) {
                continue;
            }
            touched = apply_stylesheet_rule(widget, *reinterpret_cast<KatanaStyleRule*>(base_rule), css_style, keyframes) || touched;
        }
        widget.stylesheet_style_ = css_style;
        widget.has_stylesheet_style_ = touched;
        widget.recompute_style();
    };

    const std::function<void(Widget&)> apply_recursive = [&](Widget& widget) {
        apply_to(widget);
        for (auto* child : widget.children_) {
            if (child != nullptr) {
                apply_recursive(*child);
            }
        }
    };

    apply_recursive(*this);
#else
    (void)this;
#endif
}

bool Widget::handle_key_event(const KeyEvent& event)
{
    if ((event.action == KeyAction::Press || event.action == KeyAction::Repeat)
        && event.key == static_cast<std::uint32_t>(Key::Tab)) {
        return focus_next(event.shift);
    }

    if (focused_widget_ != nullptr && contains_widget(*focused_widget_)) {
        return focused_widget_->handle_focused_key_event(event);
    }

    return handle_focused_key_event(event);
}

bool Widget::handle_focused_key_event(const KeyEvent& event)
{
    bool handled = false;
    if (event.action == KeyAction::Release) {
        handled = on_key_up(event);
    } else {
        handled = on_key_down(event);
    }

    if (handled) {
        return true;
    }

    const bool activate_key = event.key == static_cast<std::uint32_t>(Key::Enter)
        || event.key == static_cast<std::uint32_t>(Key::Space);
    if (keyboard_activation_enabled_ && activate_key && (event.action == KeyAction::Press || event.action == KeyAction::Repeat)) {
        return on_keyboard_activate(event);
    }

    return false;
}

void Widget::collect_focusable_widgets(std::vector<Widget*>& widgets) noexcept
{
    if (can_focus()) {
        widgets.push_back(this);
    }

    for (auto* child : children_) {
        if (child != nullptr) {
            child->collect_focusable_widgets(widgets);
        }
    }
}

bool Widget::contains_widget(const Widget& widget) const noexcept
{
    if (&widget == this) {
        return true;
    }

    return std::any_of(children_.begin(), children_.end(), [&widget](const auto* child) {
        return child != nullptr && child->contains_widget(widget);
    });
}

void Widget::clear_mouse_state(Point position) noexcept
{
    for (auto* child : children_) {
        if (child != nullptr) {
            child->clear_mouse_state(position);
        }
    }

    if (hovered_) {
        hovered_ = false;
        MouseEvent leave { MouseEventType::Leave, position, to_local(position), MouseButton::Left };
        on_mouse_leave(leave);
    }

    pressed_ = false;
}

Point Widget::to_local(Point point) const noexcept
{
    return { point.x - bounds_.x, point.y - bounds_.y };
}

std::optional<MouseEvent> Widget::mouse_event_from(const Event& event) const noexcept
{
    if (const auto* move = std::get_if<MouseMoveEvent>(&event)) {
        return MouseEvent { MouseEventType::Move, move->position, to_local(move->position), MouseButton::Left };
    }

    if (const auto* button = std::get_if<MouseButtonEvent>(&event)) {
        return MouseEvent {
            button->pressed ? MouseEventType::Down : MouseEventType::Up,
            button->position,
            to_local(button->position),
            button->button,
        };
    }

    if (const auto* mouse = std::get_if<MouseEvent>(&event)) {
        MouseEvent local = *mouse;
        local.local_position = to_local(mouse->position);
        return local;
    }

    return std::nullopt;
}

} // namespace ouif
