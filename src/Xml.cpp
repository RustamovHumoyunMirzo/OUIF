#include <OUIF/Application.h>

#include <OUIF/Layout.h>
#include <OUIF/Widgets.h>

#include <algorithm>
#include <charconv>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#if OUIF_WITH_PUGIXML
#include <pugixml.hpp>
#endif

namespace ouif {

#if OUIF_WITH_PUGIXML
class XmlLoaderAccess {
public:
    static ApplicationConfig& config(Application& app) noexcept { return app.config_; }
    static std::unordered_map<std::string, XmlWidgetFactory>& factories(Application& app) noexcept { return app.xml_factories_; }
};
#endif

namespace {

std::string lower_copy(std::string_view value)
{
    std::string copy(value);
    std::transform(copy.begin(), copy.end(), copy.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return copy;
}

std::string trim_copy(std::string_view value)
{
    while (!value.empty() && std::isspace(static_cast<unsigned char>(value.front())) != 0) {
        value.remove_prefix(1);
    }
    while (!value.empty() && std::isspace(static_cast<unsigned char>(value.back())) != 0) {
        value.remove_suffix(1);
    }
    return std::string(value);
}

std::vector<std::string> split_words(std::string_view value)
{
    std::vector<std::string> words;
    std::istringstream stream { std::string(value) };
    std::string word;
    while (stream >> word) {
        words.push_back(std::move(word));
    }
    return words;
}

std::vector<std::string> split_list(std::string_view value)
{
    std::vector<std::string> parts;
    std::string current;
    for (const char ch : value) {
        if (ch == ',') {
            parts.push_back(trim_copy(current));
            current.clear();
        } else {
            current.push_back(ch);
        }
    }
    parts.push_back(trim_copy(current));
    return parts;
}

std::optional<float> parse_float(std::string_view value)
{
    const auto trimmed = trim_copy(value);
    if (trimmed.empty()) {
        return std::nullopt;
    }

    float parsed = 0.0f;
    const auto* first = trimmed.data();
    const auto* last = trimmed.data() + trimmed.size();
    const auto result = std::from_chars(first, last, parsed);
    if (result.ec == std::errc()) {
        return parsed;
    }
    return std::nullopt;
}

Length parse_length(std::string_view value)
{
    auto text = trim_copy(value);
    auto lower = lower_copy(text);
    if (lower.empty() || lower == "auto") {
        return Length::auto_value();
    }

    const auto read_number = [](std::string_view number) {
        return parse_float(number).value_or(0.0f);
    };

    if (lower.ends_with("px")) {
        return Length::px(read_number(std::string_view(text).substr(0, text.size() - 2)));
    }
    if (lower.ends_with("vw")) {
        return Length::vw(read_number(std::string_view(text).substr(0, text.size() - 2)));
    }
    if (lower.ends_with("vh")) {
        return Length::vh(read_number(std::string_view(text).substr(0, text.size() - 2)));
    }
    if (lower.ends_with("%")) {
        return Length::percent(read_number(std::string_view(text).substr(0, text.size() - 1)));
    }

    return Length::px(read_number(text));
}

Insets parse_insets(std::string_view value)
{
    const auto parts = split_list(value);
    if (parts.size() == 1) {
        return Insets(parse_float(parts[0]).value_or(0.0f));
    }
    if (parts.size() == 2) {
        return Insets(parse_float(parts[0]).value_or(0.0f), parse_float(parts[1]).value_or(0.0f));
    }
    if (parts.size() == 3) {
        const float top = parse_float(parts[0]).value_or(0.0f);
        const float horizontal = parse_float(parts[1]).value_or(0.0f);
        const float bottom = parse_float(parts[2]).value_or(0.0f);
        return Insets(horizontal, top, horizontal, bottom);
    }
    return Insets(
        parse_float(parts[0]).value_or(0.0f),
        parse_float(parts.size() > 1 ? parts[1] : "").value_or(0.0f),
        parse_float(parts.size() > 2 ? parts[2] : "").value_or(0.0f),
        parse_float(parts.size() > 3 ? parts[3] : "").value_or(0.0f)
    );
}

Size parse_size(std::string_view value)
{
    const auto parts = split_list(value);
    return {
        parse_float(parts.empty() ? "" : parts[0]).value_or(0.0f),
        parse_float(parts.size() > 1 ? parts[1] : "").value_or(0.0f),
    };
}

SizePolicy parse_policy(std::string_view value)
{
    const auto lower = lower_copy(trim_copy(value));
    if (lower == "fixed") {
        return SizePolicy::Fixed;
    }
    if (lower == "content" || lower == "wrap" || lower == "wrap-content") {
        return SizePolicy::Content;
    }
    return SizePolicy::Fill;
}

Align parse_align(std::string_view value)
{
    const auto lower = lower_copy(trim_copy(value));
    if (lower == "center" || lower == "middle") {
        return Align::Center;
    }
    if (lower == "end" || lower == "right" || lower == "bottom") {
        return Align::End;
    }
    return Align::Start;
}

AccessibilityRole parse_role(std::string_view value)
{
    const auto lower = lower_copy(trim_copy(value));
    if (lower == "button") {
        return AccessibilityRole::Button;
    }
    if (lower == "checkbox") {
        return AccessibilityRole::Checkbox;
    }
    if (lower == "radio") {
        return AccessibilityRole::Radio;
    }
    if (lower == "slider") {
        return AccessibilityRole::Slider;
    }
    if (lower == "textinput" || lower == "text-input" || lower == "input") {
        return AccessibilityRole::TextInput;
    }
    if (lower == "label") {
        return AccessibilityRole::Label;
    }
    if (lower == "container") {
        return AccessibilityRole::Container;
    }
    if (lower == "none") {
        return AccessibilityRole::None;
    }
    return AccessibilityRole::Widget;
}

Orientation parse_orientation(std::string_view value)
{
    const auto lower = lower_copy(trim_copy(value));
    if (lower == "vertical" || lower == "v" || lower == "column" || lower == "col") {
        return Orientation::Vertical;
    }
    return Orientation::Horizontal;
}

std::string read_text_file(const std::filesystem::path& path)
{
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Failed to open file: " + path.string());
    }

    std::ostringstream stream;
    stream << file.rdbuf();
    return stream.str();
}

std::filesystem::path resolve_path(const std::filesystem::path& base_path, std::string_view path)
{
    std::filesystem::path result { std::string(path) };
    if (result.is_relative() && !base_path.empty()) {
        result = base_path / result;
    }
    return result.lexically_normal();
}

#if OUIF_WITH_PUGIXML
XmlElement element_from_node(const pugi::xml_node& node)
{
    std::vector<XmlAttribute> attributes;
    for (auto attribute : node.attributes()) {
        attributes.push_back({ attribute.name(), attribute.value() });
    }
    return XmlElement(node.name(), std::move(attributes));
}

bool tag_is(std::string_view actual, std::string_view expected)
{
    return lower_copy(actual) == lower_copy(expected);
}

void apply_window_config(ApplicationConfig& config, const XmlElement& element)
{
    if (element.has_attribute("title")) {
        config.title = std::string(element.attribute("title"));
    }
    if (auto size = element.attribute_size("size")) {
        config.width = static_cast<std::uint32_t>(std::max(0.0f, size->width));
        config.height = static_cast<std::uint32_t>(std::max(0.0f, size->height));
    }
    if (auto width = element.attribute_float("width")) {
        config.width = static_cast<std::uint32_t>(std::max(0.0f, *width));
    }
    if (auto height = element.attribute_float("height")) {
        config.height = static_cast<std::uint32_t>(std::max(0.0f, *height));
    }
    if (auto color = element.attribute_color("clear_color")) {
        config.clear_color = *color;
    } else if (auto color = element.attribute_color("clear-color")) {
        config.clear_color = *color;
    }
}

std::unique_ptr<Widget> make_builtin_widget(std::string_view tag)
{
    if (tag_is(tag, "Widget")) {
        return std::make_unique<Widget>();
    }
    if (tag_is(tag, "RowLayout") || tag_is(tag, "Row")) {
        return std::make_unique<RowLayout>();
    }
    if (tag_is(tag, "ColLayout") || tag_is(tag, "Column") || tag_is(tag, "Col")) {
        return std::make_unique<ColLayout>();
    }
    if (tag_is(tag, "RowScroll") || tag_is(tag, "HorizontalScroll")) {
        return std::make_unique<RowScroll>();
    }
    if (tag_is(tag, "ColScroll") || tag_is(tag, "VerticalScroll")) {
        return std::make_unique<ColScroll>();
    }
    if (tag_is(tag, "Spacer")) {
        return std::make_unique<Spacer>();
    }
    if (tag_is(tag, "Divider")) {
        return std::make_unique<Divider>();
    }
    if (tag_is(tag, "Label") || tag_is(tag, "Text")) {
        return std::make_unique<Label>();
    }
    return nullptr;
}

void apply_common_attributes(Widget& widget, const XmlElement& element, std::string& inline_css, std::size_t& inline_counter)
{
    widget.set_type_name(std::string(element.name()));

    if (element.has_attribute("id")) {
        widget.set_name(std::string(element.attribute("id")));
    } else if (element.has_attribute("name")) {
        widget.set_name(std::string(element.attribute("name")));
    }

    if (element.has_attribute("class")) {
        for (auto class_name : split_words(element.attribute("class"))) {
            widget.add_class(std::move(class_name));
        }
    } else if (element.has_attribute("classes")) {
        for (auto class_name : split_words(element.attribute("classes"))) {
            widget.add_class(std::move(class_name));
        }
    }

    if (auto size = element.attribute_size("size")) {
        widget.set_size(*size);
    }
    if (element.has_attribute("width")) {
        widget.set_width(parse_length(element.attribute("width")));
    }
    if (element.has_attribute("height")) {
        widget.set_height(parse_length(element.attribute("height")));
    }
    if (auto flex = element.attribute_float("flex")) {
        widget.set_flex(*flex);
    } else if (auto weight = element.attribute_float("weight")) {
        widget.set_flex(*weight);
    }
    if (element.has_attribute("margin")) {
        widget.set_margin(parse_insets(element.attribute("margin")));
    }
    if (element.has_attribute("padding")) {
        widget.set_padding(parse_insets(element.attribute("padding")));
    }
    if (element.has_attribute("policy")) {
        const auto parts = split_list(element.attribute("policy"));
        widget.set_layout_policy(
            parse_policy(parts.empty() ? "fill" : parts[0]),
            parse_policy(parts.size() > 1 ? parts[1] : (parts.empty() ? "fill" : parts[0]))
        );
    }
    if (element.has_attribute("layout-policy")) {
        const auto parts = split_list(element.attribute("layout-policy"));
        widget.set_layout_policy(
            parse_policy(parts.empty() ? "fill" : parts[0]),
            parse_policy(parts.size() > 1 ? parts[1] : (parts.empty() ? "fill" : parts[0]))
        );
    }

    if (auto* layout = dynamic_cast<LinearLayout*>(&widget)) {
        if (element.has_attribute("alignment")) {
            layout->set_alignment(parse_align(element.attribute("alignment")));
        } else if (element.has_attribute("align")) {
            layout->set_alignment(parse_align(element.attribute("align")));
        }
        if (element.has_attribute("cross_alignment")) {
            layout->set_cross_alignment(parse_align(element.attribute("cross_alignment")));
        } else if (element.has_attribute("cross-alignment")) {
            layout->set_cross_alignment(parse_align(element.attribute("cross-alignment")));
        }
        if (auto gap = element.attribute_float("gap")) {
            layout->set_gap(*gap);
        }
    }

    if (auto* scroll = dynamic_cast<ScrollLayout*>(&widget)) {
        if (auto offset = element.attribute_float("scroll_offset")) {
            scroll->set_scroll_offset(*offset);
        } else if (auto offset = element.attribute_float("scroll-offset")) {
            scroll->set_scroll_offset(*offset);
        }
        if (auto step = element.attribute_float("scroll_step")) {
            scroll->set_scroll_step(*step);
        } else if (auto step = element.attribute_float("scroll-step")) {
            scroll->set_scroll_step(*step);
        }
        if (auto smooth = element.attribute_bool("smooth_scroll")) {
            scroll->set_smooth_scroll_enabled(*smooth);
        } else if (auto smooth = element.attribute_bool("smooth-scroll")) {
            scroll->set_smooth_scroll_enabled(*smooth);
        }
        if (auto smoothing = element.attribute_float("scroll_smoothing")) {
            scroll->set_scroll_smoothing(*smoothing);
        } else if (auto smoothing = element.attribute_float("scroll-smoothing")) {
            scroll->set_scroll_smoothing(*smoothing);
        }
    }

    if (auto* divider = dynamic_cast<Divider*>(&widget)) {
        if (element.has_attribute("orientation")) {
            divider->set_orientation(parse_orientation(element.attribute("orientation")));
        } else if (element.has_attribute("axis")) {
            divider->set_orientation(parse_orientation(element.attribute("axis")));
        }
        if (auto thickness = element.attribute_float("thickness")) {
            divider->set_thickness(*thickness);
        }
        if (auto color = element.attribute_color("color")) {
            divider->set_color(*color);
        }
    }

    if (auto* label = dynamic_cast<Label*>(&widget)) {
        if (element.has_attribute("text")) {
            label->set_text(std::string(element.attribute("text")));
        } else if (element.has_attribute("value")) {
            label->set_text(std::string(element.attribute("value")));
        }
        if (auto font_size = element.attribute_float("font-size")) {
            label->set_font_size(*font_size);
        } else if (auto font_size = element.attribute_float("font_size")) {
            label->set_font_size(*font_size);
        }
        if (auto color = element.attribute_color("text-color")) {
            label->set_text_color(*color);
        } else if (auto color = element.attribute_color("text_color")) {
            label->set_text_color(*color);
        }
    }

    if (auto visible = element.attribute_bool("visible")) {
        widget.set_visible(*visible);
    }
    if (auto enabled = element.attribute_bool("enabled")) {
        widget.set_enabled(*enabled);
    }
    if (auto clip = element.attribute_bool("clip_content")) {
        widget.set_clip_content(*clip);
    } else if (auto clip = element.attribute_bool("clip-content")) {
        widget.set_clip_content(*clip);
    }
    if (auto focusable = element.attribute_bool("focusable")) {
        widget.set_focusable(*focusable);
    }
    if (auto keyboard = element.attribute_bool("keyboard_activation")) {
        widget.set_keyboard_activation_enabled(*keyboard);
    } else if (auto keyboard = element.attribute_bool("keyboard-activation")) {
        widget.set_keyboard_activation_enabled(*keyboard);
    }

    if (element.has_attribute("role")) {
        widget.set_accessibility_role(parse_role(element.attribute("role")));
    }
    if (element.has_attribute("aria-label")) {
        widget.set_accessibility_label(std::string(element.attribute("aria-label")));
    } else if (element.has_attribute("accessibility-label")) {
        widget.set_accessibility_label(std::string(element.attribute("accessibility-label")));
    }
    if (element.has_attribute("aria-description")) {
        widget.set_accessibility_description(std::string(element.attribute("aria-description")));
    } else if (element.has_attribute("accessibility-description")) {
        widget.set_accessibility_description(std::string(element.attribute("accessibility-description")));
    }

    std::string inline_declarations;
    if (element.has_attribute("style")) {
        inline_declarations += element.attribute("style");
        if (!inline_declarations.empty() && inline_declarations.back() != ';') {
            inline_declarations += ";";
        }
    }
    if (element.has_attribute("transition")) {
        inline_declarations += " transition: ";
        inline_declarations += element.attribute("transition");
        inline_declarations += ";";
    }
    if (element.has_attribute("animation")) {
        inline_declarations += " animation: ";
        inline_declarations += element.attribute("animation");
        inline_declarations += ";";
    }
    if (element.has_attribute("transform")) {
        inline_declarations += " transform: ";
        inline_declarations += element.attribute("transform");
        inline_declarations += ";";
    }

    if (!inline_declarations.empty()) {
        const auto inline_class = "ouif-inline-x" + std::to_string(++inline_counter);
        widget.add_class(inline_class);
        inline_css += ".";
        inline_css += inline_class;
        inline_css += " { ";
        inline_css += inline_declarations;
        inline_css += " }\n";
    }
}

std::unique_ptr<Widget> build_widget(
    Application& app,
    const pugi::xml_node& node,
    std::string& inline_css,
    std::size_t& inline_counter
)
{
    auto element = element_from_node(node);
    auto widget = make_builtin_widget(element.name());
    if (!widget) {
        auto& factories = XmlLoaderAccess::factories(app);
        const auto found = factories.find(std::string(element.name()));
        if (found == factories.end()) {
            throw std::runtime_error("Unknown XML widget tag: " + std::string(element.name()));
        }
        widget = found->second(element);
        if (!widget) {
            throw std::runtime_error("XML widget factory returned null for tag: " + std::string(element.name()));
        }
    }

    apply_common_attributes(*widget, element, inline_css, inline_counter);
    if (auto* label = dynamic_cast<Label*>(widget.get()); label != nullptr && label->text().empty()) {
        const auto text = trim_copy(node.child_value());
        if (!text.empty()) {
            label->set_text(text);
        }
    }

    for (auto child : node.children()) {
        if (child.type() != pugi::node_element) {
            continue;
        }
        if (tag_is(child.name(), "Stylesheet") || tag_is(child.name(), "Style")) {
            continue;
        }
        widget->add_child(build_widget(app, child, inline_css, inline_counter));
    }

    return widget;
}

Widget& load_xml_document(Application& app, pugi::xml_document& document, const std::filesystem::path& base_path)
{
    auto window = document.child("Window");
    pugi::xml_node root_node;
    std::string stylesheet;
    std::string inline_css;
    std::size_t inline_counter = 0;

    if (window) {
        apply_window_config(XmlLoaderAccess::config(app), element_from_node(window));
        for (auto child : window.children()) {
            if (child.type() != pugi::node_element) {
                continue;
            }
            if (tag_is(child.name(), "Stylesheet")) {
                const auto element = element_from_node(child);
                const auto src = element.attribute("src", element.attribute("href"));
                if (!src.empty()) {
                    stylesheet += read_text_file(resolve_path(base_path, src));
                    stylesheet += "\n";
                }
                continue;
            }
            if (tag_is(child.name(), "Style")) {
                stylesheet += child.child_value();
                stylesheet += "\n";
                continue;
            }
            if (!root_node) {
                root_node = child;
            }
        }
    } else {
        root_node = document.first_child();
    }

    if (!root_node) {
        throw std::runtime_error("XML UI does not contain a root widget");
    }

    auto root = build_widget(app, root_node, inline_css, inline_counter);
    if (!inline_css.empty()) {
        stylesheet += inline_css;
    }
    if (!stylesheet.empty()) {
        root->set_stylesheet(std::move(stylesheet));
    }
    return app.set_root(std::move(root));
}
#endif

} // namespace

XmlElement::XmlElement(std::string name, std::vector<XmlAttribute> attributes)
    : name_(std::move(name))
    , attributes_(std::move(attributes))
{
}

std::string_view XmlElement::name() const noexcept
{
    return name_;
}

bool XmlElement::has_attribute(std::string_view name) const noexcept
{
    return std::any_of(attributes_.begin(), attributes_.end(), [name](const auto& attribute) {
        return lower_copy(attribute.name) == lower_copy(name);
    });
}

std::string_view XmlElement::attribute(std::string_view name, std::string_view fallback) const noexcept
{
    const auto found = std::find_if(attributes_.begin(), attributes_.end(), [name](const auto& attribute) {
        return lower_copy(attribute.name) == lower_copy(name);
    });
    return found != attributes_.end() ? std::string_view(found->value) : fallback;
}

std::optional<float> XmlElement::attribute_float(std::string_view name) const noexcept
{
    return parse_float(attribute(name));
}

std::optional<bool> XmlElement::attribute_bool(std::string_view name) const noexcept
{
    const auto value = lower_copy(trim_copy(attribute(name)));
    if (value == "true" || value == "1" || value == "yes" || value == "on") {
        return true;
    }
    if (value == "false" || value == "0" || value == "no" || value == "off") {
        return false;
    }
    return std::nullopt;
}

std::optional<Color> XmlElement::attribute_color(std::string_view name) const noexcept
{
    return Color::from_hex(attribute(name));
}

std::optional<Size> XmlElement::attribute_size(std::string_view name) const noexcept
{
    if (!has_attribute(name)) {
        return std::nullopt;
    }
    return parse_size(attribute(name));
}

const std::vector<XmlAttribute>& XmlElement::attributes() const noexcept
{
    return attributes_;
}

Application& Application::register_xml_widget(std::string tag_name, XmlWidgetFactory factory)
{
    if (!factory) {
        throw std::invalid_argument("Cannot register an empty XML widget factory");
    }
    xml_factories_[std::move(tag_name)] = std::move(factory);
    return *this;
}

Widget& Application::load_xml(std::string_view path)
{
#if OUIF_WITH_PUGIXML
    pugi::xml_document document;
    const std::filesystem::path file_path { std::string(path) };
    const auto result = document.load_file(file_path.string().c_str());
    if (!result) {
        throw std::runtime_error("Failed to parse XML UI file: " + file_path.string());
    }
    return load_xml_document(*this, document, file_path.parent_path());
#else
    (void)path;
    throw std::runtime_error("OUIF was built without pugixml support");
#endif
}

Widget& Application::load_xml_string(std::string_view xml, std::string_view base_path)
{
#if OUIF_WITH_PUGIXML
    pugi::xml_document document;
    const auto result = document.load_buffer(xml.data(), xml.size());
    if (!result) {
        throw std::runtime_error("Failed to parse XML UI string");
    }
    return load_xml_document(*this, document, std::filesystem::path(std::string(base_path)));
#else
    (void)xml;
    (void)base_path;
    throw std::runtime_error("OUIF was built without pugixml support");
#endif
}

void Application::load_stylesheet_file(std::string_view path)
{
    if (root_ == nullptr) {
        throw std::runtime_error("Cannot load a stylesheet file before an application root exists");
    }
    root_->set_stylesheet(read_text_file(std::filesystem::path(std::string(path))));
}

void Application::join_stylesheet_file(std::string_view path)
{
    if (root_ == nullptr) {
        throw std::runtime_error("Cannot join a stylesheet file before an application root exists");
    }
    root_->join_stylesheet(read_text_file(std::filesystem::path(std::string(path))));
}

} // namespace ouif
