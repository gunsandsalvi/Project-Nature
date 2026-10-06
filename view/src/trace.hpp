// Sections of the phone's System Tracing (A3.9, PLT-04): named spans that a Perfetto trace shows on the thread that
// ran them, so a slow frame can be laid against the world's batches and the benchmark's scenarios. Nothing off
// Android.
#pragma once

#if defined(__ANDROID__)
#include <android/trace.h>
#endif

namespace kd::view {

/// Implements PLT-04: begins a section on this thread; each is ended on the thread that began it, innermost first.
inline void trace_begin(const char* name) {
#if defined(__ANDROID__)
    ATrace_beginSection(name);
#else
    (void)name;
#endif
}

/// Ends this thread's innermost section.
inline void trace_end() {
#if defined(__ANDROID__)
    ATrace_endSection();
#endif
}

/// A section for as long as this lives, on the thread that made it.
class TraceSection {
public:
    explicit TraceSection(const char* name) { trace_begin(name); }
    ~TraceSection() { trace_end(); }
    TraceSection(const TraceSection&) = delete;
    TraceSection& operator=(const TraceSection&) = delete;
    TraceSection(TraceSection&&) = delete;
    TraceSection& operator=(TraceSection&&) = delete;
};

}  // namespace kd::view
