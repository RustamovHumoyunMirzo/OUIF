#include <OUIF/OUIF.h>

class ColorTile : public ouif::Widget {
public:
    ColorTile(ouif::Color base, ouif::Color active, ouif::Size size)
    {
        set_size(size);
        set_style(ouif::Style {
            .background = base,
            .hovered = active,
            .pressed = "#161b23",
            .selected = active,
            .border = { "#e8edf3dc", 2.0f },
            .border_selected = { "#e8edf3dc", 4.0f },
        });
    }
};

class DemoSurface : public ouif::ColLayout {
public:
    DemoSurface()
        : blue_("#2f6c9c", "#4692c4", { 160.0f, 120.0f })
        , violet_("#7a529c", "#a270c6", { 160.0f, 120.0f })
        , green_("#4c8a5f", "#68b07e", { 160.0f, 120.0f })
        , title_("HELLO OUIF")
    {
        set_alignment(ouif::Align::Center);
        set_cross_alignment(ouif::Align::Center);
        set_gap(24.0f);
        set_padding(ouif::Insets(32.0f));
        set_layout_policy(ouif::SizePolicy::Fill, ouif::SizePolicy::Fill);

        set_style(ouif::Style {
            .background = "#1c1f26",
            .hovered = "#20252e",
            .pressed = "#181c23",
            .border = { "#647084", 1.0f },
        });

        title_.set_size({ 520.0f, 48.0f });
        title_.set_text_align(ouif::TextAlign::Center);
        title_.set_font_size(24.0f);
        title_.set_style(ouif::Style()
            .with_background("#263241")
            .with_foreground("#e8edf3")
            .with_border("#34445a", 1.0f)
            .with_radius(10.0f));

        row_.set_gap(32.0f);
        row_.set_alignment(ouif::Align::Center);
        row_.set_cross_alignment(ouif::Align::Center);
        row_.set_flex(1.0f);
        row_.children(blue_, violet_, green_);

        children(title_, row_);
    }

private:
    ouif::RowLayout row_;
    ColorTile blue_;
    ColorTile violet_;
    ColorTile green_;
    ouif::Label title_;
};

int main()
{
    ouif::Application app(ouif::ApplicationConfig()
            .with_title("Hello OUIF")
            .with_size(960, 540)
            .with_clear_color("#101218"));

    DemoSurface surface;
    ColorTile* custom = new ColorTile("#2f6c9c", "#4692c4", { 160.0f, 120.0f });
    custom->set_rotation(4.0f);
    surface.add_child(custom);
    app.set_root(surface);

    return app.run();
}
