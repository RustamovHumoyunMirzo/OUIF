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

class CustomCssWidget : public ouif::Widget {
public:
    int custom_value = 0;
};

class CountingEffect : public ouif::Effect {
public:
    explicit CountingEffect(float value)
        : value_(value)
    {
    }

    float value_ = 0.0f;
};

class DragSource : public ouif::Widget {
public:
    int starts = 0;
    int moves = 0;
    int ends = 0;

protected:
    bool on_drag_start(const ouif::DragEvent&) override
    {
        ++starts;
        return true;
    }

    bool on_drag_move(const ouif::DragEvent&) override
    {
        ++moves;
        return true;
    }

    bool on_drag_end(const ouif::DragEvent&) override
    {
        ++ends;
        return true;
    }
};

class DropTarget : public ouif::Widget {
public:
    int drops = 0;

protected:
    bool on_drop(const ouif::DragEvent&) override
    {
        ++drops;
        return true;
    }
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
    auto short_hex = ouif::Color::from_hex("#abc");
    assert(short_hex.has_value());
    assert(short_hex->r > 0.65f && short_hex->b > 0.78f);
    auto transparent = ouif::Color::named("transparent");
    assert(transparent.has_value());
    assert(transparent->a == 0.0f);

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
        ouif::Widget parent;
        auto* child = new ouif::Widget();
        auto& adopted = parent.add_child(child);
        assert(&adopted == child);
        assert(parent.children().size() == 1);
        assert(child->parent() == &parent);
        parent.clear_children();
        assert(parent.children().empty());
    }

    {
        ouif::Widget parent;
        ouif::Widget child;
        parent.add_child(child);
        bool threw = false;
        try {
            parent.add_child(&child);
        } catch (const std::invalid_argument&) {
            threw = true;
        }
        assert(threw);
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
    style.with_background(ouif::Gradient::Linear(90.0f, { { 0.0f, ouif::Color::named("black").value() }, { 1.0f, ouif::Color::named("white").value() } }));
    assert(style.background_gradient.has_value());

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

    {
        ouif::Widget widget;
        widget.add_class("gradient");
        widget.set_stylesheet(R"css(
            .gradient {
                background: gradient(linear 45deg (0% #000) (100% white));
                color: gradient(linear 90deg (0% #f00) (100% #00f));
            }
        )css");
        assert(widget.get_style().background_gradient.has_value());
        assert(widget.get_style().foreground_gradient.has_value());
        widget.set_background(ouif::Gradient::Linear(180.0f, { { 0.0f, ouif::Color::hex(0x101218) }, { 1.0f, ouif::Color::hex(0x4692c4) } }));
        assert(widget.get_background_gradient().has_value());
        assert(std::fabs(widget.get_background_gradient()->angle_degrees - 180.0f) < 0.01f);
    }

    {
        ouif::Input input("abc");
        assert(input.can_focus());
        assert(input.text() == "abc");
        input.set_caret(3);
        assert(input.event(ouif::TextInputEvent { 'd' }));
        assert(input.text() == "abcd");
        assert(input.event(ouif::KeyEvent { static_cast<std::uint32_t>(ouif::Key::Left), ouif::KeyAction::Press }));
        assert(input.caret() == 3);
        assert(input.event(ouif::KeyEvent { static_cast<std::uint32_t>(ouif::Key::Backspace), ouif::KeyAction::Press }));
        assert(input.text() == "abd");
        assert(input.event(ouif::KeyEvent { static_cast<std::uint32_t>(ouif::Key::A), ouif::KeyAction::Press, false, true }));
        assert(input.has_selection());
        assert(input.event(ouif::KeyEvent { static_cast<std::uint32_t>(ouif::Key::C), ouif::KeyAction::Press, false, true }));
        assert(ouif::Input::clipboard_text() == "abd");
        input.insert_text("ok");
        assert(input.text() == "ok");
        ouif::Input::set_clipboard_text(" pasted");
        input.paste_clipboard();
        assert(input.text() == "ok pasted");
        input.select(2, input.text().size());
        input.cut_selection();
        assert(input.text() == "ok");
        assert(ouif::Input::clipboard_text() == " pasted");
        input.set_composition_text("...");
        assert(input.composition_text() == "...");
        input.clear_composition();
        assert(input.composition_text().empty());
    }

    {
        ouif::Widget root;
        root.set_bounds({ 0.0f, 0.0f, 320.0f, 120.0f });
        auto& input = root.add_child<ouif::Input>("live");
        input.set_bounds({ 10.0f, 10.0f, 220.0f, 40.0f });
        assert(root.event(ouif::MouseButtonEvent { { 230.0f, 20.0f }, {}, ouif::MouseButton::Left, true }));
        assert(input.focused());
        assert(root.event(ouif::TextInputEvent { '!' }));
        assert(input.text() == "live!");
        assert(root.event(ouif::KeyEvent { static_cast<std::uint32_t>(ouif::Key::Backspace), ouif::KeyAction::Press }));
        assert(input.text() == "live");
        assert(root.event(ouif::KeyEvent { static_cast<std::uint32_t>('A'), ouif::KeyAction::Press }));
        assert(input.text() == "livea");
        assert(root.event(ouif::TextInputEvent { 'a' }));
        assert(input.text() == "livea");
    }

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

    {
        ouif::Widget container;
        container.set_bounds({ 0.0f, 0.0f, 300.0f, 200.0f });
        container.set_padding(10.0f);
        container.set_child_gravity(ouif::Gravity::BottomRight());
        auto& child = container.add_child<ouif::Widget>();
        child.set_size({ 80.0f, 40.0f });
        child.set_margin({ 4.0f, 6.0f, 8.0f, 10.0f });
        container.layout({ 300.0f, 200.0f });
        assert(std::fabs(child.bounds().x - 202.0f) < 0.01f);
        assert(std::fabs(child.bounds().y - 140.0f) < 0.01f);
        assert(container.child_gravity().horizontal == ouif::HorizontalGravity::Right);
        assert(container.child_gravity().vertical == ouif::VerticalGravity::Bottom);
    }

    {
        ouif::RowLayout gravity_row;
        gravity_row.set_bounds({ 0.0f, 0.0f, 300.0f, 100.0f });
        gravity_row.set_gravity(ouif::Gravity::BottomRight());
        auto& child = gravity_row.add_child<ouif::Widget>();
        child.set_size({ 60.0f, 30.0f });
        gravity_row.layout({ 300.0f, 100.0f });
        assert(std::fabs(child.bounds().x - 240.0f) < 0.01f);
        assert(std::fabs(child.bounds().y - 70.0f) < 0.01f);
        assert(gravity_row.gravity().horizontal == ouif::HorizontalGravity::Right);
        assert(gravity_row.gravity().vertical == ouif::VerticalGravity::Bottom);
    }

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
        ouif::RowLayout row;
        row.set_bounds({ 0.0f, 0.0f, 300.0f, 80.0f });
        auto& left = row.add_child<ouif::Widget>();
        auto& overlay = row.add_child<ouif::Overlay>();
        auto& right = row.add_child<ouif::Widget>();
        left.set_size({ 100.0f, 40.0f });
        right.set_size({ 100.0f, 40.0f });
        overlay.set_bounds({ 10.0f, 10.0f, 40.0f, 40.0f });
        overlay.set_z_index(20);
        row.layout({ 300.0f, 80.0f });

        assert(std::fabs(right.bounds().x - 100.0f) < 0.01f);
        assert(std::fabs(overlay.bounds().x - 10.0f) < 0.01f);
        assert(overlay.overlay());
    }

    {
        ouif::Spacer spacer;
        ouif::Divider divider;
        ouif::Label label("Hello");
        ouif::Image image("assets/logo.png");
        ouif::VectorImage vector_image("assets/logo.svg");
        ouif::Widget child;
        assert(!spacer.accepts_children());
        assert(!divider.accepts_children());
        assert(!label.accepts_children());
        assert(!image.accepts_children());
        assert(!vector_image.accepts_children());
        assert(label.text() == "Hello");
        label.set_font_size(18.0f);
        label.set_text_color("#e8edf3");
        label.set_text_align(ouif::TextAlign::Center);
        label.set_text_overflow(ouif::TextOverflow::Wrap);
        assert(label.font_size() == 18.0f);
        assert(label.text_color().r > 0.8f);
        assert(label.text_align() == ouif::TextAlign::Center);
        assert(label.text_overflow() == ouif::TextOverflow::Wrap);
        label.set_translation(4.0f, 8.0f);
        label.set_scale(1.5f, 0.75f);
        label.set_rotation(12.0f);
        label.set_transform_origin(0.25f, 0.75f);
        assert(label.get_transform().translate_x == 4.0f);
        assert(label.get_transform().scale_x == 1.5f);
        assert(label.get_transform().rotation_degrees == 12.0f);
        assert(label.get_transform().origin_y == 0.75f);
        assert(image.source().string().find("logo") != std::string::npos);
        image.set_fit(ouif::ImageFit::Cover);
        image.set_filter(ouif::ImageFilter::Nearest);
        image.set_tint("#e8edf3");
        assert(image.fit() == ouif::ImageFit::Cover);
        assert(image.filter() == ouif::ImageFilter::Nearest);
        assert(image.tint().r > 0.8f);
        image.set_resource(9102);
        assert(image.resource().has_value());
        image.clear_resource();
        assert(!image.resource().has_value());
        assert(vector_image.source().string().find("logo") != std::string::npos);
        vector_image.set_svg("<svg viewBox=\"0 0 10 10\"><rect width=\"10\" height=\"10\" /></svg>");
        assert(vector_image.svg().find("rect") != std::string_view::npos);
        vector_image.set_fit(ouif::ImageFit::Cover);
        vector_image.set_tint("#e8edf3");
        assert(vector_image.fit() == ouif::ImageFit::Cover);
        assert(vector_image.tint().r > 0.8f);
        vector_image.set_resource(9104);
        assert(vector_image.resource().has_value());
        vector_image.clear_resource();
        assert(!vector_image.resource().has_value());

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

        bool label_threw = false;
        try {
            label.add_child<ouif::Widget>();
        } catch (const std::invalid_argument&) {
            label_threw = true;
        }
        assert(label_threw);

        bool image_threw = false;
        try {
            image.add_child<ouif::Widget>();
        } catch (const std::invalid_argument&) {
            image_threw = true;
        }
        assert(image_threw);

        bool vector_image_threw = false;
        try {
            vector_image.add_child<ouif::Widget>();
        } catch (const std::invalid_argument&) {
            vector_image_threw = true;
        }
        assert(vector_image_threw);

        ouif::Widget container;
        container.add_child<ouif::Widget>();
        assert(container.accepts_children());
        assert(container.children().size() == 1);
        container.set_accepts_children(false);
        assert(!container.accepts_children());
        assert(container.children().empty());
    }

    {
        ouif::Renderer renderer;
        assert(renderer.default_font_family() == "OUIF Sans");
        renderer.set_default_font_family("Example Narrow");
        assert(renderer.default_font_family() == "Example Narrow");
        assert(renderer.load_font("Example Narrow", OUIF_TEST_FONT_PATH));
        auto measured = renderer.measure_text("Font Test", ouif::TextStyle()
                .with_font_family("Example Narrow")
                .with_font_size(24.0f));
        assert(measured.width > 0.0f);
        assert(measured.height > 0.0f);
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
        ouif::Widget root;
        root.add_class("shell");
        root.set_stylesheet(".shell { gravity: right bottom; }");
        assert(root.child_gravity().horizontal == ouif::HorizontalGravity::Right);
        assert(root.child_gravity().vertical == ouif::VerticalGravity::Bottom);
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
        ouif::Widget widget;
        widget.set_bounds({ 100.0f, 100.0f, 100.0f, 100.0f });
        widget.set_rotation(45.0f);
        assert(widget.hit_test({ 150.0f, 150.0f }));
        assert(!widget.hit_test({ 100.0f, 100.0f }));
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
        ouif::Widget root;
        root.set_bounds({ 0.0f, 0.0f, 200.0f, 100.0f });
        auto& bottom = root.add_child<HoverTracker>();
        auto& top = root.add_child<ouif::Widget>();
        bottom.set_bounds({ 20.0f, 20.0f, 80.0f, 60.0f });
        top.set_bounds({ 20.0f, 20.0f, 80.0f, 60.0f });
        top.set_z_index(10);
        top.set_enabled(false);

        assert(!root.event(ouif::MouseMoveEvent { { 40.0f, 40.0f }, {} }));
        assert(!bottom.hovered());

        top.set_ghost(true);
        assert(!root.event(ouif::MouseMoveEvent { { 40.0f, 40.0f }, {} }));
        assert(bottom.hovered());

        bottom.set_visible(false);
        root.layout({ 200.0f, 100.0f });
        assert(!bottom.visible());
        assert(!bottom.hit_test({ 40.0f, 40.0f }));
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
        ouif::Label label("CSS Label");
        label.add_class("title");
        label.set_stylesheet(R"css(
            .title {
                color: #e8edf3;
                font-size: 22px;
                font-family: OUIF;
                text-align: center;
                text-overflow: wrap;
                transform: translate(4px, 8px) rotate(15deg) scale(1.25, 0.75);
            }
        )css");

        assert(label.text() == "CSS Label");
        assert(label.get_foreground().r > 0.8f);
        assert(label.font_size() == 22.0f);
        assert(label.font_family() == "OUIF");
        assert(label.text_align() == ouif::TextAlign::Center);
        assert(label.text_overflow() == ouif::TextOverflow::Wrap);
        assert(label.get_transform().translate_x == 4.0f);
        assert(label.get_transform().translate_y == 8.0f);
        assert(label.get_transform().rotation_degrees == 15.0f);
        assert(label.get_transform().scale_x == 1.25f);
        assert(label.get_transform().scale_y == 0.75f);
    }

    {
        ouif::Image image;
        image.add_class("preview");
        ouif::define_var("image-path", "assets/photo.png");
        ouif::define_var("image-resource-id", "9103");
        ouif::define_var("preview-tint", "#e8edf3");
        image.set_stylesheet(R"css(
            .preview {
                image-source: res(def(image-resource-id));
                image-fit: cover;
                image-filter: pixelated;
                image-tint: var("preview-tint");
            }
        )css");

        assert(image.resource().has_value());
        assert(*image.resource() == 9103);
        assert(image.fit() == ouif::ImageFit::Cover);
        assert(image.filter() == ouif::ImageFilter::Nearest);
        assert(image.tint().r > 0.8f);

        image.set_stylesheet(R"css(
            .preview {
                image-source: path(def(image-path));
            }
        )css");
        assert(image.source().string().find("photo") != std::string::npos);

        assert(ouif::get_var("image-path").has_value());
        assert(ouif::edit_var("image-path", "assets/edited.png"));
        assert(ouif::get_var("image-path")->find("edited") != std::string::npos);
        assert(ouif::delete_var("image-path"));
        ouif::clear_vars();
    }

    {
        ouif::Image parent;
        ouif::Image child;
        parent.set_accepts_children(true);
        parent.set_fit(ouif::ImageFit::Cover);
        parent.set_filter(ouif::ImageFilter::Nearest);
        parent.set_tint("#e8edf3");
        parent.add_child(child);

        child.set_fit(ouif::inherit);
        child.set_filter(ouif::inherit);
        child.set_tint(ouif::inherit);

        assert(child.fit() == ouif::ImageFit::Cover);
        assert(child.filter() == ouif::ImageFilter::Nearest);
        assert(child.tint().r > 0.8f);
    }

    {
        ouif::VectorImage image;
        image.add_class("mark");
        ouif::define_var("svg-resource-id", "9105");
        ouif::define_var("svg-tint", "#e8edf3");
        image.set_stylesheet(R"css(
            .mark {
                svg-source: res(def(svg-resource-id));
                svg-fit: cover;
                svg-tint: var("svg-tint");
            }
        )css");

        assert(image.resource().has_value());
        assert(*image.resource() == 9105);
        assert(image.fit() == ouif::ImageFit::Cover);
        assert(image.tint().r > 0.8f);

        image.set_stylesheet(R"css(
            .mark {
                vector-svg: "<svg viewBox=\"0 0 12 12\"><defs><linearGradient id=\"g\"><stop stop-color=\"#fff\"/><stop stop-color=\"#000\"/></linearGradient><clipPath id=\"c\"><rect width=\"12\" height=\"12\"/></clipPath><symbol id=\"s\"><circle cx=\"6\" cy=\"6\" r=\"6\" fill=\"url(#g)\" clip-path=\"url(#c)\"/></symbol><filter id=\"b\"><feGaussianBlur stdDeviation=\"2\"/></filter></defs><use href=\"#s\" filter=\"url(#b)\"/></svg>";
            }
        )css");
        assert(image.svg().find("linearGradient") != std::string_view::npos);
        assert(image.svg().find("clipPath") != std::string_view::npos);
        assert(image.svg().find("feGaussianBlur") != std::string_view::npos);
        ouif::clear_vars();
    }

    {
        ouif::VectorImage parent;
        ouif::VectorImage child;
        parent.set_accepts_children(true);
        parent.set_fit(ouif::ImageFit::Cover);
        parent.set_tint("#e8edf3");
        parent.add_child(child);

        child.set_fit(ouif::inherit);
        child.set_tint(ouif::inherit);

        assert(child.fit() == ouif::ImageFit::Cover);
        assert(child.tint().r > 0.8f);
    }

    {
        ouif::Widget parent;
        ouif::Widget child;
        parent.set_background("#203040");
        parent.set_foreground("#e8edf3");
        parent.set_border_left("#ff3355", 6.0f);
        parent.set_radius(12.0f);
        parent.set_padding(ouif::Insets(10.0f));
        parent.set_child_gravity(ouif::Gravity::BottomRight());
        parent.set_clip_content(false);
        parent.add_child(child);

        child.set_background(ouif::inherit);
        child.set_foreground(ouif::inherit);
        child.set_border_left(ouif::inherit);
        child.set_radius(ouif::inherit);
        child.set_padding(ouif::inherit);
        child.set_child_gravity(ouif::inherit);
        child.set_clip_content(ouif::inherit);

        assert(child.get_background().b > 0.2f);
        assert(child.get_foreground().r > 0.8f);
        assert(std::fabs(child.get_border_left().width - 6.0f) < 0.01f);
        assert(std::fabs(child.get_radius().top_left - 12.0f) < 0.01f);
        assert(std::fabs(child.get_padding().left - 10.0f) < 0.01f);
        assert(child.child_gravity().horizontal == ouif::HorizontalGravity::Right);
        assert(!child.clip_content());
    }

    {
        ouif::Widget root;
        root.add_class("parent");
        root.set_child_gravity(ouif::Gravity::BottomRight());
        auto& child = root.add_child<ouif::Widget>();
        child.add_class("child");
        root.set_stylesheet(R"css(
            .parent {
                background: #203040;
                color: #e8edf3;
                padding: 12px;
            }

            .child {
                background: inherit;
                color: inherit;
                padding: inherit;
                gravity: inherit;
            }
        )css");

        assert(child.get_background().b > 0.2f);
        assert(child.get_foreground().r > 0.8f);
        assert(std::fabs(child.get_padding().left - 12.0f) < 0.01f);
        assert(child.child_gravity().horizontal == ouif::HorizontalGravity::Right);
        assert(child.child_gravity().vertical == ouif::VerticalGravity::Bottom);
    }

    {
        ouif::Label parent("Parent");
        ouif::Label child("Child");
        parent.set_accepts_children(true);
        parent.set_font_family("Example Narrow");
        parent.set_font_size(21.0f);
        parent.set_text_color("#e8edf3");
        parent.set_text_align(ouif::TextAlign::Center);
        parent.set_text_overflow(ouif::TextOverflow::Wrap);
        parent.add_child(child);

        child.set_font_family(ouif::inherit);
        child.set_font_size(ouif::inherit);
        child.set_text_color(ouif::inherit);
        child.set_text_align(ouif::inherit);
        child.set_text_overflow(ouif::inherit);

        assert(child.font_family() == "Example Narrow");
        assert(child.font_size() == 21.0f);
        assert(child.text_color().r > 0.8f);
        assert(child.text_align() == ouif::TextAlign::Center);
        assert(child.text_overflow() == ouif::TextOverflow::Wrap);
    }

    {
        CustomCssWidget widget;
        widget.add_class("custom");
        ouif::Widget::register_css_property("debug-number", [](ouif::Widget& target, const ouif::CssDeclaration& declaration) {
            auto* custom = dynamic_cast<CustomCssWidget*>(&target);
            if (custom == nullptr || declaration.values.empty() || !declaration.values.front().number) {
                return false;
            }
            custom->custom_value = static_cast<int>(*declaration.values.front().number);
            return true;
        });

        widget.set_stylesheet(".custom { debug-number: 42; }");
        assert(widget.custom_value == 42);
        assert(ouif::Widget::unregister_css_property("debug-number"));
        ouif::Widget::clear_css_properties();
    }

    {
        ouif::Widget widget;
        widget.add_layer_effect("blur", { 6.0f });
        assert(widget.layer_effects().size() == 1);
        auto* layer_blur = dynamic_cast<ouif::BlurEffect*>(widget.layer_effects().front().get());
        assert(layer_blur != nullptr);
        assert(layer_blur->type() == ouif::BlurType::Gaussian);
        widget.clear_layer_effects();
        assert(widget.layer_effects().empty());

        ouif::BlurEffect dual_blur(9.0f, ouif::BlurType::DualKawase);
        assert(dual_blur.radius() == 9.0f);
        assert(dual_blur.type() == ouif::BlurType::DualKawase);
        dual_blur.set_type(ouif::BlurType::Gaussian);
        assert(dual_blur.type() == ouif::BlurType::Gaussian);

        ouif::Widget::register_effect("counting", [](const ouif::EffectParameters& parameters) {
            const float value = parameters.numbers.empty() ? 0.0f : parameters.numbers.front();
            return std::make_shared<CountingEffect>(value);
        });

        widget.add_class("effected");
        widget.set_stylesheet(R"css(
            .effected {
                layer-effect: counting(7px);
                backdrop-effect: blur(4px, 1);
            }
        )css");
        assert(widget.layer_effects().empty());
        assert(widget.backdrop_effects().empty());
        assert(widget.stylesheet_layer_effects().size() == 1);
        assert(widget.stylesheet_backdrop_effects().size() == 1);
        auto* stylesheet_blur = dynamic_cast<ouif::BlurEffect*>(widget.stylesheet_backdrop_effects().front().get());
        assert(stylesheet_blur != nullptr);
        assert(stylesheet_blur->type() == ouif::BlurType::DualKawase);
        assert(ouif::Widget::unregister_effect("counting"));
    }

    {
        ouif::Widget widget;
        widget.add_class("layered");
        widget.set_stylesheet(R"css(
            .layered {
                z-index: 24;
                ghost: true;
                overlay: true;
                draggable: true;
                accepts-drop: true;
                visibility: false;
            }
        )css");
        assert(widget.z_index() == 24);
        assert(widget.ghost());
        assert(widget.overlay());
        assert(widget.draggable());
        assert(widget.accepts_drop());
        assert(!widget.visible());
    }

    {
        DragSource source;
        source.set_bounds({ 0.0f, 0.0f, 80.0f, 80.0f });
        source.set_draggable(true);
        assert(source.draggable());
        assert(source.event(ouif::MouseButtonEvent { { 10.0f, 10.0f }, {}, ouif::MouseButton::Left, true }));
        assert(source.dragging());
        assert(source.starts == 1);
        source.event(ouif::MouseMoveEvent { { 30.0f, 30.0f }, {} });
        assert(source.moves == 1);
        assert(source.event(ouif::MouseButtonEvent { { 30.0f, 30.0f }, {}, ouif::MouseButton::Left, false }));
        assert(!source.dragging());
        assert(source.ends == 1);

        DropTarget target;
        target.set_accepts_drop(true);
        assert(target.accepts_drop());
    }

    {
        static const std::uint8_t css[] = ".resource { background: #ffffff; }";
        ouif::Resources::register_bytes(9101, css, sizeof(css) - 1U);
        const auto loaded = ouif::Resources::load(9101);
        assert(loaded.has_value());
        assert(loaded->as_string().find(".resource") != std::string::npos);
        assert(ouif::Resources::contains(9101));
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
                <RowLayout id="surface" class="surface" gap="12" gravity="right bottom" policy="fill,fill" transition="200ms ease-out">
                    <Label id="caption" text="Hello Text" font-size="18" text-color="#e8edf3" transform="translate(2px, 3px) scale(1.1)" />
                    <Input id="entry" value="Typed" placeholder="Name" font-size="16" text-color="#ffffff" />
                    <Image id="preview" src="assets/panel.png" fit="cover" filter="nearest" tint="#e8edf3" />
                    <VectorImage id="logo" svg="&lt;svg viewBox=&quot;0 0 16 16&quot;&gt;&lt;rect width=&quot;16&quot; height=&quot;16&quot; fill=&quot;#4692c4&quot; /&gt;&lt;/svg&gt;" fit="contain" tint="#ffffff" />
                    <XmlTile id="tile_a" class="tile" size="80,40" animation="xmlPulse 1s linear infinite" style="background: #2f6c9c; border: 2px solid #e8edf3;" />
                    <Overlay id="overlay" z-index="12" ghost="true" draggable="true" accepts-drop="true" layer-effect="blur(4px)" />
                    <Spacer flex="1" />
                    <Divider orientation="vertical" thickness="2" color="#e8edf3" />
                </RowLayout>
            </Window>
        )xml");

        auto* row = dynamic_cast<ouif::RowLayout*>(&loaded);
        assert(row != nullptr);
        assert(row->name() == "surface");
        assert(row->gap() == 12.0f);
        assert(row->gravity().horizontal == ouif::HorizontalGravity::Right);
        assert(row->gravity().vertical == ouif::VerticalGravity::Bottom);
        assert(row->children().size() == 8);
        auto* caption = dynamic_cast<ouif::Label*>(row->children()[0]);
        assert(caption != nullptr);
        assert(caption->text() == "Hello Text");
        assert(caption->font_size() == 18.0f);
        assert(caption->get_transform().translate_x == 2.0f);
        assert(caption->get_transform().scale_x == 1.1f);
        auto* entry = dynamic_cast<ouif::Input*>(row->children()[1]);
        assert(entry != nullptr);
        assert(entry->text() == "Typed");
        assert(entry->placeholder() == "Name");
        assert(entry->font_size() == 16.0f);
        assert(entry->text_color().r > 0.9f);
        auto* preview = dynamic_cast<ouif::Image*>(row->children()[2]);
        assert(preview != nullptr);
        assert(preview->source().string().find("panel") != std::string::npos);
        assert(preview->fit() == ouif::ImageFit::Cover);
        assert(preview->filter() == ouif::ImageFilter::Nearest);
        assert(preview->tint().r > 0.8f);
        auto* logo = dynamic_cast<ouif::VectorImage*>(row->children()[3]);
        assert(logo != nullptr);
        assert(logo->svg().find("rect") != std::string_view::npos);
        assert(logo->fit() == ouif::ImageFit::Contain);
        assert(row->children()[4]->name() == "tile_a");
        assert(row->children()[4]->layout_rules().preferred_size.width == 80.0f);
        assert(std::fabs(row->children()[4]->get_border().width - 2.0f) < 0.01f);
        assert(row->get_transition().enabled);
        assert(row->children()[4]->get_animation().has_value());
        auto* overlay = dynamic_cast<ouif::Overlay*>(row->children()[5]);
        assert(overlay != nullptr);
        assert(overlay->z_index() == 12);
        assert(overlay->ghost());
        assert(overlay->draggable());
        assert(overlay->accepts_drop());
        assert(overlay->stylesheet_layer_effects().size() == 1);
        assert(dynamic_cast<ouif::Spacer*>(row->children()[6]) != nullptr);
        auto* divider = dynamic_cast<ouif::Divider*>(row->children()[7]);
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
