#include <OUIF/OUIF.h>

#include <memory>
#include <utility>

class ColorTile : public ouif::Widget {
public:
    // Notice we only pass Size now, not Rect. The parent layout will handle X/Y positions.
    ColorTile(ouif::Color base, ouif::Color active, ouif::Size size)
    {
        set_size(size);

        // Fluent Style API & declarative states.
        set_style(ouif::Style()
            .with_background(base)
            .with_background_hovered(active)
            .with_background_pressed(ouif::Color::rgba(22, 27, 35, 255))
            .with_background_selected(active)
            .with_border(ouif::Color::rgba(232, 237, 243, 220), 2.0f)
            .with_border_selected(ouif::Color::rgba(232, 237, 243, 220), 4.0f));
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
    {
        set_alignment(ouif::Align::Center);
        set_gap(32.0f);
        set_layout_policy(ouif::SizePolicy::Fill, ouif::SizePolicy::Fill);

        set_style(ouif::Style()
            .with_background(ouif::Color::rgba(28, 31, 38, 255))
            .with_background_hovered(ouif::Color::rgba(32, 37, 46, 255))
            .with_background_pressed(ouif::Color::rgba(24, 28, 35, 255))
            .with_border(ouif::Color::rgba(100, 112, 132, 255), 1.0f));

        add_child(std::make_unique<ColorTile>(
            ouif::Color::rgba(47, 108, 156, 255),
            ouif::Color::rgba(70, 146, 196, 255),
            ouif::Size { 160.0f, 120.0f }));

        add_child(std::make_unique<ColorTile>(
            ouif::Color::rgba(122, 82, 156, 255),
            ouif::Color::rgba(162, 112, 198, 255),
            ouif::Size { 160.0f, 120.0f }));

        add_child(std::make_unique<ColorTile>(
            ouif::Color::rgba(76, 138, 95, 255),
            ouif::Color::rgba(104, 176, 126, 255),
            ouif::Size { 160.0f, 120.0f }));
    }
};

int main()
{
    ouif::Application app(ouif::ApplicationConfig()
            .with_title("Hello OUIF")
            .with_size(960, 540)
            .with_clear_color(ouif::Color::rgba(16, 18, 24, 255)));

    app.set_root(std::make_unique<DemoSurface>());

    return app.run();
}
