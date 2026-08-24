#pragma once

#include <OUIF/Export.h>

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

namespace ouif {

struct OUIF_API ResourceData {
    const std::uint8_t* data = nullptr;
    std::size_t size = 0;

    [[nodiscard]] constexpr bool empty() const noexcept { return data == nullptr || size == 0; }
    [[nodiscard]] std::string as_string() const;
};

class OUIF_API Resources {
public:
    static void register_bytes(int id, const std::uint8_t* data, std::size_t size);
    [[nodiscard]] static std::optional<ResourceData> load(int id);
    [[nodiscard]] static bool contains(int id);
};

} // namespace ouif
