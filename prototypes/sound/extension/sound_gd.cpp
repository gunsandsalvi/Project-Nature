// P14 Sound on the phone (IMPLEMENTATION α0.7c, A16): the sound library (../src) as a Godot extension.
// - SoundMaker makes the base sounds and the murmur's phrases and cries as 16-bit samples for an AudioStreamWAV, from
//   any thread, so talk is made beside the game.
// - SoundLive is an audio stream made as it plays, on the audio thread: the fire by its heat, the wind by its speed.
// - AudioEffectProbe, the last effect on the master bus, passes the sound through untouched and measures the audio
//   thread's own time (the CPU it used) for each block it mixes, against the block's length.
// Pre-production code (research 00).
#include <gdextension_interface.h>

#include <godot_cpp/classes/audio_effect.hpp>
#include <godot_cpp/classes/audio_effect_instance.hpp>
#include <godot_cpp/classes/audio_frame.hpp>
#include <godot_cpp/classes/audio_stream.hpp>
#include <godot_cpp/classes/audio_stream_playback_resampled.hpp>
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/godot.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/packed_byte_array.hpp>
#include <godot_cpp/variant/packed_string_array.hpp>

#include <time.h>

#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstring>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include "sound.hpp"

namespace {

// Samples as 16-bit little-endian bytes, as AudioStreamWAV's data holds them.
godot::PackedByteArray pcm(const std::vector<float>& s) {
    godot::PackedByteArray out;
    out.resize(static_cast<std::int64_t>(s.size() * 2));
    std::uint8_t* w = out.ptrw();
    for (std::size_t i = 0; i < s.size(); ++i) {
        const auto v = static_cast<std::int16_t>(std::lround(std::clamp(s[i], -1.0f, 1.0f) * 32767.0f));
        std::memcpy(w + i * 2, &v, 2);
    }
    return out;
}

std::vector<float> samples(const godot::PackedByteArray& bytes) {
    std::vector<float> out(static_cast<std::size_t>(bytes.size() / 2));
    const std::uint8_t* r = bytes.ptr();
    for (std::size_t i = 0; i < out.size(); ++i) {
        std::int16_t v = 0;
        std::memcpy(&v, r + i * 2, 2);
        out[i] = static_cast<float>(v) / 32767.0f;
    }
    return out;
}

float number(const godot::Dictionary& d, const char* key, float fallback) {
    return static_cast<float>(static_cast<double>(d.get(key, fallback)));
}

std::uint32_t seed_of(std::int64_t seed) {
    return static_cast<std::uint32_t>(seed & 0xFFFFFFFF);
}

class SoundMaker : public godot::RefCounted {
    GDCLASS(SoundMaker, godot::RefCounted)

protected:
    static void _bind_methods() {
        godot::ClassDB::bind_method(godot::D_METHOD("make", "base", "hard", "size", "wet", "seed"), &SoundMaker::make);
        godot::ClassDB::bind_method(godot::D_METHOD("loops", "base"), &SoundMaker::loops);
        godot::ClassDB::bind_method(godot::D_METHOD("rate"), &SoundMaker::rate);
        godot::ClassDB::bind_method(godot::D_METHOD("load_bank", "voice", "bytes"), &SoundMaker::load_bank);
        godot::ClassDB::bind_method(godot::D_METHOD("bank_pitch", "voice"), &SoundMaker::bank_pitch);
        godot::ClassDB::bind_method(godot::D_METHOD("language", "seed", "count"), &SoundMaker::language);
        godot::ClassDB::bind_method(godot::D_METHOD("phrase", "voice", "syllables", "speaker", "feeling", "seed"),
                                    &SoundMaker::phrase);
        godot::ClassDB::bind_method(godot::D_METHOD("cry", "voice", "kind", "speaker", "seed"), &SoundMaker::cry);
        godot::ClassDB::bind_method(godot::D_METHOD("blend", "sounds", "seconds", "per_second", "seed"),
                                    &SoundMaker::blend);
        godot::ClassDB::bind_method(godot::D_METHOD("rhythm", "hits", "seconds", "beat", "seed"), &SoundMaker::rhythm);
        godot::ClassDB::bind_method(godot::D_METHOD("measure", "pcm"), &SoundMaker::measure);
    }

public:
    // A base sound (sound::Base's order) as 16-bit samples at rate().
    godot::PackedByteArray make(int base, double hard, double size, double wet, std::int64_t seed) const {
        if (base < 0 || base >= sound::kBases) {
            return {};
        }
        sound::Stuff stuff;
        stuff.hard = static_cast<float>(hard);
        stuff.size = static_cast<float>(size);
        stuff.wet = static_cast<float>(wet);
        return pcm(sound::make(static_cast<sound::Base>(base), stuff, seed_of(seed)));
    }

