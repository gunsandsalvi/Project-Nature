#include "device.hpp"

#include <unistd.h>

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <optional>
#include <set>
#include <sstream>
#include <string>
#include <system_error>
#include <vector>

#include <godot_cpp/core/class_db.hpp>

#include "frames.hpp"
#include "kd/bench/code.hpp"
#include "kd/bench/scenarios.hpp"
#include "kd/num/digest.hpp"
#include "kd/num/fenv.hpp"
#include "kd/proof/proof.hpp"
#include "kd/run/workers.hpp"
#include "trace.hpp"

#if defined(__ANDROID__)
#include <android/system_health.h>
#include <android/thermal.h>
#include <dlfcn.h>
#include <vulkan/vulkan.h>

#include <atomic>
#endif

namespace kd::view {

namespace {

/// A small text file's first line, or an empty string.
std::string first_line(const char* path) {
    std::FILE* f = std::fopen(path, "r");
    if (f == nullptr) {
        return {};
    }
    char line[256] = {};
    const bool read = std::fgets(line, sizeof line, f) != nullptr;
    std::fclose(f);
    if (!read) {
        return {};
    }
    std::string s(line);
    while (!s.empty() && (s.back() == '\n' || s.back() == '\r')) {
        s.pop_back();
    }
    return s;
}

struct Core {
    int cpu = 0;
    std::int64_t max_khz = -1;
};

std::vector<Core> read_cores() {
    std::vector<Core> out;
    for (int cpu = 0; cpu < 64; ++cpu) {
        char path[96];
        std::snprintf(path, sizeof path, "/sys/devices/system/cpu/cpu%d/online", cpu);
        char freq[96];
        std::snprintf(freq, sizeof freq, "/sys/devices/system/cpu/cpu%d/cpufreq/cpuinfo_max_freq", cpu);
        const std::string online = first_line(path);
        const std::string khz = first_line(freq);
        if (online.empty() && khz.empty() && cpu > 0) {
            // cpu0 often has no "online" file; past it, a core with neither file does not exist
            char dir[96];
            std::snprintf(dir, sizeof dir, "/sys/devices/system/cpu/cpu%d/topology/core_id", cpu);
            if (first_line(dir).empty()) {
                break;
            }
        }
        Core c;
        c.cpu = cpu;
        if (!khz.empty()) {
            c.max_khz = std::strtoll(khz.c_str(), nullptr, 10);
        }
        out.push_back(c);
    }
    return out;
}

#if defined(__ANDROID__)
// What the phone last pushed to its headroom listener (Android 16): the headroom now, its forecast and how far ahead,
// and how many times it has called. Written on the system's binder threads, read on the main thread.
struct Pushed {
    std::atomic<float> headroom{-1.0F};
    std::atomic<float> forecast{-1.0F};
    std::atomic<int> seconds{0};
    std::atomic<std::int64_t> calls{0};
};

Pushed& pushed() {
    static Pushed p;
    return p;
}

void on_headroom(void* /*data*/, float headroom, float forecast, int seconds,
                 const AThermalHeadroomThreshold* /*thresholds*/, size_t /*count*/) {
    Pushed& p = pushed();
    p.headroom.store(headroom);
    p.forecast.store(forecast);
    p.seconds.store(seconds);
    p.calls.fetch_add(1);
}

AThermalManager* thermal_manager() {
    static AThermalManager* manager = [] {
        AThermalManager* m = nullptr;
        if (__builtin_available(android 30, *)) {
            m = AThermal_acquireManager();
        }
        if (m != nullptr) {
            if (__builtin_available(android 36, *)) {
                AThermal_registerThermalHeadroomListener(m, on_headroom, nullptr);
            }
        }
        return m;
    }();
    return manager;
}

// The headroom at which the phone's light, moderate and severe throttling begin (Android 15), read once: the array
// Android returns is the manager's own before Android 16 and the caller's after, so it is never freed, a few bytes
// kept for the app's life rather than a free that is wrong on one of them.
struct Thresholds {
    float light = -1.0F;
    float moderate = -1.0F;
    float severe = -1.0F;
};

const Thresholds& thresholds(AThermalManager* manager) {
    static const Thresholds read = [manager] {
        Thresholds t;
        if (__builtin_available(android 35, *)) {
            const AThermalHeadroomThreshold* list = nullptr;
            size_t count = 0;
            if (AThermal_getThermalHeadroomThresholds(manager, &list, &count) == 0 && list != nullptr) {
                for (size_t i = 0; i < count; ++i) {
                    if (list[i].thermalStatus == ATHERMAL_STATUS_LIGHT) {
                        t.light = list[i].headroom;
                    } else if (list[i].thermalStatus == ATHERMAL_STATUS_MODERATE) {
                        t.moderate = list[i].headroom;
                    } else if (list[i].thermalStatus == ATHERMAL_STATUS_SEVERE) {
                        t.severe = list[i].headroom;
                    }
                }
            }
        }
        return t;
    }();
    return read;
}
#endif

// A number from a file of the phone's power supply, or nothing where the system hides it.
std::optional<double> power_supply(const char* name) {
    char path[96];
    std::snprintf(path, sizeof path, "/sys/class/power_supply/battery/%s", name);
    const std::string line = first_line(path);
    if (line.empty()) {
        return std::nullopt;
    }
    char* end = nullptr;
    const double v = std::strtod(line.c_str(), &end);
    return end != line.c_str() ? std::optional<double>(v) : std::nullopt;
}

}  // namespace

void KdDevice::_bind_methods() {
    using godot::ClassDB;
    using godot::D_METHOD;
    ClassDB::bind_method(D_METHOD("cores"), &KdDevice::cores);
    ClassDB::bind_method(D_METHOD("middle_cores"), &KdDevice::middle_cores);
    ClassDB::bind_method(D_METHOD("thread_check"), &KdDevice::thread_check);
    ClassDB::bind_method(D_METHOD("thermal"), &KdDevice::thermal);
    ClassDB::bind_method(D_METHOD("gpu_headroom"), &KdDevice::gpu_headroom);
    ClassDB::bind_method(D_METHOD("battery_supply"), &KdDevice::battery_supply);
    ClassDB::bind_method(D_METHOD("shading_rates"), &KdDevice::shading_rates);
    ClassDB::bind_method(D_METHOD("storage"), &KdDevice::storage);
    ClassDB::bind_method(D_METHOD("proof_suites"), &KdDevice::proof_suites);
    ClassDB::bind_method(D_METHOD("proof", "suite", "threads"), &KdDevice::proof);
    ClassDB::bind_method(D_METHOD("built_with"), &KdDevice::built_with);
    ClassDB::bind_method(D_METHOD("clocks"), &KdDevice::clocks);
    ClassDB::bind_method(D_METHOD("thread_times"), &KdDevice::thread_times);
    ClassDB::bind_method(D_METHOD("memory"), &KdDevice::memory);
    ClassDB::bind_method(D_METHOD("trace_begin", "name"), &KdDevice::trace_begin);
    ClassDB::bind_method(D_METHOD("trace_end"), &KdDevice::trace_end);
    ClassDB::bind_method(D_METHOD("frames_reset", "period_ms", "refresh_hz"), &KdDevice::frames_reset);
    ClassDB::bind_method(D_METHOD("frames"), &KdDevice::frames);
    ClassDB::bind_method(D_METHOD("bench_scenarios"), &KdDevice::bench_scenarios);
    ClassDB::bind_method(D_METHOD("bench_code", "values"), &KdDevice::bench_code);
    ClassDB::bind_method(D_METHOD("bench_read", "code"), &KdDevice::bench_read);
}

godot::Array KdDevice::cores() const {
    godot::Array out;
    for (const Core& c : read_cores()) {
        godot::Dictionary d;
        d["cpu"] = c.cpu;
        d["max_khz"] = c.max_khz;
        out.push_back(d);
    }
    return out;
}

godot::PackedInt32Array KdDevice::middle_cores() const {
    const std::vector<Core> all = read_cores();
    std::set<std::int64_t> clocks;
    for (const Core& c : all) {
        if (c.max_khz > 0) {
            clocks.insert(c.max_khz);
        }
    }
    godot::PackedInt32Array out;
    if (clocks.size() < 3) {
        return out;
    }
    const std::int64_t lowest = *clocks.begin();
    const std::int64_t highest = *clocks.rbegin();
    for (const Core& c : all) {
        if (c.max_khz > lowest && c.max_khz < highest) {
            out.push_back(c.cpu);
        }
    }
    return out;
}

godot::Dictionary KdDevice::thread_check() const {
    godot::Dictionary d;
    d["main_thread"] = num::to_hex(num::fenv_read()).c_str();
    run::Workers workers(1, "kd-check");
    const std::uint64_t inherited = workers.inherited_fenv().front();
    std::uint64_t in_work = 0;
    workers.for_each(1, [&](std::size_t) { in_work = num::fenv_read(); });
    d["inherited"] = num::to_hex(inherited).c_str();
    d["inherited_default"] = num::fenv_is_default(inherited);
    d["in_work"] = num::to_hex(in_work).c_str();
    d["default_in_work"] = num::fenv_is_default(in_work);
    d["stack_mib"] = static_cast<std::int64_t>(workers.stack_sizes().front() >> 20);
    return d;
}

godot::Dictionary KdDevice::thermal() const {
    godot::Dictionary d;
    d["available"] = false;
#if defined(__ANDROID__)
    AThermalManager* manager = thermal_manager();
    if (manager == nullptr) {
        return d;
    }
    if (__builtin_available(android 31, *)) {
        d["available"] = true;
        d["headroom"] = AThermal_getThermalHeadroom(manager, 0);
        d["forecast_10s"] = AThermal_getThermalHeadroom(manager, 10);
        d["status"] = static_cast<std::int64_t>(AThermal_getCurrentThermalStatus(manager));
    }
    // the headroom at which each level of throttling begins, where the phone gives them (A3.9)
    const Thresholds& t = thresholds(manager);
    if (t.light > 0.0F) {
        d["light"] = t.light;
    }
    if (t.moderate > 0.0F) {
        d["moderate"] = t.moderate;
    }
    if (t.severe > 0.0F) {
        d["severe"] = t.severe;
    }
    const Pushed& p = pushed();
    d["listener_calls"] = p.calls.load();
    if (p.calls.load() > 0) {
        d["listener_headroom"] = p.headroom.load();
        d["listener_forecast"] = p.forecast.load();
        d["listener_seconds"] = p.seconds.load();
    }
#endif
    return d;
}

godot::Dictionary KdDevice::gpu_headroom() const {
    godot::Dictionary d;
    d["available"] = false;
#if defined(__ANDROID__)
    if (__builtin_available(android 36, *)) {
        float headroom = -1.0F;
        if (ASystemHealth_getGpuHeadroom(nullptr, &headroom) == 0) {
            d["available"] = true;
            d["headroom"] = headroom;
        }
        std::int64_t interval = 0;
        if (ASystemHealth_getGpuHeadroomMinIntervalMillis(&interval) == 0) {
            d["min_interval_ms"] = interval;
        }
    }
#endif
    return d;
}

godot::Dictionary KdDevice::shading_rates() const {
    godot::Dictionary d;
    d["available"] = false;
#if defined(__ANDROID__)
    // a Vulkan instance of the app's own, beside Godot's, asked only what the driver offers
    void* vulkan = dlopen("libvulkan.so", RTLD_NOW | RTLD_LOCAL);
    if (vulkan == nullptr) {
        return d;
    }
    const auto get_proc = reinterpret_cast<PFN_vkGetInstanceProcAddr>(dlsym(vulkan, "vkGetInstanceProcAddr"));
    const auto create = get_proc == nullptr
                            ? nullptr
                            : reinterpret_cast<PFN_vkCreateInstance>(get_proc(VK_NULL_HANDLE, "vkCreateInstance"));
    VkInstance instance = VK_NULL_HANDLE;
    VkApplicationInfo app{};
    app.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app.pApplicationName = "kindling-probe";
    app.apiVersion = VK_API_VERSION_1_1;
    VkInstanceCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    info.pApplicationInfo = &app;
    if (create == nullptr || create(&info, nullptr, &instance) != VK_SUCCESS) {
        return d;
    }
    const auto destroy = reinterpret_cast<PFN_vkDestroyInstance>(get_proc(instance, "vkDestroyInstance"));
    const auto devices_of =
        reinterpret_cast<PFN_vkEnumeratePhysicalDevices>(get_proc(instance, "vkEnumeratePhysicalDevices"));
    const auto extensions_of = reinterpret_cast<PFN_vkEnumerateDeviceExtensionProperties>(
        get_proc(instance, "vkEnumerateDeviceExtensionProperties"));
    const auto features_of =
        reinterpret_cast<PFN_vkGetPhysicalDeviceFeatures2>(get_proc(instance, "vkGetPhysicalDeviceFeatures2"));
    const auto rates_of = reinterpret_cast<PFN_vkGetPhysicalDeviceFragmentShadingRatesKHR>(
        get_proc(instance, "vkGetPhysicalDeviceFragmentShadingRatesKHR"));
    std::uint32_t count = 0;
    std::vector<VkPhysicalDevice> devices;
    if (devices_of != nullptr && devices_of(instance, &count, nullptr) == VK_SUCCESS && count > 0) {
        devices.resize(count);
        devices_of(instance, &count, devices.data());
    }
    if (!devices.empty() && extensions_of != nullptr && features_of != nullptr) {
        const VkPhysicalDevice device = devices.front();
        std::uint32_t n = 0;
        extensions_of(device, nullptr, &n, nullptr);
        std::vector<VkExtensionProperties> extensions(n);
        extensions_of(device, nullptr, &n, extensions.data());
        bool offered = false;
        for (const VkExtensionProperties& e : extensions) {
            offered = offered || std::strcmp(e.extensionName, VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME) == 0;
        }
        d["available"] = true;
        d["extension"] = offered;
        if (offered) {
            VkPhysicalDeviceFragmentShadingRateFeaturesKHR rate{};
            rate.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_FEATURES_KHR;
            VkPhysicalDeviceFeatures2 features{};
            features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
            features.pNext = &rate;
            features_of(device, &features);
            d["per_draw"] = rate.pipelineFragmentShadingRate == VK_TRUE;
            d["per_primitive"] = rate.primitiveFragmentShadingRate == VK_TRUE;
            d["from_picture"] = rate.attachmentFragmentShadingRate == VK_TRUE;
            if (rates_of != nullptr) {
                std::uint32_t m = 0;
                rates_of(device, &m, nullptr);
                std::vector<VkPhysicalDeviceFragmentShadingRateKHR> rates(m);
                for (VkPhysicalDeviceFragmentShadingRateKHR& r : rates) {
                    r.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_KHR;
                    r.pNext = nullptr;
                }
                rates_of(device, &m, rates.data());
                godot::PackedStringArray names;
                for (const VkPhysicalDeviceFragmentShadingRateKHR& r : rates) {
                    names.append(godot::String::num_int64(r.fragmentSize.width) + godot::String("x") +
                                 godot::String::num_int64(r.fragmentSize.height));
                }
                d["rates"] = names;
            }
        }
    }
    if (destroy != nullptr) {
        destroy(instance, nullptr);
    }
#endif
    return d;
}

godot::Dictionary KdDevice::battery_supply() const {
    godot::Dictionary d;
    // the kernel's own figures, in microvolts and microamperes, where the system lets an app read them
    if (const std::optional<double> uv = power_supply("voltage_now"); uv && *uv > 0.0) {
        d["voltage_v"] = *uv / 1.0e6;
    }
    if (const std::optional<double> ua = power_supply("current_now"); ua && *ua != 0.0) {
        d["current_a"] = (*ua < 0.0 ? -*ua : *ua) / 1.0e6;
    }
    return d;
}

godot::String KdDevice::storage() const {
    std::FILE* f = std::fopen("/proc/self/mounts", "r");
    if (f == nullptr) {
        return {};
    }
#if defined(__ANDROID__)
    const char* wanted = " /data ";
#else
    const char* wanted = " / ";
#endif
    std::string found;
    char line[1024];
    while (std::fgets(line, sizeof line, f) != nullptr) {
        std::string s(line);
        if (s.find(wanted) != std::string::npos) {
            while (!s.empty() && s.back() == '\n') {
                s.pop_back();
            }
            found = s;
        }
    }
    std::fclose(f);
    return found.c_str();
}

godot::PackedStringArray KdDevice::proof_suites() const {
    godot::PackedStringArray out;
    for (const auto& s : proof::suites()) {
        out.push_back(godot::String(std::string(s.name).c_str()));
    }
    return out;
}

godot::String KdDevice::proof(const godot::String& suite, int threads) const {
    if (threads < 1 || threads > 16) {
        return {};
    }
    run::Workers workers(threads);
    const std::string name = suite.utf8().get_data();
    return proof::run(name, workers).c_str();
}

godot::String KdDevice::built_with() const {
    std::string s;
#if defined(__clang__)
    s = std::string("clang ") + __clang_version__;
#elif defined(__GNUC__)
    s = std::string("gcc ") + __VERSION__;
#endif
#if defined(_LIBCPP_VERSION)
    s += ", libc++ " + std::to_string(_LIBCPP_VERSION);
#elif defined(__GLIBCXX__)
    s += ", libstdc++ " + std::to_string(__GLIBCXX__);
#endif
    return s.c_str();
}

godot::PackedInt64Array KdDevice::clocks() const {
    godot::PackedInt64Array out;
    for (const Core& c : read_cores()) {
        char path[96];
        std::snprintf(path, sizeof path, "/sys/devices/system/cpu/cpu%d/cpufreq/scaling_cur_freq", c.cpu);
        const std::string khz = first_line(path);
        out.push_back(khz.empty() ? -1 : std::strtoll(khz.c_str(), nullptr, 10));
    }
    return out;
}

godot::Dictionary KdDevice::thread_times() const {
    godot::Dictionary out;
    const long ticks = sysconf(_SC_CLK_TCK);
    std::error_code error;
    for (std::filesystem::directory_iterator it("/proc/self/task", error);
         !error && it != std::filesystem::directory_iterator(); it.increment(error)) {
        const std::string name = first_line((it->path() / "comm").c_str());
        if (name.rfind("kd-", 0) != 0) {
            continue;
        }
        std::ifstream stat(it->path() / "stat");
        const std::string text((std::istreambuf_iterator<char>(stat)), std::istreambuf_iterator<char>());
        // after the name in brackets: the state is the 3rd field, user time the 14th and system time the 15th
        const std::size_t close = text.rfind(')');
        if (close == std::string::npos || close + 2 >= text.size()) {
            continue;
        }
        std::istringstream rest(text.substr(close + 2));
        std::string field;
        long long used = 0;
        for (int i = 3; i <= 15 && rest >> field; ++i) {
            if (i >= 14) {
                used += std::atoll(field.c_str());
            }
        }
        const godot::String key(name.c_str());
        const double before = out.has(key) ? static_cast<double>(out[key]) : 0.0;
        out[key] = before + static_cast<double>(used) / static_cast<double>(ticks > 0 ? ticks : 100);
    }
    return out;
}

godot::Dictionary KdDevice::memory() const {
    godot::Dictionary out;
    std::ifstream status("/proc/self/status");
    std::string line;
    while (std::getline(status, line)) {
        for (const auto& [key, field] : {std::pair{"VmRSS:", "resident_mb"}, std::pair{"VmHWM:", "peak_mb"}}) {
            if (line.rfind(key, 0) == 0) {
                out[field] = static_cast<double>(std::atoll(line.c_str() + std::strlen(key))) / 1024.0;
            }
        }
    }
    return out;
}

void KdDevice::trace_begin(const godot::String& name) const {
    view::trace_begin(name.utf8().get_data());
}

void KdDevice::trace_end() const {
    view::trace_end();
}

void KdDevice::frames_reset(double period_ms, double refresh_hz) const {
    frame_meter().reset(period_ms, refresh_hz);
}

godot::Dictionary KdDevice::frames() const {
    const FrameMeter::Stats& s = frame_meter().stats();
    godot::Dictionary out;
    out["frames"] = s.frames;
    out["on_time"] = s.on_time;
    out["stalls"] = s.stalls;
    out["late"] = s.late;
    out["slowest_ms"] = s.slowest_ms;
    return out;
}

godot::Array KdDevice::bench_scenarios() const {
    godot::Array out;
    for (const bench::Scenario& s : bench::scenarios()) {
        godot::Dictionary d;
        d["name"] = godot::String(std::string(s.name).c_str());
        d["about"] = godot::String::utf8(std::string(s.about).c_str());
        d["ground"] = s.ground == bench::Ground::calendar ? "calendar" : "crowd";
        d["speed"] = godot::String(std::string(s.speed).c_str());
        d["camera"] = s.camera == bench::Camera::tour ? "tour" : "still";
        d["pinned"] = s.pinned;
        d["saves"] = s.saves;
        d["seconds"] = s.seconds;
        d["mark"] = s.mark;
        d["call_at"] = s.call_at;
        d["call_camp"] = s.call_camp;
        d["on_time"] = s.on_time;
        d["slowest"] = s.slowest;
        d["open"] = s.open;
        out.push_back(d);
    }
    return out;
}

godot::String KdDevice::bench_code(const godot::Dictionary& values) const {
    bench::Values v;
    const godot::Array keys = values.keys();
    for (int64_t i = 0; i < keys.size(); ++i) {
        const godot::Variant& value = values[keys[i]];
        if (value.get_type() == godot::Variant::FLOAT || value.get_type() == godot::Variant::INT) {
            v[godot::String(keys[i]).utf8().get_data()] = static_cast<double>(value);
        }
    }
    return bench::encode(v).c_str();
}

godot::Dictionary KdDevice::bench_read(const godot::String& code) const {
    const bench::Read read = bench::decode(code.utf8().get_data());
    godot::Dictionary values;
    for (const auto& [name, value] : read.values) {
        values[godot::String(name.c_str())] = value;
    }
    godot::Dictionary out;
    out["values"] = values;
    out["why"] = godot::String(read.why.c_str());
    return out;
}

}  // namespace kd::view
