#pragma once

#include <OUIF/Color.h>
#include <OUIF/Export.h>
#include <OUIF/Geometry.h>

#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace ouif {

struct OUIF_API XmlAttribute {
    std::string name;
    std::string value;
};

class OUIF_API XmlElement {
public:
    XmlElement() = default;
    XmlElement(std::string name, std::vector<XmlAttribute> attributes);

    [[nodiscard]] std::string_view name() const noexcept;
    [[nodiscard]] bool has_attribute(std::string_view name) const noexcept;
    [[nodiscard]] std::string_view attribute(std::string_view name, std::string_view fallback = {}) const noexcept;
    [[nodiscard]] std::optional<float> attribute_float(std::string_view name) const noexcept;
    [[nodiscard]] std::optional<bool> attribute_bool(std::string_view name) const noexcept;
    [[nodiscard]] std::optional<Color> attribute_color(std::string_view name) const noexcept;
    [[nodiscard]] std::optional<Size> attribute_size(std::string_view name) const noexcept;
    [[nodiscard]] const std::vector<XmlAttribute>& attributes() const noexcept;

private:
    std::string name_;
    std::vector<XmlAttribute> attributes_;
};

} // namespace ouif