    bool loops(int base) const {
        return base >= 0 && base < sound::kBases && sound::loops(static_cast<sound::Base>(base));
    }

    int rate() const { return sound::kRate; }

    // The murmur's bank for a base voice: 0 the woman's, 1 the man's. Loaded once, before any talk is made.
    bool load_bank(int voice, const godot::PackedByteArray& bytes) {
        if (voice < 0 || voice > 1) {
            return false;
        }
        auto bank =
            std::make_shared<sound::Bank>(sound::read_bank(bytes.ptr(), static_cast<std::size_t>(bytes.size())));
        if (bank->syllables.empty()) {
            return false;
        }
        banks_[static_cast<std::size_t>(voice)] = std::move(bank);
        return true;
    }

    double bank_pitch(int voice) const {
        const sound::Bank* b = bank(voice);
        return b != nullptr ? static_cast<double>(b->pitch) : 0.0;
    }

    godot::PackedStringArray language(std::int64_t seed, int count) const {
        godot::PackedStringArray out;
        if (const sound::Bank* b = bank(0); b != nullptr) {
            for (const std::string& s : sound::language(*b, seed_of(seed), count)) {
                out.push_back(godot::String(s.c_str()));
            }
        }
        return out;
    }

    // A phrase of talk: the speaker's shift {pitch, formants, breath, tremor} and the feeling's {tempo, range, loud,
    // fall, lift}, each a number, any left out at its plain value.
    godot::PackedByteArray phrase(int voice, const godot::PackedStringArray& syllables,
                                  const godot::Dictionary& speaker, const godot::Dictionary& feeling,
                                  std::int64_t seed) const {
        const sound::Bank* b = bank(voice);
        if (b == nullptr) {
            return {};
        }
        std::vector<std::string> words;
        for (std::int64_t i = 0; i < syllables.size(); ++i) {
            words.emplace_back(syllables[i].utf8().get_data());
        }
        sound::Feeling f;
        f.tempo = number(feeling, "tempo", f.tempo);
        f.range = number(feeling, "range", f.range);
        f.loud = number(feeling, "loud", f.loud);
        f.fall = number(feeling, "fall", f.fall);
        f.lift = number(feeling, "lift", f.lift);
        return pcm(sound::phrase(*b, words, speaker_of(speaker), f, seed_of(seed)));
    }

    // A laugh (0), a cry (1), a call (2) or a scream (3).
    godot::PackedByteArray cry(int voice, int kind, const godot::Dictionary& speaker, std::int64_t seed) const {
        const sound::Bank* b = bank(voice);
        if (b == nullptr || kind < 0 || kind > 3) {
            return {};
        }
        return pcm(sound::cry(*b, static_cast<sound::Cry>(kind), speaker_of(speaker), seed_of(seed)));
    }

    // A loop of `seconds` holding copies of the sounds given, about per_second a second: a share's hum.
    godot::PackedByteArray blend(const godot::Array& sounds, double seconds, double per_second,
                                 std::int64_t seed) const {
        std::vector<std::vector<float>> all;
        for (std::int64_t i = 0; i < sounds.size(); ++i) {
            all.push_back(samples(sounds[i]));
        }
        return pcm(sound::blend(all, static_cast<float>(seconds), static_cast<float>(per_second), seed_of(seed)));
    }

    // A drummer's loop of `seconds` from the hits given, a beat each `beat` seconds.
    godot::PackedByteArray rhythm(const godot::Array& hits, double seconds, double beat, std::int64_t seed) const {
        std::vector<std::vector<float>> all;
        for (std::int64_t i = 0; i < hits.size(); ++i) {
            all.push_back(samples(hits[i]));
        }
        return pcm(sound::rhythm(all, static_cast<float>(seconds), static_cast<float>(beat), seed_of(seed)));
    }

