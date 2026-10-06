// Helpers shared by the simulation's tests.
#pragma once

#include <csignal>
#include <cstdint>
#include <cstdio>

#include <fcntl.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <unistd.h>

#include "kd/core/check.hpp"
#include "kd/data/catalogue.hpp"
#include "kd/proof/fixture.hpp"

namespace kd::test {

// The proof suites' own catalogue (kd/proof/fixture.hpp), loaded once for every test that builds a world from it.
inline const kd::data::Catalogue& fixture() {
    static const kd::data::Catalogue catalogue = [] {
        kd::data::Catalogue c;
        KD_CHECK(c.load(kd::proof::fixture_files()).empty(), "tests: the proof suites' catalogue loads");
        return c;
    }();
    return catalogue;
}

// Whether running f stops the program, as a failed check does (kd::fail aborts). It runs in a child process, with
// its messages silenced, doctest's crash report off and no core file left behind.
template <typename F>
bool stops(F f) {
    std::fflush(nullptr);
    const pid_t child = fork();
    if (child == 0) {
        const rlimit none{0, 0};
        setrlimit(RLIMIT_CORE, &none);
        std::signal(SIGABRT, SIG_DFL);
        const int null = open("/dev/null", O_WRONLY);
        dup2(null, 1);
        dup2(null, 2);
        f();
        _exit(0);
    }
    int status = 0;
    waitpid(child, &status, 0);
    return WIFSIGNALED(status) && WTERMSIG(status) == SIGABRT;
}

// Turns flushing of tiny numbers to zero on for this thread, as Godot's raycast module does on x86.
inline void set_flush_to_zero() {
#if defined(__x86_64__)
    std::uint32_t mxcsr = 0;
    __asm__ volatile("stmxcsr %0" : "=m"(mxcsr));
    mxcsr |= (1U << 15) | (1U << 6);
    __asm__ volatile("ldmxcsr %0" : : "m"(mxcsr));
#else
    std::uint64_t fpcr = 0;
    __asm__ volatile("mrs %0, fpcr" : "=r"(fpcr));
    fpcr |= std::uint64_t{1} << 24;
    __asm__ volatile("msr fpcr, %0" : : "r"(fpcr));
#endif
}

}  // namespace kd::test
