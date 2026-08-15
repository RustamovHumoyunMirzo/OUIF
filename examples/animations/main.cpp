#include <OUIF/OUIF.h>

class TransitionTile : public ouif::Widget {
public:
    TransitionTile()
    {
        set_size({ 180.0f, 120.0f });
        set_transition(0.28f, ouif::Easing::EaseInOut);
        set_keyboard_activation_enabled(true);
        set_accessibility_role(ouif::AccessibilityRole::Button);
        set_accessibility_label("Transition tile");
        set_style(rest_style_);
    }

protected:
    bool on_click(const ouif::MouseEvent&) override
    {
        active_ = !active_;
        set_style(active_ ? active_style_ : rest_style_);
        return true;
    }

private:
    bool active_ = false;
    ouif::Style rest_style_ = ouif::Style()
        .with_background(ouif::Color::hex(0x2f6c9c))
        .with_background_hovered(ouif::Color::hex(0x4692c4))
        .with_border(ouif::Color::hexa(0xa8d8ffcc), 2.0f)
        .with_radius({ 10.0f, 28.0f, 10.0f, 28.0f });
    ouif::Style active_style_ = ouif::Style()
        .with_background(ouif::Color::hex(0xa270c6))
        .with_background_hovered(ouif::Color::hex(0xbe8be4))
        .with_border(ouif::Color::hexa(0xf0d4ffdd), 8.0f)
        .with_radius({ 34.0f, 10.0f, 34.0f, 10.0f });
};

class CppKeyframeTile : public ouif::Widget {
public:
    CppKeyframeTile()
    {
        set_size({ 180.0f, 120.0f });
        set_style(ouif::Style()
            .with_background(ouif::Color::hex(0x4c8a5f))
            .with_border(ouif::Color::hex(0xb7f2c0), 2.0f)
            .with_radius(18.0f)
            .with_opacity(1.0f));

        set_animation({
            .name = "cppPulse",
            .duration = 1.6f,
            .easing = ouif::Easing::EaseInOut,
            .loop = true,
            .keyframes = {
                {
                    0.0f,
                    ouif::Style()
                        .with_background(ouif::Color::hex(0x4c8a5f))
                        .with_radius(12.0f)
                        .with_opacity(0.72f),
                    ouif::style_property_mask(ouif::StyleProperty::Background)
                        | ouif::style_property_mask(ouif::StyleProperty::Radius)
                        | ouif::style_property_mask(ouif::StyleProperty::Opacity),
                },
                {
                    0.5f,
                    ouif::Style()
                        .with_background(ouif::Color::hex(0x68b07e))
                        .with_radius(32.0f)
                        .with_opacity(1.0f),
                    ouif::style_property_mask(ouif::StyleProperty::Background)
                        | ouif::style_property_mask(ouif::StyleProperty::Radius)
                        | ouif::style_property_mask(ouif::StyleProperty::Opacity),
                },
                {
                    1.0f,
                    ouif::Style()
                        .with_background(ouif::Color::hex(0x4c8a5f))
                        .with_radius(12.0f)
                        .with_opacity(0.72f),
                    ouif::style_property_mask(ouif::StyleProperty::Background)
                        | ouif::style_property_mask(ouif::StyleProperty::Radius)
                        | ouif::style_property_mask(ouif::StyleProperty::Opacity),
                },
            },
        });
    }
};

class CssKeyframeTile : public ouif::Widget {
public:
    CssKeyframeTile()
    {
        add_class("css-keyframe-tile");
        set_size({ 180.0f, 120.0f });
    }
};

class AnimationSurface : public ouif::ColLayout {
public:
    AnimationSurface()
    {
        set_layout_policy(ouif::SizePolicy::Fill, ouif::SizePolicy::Fill);
        set_padding(ouif::Insets(36.0f));
        set_gap(28.0f);
        set_style(ouif::Style()
            .with_background(ouif::Color::hex(0x10141c))
            .with_border(ouif::Color::hex(0x263241), 1.0f)
            .with_radius(16.0f));

        strip_.set_gap(28.0f);
        strip_.set_alignment(ouif::Align::Center);
        strip_.set_cross_alignment(ouif::Align::Center);
        strip_.set_flex(1.0f);
        strip_.set_style(ouif::Style()
            .with_background(ouif::Color::hex(0x151a22))
            .with_border(ouif::Color::hex(0x2d394a), 1.0f)
            .with_radius(18.0f));

        glow_.add_class("glow-bar");
        glow_.set_height(ouif::Length::px(72.0f));

        children(glow_, strip_);
        strip_.children(transition_, cpp_keyframes_, css_keyframes_);

        set_stylesheet(R"css(
            @keyframes cssBreathe {
                from {
                    background: #7a529c;
                    border-radius: 10px;
                    opacity: 0.70;
                }
                50% {
                    background: #a270c6;
                    border-radius: 34px;
                    opacity: 1.0;
                }
                to {
                    background: #7a529c;
                    border-radius: 10px;
                    opacity: 0.70;
                }
            }

            @keyframes glowSweep {
                from {
                    background: #1e2633;
                    border-left: #5aa7d8 4px;
                    border-right: #83d28f 12px;
                }
                50% {
                    background: #263241;
                    border-left: #83d28f 12px;
                    border-right: #5aa7d8 4px;
                }
                to {
                    background: #1e2633;
                    border-left: #5aa7d8 4px;
                    border-right: #83d28f 12px;
                }
            }

            .css-keyframe-tile {
                background: #7a529c;
                border: #e0b5ff 2px;
                border-radius: 10px;
                animation: cssBreathe 1.8s ease-in-out infinite;
            }

            .glow-bar {
                background: #1e2633;
                border-top: #34445a 1px;
                border-bottom: #34445a 1px;
                border-left: #5aa7d8 4px;
                border-right: #83d28f 12px;
                border-radius: 14px;
                animation: glowSweep 2.4s ease-in-out infinite;
            }
        )css");
    }

private:
    ouif::Widget glow_;
    ouif::RowLayout strip_;
    TransitionTile transition_;
    CppKeyframeTile cpp_keyframes_;
    CssKeyframeTile css_keyframes_;
};

int main()
{
    ouif::Application app(ouif::ApplicationConfig()
        .with_title("OUIF Animations")
        .with_size(1040, 620)
        .with_clear_color(ouif::Color::hex(0x0d1118))
        .with_render_quality(ouif::RendererQuality::Ultra));

    AnimationSurface surface;
    app.set_root(surface);
    return app.run();
}