    // For the tests: a sound's pitch (60 to 600 Hz), loudness, brightness and length in seconds.
    godot::Dictionary measure(const godot::PackedByteArray& bytes) const {
        const std::vector<float> s = samples(bytes);
        godot::Dictionary out;
        out["pitch"] = sound::pitch(s, 60.0f, 600.0f);
        out["loudness"] = sound::loudness(s);
        out["brightness"] = sound::brightness(s);
        out["seconds"] = static_cast<double>(s.size()) / sound::kRate;
        return out;
    }

private:
    const sound::Bank* bank(int voice) const {
        return voice >= 0 && voice <= 1 ? banks_[static_cast<std::size_t>(voice)].get() : nullptr;
    }
    static sound::Speaker speaker_of(const godot::Dictionary& d) {
        sound::Speaker s;
        s.pitch = number(d, "pitch", s.pitch);
        s.formants = number(d, "formants", s.formants);
        s.breath = number(d, "breath", s.breath);
        s.tremor = number(d, "tremor", s.tremor);
        return s;
    }
    std::array<std::shared_ptr<const sound::Bank>, 2> banks_;
};

// --- Made live
// ---------------------------------------------------------------------------------------------------------

class SoundLivePlayback : public godot::AudioStreamPlaybackResampled {
    GDCLASS(SoundLivePlayback, godot::AudioStreamPlaybackResampled)

protected:
    static void _bind_methods() {}

public:
    void setup(sound::Base base, std::uint32_t seed, std::shared_ptr<std::atomic<float>> amount) {
        live_ = std::make_unique<sound::Live>(base, seed, amount->load());
        amount_ = std::move(amount);
    }
    void _start(double /*from_pos*/) override {
        active_ = true;
        begin_resample();
    }
    void _stop() override { active_ = false; }
    bool _is_playing() const override { return active_; }
    int32_t _get_loop_count() const override { return 0; }
    double _get_playback_position() const override { return 0.0; }
    void _seek(double /*position*/) override {}
    float _get_stream_sampling_rate() const override { return static_cast<float>(sound::kRate); }
    // on the audio thread: never waits and never allocates
    int32_t _mix_resampled(godot::AudioFrame* dst, int32_t frames) override {
        if (!active_ || !live_) {
            for (int32_t i = 0; i < frames; ++i) {
                dst[i].left = 0.0f;
                dst[i].right = 0.0f;
            }
            return frames;
        }
        live_->set(amount_->load(std::memory_order_relaxed));
        int32_t done = 0;
        while (done < frames) {
            const int n = std::min(frames - done, static_cast<int32_t>(buffer_.size()));
            live_->render(buffer_.data(), n);
            for (int i = 0; i < n; ++i) {
                dst[done + i].left = buffer_[static_cast<std::size_t>(i)];
                dst[done + i].right = buffer_[static_cast<std::size_t>(i)];
            }
            done += n;
        }
        return frames;
    }

private:
    std::unique_ptr<sound::Live> live_;
    std::shared_ptr<std::atomic<float>> amount_;
    std::array<float, 512> buffer_{};
    bool active_ = false;
};

// The fire (kind 0) or the wind (kind 1), made live; amount is the fire's heat or the wind's speed, 0 to 1, followed
// by every playback as it changes.
class SoundLive : public godot::AudioStream {
    GDCLASS(SoundLive, godot::AudioStream)

protected:
    static void _bind_methods() {
        godot::ClassDB::bind_method(godot::D_METHOD("set_kind", "kind"), &SoundLive::set_kind);
        godot::ClassDB::bind_method(godot::D_METHOD("get_kind"), &SoundLive::get_kind);
        godot::ClassDB::bind_method(godot::D_METHOD("set_seed", "seed"), &SoundLive::set_seed);
        godot::ClassDB::bind_method(godot::D_METHOD("get_seed"), &SoundLive::get_seed);
        godot::ClassDB::bind_method(godot::D_METHOD("set_amount", "amount"), &SoundLive::set_amount);
        godot::ClassDB::bind_method(godot::D_METHOD("get_amount"), &SoundLive::get_amount);
        ADD_PROPERTY(godot::PropertyInfo(godot::Variant::INT, "kind"), "set_kind", "get_kind");
        ADD_PROPERTY(godot::PropertyInfo(godot::Variant::INT, "seed"), "set_seed", "get_seed");
        ADD_PROPERTY(godot::PropertyInfo(godot::Variant::FLOAT, "amount"), "set_amount", "get_amount");
    }

public:
    godot::Ref<godot::AudioStreamPlayback> _instantiate_playback() const override {
        godot::Ref<SoundLivePlayback> playback;
        playback.instantiate();
        playback->setup(kind_ == 1 ? sound::Base::kWind : sound::Base::kFire, seed_of(seed_), amount_);
        return playback;
    }
    godot::String _get_stream_name() const override { return kind_ == 1 ? "wind" : "fire"; }
    double _get_length() const override { return 0.0; }
    bool _is_monophonic() const override { return false; }

