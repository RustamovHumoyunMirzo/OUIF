#include <OUIF/OUIF.h>

#include <cassert>
#include <memory>

int main()
{
    ouif::Rect rect { 10.0f, 20.0f, 100.0f, 50.0f };
    assert(rect.contains({ 10.0f, 20.0f }));
    assert(rect.contains({ 110.0f, 70.0f }));
    assert(!rect.contains({ 111.0f, 70.0f }));

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
    assert(root.children().front()->bounds().width == 400.0f);
    assert(root.children().front()->bounds().height == 300.0f);

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
        .border = { "#647084", 1.0f },
    };
    assert(aggregate_style.border.width == 1.0f);

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

    return 0;
}
