#include <OUIF/OUIF.h>

class ColorTile : public ouif::Widget {
public:
    ColorTile(ouif::Color base, ouif::Color active, ouif::Size size)
    {
        set_size(size);
        set_style(ouif::Style()
            .with_background(base)
            .with_background_hovered(active)
            .with_background_pressed(ouif::Color::hex(0x161b23))
            .with_background_selected(active)
            .with_border(ouif::Color::hexa(0xe8edf3dc), 2.0f)
            .with_border_selected(ouif::Color::hexa(0xe8edf3dc), 4.0f));
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
        : blue_(ouif::Color::hex(0x2f6c9c), ouif::Color::hex(0x4692c4), { 160.0f, 120.0f })
        , violet_(ouif::Color::hex(0x7a529c), ouif::Color::hex(0xa270c6), { 160.0f, 120.0f })
        , green_(ouif::Color::hex(0x4c8a5f), ouif::Color::hex(0x68b07e), { 160.0f, 120.0f })
    {
        set_alignment(ouif::Align::Center);
        set_gap(32.0f);
        set_layout_policy(ouif::SizePolicy::Fill, ouif::SizePolicy::Fill);

        set_style(ouif::Style()
            .with_background(ouif::Color::hex(0x1c1f26))
            .with_background_hovered(ouif::Color::hex(0x20252e))
            .with_background_pressed(ouif::Color::hex(0x181c23))
            .with_border(ouif::Color::hex(0x647084), 1.0f));

        add_child(blue_);
        add_child(violet_);
        add_child(green_);
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
            .with_clear_color(ouif::Color::hex(0x101218)));

    DemoSurface surface;
    app.set_root(surface);

    return app.run();
}
