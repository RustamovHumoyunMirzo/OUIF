#include <OUIF/OUIF.h>

#include <filesystem>
#include <memory>

class ColorTile : public ouif::Widget {
public:
    ColorTile()
    {
        set_keyboard_activation_enabled(true);
        set_accessibility_role(ouif::AccessibilityRole::Button);
    }
};

int main()
{
    ouif::Application app;
    app.register_xml_widget("ColorTile", [](const ouif::XmlElement& element) {
        auto tile = std::make_unique<ColorTile>();
        tile->set_accessibility_label(std::string(element.attribute("id", "Color tile")));
        return tile;
    });

    app.load_xml((std::filesystem::path(OUIF_XML_UI_DIR) / "myui.xml").string());
    return app.run();
}