    void set_kind(int kind) { kind_ = kind; }
    int get_kind() const { return kind_; }
    void set_seed(std::int64_t seed) { seed_ = seed; }
    std::int64_t get_seed() const { return seed_; }
    void set_amount(double amount) { amount_->store(static_cast<float>(amount), std::memory_order_relaxed); }
    double get_amount() const { return amount_->load(std::memory_order_relaxed); }

private:
    int kind_ = 0;
    std::int64_t seed_ = 1;
    std::shared_ptr<std::atomic<float>> amount_ = std::make_shared<std::atomic<float>>(0.5f);
};

// --- The probe
// ---------------------------------------------------------------------------------------------------------

// What the probe has seen, written on the audio thread and read on the main one, all as atomics so neither waits.
struct ProbeStats {
    static constexpr int kBins = 200;  // each block's share of its length, in steps of 1%, up to 200%
    std::atomic<std::int64_t> blocks{0};
    std::atomic<std::int64_t> used_ns{0};
    std::atomic<std::int64_t> length_ns{0};
    std::atomic<std::int64_t> worst_permille{0};
    std::atomic<std::int64_t> over{0};
    std::atomic<std::int64_t> late{0};
    std::atomic<std::int64_t> threads{0};
    std::atomic<std::int64_t> frames{0};
    std::array<std::atomic<std::int64_t>, kBins> shares{};
    std::atomic<double> rate{48000.0};
    std::atomic<bool> reset{false};

    void clear() {
        blocks = 0;
        used_ns = 0;
        length_ns = 0;
        worst_permille = 0;
        over = 0;
        late = 0;
        threads = 0;
        for (auto& s : shares) {
            s = 0;
        }
    }
};

std::int64_t thread_cpu_ns() {
    timespec ts{};
    clock_gettime(CLOCK_THREAD_CPUTIME_ID, &ts);
    return static_cast<std::int64_t>(ts.tv_sec) * 1000000000 + static_cast<std::int64_t>(ts.tv_nsec);
}

std::int64_t wall_ns() {
    return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now().time_since_epoch())
        .count();
}

class AudioEffectProbeInstance : public godot::AudioEffectInstance {
    GDCLASS(AudioEffectProbeInstance, godot::AudioEffectInstance)

protected:
    static void _bind_methods() {}

public:
    void setup(std::shared_ptr<ProbeStats> stats) { stats_ = std::move(stats); }

    // Called once a block, after every sound and every other effect is mixed: the thread's CPU time since the call
    // before is all it did for this block.
    void _process(const void* src, godot::AudioFrame* dst, int32_t frames) override {
        std::memmove(dst, src, sizeof(godot::AudioFrame) * static_cast<std::size_t>(std::max(frames, 0)));
        const std::int64_t cpu = thread_cpu_ns();
        const std::int64_t wall = wall_ns();
        const std::thread::id thread = std::this_thread::get_id();
        ProbeStats& s = *stats_;
        if (s.reset.exchange(false)) {
            s.clear();
            last_cpu_ = 0;
        }
        if (last_cpu_ != 0 && thread == last_thread_ && frames > 0) {
            const double rate = s.rate.load(std::memory_order_relaxed);
            const auto length = static_cast<std::int64_t>(static_cast<double>(frames) * 1e9 / rate);
            const std::int64_t used = cpu - last_cpu_;
            s.blocks.fetch_add(1, std::memory_order_relaxed);
            s.used_ns.fetch_add(used, std::memory_order_relaxed);
            s.length_ns.fetch_add(length, std::memory_order_relaxed);
            s.frames.store(frames, std::memory_order_relaxed);
            const std::int64_t permille = used * 1000 / std::max<std::int64_t>(length, 1);
            if (permille > s.worst_permille.load(std::memory_order_relaxed)) {
                s.worst_permille.store(permille, std::memory_order_relaxed);
            }
            if (used > length) {
                s.over.fetch_add(1, std::memory_order_relaxed);
            }
            // a block that began more than four blocks' time after the last: the device waited on the mix
            if (wall - last_wall_ > 4 * length) {
                s.late.fetch_add(1, std::memory_order_relaxed);
            }
            const auto bin =
                static_cast<std::size_t>(std::clamp<std::int64_t>(permille / 10, 0, ProbeStats::kBins - 1));
            s.shares[bin].fetch_add(1, std::memory_order_relaxed);
        } else if (last_cpu_ != 0 && thread != last_thread_) {
            s.threads.fetch_add(1, std::memory_order_relaxed);
        }
        last_cpu_ = cpu;
        last_wall_ = wall;
        last_thread_ = thread;
    }
    bool _process_silence() const override { return true; }

private:
    std::shared_ptr<ProbeStats> stats_;
    std::int64_t last_cpu_ = 0;
    std::int64_t last_wall_ = 0;
    std::thread::id last_thread_;
};

