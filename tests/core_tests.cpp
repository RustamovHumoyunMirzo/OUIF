#include <OUIF/OUIF.h>

#include <cassert>
#include <memory>

int main()
{
    ouif::Rect rect { 10.0f, 20.0f, 100.0f, 50.0f };
    assert(rect.contains({ 10.0f, 20.0f }));
    assert(rect.contains({ 110.0f, 70.0f }));
    assert(!rect.contains({ 111.0f, 70.0f }));

    auto root = std::make_unique<ouif::Widget>();
    root->set_bounds({ 0.0f, 0.0f, 400.0f, 300.0f });
    root->add_child(std::make_unique<ouif::Widget>());
    root->layout({ 400.0f, 300.0f });

    assert(root->children().size() == 1);
    assert(root->children().front()->bounds().width == 400.0f);
    assert(root->children().front()->bounds().height == 300.0f);

    ouif::Style style = ouif::Style()
        .with_background(ouif::Color::rgb(1, 2, 3))
        .with_background_selected(ouif::Color::rgb(4, 5, 6))
        .with_border(ouif::Color::rgb(7, 8, 9), 2.0f)
        .with_border_selected(ouif::Color::rgb(10, 11, 12), 4.0f);
    assert(style.border_width == 2.0f);
    assert(style.border_width_selected == 4.0f);

    ouif::RowLayout row;
    row.set_bounds({ 0.0f, 0.0f, 400.0f, 100.0f });
    row.set_alignment(ouif::Align::Center);
    row.set_gap(20.0f);

    auto first = std::make_unique<ouif::Widget>();
    first->set_size({ 50.0f, 40.0f });
    auto second = std::make_unique<ouif::Widget>();
    second->set_size({ 50.0f, 40.0f });
    row.add_child(std::move(first));
    row.add_child(std::move(second));
    row.layout({ 400.0f, 100.0f });

    assert(row.children()[0]->bounds().x == 140.0f);
    assert(row.children()[1]->bounds().x == 210.0f);

    row.children()[0]->toggle_state(ouif::WidgetState::Selected);
    assert(row.children()[0]->has_state(ouif::WidgetState::Selected));

    return 0;
}
