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

protected:
    bool on_click(const ouif::MouseEvent&) override
    {
        toggle_state(ouif::WidgetState::Selected);
        return true;
    }
};

class DemoSurface : public ouif::RowLayout {
public:
    DemoSurface()
        : blue_("#2f6c9c", "#4692c4", { 160.0f, 120.0f })
        , violet_("#7a529c", "#a270c6", { 160.0f, 120.0f })
        , green_("#4c8a5f", "#68b07e", { 160.0f, 120.0f })
    {
        set_alignment(ouif::Align::Center);
        set_gap(32.0f);
        set_layout_policy(ouif::SizePolicy::Fill, ouif::SizePolicy::Fill);

        set_style(ouif::Style {
            .background = "#1c1f26",
            .hovered = "#20252e",
            .pressed = "#181c23",
            .border = { "#647084", 1.0f },
        });

        children(blue_, violet_, green_);
    }

private:
    ColorTile blue_;
    ColorTile violet_;
    ColorTile green_;
};

int main()
{
    ouif::Application app(ouif::ApplicationConfig()
            .with_title("Hello OUIF")
            .with_size(960, 540)
            .with_clear_color("#101218"));

    DemoSurface surface;
    app.set_root(surface);

    return app.run();
}
