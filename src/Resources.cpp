#include <OUIF/Resources.h>

#include <unordered_map>

namespace ouif {
namespace {

std::unordered_map<int, ResourceData>& resources()
{
    static std::unordered_map<int, ResourceData> table;
    return table;
}

} // namespace

std::string ResourceData::as_string() const
{
    if (empty()) {
        return {};
    }
    return std::string(reinterpret_cast<const char*>(data), size);
}

void Resources::register_bytes(int id, const std::uint8_t* data, std::size_t size)
{
    if (data == nullptr || size == 0) {
        return;
    }
    resources()[id] = { data, size };
}

std::optional<ResourceData> Resources::load(int id)
{
    const auto found = resources().find(id);
    if (found == resources().end()) {
        return std::nullopt;
    }
    return found->second;
}

bool Resources::contains(int id)
{
    return resources().find(id) != resources().end();
}

} // namespace ouif
