#include <OUIF/OUIF.h>

#include <cassert>
#include <cmath>
#include <memory>
#include <stdexcept>

namespace {

class ActivatableWidget : public ouif::Widget {
public:
    int clicks = 0;

protected:
    bool on_click(const ouif::MouseEvent&) override
    {
        ++clicks;
        return true;
    }
};

class HoverTracker : public ouif::Widget {
public:
    int leaves = 0;

protected:
    void on_mouse_leave(const ouif::MouseEvent&) override
    {
        ++leaves;
    }
};

class XmlTile : public ouif::Widget {
public:
    XmlTile() = default;
};

} // namespace

int main()
{
    ouif::Rect rect { 10.0f, 20.0f, 100.0f, 50.0f };
    assert(rect.contains({ 10.0f, 20.0f }));
    assert(rect.contains({ 110.0f, 70.0f }));
    assert(!rect.contains({ 111.0f, 70.0f }));
    auto over_inset = rect.inset(ouif::Insets(200.0f));
    assert(over_inset.width == 0.0f);
    assert(over_inset.height == 0.0f);

    auto color = ouif::Color::hex(0x2f6c9c);
    assert(color.b > color.r);
    auto alpha = ouif::Color::from_hex("#e8edf3dc");
    assert(alpha.has_value());
    assert(alpha->a > 0.8f && alpha->a < 0.9f);

    ouif::Widget root;
    root.set_bounds({ 0.0f, 0.0f, 400.0f, 300.0f });
    root.add_child<ouif::Widget>();
    root.layout({ 400.0f, 300.0f });

    assert(root.children().size() == 1);
    assert(root.children().front()->parent() == &root);
    assert(root.children().front()->bounds().width == 400.0f);
    assert(root.children().front()->bounds().height == 300.0f);

    root.clear_children();
    assert(root.children().empty());

    {
        ouif::Widget parent;
        {
            ouif::Widget child;
            parent.add_child(child);
            parent.add_child(child);
            assert(parent.children().size() == 1);
            assert(child.parent() == &parent);
        }
        assert(parent.children().empty());
    }

    {
        ouif::Widget first_parent;
        ouif::Widget second_parent;
        ouif::Widget child;
        first_parent.add_child(child);
        second_parent.add_child(child);
        assert(first_parent.children().empty());
        assert(second_parent.children().size() == 1);
        assert(child.parent() == &second_parent);
        assert(second_parent.remove_child(child));
        assert(child.parent() == nullptr);
    }

    {
        ouif::Widget first_parent;
        ouif::Widget second_parent;
        auto& owned = first_parent.add_child<ouif::Widget>();
        bool threw = false;
        try {
            second_parent.add_child(owned);
        } catch (const std::invalid_argument&) {
            threw = true;
        }
        assert(threw);
        assert(owned.parent() == &first_parent);
    }

    ouif::Style style = ouif::Style()
        .with_background(ouif::Color::rgb(1, 2, 3))
        .with_background_selected(ouif::Color::rgb(4, 5, 6))
        .with_border(ouif::Color::rgb(7, 8, 9), 2.0f)
        .with_border_selected(ouif::Color::rgb(10, 11, 12), 4.0f);
    assert(style.border.width == 2.0f);
    assert(style.border_selected.width == 4.0f);

    ouif::Style aggregate_style {
        .background = "#101218",
        .hovered = "#20252e",
        .focused = "#223148",
        .border = { "#647084", 1.0f },
        .border_focused = { "#83b7ff", 2.0f },
        .radius = ouif::CornerRadius(4.0f, 8.0f, 12.0f, 16.0f),
    };
    assert(aggregate_style.border.width == 1.0f);
    assert(aggregate_style.border_focused.width == 2.0f);
    assert(aggregate_style.radius.bottom_left == 16.0f);

    ouif::RowLayout row;
    row.set_bounds({ 0.0f, 0.0f, 400.0f, 100.0f });
    row.set_alignment(ouif::Align::Center);
    row.set_gap(20.0f);

    ouif::Widget first;
    first.set_size({ 50.0f, 40.0f });
    auto& second = row.add_child<ouif::Widget>();
    second.set_size({ 50.0f, 40.0f });
    row.add_child(first);
    assert(row.children().size() == 2);
    row.layout({ 400.0f, 100.0f });

    assert(row.children()[0]->bounds().x == 140.0f);
    assert(row.children()[1]->bounds().x == 210.0f);

    row.children()[0]->toggle_state(ouif::WidgetState::Selected);
    assert(row.children()[0]->has_state(ouif::WidgetState::Selected));

    ouif::RowLayout fill_row;
    fill_row.set_bounds({ 0.0f, 0.0f, 300.0f, 100.0f });
    fill_row.set_gap(20.0f);
    auto& fill_a = fill_row.add_child<ouif::Widget>();
    auto& fill_b = fill_row.add_child<ouif::Widget>();
    fill_a.set_layout_policy(ouif::SizePolicy::Fill, ouif::SizePolicy::Fill);
    fill_b.set_layout_policy(ouif::SizePolicy::Fill, ouif::SizePolicy::Fill);
    fill_row.layout({ 300.0f, 100.0f });
    assert(fill_a.bounds().width == 140.0f);
    assert(fill_b.bounds().width == 140.0f);
    assert(fill_b.bounds().x == 160.0f);

    {
        ouif::RowLayout spacer_row;
        spacer_row.set_bounds({ 0.0f, 0.0f, 300.0f, 80.0f });
        auto& left = spacer_row.add_child<ouif::Widget>();
        auto& spacer = spacer_row.add_child<ouif::Spacer>(1.0f);
        auto& divider = spacer_row.add_child<ouif::Divider>(ouif::Orientation::Vertical, 3.0f);
        auto& right = spacer_row.add_child<ouif::Widget>();
        left.set_size({ 60.0f, 40.0f });
        right.set_size({ 60.0f, 40.0f });
        spacer_row.layout({ 300.0f, 80.0f });

        assert(spacer.get_flex() == 1.0f);
        assert(std::fabs(spacer.bounds().width - 177.0f) < 0.01f);
        assert(divider.orientation() == ouif::Orientation::Vertical);
        assert(std::fabs(divider.thickness() - 3.0f) < 0.01f);
        assert(std::fabs(divider.bounds().width - 3.0f) < 0.01f);
        assert(!spacer.event(ouif::MouseButtonEvent { { 70.0f, 10.0f }, {}, ouif::MouseButton::Left, true }));
        assert(!divider.event(ouif::MouseButtonEvent { { 240.0f, 10.0f }, {}, ouif::MouseButton::Left, true }));

        divider.set_color("#e8edf3");
        assert(divider.color().r > 0.8f);
    }

    {
        ouif::Spacer spacer;
        ouif::Divider divider;
        ouif::Widget child;
        assert(!spacer.accepts_children());
        assert(!divider.accepts_children());

        bool spacer_threw = false;
        try {
            spacer.add_child(child);
        } catch (const std::invalid_argument&) {
            spacer_threw = true;
        }
        assert(spacer_threw);

        bool divider_threw = false;
        try {
            divider.add_child<ouif::Widget>();
        } catch (const std::invalid_argument&) {
            divider_threw = true;
        }
        assert(divider_threw);

        ouif::Widget container;
        container.add_child<ouif::Widget>();
        assert(container.accepts_children());
        assert(container.children().size() == 1);
        container.set_accepts_children(false);
        assert(!container.accepts_children());
        assert(container.children().empty());
    }

    ouif::RowLayout weighted_row;
    weighted_row.set_bounds({ 0.0f, 0.0f, 320.0f, 100.0f });
    weighted_row.set_gap(20.0f);
    auto& weighted_a = weighted_row.add_child<ouif::Widget>();
    auto& weighted_b = weighted_row.add_child<ouif::Widget>();
    weighted_a.set_flex(1.0f);
    weighted_b.set_flex(2.0f);
    weighted_b.set_margin({ 10.0f, 0.0f, 0.0f, 0.0f });
    weighted_row.layout({ 320.0f, 100.0f });
    assert(std::fabs(weighted_a.bounds().width - 96.66666f) < 0.01f);
    assert(std::fabs(weighted_b.bounds().width - 193.33333f) < 0.01f);

    weighted_a.focus();
    assert(weighted_a.focused());
    weighted_b.focus();
    assert(!weighted_a.focused());
    assert(weighted_b.focused());

    {
        ouif::RowLayout root;
        ActivatableWidget first;
        ActivatableWidget second;
        first.set_keyboard_activation_enabled(true);
        second.set_keyboard_activation_enabled(true);
        first.set_accessibility({
            ouif::AccessibilityRole::Button,
            "First tile",
            "Activates the first tile",
        });
        second.set_accessibility_role(ouif::AccessibilityRole::Button);
        second.set_accessibility_label("Second tile");
        root.children(first, second);

        assert(first.can_focus());
        assert(first.accessibility_role() == ouif::AccessibilityRole::Button);
        assert(first.accessibility_label() == "First tile");
        assert(first.accessibility_description() == "Activates the first tile");

        assert(root.event(ouif::KeyEvent { static_cast<std::uint32_t>(ouif::Key::Tab), ouif::KeyAction::Press }));
        assert(first.focused());
        assert(root.event(ouif::KeyEvent { static_cast<std::uint32_t>(ouif::Key::Enter), ouif::KeyAction::Press }));
        assert(first.clicks == 1);

        assert(root.event(ouif::KeyEvent { static_cast<std::uint32_t>(ouif::Key::Tab), ouif::KeyAction::Press }));
        assert(second.focused());
        assert(root.event(ouif::KeyEvent { static_cast<std::uint32_t>(ouif::Key::Space), ouif::KeyAction::Press }));
        assert(second.clicks == 1);

        assert(root.event(ouif::KeyEvent { static_cast<std::uint32_t>(ouif::Key::Tab), ouif::KeyAction::Press, true }));
        assert(first.focused());
        first.set_focusable(false);
        assert(!first.focused());
    }

    {
        ouif::RowLayout root;
        root.set_bounds({ 0.0f, 0.0f, 400.0f, 100.0f });
        root.set_gap(0.0f);

        auto& tile = root.add_child<ouif::Widget>();
        tile.add_class("tile");
        tile.set_name("primary");
        root.set_stylesheet(R"css(
            .tile {
                background: #203040;
                background-hovered: #304050;
                width: 50%;
                height: 40px;
                border-radius: 8px;
                border: #e8edf3 2px;
            }

            #primary:selected {
                background: #506070;
                border: #ffffff 4px;
            }
        )css");

        root.layout({ 400.0f, 100.0f });
        assert(std::fabs(tile.bounds().width - 200.0f) < 0.01f);
        assert(std::fabs(tile.bounds().height - 40.0f) < 0.01f);
        assert(std::fabs(tile.get_style().radius.top_left - 8.0f) < 0.01f);
        assert(std::fabs(tile.get_style().border.width - 2.0f) < 0.01f);
        assert(std::fabs(tile.get_style().border_selected.width - 4.0f) < 0.01f);
        assert(!root.get_stylesheet().empty());
    }

    {
        ouif::Widget widget;
        widget.add_class("tile");
        widget.set_style(ouif::Style().with_background(ouif::Color::hex(0x010203)));
        widget.set_stylesheet(".tile { background: #ffffff; }");
        assert(widget.get_style().background.r < 0.01f);
        widget.join_stylesheet(".tile:hover { background: #111111; }");
        assert(widget.get_stylesheet().find(":hover") != std::string_view::npos);
    }

    {
        ouif::Widget widget;
        widget.add_class("tile");
        widget.set_stylesheet(".tile { background: #ffffff; border: #222222 2px; border-radius: 4px; }");
        widget.set_background(ouif::Color::hex(0x010203));
        widget.set_background_hovered(ouif::Color::hex(0x040506));
        widget.set_border(ouif::Color::hex(0x070809), 3.0f);
        widget.set_radius({ 1.0f, 2.0f, 3.0f, 4.0f });
        widget.set_opacity(2.0f);
        widget.set_margin(ouif::Insets(8.0f));
        widget.set_padding(ouif::Insets(4.0f));
        widget.set_flex(3.0f);

        assert(widget.get_background().r < 0.01f);
        assert(widget.get_background_hovered().g > 0.01f);
        assert(std::fabs(widget.get_border().width - 3.0f) < 0.01f);
        assert(std::fabs(widget.get_radius().bottom_left - 4.0f) < 0.01f);
        assert(std::fabs(widget.get_opacity() - 1.0f) < 0.01f);
        assert(std::fabs(widget.get_margin().left - 8.0f) < 0.01f);
        assert(std::fabs(widget.get_padding().top - 4.0f) < 0.01f);
        assert(std::fabs(widget.get_flex() - 3.0f) < 0.01f);
    }

    {
        ouif::Widget widget;
        widget.set_bounds({ 10.0f, 20.0f, 100.0f, 80.0f });
        widget.set_radius(20.0f);

        assert(widget.hit_test({ 60.0f, 60.0f }));
        assert(widget.hit_test({ 30.0f, 20.0f }));
        assert(!widget.hit_test({ 10.0f, 20.0f }));
        assert(!widget.hit_test({ 109.0f, 20.0f }));

        widget.set_radius({ 0.0f, 20.0f, 0.0f, 20.0f });
        assert(widget.hit_test({ 10.0f, 20.0f }));
        assert(!widget.hit_test({ 109.0f, 20.0f }));
        assert(!widget.hit_test({ 10.0f, 99.0f }));
    }

    {
        ouif::Widget root;
        root.set_bounds({ 0.0f, 0.0f, 200.0f, 120.0f });
        auto& child = root.add_child<HoverTracker>();
        child.set_bounds({ 20.0f, 20.0f, 80.0f, 60.0f });

        root.event(ouif::MouseMoveEvent { { 40.0f, 40.0f }, {} });
        assert(child.hovered());
        assert(root.event(ouif::MouseButtonEvent { { 40.0f, 40.0f }, {}, ouif::MouseButton::Left, true }));
        assert(child.pressed());

        root.event(ouif::MouseEvent { ouif::MouseEventType::Leave, { -10.0f, 40.0f }, {}, ouif::MouseButton::Left });
        assert(!child.hovered());
        assert(!child.pressed());
        assert(child.leaves == 1);

        root.event(ouif::MouseMoveEvent { { 40.0f, 40.0f }, {} });
        assert(child.hovered());
        root.event(ouif::MouseMoveEvent { { 220.0f, 40.0f }, {} });
        assert(!child.hovered());
        assert(child.leaves == 2);
    }

    {
        ouif::Widget widget;
        widget.set_style(ouif::Style().with_background(ouif::Color::hex(0x000000)).with_opacity(1.0f));
        widget.set_transition(0.5f, ouif::Easing::Linear);
        widget.set_background(ouif::Color::hex(0xffffff));
        assert(widget.get_background().r < 0.01f);
        widget.layout({ 100.0f, 100.0f });
        assert(widget.get_background().r > 0.01f && widget.get_background().r < 1.0f);
        assert(widget.get_transition().enabled);

        widget.set_animation({
            .name = "fade",
            .duration = 1.0f,
            .easing = ouif::Easing::Linear,
            .loop = true,
            .keyframes = {
                { 0.0f, ouif::Style().with_opacity(0.25f), ouif::style_property_mask(ouif::StyleProperty::Opacity) },
                { 1.0f, ouif::Style().with_opacity(1.0f), ouif::style_property_mask(ouif::StyleProperty::Opacity) },
            },
        });
        widget.layout({ 100.0f, 100.0f });
        assert(widget.animation_running());
        assert(widget.get_opacity() >= 0.25f && widget.get_opacity() <= 1.0f);
    }

    {
        ouif::Widget widget;
        widget.add_class("motion");
        widget.set_stylesheet(R"css(
            @keyframes breathe {
                from { opacity: 0.20; }
                to { opacity: 1.0; }
            }

            .motion {
                transition: 300ms ease-in-out;
                animation: breathe 1s linear infinite;
            }
        )css");
        assert(widget.get_transition().enabled);
        assert(widget.get_animation().has_value());
        assert(widget.get_animation()->name == "breathe");
        widget.layout({ 100.0f, 100.0f });
        assert(widget.get_opacity() >= 0.20f && widget.get_opacity() <= 1.0f);
    }

    {
        ouif::Widget widget;
        widget.add_class("accented");
        widget.set_stylesheet(R"css(
            .accented {
                border-left: #ff3355 6px;
                border-top: #33dd88 2px;
                border-right: #5599ff 4px;
                border-bottom: #f5c542 8px;
            }
        )css");

        assert(std::fabs(widget.get_border_left().width - 6.0f) < 0.01f);
        assert(std::fabs(widget.get_border_top().width - 2.0f) < 0.01f);
        assert(std::fabs(widget.get_border_right().width - 4.0f) < 0.01f);
        assert(std::fabs(widget.get_border_bottom().width - 8.0f) < 0.01f);

        widget.set_border_left(ouif::Color::hex(0xffffff), 10.0f);
        assert(std::fabs(widget.get_border_left().width - 10.0f) < 0.01f);
    }

    {
        ouif::ColScroll scroller;
        scroller.set_smooth_scroll_enabled(false);
        scroller.set_bounds({ 0.0f, 0.0f, 120.0f, 100.0f });
        scroller.set_gap(10.0f);
        auto& a = scroller.add_child<ouif::Widget>();
        auto& b = scroller.add_child<ouif::Widget>();
        auto& c = scroller.add_child<ouif::Widget>();
        a.set_size({ 100.0f, 60.0f });
        b.set_size({ 100.0f, 60.0f });
        c.set_size({ 100.0f, 60.0f });

        scroller.layout({ 120.0f, 100.0f });
        assert(scroller.clip_content());
        assert(std::fabs(scroller.content_size().height - 200.0f) < 0.01f);
        assert(std::fabs(scroller.max_scroll_offset() - 100.0f) < 0.01f);
        assert(std::fabs(a.bounds().y) < 0.01f);

        assert(scroller.event(ouif::MouseWheelEvent { { 10.0f, 10.0f }, {}, 0.0f, -1.0f }));
        assert(std::fabs(scroller.scroll_offset() - 48.0f) < 0.01f);
        assert(std::fabs(a.bounds().y + 48.0f) < 0.01f);

        scroller.set_scroll_offset(1000.0f);
        scroller.layout({ 120.0f, 100.0f });
        assert(std::fabs(scroller.scroll_offset() - 100.0f) < 0.01f);

        scroller.set_clip_content(false);
        assert(!scroller.clip_content());
    }

    {
        ouif::RowScroll scroller;
        scroller.set_smooth_scroll_enabled(false);
        scroller.set_bounds({ 0.0f, 0.0f, 100.0f, 80.0f });
        auto& a = scroller.add_child<ouif::Widget>();
        auto& b = scroller.add_child<ouif::Widget>();
        a.set_size({ 70.0f, 40.0f });
        b.set_size({ 70.0f, 40.0f });
        scroller.layout({ 100.0f, 80.0f });
        assert(std::fabs(scroller.max_scroll_offset() - 40.0f) < 0.01f);
        assert(scroller.event(ouif::MouseWheelEvent { { 4.0f, 4.0f }, {}, 0.0f, -1.0f }));
        assert(std::fabs(scroller.scroll_offset() - 40.0f) < 0.01f);
        assert(std::fabs(a.bounds().x + 40.0f) < 0.01f);
    }

    {
        ouif::Application app;
        app.register_xml_widget("XmlTile", [](const ouif::XmlElement& element) {
            (void)element;
            return std::make_unique<XmlTile>();
        });

        auto& loaded = app.load_xml_string(R"xml(
            <Window title="XML Test" width="640" height="360" clear_color="#101218">
                <Style>
                    @keyframes xmlPulse { from { opacity: 0.5; } to { opacity: 1.0; } }
                    .tile { background-hovered: #4692c4; }
                </Style>
                <RowLayout id="surface" class="surface" gap="12" alignment="center" policy="fill,fill" transition="200ms ease-out">
                    <XmlTile id="tile_a" class="tile" size="80,40" animation="xmlPulse 1s linear infinite" style="background: #2f6c9c; border: 2px solid #e8edf3;" />
                    <Spacer flex="1" />
                    <Divider orientation="vertical" thickness="2" color="#e8edf3" />
                </RowLayout>
            </Window>
        )xml");

        auto* row = dynamic_cast<ouif::RowLayout*>(&loaded);
        assert(row != nullptr);
        assert(row->name() == "surface");
        assert(row->gap() == 12.0f);
        assert(row->children().size() == 3);
        assert(row->children()[0]->name() == "tile_a");
        assert(row->children()[0]->layout_rules().preferred_size.width == 80.0f);
        assert(std::fabs(row->children()[0]->get_border().width - 2.0f) < 0.01f);
        assert(row->get_transition().enabled);
        assert(row->children()[0]->get_animation().has_value());
        assert(dynamic_cast<ouif::Spacer*>(row->children()[1]) != nullptr);
        auto* divider = dynamic_cast<ouif::Divider*>(row->children()[2]);
        assert(divider != nullptr);
        assert(divider->orientation() == ouif::Orientation::Vertical);
        assert(std::fabs(divider->thickness() - 2.0f) < 0.01f);
    }

    {
        ouif::Application app;
        bool threw = false;
        try {
            app.load_xml_string(R"xml(
                <Window>
                    <Spacer>
                        <Widget />
                    </Spacer>
                </Window>
            )xml");
        } catch (const std::invalid_argument&) {
            threw = true;
        }
        assert(threw);
    }

    {
        ouif::Application app;
        ouif::Widget parent;
        ouif::Widget child;
        parent.add_child(child);
        bool threw = false;
        try {
            app.set_root(child);
        } catch (const std::invalid_argument&) {
            threw = true;
        }
        assert(threw);
    }

    {
        auto config = ouif::WindowConfig()
            .with_title("Tools")
            .with_size(640, 360)
            .with_position(40.0f, 60.0f)
            .with_decorated(false)
            .with_resizable(false)
            .with_always_on_top(true)
            .with_transparent_framebuffer(true)
            .with_theme(ouif::WindowTheme::Dark)
            .with_material(ouif::WindowMaterial::Transparent)
            .with_background("#101218");

        assert(config.title == "Tools");
        assert(config.width == 640);
        assert(config.height == 360);
        assert(config.position.has_value());
        assert(!config.decorated);
        assert(!config.resizable);
        assert(config.always_on_top);
        assert(config.transparent_framebuffer);
        assert(config.theme == ouif::WindowTheme::Dark);
        assert(config.material == ouif::WindowMaterial::Transparent);

        ouif::ApplicationConfig app_config;
        app_config.with_window(config);
        assert(app_config.title == "Tools");
        assert(app_config.width == 640);
        assert(app_config.height == 360);

        ouif::Window detached;
        assert(!detached.valid());
        detached.set_title("Detached");
        detached.set_size(320, 200);
        detached.set_position(12.0f, 24.0f);
        detached.set_decorated(false);
        detached.set_resizable(false);
        detached.set_always_on_top(true);
        detached.set_theme(ouif::WindowTheme::Light);
        detached.set_material(ouif::WindowMaterial::Solid);
        assert(detached.title() == "Detached");
        assert(detached.size().width == 320.0f);
        assert(detached.position().x == 12.0f);
        assert(!detached.decorated());
        assert(!detached.resizable());
        assert(detached.always_on_top());
        assert(detached.theme() == ouif::WindowTheme::Light);
        assert(detached.material() == ouif::WindowMaterial::Solid);
        config.with_owner(detached);
        assert(config.owner == &detached);

        auto dialog = ouif::DialogBuilder()
            .with_title("Confirm")
            .with_size(420, 220)
            .with_modal(true)
            .with_theme(ouif::WindowTheme::Dark)
            .with_material(ouif::WindowMaterial::Solid)
            .with_background("#181c23")
            .with_owner(detached);
        assert(dialog.config().title == "Confirm");
        assert(dialog.config().width == 420);
        assert(dialog.config().modal);
        assert(dialog.config().owner == &detached);
    }

    return 0;
}
