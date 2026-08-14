#include <OUIF/OUIF.h>

#include <array>

class Tile : public ouif::Widget {
public:
    Tile(ouif::Color base, ouif::Size size)
    {
        set_size(size);
        set_keyboard_activation_enabled(true);
        set_accessibility_role(ouif::AccessibilityRole::Button);
        set_style(ouif::Style()
                .with_background(base)
                .with_background_hovered(ouif::Color::hex(0x3f5874))
                .with_background_pressed(ouif::Color::hex(0x182130))
                .with_background_selected(ouif::Color::hex(0x496f9c))
                .with_background_focused(ouif::Color::hex(0x344760))
                .with_border(ouif::Color::hexa(0xe8edf388), 1.0f)
                .with_border_selected(ouif::Color::hex(0xe8edf3), 3.0f)
                .with_border_focused(ouif::Color::hex(0xf5d36c), 3.0f)
                .with_radius(14.0f));
    }

protected:
    bool on_click(const ouif::MouseEvent&) override
    {
        toggle_state(ouif::WidgetState::Selected);
        return true;
    }
};

class OverflowTile : public ouif::Widget {
public:
    OverflowTile()
    {
        set_size({ 260.0f, 120.0f });
        set_style(ouif::Style()
                .with_background(ouif::Color::hex(0xb94f5d))
                .with_background_hovered(ouif::Color::hex(0xd76876))
                .with_border(ouif::Color::hex(0xffd2d8), 2.0f)
                .with_radius({ 16.0f, 36.0f, 16.0f, 36.0f }));
    }
};

class DemoSurface : public ouif::ColLayout {
public:
    DemoSurface()
    {
        set_gap(24.0f);
        set_padding(ouif::Insets(24.0f));
        set_layout_policy(ouif::SizePolicy::Fill, ouif::SizePolicy::Fill);
        set_style(ouif::Style()
                .with_background(ouif::Color::hex(0x101722))
                .with_background_focused(ouif::Color::hex(0x101722))
                .with_border(ouif::Color::hex(0x2e4057), 1.0f));

        horizontal_.set_size(ouif::Size { 0.0f, 160.0f });
        horizontal_.set_layout_policy(ouif::SizePolicy::Fill, ouif::SizePolicy::Fixed);
        horizontal_.set_gap(16.0f);
        horizontal_.set_padding(ouif::Insets(16.0f));
        horizontal_.set_scroll_step(72.0f);
        horizontal_.set_style(ouif::Style()
                .with_background(ouif::Color::hex(0x172232))
                .with_border(ouif::Color::hex(0x405675), 1.0f)
                .with_radius(18.0f));

        vertical_.set_layout_policy(ouif::SizePolicy::Fill, ouif::SizePolicy::Fill);
        vertical_.set_gap(12.0f);
        vertical_.set_padding(ouif::Insets(16.0f));
        vertical_.set_scroll_step(64.0f);
        vertical_.set_style(ouif::Style()
                .with_background(ouif::Color::hex(0x172232))
                .with_border(ouif::Color::hex(0x405675), 1.0f)
                .with_radius(18.0f));

        clip_demo_.set_size({ 340.0f, 150.0f });
        clip_demo_.set_clip_content(true);
        clip_demo_.set_style(ouif::Style()
                .with_background(ouif::Color::hex(0x202a39))
                .with_border(ouif::Color::hex(0x83b7ff), 2.0f)
                .with_radius(18.0f));
        overflow_.set_bounds({ 170.0f, 16.0f, 260.0f, 120.0f });
        clip_demo_.add_child(overflow_);

        const std::array<ouif::Color, 8> palette {
            ouif::Color::hex(0x2f6c9c),
            ouif::Color::hex(0x7a529c),
            ouif::Color::hex(0x4c8a5f),
            ouif::Color::hex(0x9b6b3d),
            ouif::Color::hex(0x9b4d61),
            ouif::Color::hex(0x446c7d),
            ouif::Color::hex(0x6f7342),
            ouif::Color::hex(0x5c668a),
        };

        for (auto color : palette) {
            horizontal_.add_child<Tile>(color, ouif::Size { 180.0f, 104.0f });
        }

        for (int index = 0; index < 14; ++index) {
            vertical_.add_child<Tile>(palette[static_cast<std::size_t>(index) % palette.size()], ouif::Size { 0.0f, 56.0f })
                .set_layout_policy(ouif::SizePolicy::Fill, ouif::SizePolicy::Fixed);
        }

        children(horizontal_, vertical_, clip_demo_);
    }

    bool event(const ouif::Event& event) override
    {
        if (const auto* key = std::get_if<ouif::KeyEvent>(&event)) {
            if (key->action == ouif::KeyAction::Press || key->action == ouif::KeyAction::Repeat) {
                if (handle_demo_key(*key)) {
                    return true;
                }
            }
        }
        return ouif::ColLayout::event(event);
    }

private:
    bool handle_demo_key(const ouif::KeyEvent& event)
    {
        if (event.key == 'A') {
            horizontal_.set_scroll_offset(horizontal_.scroll_offset() - horizontal_.scroll_step());
            return true;
        }
        if (event.key == 'D') {
            horizontal_.set_scroll_offset(horizontal_.scroll_offset() + horizontal_.scroll_step());
            return true;
        }
        if (event.key == 'Z') {
            vertical_.set_scroll_offset(vertical_.scroll_offset() - vertical_.scroll_step());
            return true;
        }
        if (event.key == 'X') {
            vertical_.set_scroll_offset(vertical_.scroll_offset() + vertical_.scroll_step());
            return true;
        }
        if (event.key == 'C') {
            clip_demo_.set_clip_content(!clip_demo_.clip_content());
            clip_demo_.set_border(
                clip_demo_.clip_content() ? ouif::Color::hex(0x83b7ff) : ouif::Color::hex(0xf5d36c),
                2.0f
            );
            return true;
        }
        return false;
    }

    ouif::RowScroll horizontal_;
    ouif::ColScroll vertical_;
    ouif::Widget clip_demo_;
    OverflowTile overflow_;
};

int main()
{
    ouif::Application app(ouif::ApplicationConfig()
            .with_title("OUIF Clipping And Scroll")
            .with_size(980, 680)
            .with_clear_color(ouif::Color::hex(0x0c111a))
            .with_render_quality(ouif::RendererQuality::Ultra));

    DemoSurface surface;
    app.set_root(surface);
    return app.run();
}
