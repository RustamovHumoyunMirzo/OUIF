#include <OUIF/OUIF.h>

class ColorBand : public ouif::Widget {
public:
    ColorBand(ouif::Color color, ouif::Size size)
    {
        set_size(size);
        set_style(ouif::Style()
            .with_background(color)
            .with_radius(18.0f));
    }
};

class GlassPanel : public ouif::ColLayout {
public:
    GlassPanel(const char* title, const char* body)
        : title_(title)
        , body_(body)
    {
        add_class("glass");
        set_gap(12.0f);
        set_padding(ouif::Insets(24.0f));
        set_size({ 360.0f, 210.0f });

        title_.add_class("title");
        title_.set_text_align(ouif::TextAlign::Center);
        title_.set_size({ 312.0f, 42.0f });

        body_.add_class("body");
        body_.set_text_align(ouif::TextAlign::Center);
        body_.set_text_overflow(ouif::TextOverflow::Wrap);
        body_.set_size({ 312.0f, 92.0f });

        children(title_, body_);
    }

private:
    ouif::Label title_;
    ouif::Label body_;
};

class BlurDemo : public ouif::Widget {
public:
    BlurDemo()
        : blue_("#2f6c9c", { 300.0f, 120.0f })
        , violet_("#7a529c", { 360.0f, 140.0f })
        , green_("#4c8a5f", { 320.0f, 120.0f })
        , css_panel_("CSS Blur", "backdrop-effect: blur(18px) is parsed from the stylesheet.")
        , cpp_panel_("C++ Blur", "add_layer_effect(\"blur\", { 12.0f }) uses the same effect registry.")
    {
        set_layout_policy(ouif::SizePolicy::Fill, ouif::SizePolicy::Fill);
        set_style(ouif::Style()
            .with_background("#101218")
            .with_foreground("#edf3ff"));

        blue_.set_bounds({ 90.0f, 88.0f, 300.0f, 120.0f });
        blue_.set_rotation(-8.0f);
        violet_.set_bounds({ 430.0f, 110.0f, 360.0f, 140.0f });
        violet_.set_rotation(7.0f);
        green_.set_bounds({ 620.0f, 320.0f, 320.0f, 120.0f });
        green_.set_rotation(-5.0f);

        css_panel_.set_bounds({ 170.0f, 250.0f, 360.0f, 210.0f });
        css_panel_.set_z_index(10);
        cpp_panel_.set_bounds({ 570.0f, 210.0f, 360.0f, 210.0f });
        cpp_panel_.set_z_index(10);
        cpp_panel_.add_layer_effect("blur", { 12.0f });

        children(blue_, violet_, green_, css_panel_, cpp_panel_);

        set_stylesheet(R"css(
            .glass {
                background: #1f2733cc;
                border: #e8edf366 1px;
                border-radius: 24px;
                backdrop-effect: blur(18px);
            }

            .glass:hover {
                background: #293446dd;
                border: #9fd7ff99 2px;
            }

            .title {
                background: #ffffff00;
                color: #f5f8ff;
                font-size: 24px;
            }

            .body {
                background: #ffffff00;
                color: #cbd7e8;
                font-size: 17px;
                text-overflow: wrap;
            }
        )css");
    }

protected:
    void on_layout(ouif::Rect) override
    {
        blue_.set_bounds({ 90.0f, 88.0f, 300.0f, 120.0f });
        violet_.set_bounds({ 430.0f, 110.0f, 360.0f, 140.0f });
        green_.set_bounds({ 620.0f, 320.0f, 320.0f, 120.0f });
        css_panel_.set_bounds({ 170.0f, 250.0f, 360.0f, 210.0f });
        cpp_panel_.set_bounds({ 570.0f, 210.0f, 360.0f, 210.0f });
    }

private:
    ColorBand blue_;
    ColorBand violet_;
    ColorBand green_;
    GlassPanel css_panel_;
    GlassPanel cpp_panel_;
};

int main()
{
    ouif::Application app(ouif::ApplicationConfig()
        .with_title("OUIF Blur Effect")
        .with_size(1100, 680)
        .with_clear_color("#101218")
        .with_render_quality(ouif::RendererQuality::Ultra));

    BlurDemo demo;
    app.set_root(demo);
    return app.run();
}
