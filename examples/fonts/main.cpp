#include <OUIF/OUIF.h>

class FontSurface : public ouif::ColLayout {
public:
    FontSurface()
        : default_title_("Default OS font")
        , default_line_("Loaded automatically from the platform's native sans font.")
        , ttf_title_("TTF font")
        , ttf_line_("This line uses examples/arial_narrow_7.ttf.")
    {
        set_layout_policy(ouif::SizePolicy::Fill, ouif::SizePolicy::Fill);
        set_alignment(ouif::Align::Center);
        set_cross_alignment(ouif::Align::Center);
        set_gap(18.0f);
        set_padding(48.0f);
        set_style(ouif::Style()
            .with_background("#101218")
            .with_radius(18.0f)
            .with_border("#2d394c", 1.0f));

        setup_label(default_title_, 32.0f, "#f2f4f8", "OUIF Sans");
        setup_label(default_line_, 18.0f, "#aeb9c8", "OUIF Sans");
        setup_label(ttf_title_, 34.0f, "#9bd0ff", "Example Narrow");
        setup_label(ttf_line_, 22.0f, "#d7e7ff", "Example Narrow");

        default_title_.set_margin({ 0.0f, 0.0f, 0.0f, 18.0f });
        ttf_title_.set_margin({ 28.0f, 0.0f, 0.0f, 8.0f });
        children(default_title_, default_line_, ttf_title_, ttf_line_);
    }

private:
    static void setup_label(ouif::Label& label, float size, ouif::Color color, std::string family)
    {
        label.set_size({ 760.0f, size * 1.6f });
        label.set_text_align(ouif::TextAlign::Center);
        label.set_text_style(ouif::TextStyle()
            .with_font_family(std::move(family))
            .with_font_size(size)
            .with_color(color));
    }

    ouif::Label default_title_;
    ouif::Label default_line_;
    ouif::Label ttf_title_;
    ouif::Label ttf_line_;
};

int main()
{
    ouif::Application app(ouif::ApplicationConfig()
            .with_title("OUIF Fonts")
            .with_size(960, 540)
            .with_clear_color("#0b0f16"));

    app.load_font("Example Narrow", OUIF_EXAMPLE_FONT_PATH);
    FontSurface surface;
    app.set_root(surface);
    return app.run();
}
