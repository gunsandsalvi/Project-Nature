#include "kd/core/check.hpp"

#include <cstdio>
#include <cstdlib>

#if defined(__ANDROID__)
#include <android/set_abort_message.h>
#endif

namespace kd {

void fail(const char* file, int line, const char* message) {
    char text[512];
    std::snprintf(text, sizeof text, "Kindling: %s (%s:%d)", message, file, line);
    std::fprintf(stderr, "%s\n", text);
    std::fflush(stderr);
#if defined(__ANDROID__)
    android_set_abort_message(text);
#endif
    std::abort();
}

}  // namespace kd