class AudioEffectProbe : public godot::AudioEffect {
    GDCLASS(AudioEffectProbe, godot::AudioEffect)

protected:
    static void _bind_methods() {
        godot::ClassDB::bind_method(godot::D_METHOD("set_rate", "rate"), &AudioEffectProbe::set_rate);
        godot::ClassDB::bind_method(godot::D_METHOD("stats"), &AudioEffectProbe::stats);
        godot::ClassDB::bind_method(godot::D_METHOD("reset"), &AudioEffectProbe::reset);
    }

public:
    godot::Ref<godot::AudioEffectInstance> _instantiate() override {
        godot::Ref<AudioEffectProbeInstance> instance;
        instance.instantiate();
        instance->setup(stats_);
        return instance;
    }

    // The mix rate, AudioServer.get_mix_rate(), which sets each block's length.
    void set_rate(double rate) { stats_->rate.store(rate > 0.0 ? rate : 48000.0); }

    // Starts the count again from the next block.
    void reset() { stats_->reset.store(true); }

    // The blocks measured; the audio thread's mean share of their length, the share 99 in 100 stay within and the
    // worst, each as a fraction; the blocks that took longer than their length; the late ones; how often the thread
    // changed; and the frames in a block.
    godot::Dictionary stats() const {
        const ProbeStats& s = *stats_;
        godot::Dictionary out;
        const std::int64_t blocks = s.blocks.load();
        out["blocks"] = blocks;
        out["mean"] = s.length_ns.load() > 0
                          ? static_cast<double>(s.used_ns.load()) / static_cast<double>(s.length_ns.load())
                          : 0.0;
        std::int64_t seen = 0;
        double p99 = 0.0;
        for (int b = 0; b < ProbeStats::kBins; ++b) {
            seen += s.shares[static_cast<std::size_t>(b)].load();
            if (blocks > 0 && seen * 100 >= blocks * 99) {
                p99 = static_cast<double>(b + 1) / 100.0;
                break;
            }
        }
        out["p99"] = p99;
        out["worst"] = static_cast<double>(s.worst_permille.load()) / 1000.0;
        out["over"] = s.over.load();
        out["late"] = s.late.load();
        out["threads"] = s.threads.load();
        out["frames"] = s.frames.load();
        return out;
    }

private:
    std::shared_ptr<ProbeStats> stats_ = std::make_shared<ProbeStats>();
};

void initialize(godot::ModuleInitializationLevel level) {
    if (level == godot::MODULE_INITIALIZATION_LEVEL_SCENE) {
        GDREGISTER_CLASS(SoundMaker);
        GDREGISTER_CLASS(SoundLivePlayback);
        GDREGISTER_CLASS(SoundLive);
        GDREGISTER_CLASS(AudioEffectProbeInstance);
        GDREGISTER_CLASS(AudioEffectProbe);
    }
}

void uninitialize(godot::ModuleInitializationLevel /*level*/) {}

}  // namespace

extern "C" {
GDExtensionBool GDE_EXPORT sound_init(GDExtensionInterfaceGetProcAddress get_proc_address,
                                      GDExtensionClassLibraryPtr library, GDExtensionInitialization* initialization) {
    godot::GDExtensionBinding::InitObject init(get_proc_address, library, initialization);
    init.register_initializer(initialize);
    init.register_terminator(uninitialize);
    init.set_minimum_library_initialization_level(godot::MODULE_INITIALIZATION_LEVEL_SCENE);
    return init.init();
}
}
