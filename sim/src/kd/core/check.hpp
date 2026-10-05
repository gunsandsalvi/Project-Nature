// Checks that stop the simulation on a broken rule (A2.2): nothing throws, so a check that fails names its file
// and line and aborts, and the phone keeps the message in its crash record.
#pragma once

namespace kd {

/// Stops the program with a message naming where a rule broke. Implements RES-05, see A3.4: a run that breaks the
/// rules of the same bits never carries on with a wrong answer.
[[noreturn]] void fail(const char* file, int line, const char* message);

}  // namespace kd

#define KD_CHECK(condition, message) ((condition) ? static_cast<void>(0) : ::kd::fail(__FILE__, __LINE__, (message)))
