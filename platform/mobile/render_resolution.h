#pragma once

#include <array>
#include <cstddef>

namespace mobile_ui::resolution {

struct Option { int id; int height; const char* label; };
// Stored IDs 0..3 keep their original meaning. Display new lower modes first.
inline constexpr std::array<Option, 6> options{{
    {4, 240, "240p"}, {5, 360, "360p"}, {0, 480, "480p"},
    {1, 720, "720p"}, {2, 1080, "1080p"}, {3, 1440, "1440p"}
}};
inline constexpr int defaultScale = 5; // 360p for new settings files

inline constexpr int sanitize(int id) { return id >= 0 && id <= 5 ? id : defaultScale; }
inline constexpr int idAt(int index) {
    return options[index < 0 ? 0 : index >= (int)options.size() ? options.size() - 1 : (size_t)index].id;
}
inline constexpr int indexOf(int id) {
    for (size_t i = 0; i < options.size(); ++i) if (options[i].id == id) return (int)i;
    return 1;
}
inline constexpr const char* label(int id) { return options[indexOf(id)].label; }
inline constexpr int height(int id, bool batterySaver = false) {
    int value = options[indexOf(id)].height;
    return batterySaver && value > 720 ? 720 : value;
}

} // namespace mobile_ui::resolution
