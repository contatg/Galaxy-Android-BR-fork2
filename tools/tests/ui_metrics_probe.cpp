#include <bits/stdc++.h>
#define private public
#include "mobile/mobile_ui.h"
#undef private
static int64_t nowNs = 1000000000ll, cpuNs = 100000000ll;
static unsigned fileReads = 0;
extern "C" {
int __android_log_print(int, const char*, const char*, ...) { return 0; }
int __system_property_get(const char*, char*) { return 0; }
int port_is_title_screen() { return 0; }
float port_get_game_speed() { return 1.0f; }
FILE* __wrap_fopen(const char*, const char*) { fileReads++; return nullptr; }
int __wrap_clock_gettime(clockid_t id, struct timespec* ts) {
    int64_t value = id == CLOCK_PROCESS_CPUTIME_ID ? cpuNs : nowNs;
    ts->tv_sec = value / 1000000000ll; ts->tv_nsec = value % 1000000000ll;
    return 0;
}
}
int main() {
    auto& ui = mobile_ui::UIOverlay::get();
    PortPadState pad{}; int spin = 0;
    auto step = [&] { nowNs += 600000000ll; cpuNs += 100000000ll; ui.update(pad, spin); };
    ui.settings().perfHudMode = 0;
    step(); step(); assert(fileReads == 0);
    ui.settings().perfHudMode = 1;
    step(); assert(fileReads == 0);
    ui.settings().perfHudMode = 2;
    step(); assert(fileReads > 0); step(); assert(ui.mCpuUsagePercent > 0);
    ui.settings().perfHudMode = 0;
    unsigned previous = fileReads;
    step(); assert(fileReads == previous && ui.mLastHwSampleTime == 0);
    ui.settings().perfHudMode = 2;
    step(); assert(ui.mLastHwSampleTime == nowNs);
    std::puts("PASS: HUD off/FPS do not open metric files, full HUD samples CPU, toggling resets baseline");
}
