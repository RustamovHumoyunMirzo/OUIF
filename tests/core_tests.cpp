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

    return 0;
}
