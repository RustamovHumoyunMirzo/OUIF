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
    has_text_color_ = true;
}

void Label::set_text_color(InheritTag) noexcept
{
    if (auto* parent_label = dynamic_cast<const Label*>(parent())) {
        set_text_color(parent_label->text_color());
    } else if (parent() != nullptr) {
        set_text_color(parent()->get_foreground());
    }
}

Color Label::text_color() const noexcept
{
    return has_text_color_ ? text_style_.color : get_style().foreground;
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
    renderer.draw_text(text_, bounds().inset(layout_rules().padding), style);
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
