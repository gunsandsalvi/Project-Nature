// Output-only work scopes (PLT-04, A19). Clocks belong to the host observer;
// no measurements feed scheduling, choices, chance, saves or logical digests.
#pragma once
#include <cstdint>
namespace kd::world {
enum class Cost : std::uint8_t {
    people,
    camp,
    layers,
    chooser,
    inputs,
    thermal,
    food,
    retention,
    observation,
    evidence,
    count
};
class CostProbe {
public:
    virtual ~CostProbe() = default;
    virtual void enter(Cost kind) noexcept = 0;
    virtual void leave(Cost kind) noexcept = 0;
};
class CostSpan {
public:
    CostSpan(CostProbe* probe, Cost kind) : probe_(probe), kind_(kind) {
        if (probe_) probe_->enter(kind_);
    }
    ~CostSpan() {
        if (probe_) probe_->leave(kind_);
    }
    CostSpan(const CostSpan&) = delete;
    CostSpan& operator=(const CostSpan&) = delete;

private:
    CostProbe* probe_;
    Cost kind_;
};
}  // namespace kd::world
