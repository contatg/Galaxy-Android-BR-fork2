#pragma once
// The Android build force-includes port/compat.h. Host tests only need this
// declaration; its PowerPC/MSL compatibility macros do not apply to glibc.
extern "C" int port_is_title_screen(void);
