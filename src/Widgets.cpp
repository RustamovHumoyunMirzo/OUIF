#include <OUIF/Widgets.h>

#include <OUIF/Renderer.h>
#include <OUIF/Resources.h>

#include <algorithm>
#include <utility>

namespace ouif {

Spacer::Spacer() noexcept
{
    set_accepts_children(false);
}

Spacer::Spacer(float flex) noexcept
{
    set_accepts_children(false);
    set_flex(flex);
}

Spacer::Spacer(Size size) noexcept
{
    set_accepts_children(false);
    set_size(size);
}

bool Spacer::event(const Event& event)
{
    (void)event;
    return false;
}

void Spacer::draw(Renderer& renderer)
{
    (void)renderer;
}

Divider::Divider(Orientation orientation, float thickness)
    : orientation_(orientation)
    , thickness_(std::max(0.0f, thickness))
{
    set_accepts_children(false);
    apply_axis_size();
}

void Divider::set_orientation(Orientation orientation) noexcept
{
    orientation_ = orientation;
    apply_axis_size();
}

Orientation Divider::orientation() const noexcept
{
    return orientation_;
}

void Divider::set_thickness(float thickness) noexcept
{
    thickness_ = std::max(0.0f, thickness);
    apply_axis_size();
}

float Divider::thickness() const noexcept
{
    return thickness_;
}

void Divider::set_color(Color color) noexcept
{
    set_background(color);
}

Color Divider::color() const noexcept
{
    return get_background();
}

bool Divider::event(const Event& event)
{
    (void)event;
    return false;
}

void Divider::draw(Renderer& renderer)
{
    renderer.fill_rect(bounds(), color());
}

void Divider::apply_axis_size() noexcept
{
    if (orientation_ == Orientation::Horizontal) {
        set_layout_policy(SizePolicy::Fill, SizePolicy::Fixed);
        set_size({ 0.0f, thickness_ });
    } else {
        set_layout_policy(SizePolicy::Fixed, SizePolicy::Fill);
        set_size({ thickness_, 0.0f });
    }
}

Label::Label()
{
    set_accepts_children(false);
    set_size({ 160.0f, 32.0f });
}

Label::Label(std::string text)
    : Label()
{
    set_text(std::move(text));
}

void Label::set_text(std::string text)
{
    text_ = std::move(text);
}

std::string_view Label::text() const noexcept
{
    return text_;
}

std::string_view Label::get_text() const noexcept
{
    return text();
}

void Label::set_text_style(TextStyle style) noexcept
{
    text_style_ = std::move(style);
    has_text_color_ = true;
}

void Label::set_text_style(InheritTag) noexcept
{
    if (auto* parent_label = dynamic_cast<const Label*>(parent())) {
        set_text_style(parent_label->get_text_style());
    } else if (parent() != nullptr) {
        text_style_.color = parent()->get_foreground();
        has_text_color_ = true;
    }
}

const TextStyle& Label::text_style() const noexcept
{
    return text_style_;
}

const TextStyle& Label::get_text_style() const noexcept
{
    return text_style();
}

void Label::set_font_family(std::string family)
{
    text_style_.font_family = std::move(family);
}

void Label::set_font_family(InheritTag)
{
    if (auto* parent_label = dynamic_cast<const Label*>(parent())) {
        set_font_family(std::string(parent_label->font_family()));
    }
}

std::string_view Label::font_family() const noexcept
{
    return text_style_.font_family;
}

void Label::set_font_size(float size) noexcept
{
    text_style_.font_size = std::max(1.0f, size);
}

void Label::set_font_size(InheritTag) noexcept
{
    if (auto* parent_label = dynamic_cast<const Label*>(parent())) {
        set_font_size(parent_label->font_size());
    }
}

float Label::font_size() const noexcept
{
    return text_style_.font_size;
}

void Label::set_text_color(Color color) noexcept
{
    text_style_.color = color;
    text_style_.color_gradient.reset();
    has_text_color_ = true;
}

void Label::set_text_color(Gradient gradient)
{
    text_style_.color_gradient = std::move(gradient);
    has_text_color_ = true;
}

void Label::set_text_color(InheritTag) noexcept
{
    if (auto* parent_label = dynamic_cast<const Label*>(parent())) {
        set_text_color(parent_label->text_color());
        if (parent_label->text_gradient()) {
            text_style_.color_gradient = parent_label->text_gradient();
        }
    } else if (parent() != nullptr) {
        set_text_color(parent()->get_foreground());
    }
}

Color Label::text_color() const noexcept
{
    return has_text_color_ ? text_style_.color : get_style().foreground;
}

const std::optional<Gradient>& Label::text_gradient() const noexcept
{
    return text_style_.color_gradient;
}

const std::optional<Gradient>& Label::get_text_gradient() const noexcept
{
    return text_gradient();
}

void Label::set_text_align(TextAlign align) noexcept
{
    text_style_.align = align;
}

void Label::set_text_align(InheritTag) noexcept
{
    if (auto* parent_label = dynamic_cast<const Label*>(parent())) {
        set_text_align(parent_label->text_align());
    }
}

TextAlign Label::text_align() const noexcept
{
    return text_style_.align;
}

void Label::set_text_overflow(TextOverflow overflow) noexcept
{
    text_style_.overflow = overflow;
}

void Label::set_text_overflow(InheritTag) noexcept
{
    if (auto* parent_label = dynamic_cast<const Label*>(parent())) {
        set_text_overflow(parent_label->text_overflow());
    }
}

TextOverflow Label::text_overflow() const noexcept
{
    return text_style_.overflow;
}

bool Label::event(const Event& event)
{
    return Widget::event(event);
}

void Label::draw(Renderer& renderer)
{
    Widget::draw(renderer);
    auto style = text_style_;
    style.color = text_color();
    if (get_style().foreground_gradient) {
        style.color_gradient = get_style().foreground_gradient;
    }
    renderer.draw_text(text_, bounds().inset(layout_rules().padding), style);
}

namespace {

std::string utf8_from_codepoint(std::uint32_t codepoint)
{
    std::string text;
    if (codepoint <= 0x7fU) {
        text.push_back(static_cast<char>(codepoint));
    } else if (codepoint <= 0x7ffU) {
        text.push_back(static_cast<char>(0xc0U | ((codepoint >> 6U) & 0x1fU)));
        text.push_back(static_cast<char>(0x80U | (codepoint & 0x3fU)));
    } else if (codepoint <= 0xffffU) {
        text.push_back(static_cast<char>(0xe0U | ((codepoint >> 12U) & 0x0fU)));
        text.push_back(static_cast<char>(0x80U | ((codepoint >> 6U) & 0x3fU)));
        text.push_back(static_cast<char>(0x80U | (codepoint & 0x3fU)));
    } else if (codepoint <= 0x10ffffU) {
        text.push_back(static_cast<char>(0xf0U | ((codepoint >> 18U) & 0x07U)));
        text.push_back(static_cast<char>(0x80U | ((codepoint >> 12U) & 0x3fU)));
        text.push_back(static_cast<char>(0x80U | ((codepoint >> 6U) & 0x3fU)));
        text.push_back(static_cast<char>(0x80U | (codepoint & 0x3fU)));
    }
    return text;
}

std::uint32_t printable_codepoint_from_key(const KeyEvent& event) noexcept
{
    if (event.control || event.alt || event.super) {
        return 0;
    }

    const auto key = event.key;
    if (key >= static_cast<std::uint32_t>('A') && key <= static_cast<std::uint32_t>('Z')) {
        const auto base = event.shift ? 'A' : 'a';
        return static_cast<std::uint32_t>(base + static_cast<char>(key - static_cast<std::uint32_t>('A')));
    }
    if (key >= static_cast<std::uint32_t>(Key::Num0) && key <= static_cast<std::uint32_t>(Key::Num9)) {
        static constexpr char shifted[] = { ')', '!', '@', '#', '$', '%', '^', '&', '*', '(' };
        const auto index = key - static_cast<std::uint32_t>(Key::Num0);
        return static_cast<std::uint32_t>(event.shift ? shifted[index] : static_cast<char>('0' + index));
    }

    switch (key) {
    case static_cast<std::uint32_t>(Key::Space):
        return ' ';
    case static_cast<std::uint32_t>(Key::Apostrophe):
        return event.shift ? '"' : '\'';
    case static_cast<std::uint32_t>(Key::Comma):
        return event.shift ? '<' : ',';
    case static_cast<std::uint32_t>(Key::Minus):
        return event.shift ? '_' : '-';
    case static_cast<std::uint32_t>(Key::Period):
        return event.shift ? '>' : '.';
    case static_cast<std::uint32_t>(Key::Slash):
        return event.shift ? '?' : '/';
    case static_cast<std::uint32_t>(Key::Semicolon):
        return event.shift ? ':' : ';';
    case static_cast<std::uint32_t>(Key::Equal):
        return event.shift ? '+' : '=';
    default:
        return 0;
    }
}

std::string& input_clipboard_storage()
{
    static std::string text;
    return text;
}

} // namespace

Input::Input()
{
    set_accepts_children(false);
    set_focusable(true);
    set_accessibility_role(AccessibilityRole::TextInput);
    set_size({ 220.0f, 40.0f });
    set_padding(Insets(10.0f, 8.0f, 10.0f, 8.0f));
    set_style(Style()
        .with_background(Color::hex(0x151b24))
        .with_background_hovered(Color::hex(0x192230))
        .with_background_focused(Color::hex(0x1d2735))
        .with_border(Color::hexa(0x56657a99), 1.0f)
        .with_border_focused(Color::hex(0x8dc7ff), 2.0f)
        .with_radius(8.0f));
}

Input::Input(std::string text)
    : Input()
{
    set_text(std::move(text));
}

void Input::set_text(std::string text)
{
    text_ = std::move(text);
    caret_ = std::min(caret_, text_.size());
    selection_anchor_ = caret_;
}

std::string_view Input::text() const noexcept
{
    return text_;
}

std::string_view Input::get_text() const noexcept
{
    return text();
}

void Input::clear_text() noexcept
{
    text_.clear();
    caret_ = 0;
    selection_anchor_ = 0;
}

void Input::set_placeholder(std::string placeholder)
{
    placeholder_ = std::move(placeholder);
}

std::string_view Input::placeholder() const noexcept
{
    return placeholder_;
}

std::string_view Input::get_placeholder() const noexcept
{
    return placeholder();
}

void Input::set_composition_text(std::string text)
{
    composition_text_ = std::move(text);
}

std::string_view Input::composition_text() const noexcept
{
    return composition_text_;
}

std::string_view Input::get_composition_text() const noexcept
{
    return composition_text();
}

void Input::clear_composition() noexcept
{
    composition_text_.clear();
}

void Input::set_text_style(TextStyle style) noexcept
{
    text_style_ = std::move(style);
}

void Input::set_text_style(InheritTag) noexcept
{
    if (auto* parent_input = dynamic_cast<const Input*>(parent())) {
        set_text_style(parent_input->text_style());
    } else if (parent() != nullptr) {
        text_style_.color = parent()->get_foreground();
    }
}

const TextStyle& Input::text_style() const noexcept
{
    return text_style_;
}

const TextStyle& Input::get_text_style() const noexcept
{
    return text_style();
}

void Input::set_font_family(std::string family)
{
    text_style_.font_family = std::move(family);
}

std::string_view Input::font_family() const noexcept
{
    return text_style_.font_family;
}

void Input::set_font_size(float size) noexcept
{
    text_style_.font_size = std::max(1.0f, size);
}

float Input::font_size() const noexcept
{
    return text_style_.font_size;
}

void Input::set_text_color(Color color) noexcept
{
    text_style_.color = color;
    text_style_.color_gradient.reset();
}

void Input::set_text_color(Gradient gradient)
{
    text_style_.color_gradient = std::move(gradient);
}

void Input::set_placeholder_color(Color color) noexcept
{
    placeholder_color_ = color;
}

Color Input::text_color() const noexcept
{
    return text_style_.color;
}

const std::optional<Gradient>& Input::text_gradient() const noexcept
{
    return text_style_.color_gradient;
}

const std::optional<Gradient>& Input::get_text_gradient() const noexcept
{
    return text_gradient();
}

Color Input::placeholder_color() const noexcept
{
    return placeholder_color_;
}

void Input::set_caret(std::size_t index) noexcept
{
    caret_ = std::min(index, text_.size());
    selection_anchor_ = caret_;
}

std::size_t Input::caret() const noexcept
{
    return caret_;
}

std::size_t Input::get_caret() const noexcept
{
    return caret();
}

void Input::select(std::size_t anchor, std::size_t caret) noexcept
{
    selection_anchor_ = std::min(anchor, text_.size());
    caret_ = std::min(caret, text_.size());
}

void Input::select_all() noexcept
{
    selection_anchor_ = 0;
    caret_ = text_.size();
}

void Input::clear_selection() noexcept
{
    selection_anchor_ = caret_;
}

bool Input::has_selection() const noexcept
{
    return selection_anchor_ != caret_;
}

std::pair<std::size_t, std::size_t> Input::selection() const noexcept
{
    return ordered_selection();
}

void Input::insert_text(std::string_view text)
{
    erase_selection();
    text_.insert(caret_, text);
    caret_ += text.size();
    selection_anchor_ = caret_;
}

void Input::erase_selection()
{
    const auto [first, last] = ordered_selection();
    if (first == last) {
        return;
    }
    text_.erase(first, last - first);
    caret_ = first;
    selection_anchor_ = caret_;
}

void Input::erase_previous()
{
    if (has_selection()) {
        erase_selection();
        return;
    }
    if (caret_ == 0) {
        return;
    }
    text_.erase(caret_ - 1U, 1U);
    --caret_;
    selection_anchor_ = caret_;
}

void Input::erase_next()
{
    if (has_selection()) {
        erase_selection();
        return;
    }
    if (caret_ >= text_.size()) {
        return;
    }
    text_.erase(caret_, 1U);
    selection_anchor_ = caret_;
}

void Input::copy_selection()
{
    const auto [first, last] = ordered_selection();
    input_clipboard_storage() = text_.substr(first, last - first);
}

void Input::cut_selection()
{
    if (!has_selection()) {
        return;
    }
    copy_selection();
    erase_selection();
}

void Input::paste_text(std::string_view text)
{
    insert_text(text);
}

void Input::paste_clipboard()
{
    paste_text(input_clipboard_storage());
}

void Input::set_clipboard_text(std::string text)
{
    input_clipboard_storage() = std::move(text);
}

std::string_view Input::clipboard_text() noexcept
{
    return input_clipboard_storage();
}

bool Input::event(const Event& event)
{
    return Widget::event(event);
}

void Input::draw(Renderer& renderer)
{
    Widget::draw(renderer);

    const Rect content = bounds().inset(layout_rules().padding);
    auto style = text_style_;
    style.color = text_.empty() && !focused() ? placeholder_color_ : text_style_.color;
    if (get_style().foreground_gradient && (!text_.empty() || !composition_text_.empty())) {
        style.color_gradient = get_style().foreground_gradient;
    }
    std::string display_text;
    std::string_view text_to_draw = text_.empty() && !focused() ? std::string_view(placeholder_) : std::string_view(text_);
    if (!composition_text_.empty()) {
        display_text.reserve(text_.size() + composition_text_.size());
        display_text.append(text_);
        display_text.append(composition_text_);
        text_to_draw = display_text;
    }
    renderer.draw_text(text_to_draw, content, style);

    if (focused()) {
        const float advance = std::max(1.0f, style.font_size * 0.6f + style.letter_spacing);
        const float caret_x = std::min(content.x + advance * static_cast<float>(caret_), content.x + content.width - 1.0f);
        renderer.fill_rect({ caret_x, content.y + 4.0f, 1.5f, std::max(1.0f, content.height - 8.0f) }, get_style().foreground);
    }
}

bool Input::on_event(const Event& event)
{
    if (const auto* text = std::get_if<TextInputEvent>(&event)) {
        if (suppress_next_codepoint_ == text->codepoint) {
            suppress_next_codepoint_ = 0;
            return true;
        }
        suppress_next_codepoint_ = 0;
        if (text->codepoint >= 32U) {
            insert_text(utf8_from_codepoint(text->codepoint));
            return true;
        }
    }
    return false;
}

bool Input::on_key_down(const KeyEvent& event)
{
    if (event.action != KeyAction::Press && event.action != KeyAction::Repeat) {
        return false;
    }
    if (event.control && event.key == static_cast<std::uint32_t>(Key::A)) {
        select_all();
        return true;
    }
    if (event.control && event.key == static_cast<std::uint32_t>(Key::C)) {
        copy_selection();
        return true;
    }
    if (event.control && event.key == static_cast<std::uint32_t>(Key::X)) {
        cut_selection();
        return true;
    }
    if (event.control && event.key == static_cast<std::uint32_t>(Key::V)) {
        paste_clipboard();
        return true;
    }
    if (event.key == static_cast<std::uint32_t>(Key::Backspace)) {
        erase_previous();
        return true;
    }
    if (event.key == static_cast<std::uint32_t>(Key::Delete)) {
        erase_next();
        return true;
    }
    if (event.key == static_cast<std::uint32_t>(Key::Left)) {
        if (caret_ > 0) {
            --caret_;
        }
        if (!event.shift) {
            selection_anchor_ = caret_;
        }
        return true;
    }
    if (event.key == static_cast<std::uint32_t>(Key::Right)) {
        if (caret_ < text_.size()) {
            ++caret_;
        }
        if (!event.shift) {
            selection_anchor_ = caret_;
        }
        return true;
    }
    if (event.key == static_cast<std::uint32_t>(Key::Home)) {
        caret_ = 0;
        if (!event.shift) {
            selection_anchor_ = caret_;
        }
        return true;
    }
    if (event.key == static_cast<std::uint32_t>(Key::End)) {
        caret_ = text_.size();
        if (!event.shift) {
            selection_anchor_ = caret_;
        }
        return true;
    }
    if (const auto codepoint = printable_codepoint_from_key(event); codepoint >= 32U) {
        insert_text(utf8_from_codepoint(codepoint));
        suppress_next_codepoint_ = codepoint;
        return true;
    }
    return false;
}

bool Input::on_mouse_down(const MouseEvent& event)
{
    set_caret(caret_from_point(event.position));
    return true;
}

std::size_t Input::caret_from_point(Point point) const noexcept
{
    const Rect content = bounds().inset(layout_rules().padding);
    const float advance = std::max(1.0f, text_style_.font_size * 0.6f + text_style_.letter_spacing);
    if (point.x <= content.x) {
        return 0;
    }
    return std::min<std::size_t>(text_.size(), static_cast<std::size_t>((point.x - content.x) / advance + 0.5f));
}

std::pair<std::size_t, std::size_t> Input::ordered_selection() const noexcept
{
    return { std::min(selection_anchor_, caret_), std::max(selection_anchor_, caret_) };
}

Image::Image()
{
    set_accepts_children(false);
    set_size({ 160.0f, 120.0f });
}

Image::Image(std::filesystem::path source)
    : Image()
{
    set_source(std::move(source));
}

void Image::set_source(std::filesystem::path source)
{
    source_ = std::move(source);
    resource_id_.reset();
    image_dirty_ = true;
}

void Image::set_source(std::string source)
{
    set_source(std::filesystem::path(std::move(source)));
}

const std::filesystem::path& Image::source() const noexcept
{
    return source_;
}

const std::filesystem::path& Image::get_source() const noexcept
{
    return source();
}

void Image::set_resource(int id) noexcept
{
    resource_id_ = id;
    source_.clear();
    image_dirty_ = true;
}

void Image::clear_resource() noexcept
{
    resource_id_.reset();
    image_dirty_ = true;
}

std::optional<int> Image::resource() const noexcept
{
    return resource_id_;
}

std::optional<int> Image::get_resource() const noexcept
{
    return resource();
}

void Image::set_fit(ImageFit fit) noexcept
{
    fit_ = fit;
}

void Image::set_fit(InheritTag) noexcept
{
    if (auto* parent_image = dynamic_cast<const Image*>(parent())) {
        set_fit(parent_image->fit());
    }
}

ImageFit Image::fit() const noexcept
{
    return fit_;
}

ImageFit Image::get_fit() const noexcept
{
    return fit();
}

void Image::set_filter(ImageFilter filter) noexcept
{
    filter_ = filter;
}

void Image::set_filter(InheritTag) noexcept
{
    if (auto* parent_image = dynamic_cast<const Image*>(parent())) {
        set_filter(parent_image->filter());
    }
}

ImageFilter Image::filter() const noexcept
{
    return filter_;
}

ImageFilter Image::get_filter() const noexcept
{
    return filter();
}

void Image::set_tint(Color tint) noexcept
{
    tint_ = tint;
}

void Image::set_tint(InheritTag) noexcept
{
    if (auto* parent_image = dynamic_cast<const Image*>(parent())) {
        set_tint(parent_image->tint());
    } else if (parent() != nullptr) {
        set_tint(parent()->get_foreground());
    }
}

Color Image::tint() const noexcept
{
    return tint_;
}

Color Image::get_tint() const noexcept
{
    return tint();
}

bool Image::loaded() const noexcept
{
    return image_.valid();
}

Size Image::natural_size() const noexcept
{
    return natural_size_;
}

void Image::reload(Renderer& renderer)
{
    unload(renderer);
    if (resource_id_.has_value()) {
        if (const auto resource = Resources::load(*resource_id_)) {
            image_ = renderer.load_image(resource->data, resource->size);
        }
    } else if (!source_.empty()) {
        image_ = renderer.load_image(source_);
    }
    natural_size_ = renderer.image_size(image_);
    image_dirty_ = false;
}

void Image::unload(Renderer& renderer) noexcept
{
    if (image_.valid()) {
        renderer.destroy_image(image_);
    }
    reset_loaded_state();
    image_dirty_ = false;
}

bool Image::event(const Event& event)
{
    return Widget::event(event);
}

void Image::draw(Renderer& renderer)
{
    Widget::draw(renderer);
    if (image_dirty_) {
        reload(renderer);
    }
    if (image_.valid()) {
        renderer.draw_image(image_, bounds().inset(layout_rules().padding), fit_, filter_, tint_);
    }
}

void Image::reset_loaded_state() noexcept
{
    image_ = {};
    natural_size_ = {};
}

VectorImage::VectorImage()
{
    set_accepts_children(false);
    set_size({ 160.0f, 120.0f });
}

VectorImage::VectorImage(std::filesystem::path source)
    : VectorImage()
{
    set_source(std::move(source));
}

void VectorImage::set_source(std::filesystem::path source)
{
    source_ = std::move(source);
    inline_svg_.clear();
    resource_id_.reset();
    image_dirty_ = true;
}

void VectorImage::set_source(std::string source)
{
    set_source(std::filesystem::path(std::move(source)));
}

const std::filesystem::path& VectorImage::source() const noexcept
{
    return source_;
}

const std::filesystem::path& VectorImage::get_source() const noexcept
{
    return source();
}

void VectorImage::set_svg(std::string svg)
{
    inline_svg_ = std::move(svg);
    source_.clear();
    resource_id_.reset();
    image_dirty_ = true;
}

std::string_view VectorImage::svg() const noexcept
{
    return inline_svg_;
}

std::string_view VectorImage::get_svg() const noexcept
{
    return svg();
}

void VectorImage::set_resource(int id) noexcept
{
    resource_id_ = id;
    source_.clear();
    inline_svg_.clear();
    image_dirty_ = true;
}

void VectorImage::clear_resource() noexcept
{
    resource_id_.reset();
    image_dirty_ = true;
}

std::optional<int> VectorImage::resource() const noexcept
{
    return resource_id_;
}

std::optional<int> VectorImage::get_resource() const noexcept
{
    return resource();
}

void VectorImage::set_fit(ImageFit fit) noexcept
{
    fit_ = fit;
}

void VectorImage::set_fit(InheritTag) noexcept
{
    if (auto* parent_image = dynamic_cast<const VectorImage*>(parent())) {
        set_fit(parent_image->fit());
    }
}

ImageFit VectorImage::fit() const noexcept
{
    return fit_;
}

ImageFit VectorImage::get_fit() const noexcept
{
    return fit();
}

void VectorImage::set_tint(Color tint) noexcept
{
    tint_ = tint;
}

void VectorImage::set_tint(InheritTag) noexcept
{
    if (auto* parent_image = dynamic_cast<const VectorImage*>(parent())) {
        set_tint(parent_image->tint());
    } else if (parent() != nullptr) {
        set_tint(parent()->get_foreground());
    }
}

Color VectorImage::tint() const noexcept
{
    return tint_;
}

Color VectorImage::get_tint() const noexcept
{
    return tint();
}

bool VectorImage::loaded() const noexcept
{
    return image_.valid();
}

Size VectorImage::natural_size() const noexcept
{
    return natural_size_;
}

void VectorImage::reload(Renderer& renderer)
{
    unload(renderer);
    if (resource_id_.has_value()) {
        if (const auto resource = Resources::load(*resource_id_)) {
            image_ = renderer.load_vector_image(resource->data, resource->size);
        }
    } else if (!inline_svg_.empty()) {
        image_ = renderer.load_vector_image(inline_svg_);
    } else if (!source_.empty()) {
        image_ = renderer.load_vector_image(source_);
    }
    natural_size_ = renderer.vector_image_size(image_);
    image_dirty_ = false;
}

void VectorImage::unload(Renderer& renderer) noexcept
{
    if (image_.valid()) {
        renderer.destroy_vector_image(image_);
    }
    reset_loaded_state();
    image_dirty_ = false;
}

bool VectorImage::event(const Event& event)
{
    return Widget::event(event);
}

void VectorImage::draw(Renderer& renderer)
{
    Widget::draw(renderer);
    if (image_dirty_) {
        reload(renderer);
    }
    if (image_.valid()) {
        renderer.draw_vector_image(image_, bounds().inset(layout_rules().padding), fit_, tint_);
    }
}

void VectorImage::reset_loaded_state() noexcept
{
    image_ = {};
    natural_size_ = {};
}

Overlay::Overlay() noexcept
{
    set_overlay(true);
    set_layout_policy(SizePolicy::Fill, SizePolicy::Fill);
}

void Overlay::on_layout(Rect content)
{
    for (auto* child : mutable_children()) {
        if (child != nullptr && child->visible()) {
            if (child->bounds().width == 0.0f && child->bounds().height == 0.0f) {
                child->set_bounds(content);
            }
            const auto bounds = child->bounds();
            child->layout({ bounds.width, bounds.height });
        }
    }
}

} // namespace ouif
