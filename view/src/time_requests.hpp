// T2.9a.3: look requests resolve above the accepted Pace loop, never change physical rules (TIM-01, TIM-15).
#pragma once
#include <optional>
#include <vector>
namespace kd::view {
struct TimeAnchor {
    double density = 0.0;
    double rate = 1.0;
};
struct TimeRequest {
    double rate = 1.0;
    const char* source = "zoom";
};
class TimeRequests {
public:
    bool configure(std::vector<TimeAnchor> anchors, double top_density);
    void zoom(double density);
    void manual(double rate);
    void clear_manual();
    void lock(bool on);
    void pause(bool on);
    void skip(std::optional<double> rate);
    void director(std::optional<double> rate);
    void capacity(double rate);
    [[nodiscard]] TimeRequest resolve() const;
    [[nodiscard]] double zoom_rate() const;
    [[nodiscard]] bool locked() const { return locked_; }
    [[nodiscard]] double density() const { return density_; }

private:
    std::vector<TimeAnchor> anchors_;
    double density_ = 64.0;
    double top_density_ = 1.0 / 4096.0;
    double capacity_ = 1.0;
    bool paused_ = false;
    bool locked_ = false;
    double locked_rate_ = 1.0;
    std::optional<double> manual_;
    std::optional<double> skip_;
    std::optional<double> director_;
};
}  // namespace kd::view
