#include <OUIF/OUIF.h>

namespace {

enum class BlurSurface {
    Backdrop,
    Layer,
};

class BlurCard : public ouif::ColLayout {
public:
    BlurCard(const char* title, const char* subtitle, BlurSurface surface, ouif::BlurType type)
        : title_(title)
        , subtitle_(subtitle)
        , surface_(surface)
        , type_(type)
    {
        add_class("blur-card");
        set_gap(10.0f);
        set_padding(22.0f);
        set_size({ 330.0f, 190.0f });
        set_style(ouif::Style()
            .with_background(ouif::Color::hexa(0x17202bcc))
            .with_border(ouif::Color::hexa(0xf3f7ff80), 1.0f)
            .with_radius(26.0f));

        const std::vector<float> blur_args {
            24.0f,
            type == ouif::BlurType::DualKawase ? 1.0f : 0.0f,
        };
        if (surface == BlurSurface::Backdrop) {
            add_backdrop_effect("blur", blur_args);
        } else {
            add_layer_effect("blur", blur_args);
        }

        title_.set_layout_policy(ouif::SizePolicy::Fill, ouif::SizePolicy::Fixed);
        title_.set_size({ 0.0f, 42.0f });
        title_.set_text_align(ouif::TextAlign::Center);
        title_.set_font_size(25.0f);
        title_.set_text_color(ouif::Color::hex(0xf8fbff));
        title_.set_style(ouif::Style().with_background(ouif::Color::rgba(255, 255, 255, 0)));

        subtitle_.set_layout_policy(ouif::SizePolicy::Fill, ouif::SizePolicy::Fill);
        subtitle_.set_text_align(ouif::TextAlign::Center);
        subtitle_.set_text_overflow(ouif::TextOverflow::Wrap);
        subtitle_.set_font_size(16.0f);
        subtitle_.set_text_color(ouif::Color::hexa(0xdce7f4ee));
        subtitle_.set_style(ouif::Style().with_background(ouif::Color::rgba(255, 255, 255, 0)));

        children(title_, subtitle_);
    }

    void set_card_bounds(ouif::Rect bounds)
    {
        set_bounds(bounds);
    }

protected:
    void draw(ouif::Renderer& renderer) override
    {
        ouif::ColLayout::draw(renderer);

        const auto pill = ouif::Rect {
            bounds().x + 18.0f,
            bounds().y + bounds().height - 40.0f,
            bounds().width - 36.0f,
            22.0f,
        };
        const auto color = surface_ == BlurSurface::Backdrop
            ? ouif::Color::hexa(0x7dd3f766)
            : ouif::Color::hexa(0xc084fc66);
        renderer.fill_rounded_rect(pill, 11.0f, color);
    }

private:
    ouif::Label title_;
    ouif::Label subtitle_;
    BlurSurface surface_ = BlurSurface::Backdrop;
    ouif::BlurType type_ = ouif::BlurType::Gaussian;
};

class BlurDemo : public ouif::Widget {
public:
    BlurDemo()
        : background_(OUIF_EXAMPLE_CAT_PATH)
        , gaussian_backdrop_("Gaussian Backdrop", "Blurs only the cat image behind this card. Text remains sharp.", BlurSurface::Backdrop, ouif::BlurType::Gaussian)
        , gaussian_layer_("Gaussian Layer", "Blurs this card and its children as one captured layer.", BlurSurface::Layer, ouif::BlurType::Gaussian)
        , kawase_backdrop_("Kawase Backdrop", "Uses Dual Kawase on the background under this card.", BlurSurface::Backdrop, ouif::BlurType::DualKawase)
        , kawase_layer_("Kawase Layer", "Uses Dual Kawase on the card layer itself.", BlurSurface::Layer, ouif::BlurType::DualKawase)
    {
        set_layout_policy(ouif::SizePolicy::Fill, ouif::SizePolicy::Fill);
        set_style(ouif::Style().with_background(ouif::Color::hex(0x101218)));

        background_.set_fit(ouif::ImageFit::Cover);
        background_.set_filter(ouif::ImageFilter::Linear);
        background_.set_tint(ouif::Color::rgba(255, 255, 255, 235));
        background_.set_layout_policy(ouif::SizePolicy::Fill, ouif::SizePolicy::Fill);

        children(background_, gaussian_backdrop_, gaussian_layer_, kawase_backdrop_, kawase_layer_);
    }

protected:
    void on_layout(ouif::Rect content) override
    {
        background_.set_bounds(bounds());

        const float card_w = 330.0f;
        const float card_h = 190.0f;
        const float gap = 34.0f;
        const float total_w = card_w * 2.0f + gap;
        const float total_h = card_h * 2.0f + gap;
        const float x = content.x + (content.width - total_w) * 0.5f;
        const float y = content.y + (content.height - total_h) * 0.5f;

        gaussian_backdrop_.set_card_bounds({ x, y, card_w, card_h });
        gaussian_layer_.set_card_bounds({ x + card_w + gap, y, card_w, card_h });
        kawase_backdrop_.set_card_bounds({ x, y + card_h + gap, card_w, card_h });
        kawase_layer_.set_card_bounds({ x + card_w + gap, y + card_h + gap, card_w, card_h });
    }

private:
    ouif::Image background_;
    BlurCard gaussian_backdrop_;
    BlurCard gaussian_layer_;
    BlurCard kawase_backdrop_;
    BlurCard kawase_layer_;
};

} // namespace

int main()
{
    ouif::Application app(ouif::ApplicationConfig()
        .with_title("OUIF Blur Effect")
        .with_size(1120, 720)
        .with_clear_color(ouif::Color::hex(0x101218))
        .with_render_quality(ouif::RendererQuality::Ultra));

    BlurDemo demo;
    app.set_root(demo);
    return app.run();
}
