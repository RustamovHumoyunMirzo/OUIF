#include <OUIF/OUIF.h>

class CssTile : public ouif::Widget {
public:
    CssTile(const char* class_name)
    {
        add_class("tile");
        add_class(class_name);
        set_size(ouif::Length::percent(28.0f), ouif::Length::px(160.0f));
    }
};

class CssSurface : public ouif::ColLayout {
public:
    CssSurface()
    {
        set_layout_policy(ouif::SizePolicy::Fill, ouif::SizePolicy::Fill);
        set_padding(ouif::Insets(32.0f));
        set_gap(28.0f);

        header_.add_class("header");

        row_.add_class("tile-row");
        row_.set_gap(28.0f);
        row_.set_alignment(ouif::Align::Center);
        row_.set_cross_alignment(ouif::Align::Center);
        row_.set_flex(1.0f);

        children(header_, row_);
        row_.children(blue_, violet_, green_);

        set_stylesheet(R"css(
            .header {
                background: #1e2633;
                height: 72px;
                border-left: #5aa7d8 6px;
                border-right: #83d28f 6px;
                border-top: #34445a 1px;
                border-bottom: #34445a 1px;
                border-radius: 12px;
            }

            .tile-row {
                background: #151a22;
                border: #2d394a 1px;
                border-radius: 16px;
                padding: 28px;
            }

            .tile {
                background: #263241;
                background-hovered: #303f52;
                background-pressed: #1c2530;
                border-radius: 14px;
                border-top: #53677d 1px;
                border-bottom: #10151c 5px;
            }

            .blue {
                background: #2f6c9c;
                background-hovered: #4692c4;
                border-left: #9fd7ff 8px;
            }

            .violet {
                background: #7a529c;
                background-hovered: #a270c6;
                border-left: #e0b5ff 8px;
                border-right: #3c294c 4px;
            }

            .green {
                background: #4c8a5f;
                background-hovered: #68b07e;
                border-left: #b7f2c0 8px;
            }
        )css");
    }

private:
    ouif::Widget header_;
    ouif::RowLayout row_;
    CssTile blue_ { "blue" };
    CssTile violet_ { "violet" };
    CssTile green_ { "green" };
};

int main()
{
    ouif::Application app(ouif::ApplicationConfig()
        .with_title("OUIF CSS Styling")
        .with_size(1040, 640)
        .with_clear_color(ouif::Color::hex(0x10141c))
        .with_render_quality(ouif::RendererQuality::Ultra));

    CssSurface surface;
    app.set_root(surface);
    return app.run();
}
