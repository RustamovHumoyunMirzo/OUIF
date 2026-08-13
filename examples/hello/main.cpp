#include <OUIF/OUIF.h>

#include <memory>
#include <utility>

class ColorTile : public ouif::Widget {
public:
    ColorTile(ouif::Color base, ouif::Color active, ouif::Rect bounds)
        : base_(base)
        , active_(active)
    {
        ouif::Style style;
        style.background = base_;
        style.background_hovered = active_;
        style.background_pressed = ouif::Color::rgba(22, 27, 35, 255);
        style.border = ouif::Color::rgba(232, 237, 243, 220);
        style.border_width = 2.0f;
        set_style(style);
        set_bounds(bounds);
    }

protected:
    bool on_click(const ouif::MouseEvent&) override
    {
        selected_ = !selected_;
        ouif::Style style = this->style();
        style.background = selected_ ? active_ : base_;
        style.border_width = selected_ ? 4.0f : 2.0f;
        set_style(style);
        return true;
    }

private:
    ouif::Color base_;
    ouif::Color active_;
    bool selected_ = false;
};

class DemoSurface : public ouif::Widget {
public:
    DemoSurface()
    {
        ouif::Style style;
        style.background = ouif::Color::rgba(28, 31, 38, 255);
        style.background_hovered = ouif::Color::rgba(32, 37, 46, 255);
        style.background_pressed = ouif::Color::rgba(24, 28, 35, 255);
        style.border = ouif::Color::rgba(100, 112, 132, 255);
        style.border_width = 1.0f;
        set_style(style);

        set_layout({
            { 0.0f, 0.0f },
            { 420.0f, 260.0f },
            { 100000.0f, 100000.0f },
            { 28.0f, 28.0f, 28.0f, 28.0f },
            ouif::SizePolicy::Fill,
            ouif::SizePolicy::Fill,
        });

        add_child(std::make_unique<ColorTile>(
            ouif::Color::rgba(47, 108, 156, 255),
            ouif::Color::rgba(70, 146, 196, 255),
            ouif::Rect { 40.0f, 48.0f, 160.0f, 120.0f }
        ));
        add_child(std::make_unique<ColorTile>(
            ouif::Color::rgba(122, 82, 156, 255),
            ouif::Color::rgba(162, 112, 198, 255),
            ouif::Rect { 232.0f, 48.0f, 160.0f, 120.0f }
        ));
        add_child(std::make_unique<ColorTile>(
            ouif::Color::rgba(76, 138, 95, 255),
            ouif::Color::rgba(104, 176, 126, 255),
            ouif::Rect { 424.0f, 48.0f, 160.0f, 120.0f }
        ));
    }

protected:
    void on_layout(ouif::Rect content) override
    {
        const float tile_width = 160.0f;
        const float tile_height = 120.0f;
        const float gap = 32.0f;
        const float total_width = tile_width * 3.0f + gap * 2.0f;
        const float start_x = content.x + (content.width - total_width) * 0.5f;
        const float start_y = content.y + (content.height - tile_height) * 0.5f;

        for (std::size_t index = 0; index < children().size(); ++index) {
            children()[index]->set_bounds({
                start_x + static_cast<float>(index) * (tile_width + gap),
                start_y,
                tile_width,
                tile_height,
            });
        }
    }
};

int main()
{
    ouif::ApplicationConfig config;
    config.title = "Hello OUIF";
    config.width = 960;
    config.height = 540;
    config.clear_color = ouif::Color::rgba(16, 18, 24, 255);
    ouif::Application app(config);

    auto root = std::make_unique<DemoSurface>();
    app.set_root(std::move(root));

    return app.run();
}
