#include "mobile/render_resolution.h"
#include <cassert>
#include <cstring>
#include <cstdio>
int main() {
    using namespace mobile_ui::resolution;
    // Existing settings must keep their meaning after installing the update.
    const int oldHeights[] = {480, 720, 1080, 1440};
    for (int id = 0; id < 4; ++id) {
        assert(height(id) == oldHeights[id]);
        assert(sanitize(id) == id);
    }
    const int sortedHeights[] = {240, 360, 480, 720, 1080, 1440};
    for (int row = 0; row < 6; ++row) {
        int id = idAt(row);
        assert(indexOf(id) == row);
        assert(height(id) == sortedHeights[row]);
        assert(height(id, true) == (sortedHeights[row] > 720 ? 720 : sortedHeights[row]));
    }
    assert(height(defaultScale) == 360);
    assert(std::strcmp(label(idAt(0)), "240p") == 0);
    assert(std::strcmp(label(idAt(1)), "360p") == 0);
    assert(height(sanitize(-1)) == 360);
    assert(height(sanitize(99)) == 360);
    std::puts("PASS: persisted resolution IDs, six menu options and low resolution in battery saver");
}
