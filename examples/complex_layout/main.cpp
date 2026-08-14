#include <OUIF/OUIF.h>

class Block : public ouif::Widget {
public:
    Block(ouif::Color background, ouif::Color active, ouif::Size size = { 0.0f, 0.0f })
    {
        if (size.width > 0.0f || size.height > 0.0f) {
            set_size(size);
        }

        set_style(ouif::Style {
            .background = background,
            .hovered = active,
            .pressed = "#10151d",
            .selected = active,
            .focused = "#223148",
            .border = { "#344052", 1.0f },
            .border_selected = { "#d7e7ff", 3.0f },
            .border_focused = { "#83b7ff", 2.0f },
            .radius = ouif::CornerRadius(8.0f),
        });
    }
};

class Sidebar : public ouif::ColLayout {
public:
    Sidebar()
        : header_("#243244", "#31445c", { 188.0f, 64.0f })
        , nav_a_("#1b2533", "#26364a", { 188.0f, 44.0f })
        , nav_b_("#1b2533", "#26364a", { 188.0f, 44.0f })
        , nav_c_("#1b2533", "#26364a", { 188.0f, 44.0f })
        , footer_("#202a39", "#2c3a4f", { 188.0f, 72.0f })
    {
        set_size({ 236.0f, 0.0f });
        set_layout_policy(ouif::SizePolicy::Fixed, ouif::SizePolicy::Fill);
        set_alignment(ouif::Align::Start);
        set_gap(16.0f);
        set_padding(24.0f);
        set_style(ouif::Style {
            .background = "#131923",
            .hovered = "#151d28",
            .border = { "#2c3748", 1.0f },
        });

        children(header_, nav_a_, nav_b_, nav_c_);
        auto& spacer = add_child<Block>("#182231", "#26364a");
        spacer.set_flex(1.0f);
        add_child(footer_);
    }

private:
    Block header_;
    Block nav_a_;
    Block nav_b_;
    Block nav_c_;
    Block footer_;
};

class MetricRow : public ouif::RowLayout {
public:
    MetricRow()
        : revenue_("#27556e", "#367895", { 0.0f, 112.0f })
        , usage_("#5a4277", "#73559a", { 0.0f, 112.0f })
        , health_("#3f684d", "#528562", { 0.0f, 112.0f })
    {
        set_gap(20.0f);
        set_size({ 0.0f, 112.0f });
        set_layout_policy(ouif::SizePolicy::Fill, ouif::SizePolicy::Fixed);

        revenue_.set_flex(1.2f);
        usage_.set_flex(1.0f);
        health_.set_flex(0.8f);
        revenue_.set_layout_policy(ouif::SizePolicy::Fill, ouif::SizePolicy::Fixed);
        usage_.set_layout_policy(ouif::SizePolicy::Fill, ouif::SizePolicy::Fixed);
        health_.set_layout_policy(ouif::SizePolicy::Fill, ouif::SizePolicy::Fixed);

        children(revenue_, usage_, health_);
    }

private:
    Block revenue_;
    Block usage_;
    Block health_;
};

class WorkArea : public ouif::ColLayout {
public:
    WorkArea()
        : toolbar_("#1e2938", "#29384c", { 0.0f, 72.0f })
        , metrics_()
        , content_("#17202d", "#202c3c")
        , inspector_("#1f2a3a", "#2c3b51", { 280.0f, 0.0f })
    {
        set_gap(20.0f);
        set_padding(24.0f);
        set_layout_policy(ouif::SizePolicy::Fill, ouif::SizePolicy::Fill);
        set_style(ouif::Style {
            .background = "#0f141d",
            .hovered = "#121925",
        });

        toolbar_.set_layout_policy(ouif::SizePolicy::Fill, ouif::SizePolicy::Fixed);
        content_.set_layout_policy(ouif::SizePolicy::Fill, ouif::SizePolicy::Fill);
        inspector_.set_layout_policy(ouif::SizePolicy::Fixed, ouif::SizePolicy::Fill);
        content_.set_flex(1.0f);
        inspector_.set_margin({ 0.0f, 0.0f, 0.0f, 0.0f });

        main_row_.set_gap(20.0f);
        main_row_.set_layout_policy(ouif::SizePolicy::Fill, ouif::SizePolicy::Fill);
        main_row_.children(content_, inspector_);

        children(toolbar_, metrics_, main_row_);
    }

private:
    Block toolbar_;
    MetricRow metrics_;
    ouif::RowLayout main_row_;
    Block content_;
    Block inspector_;
};

class ComplexSurface : public ouif::RowLayout {
public:
    ComplexSurface()
    {
        set_gap(0.0f);
        set_layout_policy(ouif::SizePolicy::Fill, ouif::SizePolicy::Fill);
        set_style(ouif::Style {
            .background = "#0b0f16",
        });

        children(sidebar_, work_area_);
    }

private:
    Sidebar sidebar_;
    WorkArea work_area_;
};

int main()
{
    ouif::Application app(ouif::ApplicationConfig()
            .with_title("OUIF Complex Layout")
            .with_size(1280, 760)
            .with_clear_color("#0b0f16"));

    ComplexSurface surface;
    app.set_root(surface);
    return app.run();
}
