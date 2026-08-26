#include <OUIF/OUIF.h>

namespace {

constexpr const char* LogoSvg = R"svg(
<svg width="96" height="96" viewBox="0 0 96 96">
    <rect x="8" y="8" width="80" height="80" rx="18" fill="#20252e" stroke="#e8edf3" stroke-width="3" />
    <circle cx="48" cy="48" r="25" fill="#4692c4" />
    <path d="M35 51 L45 61 L63 34" fill="none" stroke="#ffffff" stroke-width="7" />
</svg>
)svg";

class VectorBadge : public ouif::Widget {
public:
    VectorBadge()
    {
        set_size({ 180.0f, 180.0f });
        set_background(ouif::Color::hex(0x1c1f26));
        set_radius(18.0f);
        set_border(ouif::Color::hex(0x647084), 2.0f);
    }

protected:
    void draw(ouif::Renderer& renderer) override
    {
        ouif::Widget::draw(renderer);
        renderer.draw_vector(bounds().inset(24.0f), [](ouif::VectorCanvas& canvas) {
            canvas.begin_path();
            canvas.circle(66.0f, 66.0f, 58.0f);
            canvas.fill(ouif::Color::hex(0x68b07e));

            canvas.begin_path();
            canvas.move_to(38.0f, 66.0f);
            canvas.line_to(58.0f, 86.0f);
            canvas.line_to(94.0f, 44.0f);
            canvas.stroke(ouif::Color::hex(0xffffff), 8.0f, ouif::VectorLineCap::Round, ouif::VectorLineJoin::Round);
        });
    }
};

class VectorShowcase : public ouif::RowLayout {
public:
    VectorShowcase()
    {
        set_gap(28.0f);
        set_alignment(ouif::Align::Center);
        set_layout_policy(ouif::SizePolicy::Fill, ouif::SizePolicy::Fill);
        set_padding(40.0f);
        set_background(ouif::Color::hex(0x101218));

        auto& inline_icon = add_child<ouif::VectorImage>();
        inline_icon.set_size({ 180.0f, 180.0f });
        inline_icon.set_svg(LogoSvg);

        auto& tinted_icon = add_child<ouif::VectorImage>();
        tinted_icon.add_class("tinted");
        tinted_icon.set_size({ 180.0f, 180.0f });
        tinted_icon.set_svg(LogoSvg);

        add_child<VectorBadge>();

        set_stylesheet(R"css(
            .tinted {
                svg-fit: contain;
                svg-tint: #a270c6;
                background: #181c23;
                border-radius: 18px;
                border: #647084 2px;
            }
        )css");
    }
};

} // namespace

int main()
{
    ouif::Application app(ouif::ApplicationConfig()
            .with_title("OUIF Vector Images")
            .with_size(920, 420)
            .with_clear_color(ouif::Color::hex(0x101218)));

    VectorShowcase surface;
    app.set_root(surface);
    return app.run();
}
