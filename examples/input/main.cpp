#include <OUIF/OUIF.h>

class InputDemo : public ouif::ColLayout {
public:
    InputDemo()
    {
        set_type_name("InputDemo");
        set_layout_policy(ouif::SizePolicy::Fill, ouif::SizePolicy::Fill);
        set_padding(ouif::Insets(36.0f));
        set_gap(18.0f);

        title_.set_text("Input");
        title_.set_size({ 0.0f, 44.0f });
        title_.set_layout_policy(ouif::SizePolicy::Fill, ouif::SizePolicy::Fixed);
        title_.set_font_size(28.0f);
        title_.set_text_color(ouif::Gradient::Linear(90.0f, {
            { 0.0f, ouif::Color::hex(0xffffff) },
            { 1.0f, ouif::Color::hex(0x8dc7ff) },
        }));

        first_.set_placeholder("Type here");
        first_.set_text("Hello OUIF");
        first_.set_size({ 0.0f, 48.0f });
        first_.set_layout_policy(ouif::SizePolicy::Fill, ouif::SizePolicy::Fixed);

        second_.set_placeholder("Gradient text");
        second_.set_size({ 0.0f, 48.0f });
        second_.set_layout_policy(ouif::SizePolicy::Fill, ouif::SizePolicy::Fixed);
        second_.set_text_color(ouif::Gradient::Linear(90.0f, {
            { 0.0f, ouif::Color::hex(0xffffff) },
            { 1.0f, ouif::Color::hex(0x68b07e) },
        }));

        panel_.add_class("panel");
        first_.add_class("field");
        second_.add_class("field");
        panel_.set_layout_policy(ouif::SizePolicy::Fill, ouif::SizePolicy::Content);
        panel_.set_padding(ouif::Insets(20.0f));
        panel_.set_gap(14.0f);
        panel_.children(first_, second_);

        children(title_, panel_);

        set_stylesheet(R"css(
            InputDemo {
                background: gradient(linear 160deg (0% #101218) (100% #1d2735));
            }

            .panel {
                background: #151b24;
                border: 1px solid #334257;
                border-radius: 14px;
            }

            .field {
                background: #0f1520;
                background-hovered: #151f2d;
                background-focused: #1d2735;
                border: 1px solid #40516a;
                border-focused: 2px solid #8dc7ff;
                border-radius: 10px;
                font-size: 17px;
                placeholder-color: #8492a6;
            }
        )css");
    }

private:
    ouif::Label title_;
    ouif::ColLayout panel_;
    ouif::Input first_;
    ouif::Input second_;
};

int main()
{
    ouif::Application app(ouif::ApplicationConfig()
        .with_title("OUIF Input")
        .with_size(760, 420)
        .with_clear_color(ouif::Color::hex(0x101218))
        .with_render_quality(ouif::RendererQuality::Ultra));

    InputDemo demo;
    app.set_root(demo);
    return app.run();
}
