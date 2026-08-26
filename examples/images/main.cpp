#include <OUIF/OUIF.h>

namespace {

class ImageCard : public ouif::ColLayout {
public:
    ImageCard(const char* caption, ouif::ImageFit fit, ouif::ImageFilter filter, ouif::Color tint)
        : image_()
        , label_(caption)
    {
        set_gap(12.0f);
        set_padding(14.0f);
        set_size({ 220.0f, 260.0f });
        set_style(ouif::Style()
                .with_background(ouif::Color::hex(0x1d2531))
                .with_border(ouif::Color::hex(0x415168), 1.0f)
                .with_radius(14.0f));

        image_.add_class("cat-photo");
        image_.set_fit(fit);
        image_.set_filter(filter);
        image_.set_tint(tint);
        image_.set_layout_policy(ouif::SizePolicy::Fill, ouif::SizePolicy::Fill);
        image_.set_style(ouif::Style()
                .with_background(ouif::Color::hex(0x101820))
                .with_radius(10.0f));

        label_.set_size({ 0.0f, 32.0f });
        label_.set_layout_policy(ouif::SizePolicy::Fill, ouif::SizePolicy::Fixed);
        label_.set_text_align(ouif::TextAlign::Center);
        label_.set_text_color(ouif::Color::hex(0xe8edf3));

        children(image_, label_);
    }

private:
    ouif::Image image_;
    ouif::Label label_;
};

class DemoSurface : public ouif::RowLayout {
public:
    DemoSurface()
        : contain_("contain + linear", ouif::ImageFit::Contain, ouif::ImageFilter::Linear, ouif::Color::rgba(255, 255, 255, 255))
        , cover_("cover + tint", ouif::ImageFit::Cover, ouif::ImageFilter::Linear, ouif::Color::rgba(150, 210, 255, 220))
        , nearest_("stretch + nearest", ouif::ImageFit::Stretch, ouif::ImageFilter::Nearest, ouif::Color::rgba(255, 255, 255, 255))
    {
        set_alignment(ouif::Align::Center);
        set_cross_alignment(ouif::Align::Center);
        set_gap(28.0f);
        set_padding(28.0f);
        set_layout_policy(ouif::SizePolicy::Fill, ouif::SizePolicy::Fill);
        set_style(ouif::Style().with_background(ouif::Color::hex(0x101218)));
        children(contain_, cover_, nearest_);
        set_stylesheet(R"css(
            .cat-photo {
                image-source: path(def(cat-path));
            }
        )css");
    }

private:
    ImageCard contain_;
    ImageCard cover_;
    ImageCard nearest_;
};

} // namespace

int main()
{
    ouif::define_var("cat-path", OUIF_EXAMPLE_CAT_PATH);

    ouif::Application app(ouif::ApplicationConfig()
            .with_title("OUIF Images")
            .with_size(980, 540)
            .with_clear_color(ouif::Color::hex(0x101218)));

    DemoSurface surface;
    app.set_root(surface);

    return app.run();
}
