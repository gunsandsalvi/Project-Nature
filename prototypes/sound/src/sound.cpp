// P14 Sound (IMPLEMENTATION α0.7c): the base sounds, the live fire and wind, and the murmur. See sound.hpp.
// Pre-production code (research 00).
#include "sound.hpp"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <initializer_list>
#include <utility>

namespace sound {

namespace {

constexpr float kPi = 3.14159265f;
constexpr float kTwoPi = 6.28318531f;
constexpr float kRateF = static_cast<float>(kRate);
constexpr float kNyquist = kRateF / 2.0f;
constexpr int kHop = 110;  // the bank's steady step where a voice is unvoiced (bank.py's HOP)

float fl(int v) {
    return static_cast<float>(v);
}
float fl(std::size_t v) {
    return static_cast<float>(v);
}
int samples(float seconds) {
    return static_cast<int>(std::lround(seconds * kRateF));
}
int size_of(const std::vector<float>& s) {
    return static_cast<int>(s.size());
}
float clamp01(float v) {
    return std::clamp(v, 0.0f, 1.0f);
}
float lerp(float a, float b, float t) {
    return a + (b - a) * t;
}

// Chance from a seed: splitmix32 spreads the seed, then xorshift32 (Marsaglia, 2003).
class Rng {
public:
    explicit Rng(std::uint32_t seed) {
        std::uint32_t z = seed + 0x9E3779B9u;
        z = (z ^ (z >> 16)) * 0x85EBCA6Bu;
        z = (z ^ (z >> 13)) * 0xC2B2AE35u;
        z ^= z >> 16;
        state_ = z != 0 ? z : 0x6D2B79F5u;
    }
    std::uint32_t next() {
        state_ ^= state_ << 13;
        state_ ^= state_ >> 17;
        state_ ^= state_ << 5;
        return state_;
    }
    float uniform() { return static_cast<float>(next() >> 8) * (1.0f / 16777216.0f); }
    float white() { return uniform() * 2.0f - 1.0f; }
    float range(float a, float b) { return a + (b - a) * uniform(); }
    int below(int n) { return n > 0 ? static_cast<int>(next() % static_cast<std::uint32_t>(n)) : 0; }
    // about normal, mean 0 and spread 1: four uniforms summed
    float gauss() { return (uniform() + uniform() + uniform() + uniform() - 2.0f) * 1.7320508f; }

private:
    std::uint32_t state_;
};

// A two-pole, two-zero filter from the Audio EQ Cookbook (Bristow-Johnson), run in transposed direct form II.
struct Biquad {
    float b0 = 1.0f, b1 = 0.0f, b2 = 0.0f, a1 = 0.0f, a2 = 0.0f;
    float z1 = 0.0f, z2 = 0.0f;

    void set(float nb0, float nb1, float nb2, float a0, float na1, float na2) {
        b0 = nb0 / a0;
        b1 = nb1 / a0;
        b2 = nb2 / a0;
        a1 = na1 / a0;
        a2 = na2 / a0;
    }
    static float omega(float f) { return kTwoPi * std::clamp(f, 10.0f, kNyquist * 0.92f) / kRateF; }
    void lowpass(float f, float q) {
        const float w = omega(f);
        const float c = std::cos(w);
        const float al = std::sin(w) / (2.0f * q);
        set((1.0f - c) / 2.0f, 1.0f - c, (1.0f - c) / 2.0f, 1.0f + al, -2.0f * c, 1.0f - al);
    }
    void highpass(float f, float q) {
        const float w = omega(f);
        const float c = std::cos(w);
        const float al = std::sin(w) / (2.0f * q);
        set((1.0f + c) / 2.0f, -(1.0f + c), (1.0f + c) / 2.0f, 1.0f + al, -2.0f * c, 1.0f - al);
    }
    // a band around f, its peak at 0 dB
    void bandpass(float f, float q) {
        const float w = omega(f);
        const float al = std::sin(w) / (2.0f * q);
        set(al, 0.0f, -al, 1.0f + al, -2.0f * std::cos(w), 1.0f - al);
    }
    float run(float x) {
        const float y = b0 * x + z1;
        z1 = b1 * x - a1 * y + z2;
        z2 = b2 * x - a2 * y;
        return y;
    }
};

// A one-pole low-pass, for gentle dulling and for following a loudness.
struct OnePole {
    float k = 1.0f, y = 0.0f;
    void lowpass(float f) { k = 1.0f - std::exp(-kTwoPi * std::min(f, kNyquist) / kRateF); }
    float run(float x) {
        y += k * (x - y);
        return y;
    }
};

// A ringing mode: a two-pole resonator at f that falls 60 dB in t60 seconds, its ring as loud as amp for a unit tap.
struct Mode {
    float c1 = 0.0f, c2 = 0.0f, g = 0.0f, y1 = 0.0f, y2 = 0.0f;
    void set(float f, float t60, float amp) {
        const float w = kTwoPi * f / kRateF;
        const float r = std::exp(-6.9078f / (std::max(t60, 0.001f) * kRateF));
        c1 = 2.0f * r * std::cos(w);
        c2 = r * r;
        g = amp * std::sin(w);
    }
    float run(float x) {
        const float y = g * x + c1 * y1 - c2 * y2;
        y2 = y1;
        y1 = y;
        return y;
    }
};

// Bubbles, as in a stream or a puddle: each a tone that rises a little as it fades (Minnaert's ringing bubble).
class Bubbles {
public:
    // starts one at about f Hz, lasting about tau seconds
    void start(float f, float tau, float amp) {
        Slot& s = slots_[next_++ % kSlots];
        s.phase = 0.0f;
        s.step = kTwoPi * f / kRateF;
        s.rise = std::exp(0.25f / (tau * kRateF));
        s.amp = amp;
        s.decay = std::exp(-1.0f / (tau * kRateF));
    }
    float run() {
        float y = 0.0f;
        for (Slot& s : slots_) {
            if (s.amp < 1e-4f) {
                continue;
            }
            s.phase += s.step;
            if (s.phase > kTwoPi) {
                s.phase -= kTwoPi;
            }
            s.step = std::min(s.step * s.rise, kPi * 0.9f);
            y += std::sin(s.phase) * s.amp;
            s.amp *= s.decay;
        }
        return y;
    }

private:
    static constexpr int kSlots = 16;
    struct Slot {
        float phase = 0.0f, step = 0.0f, rise = 1.0f, amp = 0.0f, decay = 0.0f;
    };
    Slot slots_[kSlots];
    unsigned next_ = 0;
};

void fade(std::vector<float>& s, int in, int out) {
    const int n = size_of(s);
    for (int i = 0; i < std::min(in, n); ++i) {
        s[static_cast<std::size_t>(i)] *= 0.5f - 0.5f * std::cos(kPi * fl(i) / fl(in));
    }
    for (int i = 0; i < std::min(out, n); ++i) {
        s[static_cast<std::size_t>(n - 1 - i)] *= 0.5f - 0.5f * std::cos(kPi * fl(i) / fl(out));
    }
}

void normalize(std::vector<float>& s, float peak) {
    float top = 0.0f;
    for (const float v : s) {
        top = std::max(top, std::abs(v));
    }
    if (top > 0.0f) {
        for (float& v : s) {
            v *= peak / top;
        }
    }
}

// A sound joined end to start: its last `cross` samples faded into its first, at equal power, as for noise.
std::vector<float> join_loop(const std::vector<float>& s, int cross) {
    const int body = size_of(s) - cross;
    std::vector<float> out(s.begin(), s.begin() + body);
    for (int i = 0; i < cross; ++i) {
        const float t = (fl(i) + 0.5f) / fl(cross);
        const auto at = static_cast<std::size_t>(i);
        out[at] = s[at] * std::sin(t * kPi / 2.0f) +
                  s[static_cast<std::size_t>(body) + static_cast<std::size_t>(i)] * std::cos(t * kPi / 2.0f);
    }
    return out;
}

// A gentle limit: below 0.6 untouched, above it bent towards 1.
float soft(float y) {
    const float a = std::abs(y);
    if (a < 0.6f) {
        return y;
    }
    const float bent = 0.6f + 0.4f * std::tanh((a - 0.6f) / 0.4f);
    return y < 0.0f ? -bent : bent;
}

// --- Work ----------------------------------------------------------------------------------------------------------

// Stone on stone, as in knapping: a short contact, shorter for harder stone, sets the stone's modes ringing; bigger
// stone rings lower and longer; wet stone is damped and dull.
std::vector<float> strike(const Stuff& st, Rng& rng) {
    const float hard = clamp01(st.hard);
    const float size = clamp01(st.size);
    const float wet = clamp01(st.wet);
    const float f0 = 2400.0f * std::exp2((0.5f - size) * 2.4f) * (0.7f + 0.6f * hard) * rng.range(0.9f, 1.1f);
    const float t60 = (0.012f + 0.09f * size) * (0.5f + hard) * (1.0f - 0.7f * wet);
    // a stone's shape is irregular, so its modes are too: each a random step above the last
    Mode modes[8];
    int count = 0;
    float f = f0;
    for (int k = 0; k < 8; ++k) {
        if (f > kNyquist * 0.9f) {
            break;
        }
        const float kk = fl(k + 1);
        const float amp = rng.range(0.4f, 1.0f) * std::pow(kk, -(1.6f - hard)) * std::pow(kk, -1.5f * wet);
        modes[count++].set(f, t60 / (1.0f + 0.6f * fl(k) * (1.2f - hard)) * rng.range(0.8f, 1.2f), amp);
        f *= rng.range(1.25f, 1.6f);
    }
    const int contact = std::max(2, samples(0.0003f + 0.0022f * (1.0f - hard) + 0.001f * wet));
    const int click = samples(0.004f);
    Biquad dull;
    dull.lowpass((2500.0f + 7500.0f * hard) * (1.0f - 0.6f * wet), 0.7f);
    std::vector<float> out(static_cast<std::size_t>(samples(t60 * 1.3f + 0.03f)));
    for (int i = 0; i < size_of(out); ++i) {
        const float force = i < contact ? std::sin(kPi * fl(i) / fl(contact)) : 0.0f;
        float y = 0.0f;
        for (int k = 0; k < count; ++k) {
            y += modes[k].run(force);
        }
        float c = 0.0f;
        if (i < click) {
            const float left = 1.0f - fl(i) / fl(click);
            c = rng.white() * left * left * (0.3f + hard);
        }
        out[static_cast<std::size_t>(i)] = y * 0.5f + dull.run(c);
    }
    fade(out, 0, samples(0.01f));
    normalize(out, 0.8f);
    return out;
}

// A scraper drawn over a hide: one stroke of stick-slip ticks, quicker and brighter on harder stuff, deeper on bigger.
std::vector<float> scrape(const Stuff& st, Rng& rng) {
    const float hard = clamp01(st.hard);
    const float size = clamp01(st.size);
    const float wet = clamp01(st.wet);
    const int n = samples((0.22f + 0.35f * size) * rng.range(0.85f, 1.15f));
    Biquad band;
    band.bandpass((700.0f + 2600.0f * hard) * std::exp2((0.5f - size) * 1.2f) * (1.0f - 0.5f * wet),
                  0.7f + 0.9f * hard);
    OnePole dull;
    dull.lowpass(9000.0f - 6500.0f * wet);
    const float decay = std::exp(-1.0f / ((0.0004f + 0.0012f * (1.0f - hard)) * kRateF));
    float tick = 0.0f;
    std::vector<float> out(static_cast<std::size_t>(n));
    for (int i = 0; i < n; ++i) {
        const float env = std::pow(std::sin(kPi * fl(i) / fl(n)), 0.6f);
        const float rate = (180.0f + 1000.0f * hard) * (0.3f + 0.7f * env);
        if (rng.uniform() < rate / kRateF) {
            tick = rng.range(0.3f, 1.0f);
        }
        const float x = rng.white() * (tick + 0.25f * env);
        tick *= decay;
        out[static_cast<std::size_t>(i)] = dull.run(band.run(x)) * env;
    }
    normalize(out, 0.6f);
    return out;
}

// A stone axe into wood: a thud, the log's body ringing, a crack where dry wood splits, and a chip or two.
std::vector<float> chop(const Stuff& st, Rng& rng) {
    const float hard = clamp01(st.hard);
    const float size = clamp01(st.size);
    const float wet = clamp01(st.wet);
    const float f0 =
        (160.0f + 420.0f * hard) * std::exp2((0.5f - size) * 1.6f) * (1.0f - 0.25f * wet) * rng.range(0.92f, 1.08f);
    const float t60 = (0.05f + 0.25f * size) * (1.0f - 0.6f * wet);
    static constexpr float kRatios[] = {1.0f, 2.31f, 3.89f, 5.2f};
    const float amps[] = {1.0f, 0.6f * (0.4f + hard), 0.35f * hard, 0.2f * hard};
    Mode modes[4];
    for (int k = 0; k < 4; ++k) {
        modes[k].set(f0 * kRatios[k] * rng.range(0.95f, 1.05f), t60 / (1.0f + 0.5f * fl(k)),
                     amps[k] * (1.0f - 0.5f * wet * fl(k) / 3.0f));
    }
    Biquad thud;
    thud.lowpass((250.0f + 300.0f * (1.0f - size)) * (1.0f - 0.3f * wet), 0.8f);
    Biquad crack;
    crack.bandpass(1400.0f + 3600.0f * hard, 1.0f);
    Biquad chip;
    chip.highpass(2500.0f, 0.7f);
    // soaked wood is dull all through
    OnePole sodden;
    sodden.lowpass(8000.0f - 6500.0f * wet);
    const int contact = std::max(2, samples(0.0015f + 0.0025f * (1.0f - hard)));
    const int thud_n = samples(0.018f);
    const int crack_n = samples(rng.range(0.004f, 0.012f));
    int chips[3] = {-1, -1, -1};
    const int chip_count = 1 + rng.below(3);
    for (int k = 0; k < chip_count; ++k) {
        chips[k] = samples(rng.range(0.005f, 0.04f));
    }
    std::vector<float> out(static_cast<std::size_t>(samples(t60 * 1.3f + 0.05f)));
    for (int i = 0; i < size_of(out); ++i) {
        const float force = i < contact ? std::sin(kPi * fl(i) / fl(contact)) : 0.0f;
        float y = 0.0f;
        for (Mode& m : modes) {
            y += m.run(force);
        }
        const float t = i < thud_n ? 1.0f - fl(i) / fl(thud_n) : 0.0f;
        y = y * 0.6f + thud.run(rng.white() * t * t) * 1.2f;
        const float ct = i < crack_n ? 1.0f - fl(i) / fl(crack_n) : 0.0f;
        y += crack.run(rng.white() * ct) * 0.7f * hard * (1.0f - wet);
        float c = 0.0f;
        for (const int at : chips) {
            if (at >= 0 && i >= at && i < at + 22) {
                c += rng.white() * 0.25f;
            }
        }
        out[static_cast<std::size_t>(i)] = sodden.run(y + chip.run(c));
    }
    fade(out, 0, samples(0.02f));
    normalize(out, 0.8f);
    return out;
}

// A footstep: the heel's thump and the toe's, deeper for a heavier walker, with the crunch of what is underfoot, and
// a squelch where it is wet.
std::vector<float> step(const Stuff& st, Rng& rng) {
    const float hard = clamp01(st.hard);
    const float size = clamp01(st.size);
    const float wet = clamp01(st.wet);
    const int toe = samples(rng.range(0.07f, 0.13f) * (1.0f + 0.3f * size));
    const int n = toe + samples(0.15f);
    Biquad thump;
    thump.lowpass((140.0f + 350.0f * hard) * std::exp2((0.5f - size) * 0.8f), 0.9f);
    Biquad crunch;
    crunch.bandpass(2200.0f + 3000.0f * hard, 0.9f);
    Biquad squelch;
    squelch.bandpass(350.0f, 2.0f);
    const float tau = (0.018f + 0.02f * size) * (1.0f + 0.5f * wet);
    const int crunch_n = samples(0.05f);
    float grain = 0.0f;
    std::vector<float> out(static_cast<std::size_t>(n));
    for (int i = 0; i < n; ++i) {
        float push = 0.0f;
        float rough = 0.0f;
        for (const auto& [at, amp] : {std::pair{0, 1.0f}, std::pair{toe, 0.6f}}) {
            if (i >= at) {
                const float t = fl(i - at) / kRateF;
                push += amp * std::min(1.0f, t / 0.003f) * std::exp(-t / tau);
                if (i - at < crunch_n) {
                    rough += amp * (1.0f - fl(i - at) / fl(crunch_n));
                }
            }
        }
        if (rng.uniform() < (300.0f + 2500.0f * hard) * rough / kRateF) {
            grain = rng.range(0.2f, 1.0f);
        }
        float y = thump.run(rng.white() * push) * 3.0f;
        y += crunch.run(rng.white() * grain) * (0.15f + 0.4f * hard) * (1.0f - 0.6f * wet);
        y += squelch.run(rng.white() * push) * wet * 1.5f;
        grain *= 0.9f;
        out[static_cast<std::size_t>(i)] = y;
    }
    fade(out, 0, samples(0.03f));
    normalize(out, 0.7f);
    return out;
}

// --- Fire and wind, made live -------------------------------------------------------------------------------------

// The fire: a low roar that flutters, and crackles and pops, more and bigger the hotter it is.
class Fire {
public:
    explicit Fire(std::uint32_t seed) : rng_(seed) {}
    float next(float heat) {
        if (count_++ % 64 == 0) {
            roar_.lowpass(160.0f + 520.0f * heat, 0.6f);
            if (--flutter_wait_ <= 0) {
                flutter_target_ = rng_.range(0.55f, 1.45f);
                flutter_wait_ = 4 + rng_.below(14);
            }
        }
        flutter_ += (flutter_target_ - flutter_) * 0.002f;
        brown_ = brown_ * 0.995f + rng_.white() * 0.1f;
        float y = roar_.run(brown_) * (0.08f + 0.5f * heat) * flutter_ * 1.6f;
        if (rng_.uniform() < (1.5f + 30.0f * heat * heat) / kRateF) {
            Crackle& c = crackles_[next_++ % kCrackles];
            c.left = samples(rng_.range(0.0005f, 0.004f));
            const float u = rng_.uniform();
            c.amp = 0.1f + 0.9f * u * u * u;
            c.band.bandpass(rng_.range(1500.0f, 6500.0f), 1.2f);
            c.ring_left = 0;
            if (c.amp > 0.6f) {
                c.ring.set(rng_.range(700.0f, 2600.0f), rng_.range(0.01f, 0.04f), c.amp * 0.4f);
                c.ring_left = samples(0.05f);
            }
        }
        for (Crackle& c : crackles_) {
            if (c.left > 0) {
                const float x = rng_.white() * c.amp;
                --c.left;
                y += c.band.run(x) * 1.3f;
                if (c.ring_left > 0) {
                    y += c.ring.run(x);
                }
            } else if (c.ring_left > 0) {
                --c.ring_left;
                y += c.ring.run(0.0f);
            }
        }
        return soft(y * 0.6f);
    }

private:
    static constexpr int kCrackles = 8;
    struct Crackle {
        int left = 0;
        int ring_left = 0;
        float amp = 0.0f;
        Biquad band;
        Mode ring;
    };
    Rng rng_;
    Biquad roar_;
    float brown_ = 0.0f;
    float flutter_ = 1.0f;
    float flutter_target_ = 1.0f;
    int flutter_wait_ = 0;
    unsigned count_ = 0;
    unsigned next_ = 0;
    Crackle crackles_[kCrackles];
};

// The wind: noise in a band that moves with the gusts, a whistle when it is strong, and a low rumble.
class Wind {
public:
    explicit Wind(std::uint32_t seed) : rng_(seed) {
        pink_.lowpass(1200.0f);
        rumble_.lowpass(90.0f);
    }
    float next(float speed) {
        if (count_++ % 32 == 0) {
            if (--gust_wait_ <= 0) {
                gust_target_ = clamp01(rng_.range(0.0f, 1.0f) * (0.5f + 0.7f * speed));
                gust_wait_ = samples(rng_.range(0.6f, 3.0f)) / 32;
            }
            band_.bandpass((250.0f + 750.0f * speed) * (0.7f + 0.6f * gust_), 0.8f + 0.6f * gust_);
            whistle_.bandpass(800.0f + 900.0f * gust_ * speed, 14.0f);
        }
        gust_ += (gust_target_ - gust_) / (0.7f * kRateF);
        const float w = rng_.white();
        const float p = pink_.run(w);
        float y = band_.run(p * 2.0f + w * 0.3f) * (0.15f + 0.85f * speed) * (0.15f + 0.85f * gust_);
        y += whistle_.run(w) * speed * speed * gust_ * 0.5f;
        y += rumble_.run(w) * speed * 0.6f;
        return soft(y * 0.8f);
    }

private:
    Rng rng_;
    OnePole pink_;
    OnePole rumble_;
    Biquad band_;
    Biquad whistle_;
    float gust_ = 0.5f;
    float gust_target_ = 0.5f;
    int gust_wait_ = 0;
    unsigned count_ = 0;
};

// --- Water and weather -----------------------------------------------------------------------------------------------

// A stream: bubbles over a low rush, more bubbles over rocks (harder), deeper and fuller when it is bigger.
std::vector<float> river(const Stuff& st, Rng& rng) {
    const float hard = clamp01(st.hard);
    const float size = clamp01(st.size);
    const int cross = samples(0.4f);
    const int n = samples(6.0f) + cross;
    Bubbles bubbles;
    Biquad rush;
    rush.lowpass(200.0f + 400.0f * (1.0f - size), 0.7f);
    Biquad hiss;
    hiss.bandpass(2500.0f, 0.5f);
    float brown = 0.0f;
    const float rate = 70.0f + 260.0f * hard + 120.0f * size;
    std::vector<float> out(static_cast<std::size_t>(n));
    for (int i = 0; i < n; ++i) {
        if (rng.uniform() < rate / kRateF) {
            const float f = std::exp(rng.range(std::log(350.0f), std::log(2600.0f))) * std::exp2((0.5f - size) * 1.2f);
            const float tau = rng.range(0.004f, 0.02f) * std::pow(2600.0f / f, 0.3f);
            bubbles.start(f, tau, rng.range(0.2f, 1.0f) * std::pow(f / 1000.0f, -0.5f) * 0.5f);
        }
        brown = brown * 0.996f + rng.white() * 0.08f;
        const float w = rng.white();
        out[static_cast<std::size_t>(i)] = bubbles.run() + rush.run(brown) * (0.4f + 0.8f * size) + hiss.run(w) * 0.06f;
    }
    normalize(out, 0.6f);
    return join_loop(out, cross);
}

// Rain: drops on leaves and ground in three bands, a hiss, and drips into puddles, all heavier as size grows.
std::vector<float> rain(const Stuff& st, Rng& rng) {
    const float size = clamp01(st.size);
    const float wet = clamp01(st.wet);
    const int cross = samples(0.3f);
    const int n = samples(5.0f) + cross;
    Biquad bands[3];
    bands[0].bandpass(1800.0f, 1.5f);
    bands[1].bandpass(3600.0f, 1.5f);
    bands[2].bandpass(6500.0f, 1.5f);
    Biquad hiss;
    hiss.highpass(3000.0f, 0.7f);
    Bubbles drips;
    const float rate = 400.0f + 3600.0f * size;
    std::vector<float> out(static_cast<std::size_t>(n));
    for (int i = 0; i < n; ++i) {
        float drops[3] = {0.0f, 0.0f, 0.0f};
        if (rng.uniform() < rate / kRateF) {
            const float a = rng.range(0.1f, 1.0f);
            drops[rng.below(3)] = a * a;
        }
        if (rng.uniform() < (3.0f + 12.0f * wet) / kRateF) {
            drips.start(rng.range(600.0f, 2000.0f), rng.range(0.006f, 0.02f), rng.range(0.1f, 0.4f));
        }
        float y = 0.0f;
        for (int k = 0; k < 3; ++k) {
            y += bands[k].run(drops[k]);
        }
        y = y * 2.0f + hiss.run(rng.white()) * (0.04f + 0.12f * size) + drips.run() * 0.5f;
        out[static_cast<std::size_t>(i)] = y;
    }
    normalize(out, 0.6f);
    return join_loop(out, cross);
}

// Thunder: a few rolls of low rumble, and the crack of a near strike before them, bigger and nearer as size grows.
std::vector<float> thunder(const Stuff& st, Rng& rng) {
    const float size = clamp01(st.size);
    const float length = 3.0f + 4.0f * size * rng.range(0.6f, 1.0f);
    const int n = samples(length);
    struct Roll {
        float at, attack, tau, amp;
    };
    Roll rolls[8];
    const int count = 3 + rng.below(5);
    for (int k = 0; k < count; ++k) {
        rolls[k] = {rng.range(0.0f, 0.4f * length), rng.range(0.05f, 0.3f), rng.range(0.4f, 1.4f),
                    rng.range(0.3f, 1.0f)};
    }
    Biquad rumble;
    Biquad crack;
    crack.highpass(400.0f, 0.7f);
    Biquad mid;
    mid.bandpass(800.0f, 0.8f);
    float brown = 0.0f;
    float drift = 1.0f;
    std::vector<float> out(static_cast<std::size_t>(n));
    for (int i = 0; i < n; ++i) {
        const float t = fl(i) / kRateF;
        if (i % 64 == 0) {
            drift = std::clamp(drift + rng.white() * 0.03f, 0.7f, 1.3f);
            rumble.lowpass((90.0f + 300.0f * size) * drift, 0.7f);
        }
        float env = 0.0f;
        for (int k = 0; k < count; ++k) {
            const Roll& r = rolls[k];
            if (t >= r.at) {
                const float u = t - r.at;
                env += r.amp * std::min(1.0f, u / r.attack) * std::exp(-u / r.tau);
            }
        }
        brown = brown * 0.998f + rng.white() * 0.05f;
        const float w = rng.white();
        float y = rumble.run(brown) * env * 3.0f + mid.run(w) * env * 0.15f * size;
        if (size > 0.5f) {
            y += crack.run(w) * (size - 0.5f) * 2.0f * std::exp(-t / 0.05f) * 0.8f;
        }
        out[static_cast<std::size_t>(i)] = y;
    }
    fade(out, samples(0.005f), samples(0.5f));
    normalize(out, 0.9f);
    return out;
}

// --- Stand-ins for recorded calls --------------------------------------------------------------------------------

// A small bird's song: a few whistled notes, each sweeping and sometimes trilled, higher for a smaller bird.
std::vector<float> bird(const Stuff& st, Rng& rng) {
    const float size = clamp01(st.size);
    const int notes = 2 + rng.below(6);
    const float base = rng.range(2200.0f, 5200.0f) * std::exp2((0.5f - size) * 0.8f);
    const float length = rng.range(0.03f, 0.16f);
    const float gap = rng.range(0.02f, 0.09f);
    const float from = rng.range(0.75f, 1.3f);
    const float to = rng.range(0.75f, 1.35f);
    const float trill = rng.uniform() < 0.4f ? rng.range(0.03f, 0.12f) : 0.0f;
    const float trill_rate = rng.range(20.0f, 60.0f);
    std::vector<float> out;
    float phase = 0.0f;
    for (int k = 0; k < notes; ++k) {
        const float f = base * rng.range(0.95f, 1.05f);
        const int n = samples(length * rng.range(0.9f, 1.1f));
        for (int i = 0; i < n; ++i) {
            const float t = fl(i) / fl(n);
            const float tone = f * lerp(from, to, t) * (1.0f + trill * std::sin(kTwoPi * trill_rate * fl(i) / kRateF));
            phase += kTwoPi * std::min(tone, kNyquist * 0.9f) / kRateF;
            if (phase > kTwoPi) {
                phase -= kTwoPi;
            }
            const float env = std::sin(kPi * t);
            out.push_back((std::sin(phase) + 0.12f * std::sin(2.0f * phase)) * env * env);
        }
        out.insert(out.end(), static_cast<std::size_t>(samples(gap)), 0.0f);
    }
    normalize(out, 0.6f);
    return out;
}

// A dog's bark or two: a rough voiced burst whose pitch jumps and falls, through a dog's formants; a bigger dog is
// deeper.
std::vector<float> bark(const Stuff& st, Rng& rng) {
    const float size = clamp01(st.size);
    const int barks = 1 + rng.below(3);
    Biquad formants[3];
    formants[0].bandpass(650.0f - 150.0f * size, 4.0f);
    formants[1].bandpass(1500.0f - 300.0f * size, 6.0f);
    formants[2].bandpass(2600.0f, 8.0f);
    std::vector<float> out;
    float phase = 0.0f;
    for (int b = 0; b < barks; ++b) {
        const int n = samples(rng.range(0.09f, 0.17f) * (0.8f + 0.5f * size));
        const float f0 = rng.range(380.0f, 520.0f) * std::exp2((0.5f - size) * 1.2f);
        for (int i = 0; i < n; ++i) {
            const float t = fl(i) / fl(n);
            const float f = f0 * (t < 0.2f ? 1.0f + 0.5f * t : 1.1f - 0.45f * (t - 0.2f));
            phase += kTwoPi * f / kRateF;
            if (phase > kTwoPi) {
                phase -= kTwoPi;
            }
            float voice = 0.0f;
            for (int h = 1; fl(h) * f < 4500.0f; ++h) {
                voice += std::sin(fl(h) * phase) / fl(h);
            }
            const float env =
                std::min(1.0f, fl(i) / fl(samples(0.006f))) * (t < 0.6f ? 1.0f : std::exp(-(t - 0.6f) * 8.0f));
            const float x = (voice + 0.35f * rng.white()) * env;
            out.push_back(formants[0].run(x) + 0.6f * formants[1].run(x) + 0.3f * formants[2].run(x));
        }
        out.insert(out.end(), static_cast<std::size_t>(samples(rng.range(0.15f, 0.35f))), 0.0f);
    }
    fade(out, 0, samples(0.02f));
    normalize(out, 0.8f);
    return out;
}

// A wolf's howl: a tone that glides up, holds with a slow waver, and falls away; a bigger wolf is lower.
std::vector<float> howl(const Stuff& st, Rng& rng) {
    const float size = clamp01(st.size);
    const float length = rng.range(2.5f, 4.0f);
    const int n = samples(length);
    const float start = rng.range(300.0f, 380.0f) * std::exp2((0.5f - size) * 0.8f);
    const float peak = start * rng.range(1.4f, 1.75f);
    const float rise = rng.range(0.6f, 1.1f);
    const float falls = length - rng.range(0.8f, 1.3f);
    const float end = peak * rng.range(0.55f, 0.75f);
    static constexpr float kHarmonics[] = {1.0f, 0.3f, 0.12f, 0.05f, 0.02f, 0.01f};
    Biquad warm;
    warm.lowpass(1800.0f, 0.7f);
    Biquad breath;
    breath.bandpass(1500.0f, 0.7f);
    float phase = 0.0f;
    float drift = 0.0f;
    std::vector<float> out(static_cast<std::size_t>(n));
    for (int i = 0; i < n; ++i) {
        const float t = fl(i) / kRateF;
        float f = t < rise ? lerp(start, peak, std::sin(kPi / 2.0f * t / rise)) : peak;
        if (t > falls) {
            f = lerp(peak, end, (t - falls) / (length - falls));
        }
        drift = std::clamp(drift + rng.white() * 0.0004f, -0.02f, 0.02f);
        f *= 1.0f + 0.012f * std::sin(kTwoPi * 5.0f * t) + drift;
        phase += kTwoPi * f / kRateF;
        if (phase > kTwoPi) {
            phase -= kTwoPi;
        }
        float y = 0.0f;
        for (int h = 0; h < 6; ++h) {
            y += kHarmonics[h] * std::sin(fl(h + 1) * phase);
        }
        const float a = std::min(1.0f, t / 0.35f);
        const float r = std::min(1.0f, (length - t) / 0.6f);
        const float swell = 0.6f + 0.4f * clamp01((f - start) / (peak - start));
        out[static_cast<std::size_t>(i)] = (warm.run(y) + breath.run(rng.white()) * 0.04f) * a * a * r * r * swell;
    }
    normalize(out, 0.7f);
    return out;
}

// A drum: a hide over a frame, struck: a thump whose pitch drops as it rings, and the slap of the stick; a bigger
// drum is lower and longer, a harder stick brighter, a slack wet hide duller.
std::vector<float> drum(const Stuff& st, Rng& rng) {
    const float hard = clamp01(st.hard);
    const float size = clamp01(st.size);
    const float wet = clamp01(st.wet);
    const float f0 = (90.0f + 160.0f * (1.0f - size)) * (1.0f - 0.2f * wet) * rng.range(0.95f, 1.05f);
    const float tau = (0.08f + 0.25f * size) * (1.0f - 0.4f * wet);
    const int n = samples(tau * 4.0f + 0.02f);
    Biquad slap;
    slap.bandpass((900.0f + 2600.0f * hard) * (1.0f - 0.5f * wet), 0.9f);
    OnePole slack;
    slack.lowpass(7000.0f - 5000.0f * wet);
    float phase = 0.0f;
    std::vector<float> out(static_cast<std::size_t>(n));
    for (int i = 0; i < n; ++i) {
        const float t = fl(i) / kRateF;
        // the hide's pitch falls as its stretch eases
        const float f = f0 * (1.0f + 0.6f * std::exp(-t / 0.03f));
        phase += kTwoPi * f / kRateF;
        if (phase > kTwoPi) {
            phase -= kTwoPi;
        }
        const float body = std::sin(phase) * std::exp(-t / tau);
        const float hit = slap.run(rng.white()) * std::exp(-t / (0.004f + 0.008f * (1.0f - hard))) * (0.4f + hard);
        out[static_cast<std::size_t>(i)] = slack.run(body + hit);
    }
    fade(out, samples(0.001f), samples(0.02f));
    normalize(out, 0.8f);
    return out;
}

// Each base sound's own stream of chance, so the same seed gives different bases different variations.
std::uint32_t mix(Base base, std::uint32_t seed) {
    return seed * 0x9E3779B9u ^ (static_cast<std::uint32_t>(base) + 1u) * 0x85EBCA6Bu;
}

// A loop of the fire or the wind, from the very code that makes them live.
std::vector<float> live_loop(Base base, const Stuff& st, std::uint32_t seed) {
    const int cross = samples(0.4f);
    std::vector<float> s(static_cast<std::size_t>(samples(6.0f) + cross));
    Live live(base, seed, clamp01(st.size));
    live.render(s.data(), size_of(s));
    std::vector<float> out = join_loop(s, cross);
    // the join adds two stretches at equal power, which can pass the limit the live sound keeps
    for (float& v : out) {
        v = soft(v);
    }
    return out;
}

// --- The murmur ----------------------------------------------------------------------------------------------------

// A syllable's place in what is said.
struct Piece {
    const Syllable* syllable = nullptr;
    float cons = 0.0f;   // seconds its consonant takes
    float vowel = 0.0f;  // seconds its vowel takes
    float gain = 1.0f;
    float start = 0.0f;  // its pitch, Hz, at its start, middle and end
    float mid = 0.0f;
    float end = 0.0f;
    float after = 0.0f;  // seconds from its end to the next one's start; below zero, they overlap
};

float consonant_seconds(const Syllable& s) {
    return fl(s.vowel) / kRateF;
}
float vowel_seconds(const Syllable& s) {
    return fl(static_cast<int>(s.samples.size()) - s.vowel) / kRateF;
}

// One grain of a syllable: the stretch around its mark j, between the marks on either side, faded in and out (a
// Hann window, each half as long as its side), squeezed by f to shift its formants, added into out centred at c.
void grain(std::vector<float>& out, const Syllable& s, std::size_t j, float c, float f, float gain) {
    const int count = static_cast<int>(s.marks.size());
    const int at = s.marks[j];
    const int ji = static_cast<int>(j);
    float left = ji > 0 ? fl(at - s.marks[j - 1]) : (ji + 1 < count ? fl(s.marks[j + 1] - at) : fl(kHop));
    float right = ji + 1 < count ? fl(s.marks[j + 1] - at) : left;
    left = std::clamp(left, 8.0f, 400.0f);
    right = std::clamp(right, 8.0f, 400.0f);
    const int first = std::max(0, static_cast<int>(std::ceil(c - left / f)));
    const int last = std::min(size_of(out) - 1, static_cast<int>(std::floor(c + right / f)));
    const int ns = size_of(s.samples);
    for (int n = first; n <= last; ++n) {
        const float d = (fl(n) - c) * f;
        if (d < -left || d > right) {
            continue;
        }
        const float w = 0.5f + 0.5f * std::cos(kPi * d / (d < 0.0f ? left : right));
        const float src = fl(at) + d;
        const int i0 = static_cast<int>(std::floor(src));
        if (i0 < 0 || i0 + 1 >= ns) {
            continue;
        }
        const float frac = src - fl(i0);
        const float a = s.samples[static_cast<std::size_t>(i0)];
        const float b = s.samples[static_cast<std::size_t>(i0) + 1];
        out[static_cast<std::size_t>(n)] += gain * w * (a + frac * (b - a));
    }
}

// The pieces cut from the bank and joined at their pitch (TD-PSOLA, Moulines and Charpentier, 1990): through each
// piece the output steps from grain to grain, a voice's pulse at a time where the bank's grain is voiced, at the
// bank's own steps where it is not; each grain is the bank's nearest to the matching moment of the syllable,
// stretched to the piece's timing. Then breath, the loudness and its tone.
std::vector<float> speak(const std::vector<Piece>& pieces, const Speaker& speaker, float loud, Rng& rng) {
    if (pieces.empty()) {
        return {};
    }
    const int pad = samples(0.02f);
    std::vector<int> at(pieces.size());
    std::vector<int> length(pieces.size());
    int t = pad;
    int total = 0;
    for (std::size_t i = 0; i < pieces.size(); ++i) {
        at[i] = std::max(0, t);
        length[i] = samples(pieces[i].cons) + samples(pieces[i].vowel);
        total = std::max(total, at[i] + length[i]);
        t = at[i] + length[i] + samples(pieces[i].after);
    }
    total += pad;
    // the tune: straight between each piece's start, middle and end
    std::vector<std::pair<int, float>> knots;
    for (std::size_t i = 0; i < pieces.size(); ++i) {
        knots.emplace_back(at[i], pieces[i].start);
        knots.emplace_back(at[i] + length[i] / 2, pieces[i].mid);
        knots.emplace_back(at[i] + length[i], pieces[i].end);
    }
    std::stable_sort(knots.begin(), knots.end(), [](const auto& a, const auto& b) { return a.first < b.first; });
    std::vector<float> tune(static_cast<std::size_t>(total));
    std::size_t k = 0;
    const float wobble = rng.range(0.0f, kTwoPi);
    for (int n = 0; n < total; ++n) {
        while (k + 1 < knots.size() && knots[k + 1].first <= n) {
            ++k;
        }
        float f = knots[k].second;
        if (n > knots[k].first && k + 1 < knots.size() && knots[k + 1].first > knots[k].first) {
            const float u = fl(n - knots[k].first) / fl(knots[k + 1].first - knots[k].first);
            f = lerp(knots[k].second, knots[k + 1].second, u);
        } else if (n < knots[0].first) {
            f = knots[0].second;
        }
        f *= 1.0f + speaker.tremor * std::sin(wobble + kTwoPi * 5.3f * fl(n) / kRateF);
        tune[static_cast<std::size_t>(n)] = std::clamp(f, 50.0f, 900.0f);
    }
    std::vector<float> out(static_cast<std::size_t>(total), 0.0f);
    const float shift = std::clamp(speaker.formants, 0.6f, 1.6f);
    for (std::size_t i = 0; i < pieces.size(); ++i) {
        const Piece& p = pieces[i];
        const Syllable& s = *p.syllable;
        if (s.marks.empty()) {
            continue;
        }
        const float lc = fl(samples(p.cons));
        const float c = fl(s.vowel);
        const float v = fl(size_of(s.samples) - s.vowel);
        const float sc = c > 0.0f ? lc / c : 1.0f;
        const float sv = v > 0.0f ? fl(length[i] - samples(p.cons)) / v : 1.0f;
        float tau = 0.0f;
        while (tau < fl(length[i])) {
            const float sigma = std::clamp(tau < lc ? tau / sc : c + (tau - lc) / sv, 0.0f, fl(size_of(s.samples) - 1));
            // the bank's mark nearest to that moment
            const auto it = std::lower_bound(s.marks.begin(), s.marks.end(), static_cast<int>(sigma));
            std::size_t j = static_cast<std::size_t>(it - s.marks.begin());
            if (j >= s.marks.size()) {
                j = s.marks.size() - 1;
            } else if (j > 0 && sigma - fl(s.marks[j - 1]) < fl(s.marks[j]) - sigma) {
                --j;
            }
            const float centre = fl(at[i]) + tau;
            const bool pulse = s.voiced[j] != 0;
            // unvoiced grains overlap at the bank's own steps, so squeezing them by f thins them by f
            grain(out, s, j, centre, shift, p.gain * (pulse ? 1.0f : shift));
            if (pulse) {
                const auto n = static_cast<std::size_t>(std::clamp(static_cast<int>(centre), 0, total - 1));
                tau += std::max(10.0f, kRateF / (tune[n] * (1.0f + 0.006f * rng.gauss())));
            } else {
                tau += j + 1 < s.marks.size() ? std::max(20.0f, fl(s.marks[j + 1] - s.marks[j])) : fl(kHop);
            }
        }
    }
    // breath, following the voice's loudness; then a raised voice brighter and a hushed one duller
    OnePole follow;
    follow.lowpass(30.0f);
    Biquad airy;
    airy.highpass(1200.0f, 0.7f);
    OnePole dull;
    const float bright = std::max(0.0f, loud - 1.0f) * 0.7f;
    const float dark = clamp01(1.0f - loud);
    dull.lowpass(1800.0f + 5000.0f * (1.0f - dark));
    float before = 0.0f;
    for (float& y : out) {
        const float e = follow.run(std::abs(y));
        float x = y + airy.run(rng.white()) * e * std::max(0.0f, speaker.breath) * 1.5f;
        const float lifted = x + bright * (x - before);
        before = x;
        x = lerp(lifted, dull.run(lifted), dark);
        y = soft(x * loud * 1.2f);
    }
    return out;
}

// Reads a bank's bytes in order, little-endian as both the cloud and the phone are, and notes any that run short.
struct Reader {
    const std::uint8_t* p;
    std::size_t left;
    bool ok = true;
    template <class T>
    T get() {
        T v{};
        if (!ok || left < sizeof(T)) {
            ok = false;
            return v;
        }
        std::memcpy(&v, p, sizeof(T));
        p += sizeof(T);
        left -= sizeof(T);
        return v;
    }
    bool has(std::size_t n) {
        ok = ok && left >= n;
        return ok;
    }
};

}  // namespace

bool loops(Base base) {
    return base == Base::kFire || base == Base::kWind || base == Base::kRiver || base == Base::kRain;
}

std::vector<float> make(Base base, const Stuff& stuff, std::uint32_t seed) {
    Rng rng(mix(base, seed));
    switch (base) {
        case Base::kStrike:
            return strike(stuff, rng);
        case Base::kScrape:
            return scrape(stuff, rng);
        case Base::kChop:
            return chop(stuff, rng);
        case Base::kStep:
            return step(stuff, rng);
        case Base::kFire:
        case Base::kWind:
            return live_loop(base, stuff, mix(base, seed));
        case Base::kRiver:
            return river(stuff, rng);
        case Base::kRain:
            return rain(stuff, rng);
        case Base::kThunder:
            return thunder(stuff, rng);
        case Base::kBird:
            return bird(stuff, rng);
        case Base::kBark:
            return bark(stuff, rng);
        case Base::kHowl:
            return howl(stuff, rng);
        case Base::kDrum:
            return drum(stuff, rng);
    }
    return {};
}

struct Live::State {
    State(Base b, std::uint32_t seed, float a) : base(b), fire(seed), wind(seed), amount(a) {}
    Base base;
    Fire fire;
    Wind wind;
    float amount;
};

Live::Live(Base base, std::uint32_t seed, float amount)
    : state_(std::make_unique<State>(base, seed, clamp01(amount))), target_(clamp01(amount)) {}

Live::~Live() = default;

void Live::set(float amount) {
    target_.store(clamp01(amount), std::memory_order_relaxed);
}

void Live::render(float* out, int frames) {
    State& s = *state_;
    const float target = target_.load(std::memory_order_relaxed);
    for (int i = 0; i < frames; ++i) {
        // a change glides over about half a second
        s.amount += (target - s.amount) * 0.0001f;
        out[i] = s.base == Base::kFire ? s.fire.next(s.amount) : s.wind.next(s.amount);
    }
}

const Syllable* Bank::find(const std::string& text) const {
    for (const Syllable& s : syllables) {
        if (s.text == text) {
            return &s;
        }
    }
    return nullptr;
}

Bank read_bank(const std::uint8_t* data, std::size_t size) {
    Reader r{data, size};
    static constexpr char kMagic[8] = {'K', 'D', 'B', 'A', 'N', 'K', '1', '\0'};
    if (data == nullptr || !r.has(8) || std::memcmp(data, kMagic, 8) != 0) {
        return {};
    }
    r.p += 8;
    r.left -= 8;
    Bank bank;
    const auto rate = r.get<std::uint32_t>();
    bank.pitch = r.get<float>();
    const auto count = r.get<std::uint32_t>();
    if (!r.ok || rate != static_cast<std::uint32_t>(kRate) || count > 10000 || !(bank.pitch > 20.0f)) {
        return {};
    }
    for (std::uint32_t k = 0; k < count && r.ok; ++k) {
        Syllable s;
        const auto len = r.get<std::uint8_t>();
        if (!r.has(len)) {
            break;
        }
        s.text.assign(reinterpret_cast<const char*>(r.p), len);
        r.p += len;
        r.left -= len;
        const auto vowel = r.get<std::uint32_t>();
        const auto n = r.get<std::uint32_t>();
        if (!r.ok || n > static_cast<std::uint32_t>(10 * kRate) || vowel > n || !r.has(std::size_t{n} * 2)) {
            r.ok = false;
            break;
        }
        s.vowel = static_cast<int>(vowel);
        s.samples.resize(n);
        for (std::uint32_t i = 0; i < n; ++i) {
            s.samples[i] = static_cast<float>(r.get<std::int16_t>()) / 32767.0f;
        }
        const auto m = r.get<std::uint32_t>();
        if (!r.ok || m > n || !r.has(std::size_t{m} * 5)) {
            r.ok = false;
            break;
        }
        s.marks.resize(m);
        for (std::uint32_t i = 0; i < m; ++i) {
            s.marks[i] = r.get<std::int32_t>();
            r.ok =
                r.ok && s.marks[i] >= 0 && s.marks[i] < static_cast<int>(n) && (i == 0 || s.marks[i] > s.marks[i - 1]);
        }
        s.voiced.resize(m);
        for (std::uint32_t i = 0; i < m; ++i) {
            s.voiced[i] = r.get<std::uint8_t>();
        }
        bank.syllables.push_back(std::move(s));
    }
    if (!r.ok) {
        return {};
    }
    return bank;
}

std::vector<std::string> language(const Bank& bank, std::uint32_t seed, int count) {
    Rng rng(seed ^ 0x27D4EB2Fu);
    std::vector<char> consonants;
    std::vector<char> vowels;
    for (const Syllable& s : bank.syllables) {
        if (s.text.size() == 1) {
            vowels.push_back(s.text[0]);
        } else if (!s.text.empty() && std::find(consonants.begin(), consonants.end(), s.text[0]) == consonants.end()) {
            consonants.push_back(s.text[0]);
        }
    }
    auto shuffle = [&rng](auto& v) {
        for (int i = static_cast<int>(v.size()) - 1; i > 0; --i) {
            std::swap(v[static_cast<std::size_t>(i)], v[static_cast<std::size_t>(rng.below(i + 1))]);
        }
    };
    shuffle(consonants);
    consonants.resize(std::min(consonants.size(), static_cast<std::size_t>(6 + rng.below(4))));
    shuffle(vowels);
    // every language keeps a
    const auto a = std::find(vowels.begin(), vowels.end(), 'a');
    if (a != vowels.end()) {
        std::iter_swap(vowels.begin(), a);
    }
    vowels.resize(std::min(vowels.size(), static_cast<std::size_t>(3 + rng.below(3))));
    std::vector<std::string> out;
    for (const char c : consonants) {
        for (const char v : vowels) {
            const std::string text{c, v};
            if (bank.find(text) != nullptr) {
                out.push_back(text);
            }
        }
    }
    for (const char v : vowels) {
        out.emplace_back(1, v);
    }
    shuffle(out);
    if (count > 0 && out.size() > static_cast<std::size_t>(count)) {
        out.resize(static_cast<std::size_t>(count));
    }
    return out;
}

std::vector<float> phrase(const Bank& bank, const std::vector<std::string>& syllables, const Speaker& speaker,
                          const Feeling& feeling, std::uint32_t seed) {
    std::vector<const Syllable*> pool;
    for (const std::string& text : syllables) {
        if (const Syllable* s = bank.find(text); s != nullptr && !s->marks.empty()) {
            pool.push_back(s);
        }
    }
    if (pool.empty() || bank.pitch <= 0.0f) {
        return {};
    }
    Rng rng(seed ^ 0x165667B1u);
    const float tempo = std::clamp(feeling.tempo, 0.4f, 2.5f);
    const float range = std::max(0.0f, feeling.range);
    std::vector<Piece> pieces;
    std::vector<bool> stressed;
    const Syllable* last = nullptr;
    const int words = 2 + rng.below(5);
    for (int w = 0; w < words; ++w) {
        const float r = rng.uniform();
        const int n = r < 0.35f ? 1 : (r < 0.8f ? 2 : 3);
        for (int k = 0; k < n; ++k) {
            const Syllable* s = pool[static_cast<std::size_t>(rng.below(static_cast<int>(pool.size())))];
            for (int tries = 0; tries < 3 && s == last && pool.size() > 1; ++tries) {
                s = pool[static_cast<std::size_t>(rng.below(static_cast<int>(pool.size())))];
            }
            last = s;
            Piece p;
            p.syllable = s;
            const float cs = consonant_seconds(*s);
            const float vs = vowel_seconds(*s);
            p.cons = std::clamp(0.06f / std::sqrt(tempo) * rng.range(0.85f, 1.15f), cs * 0.5f, cs * 1.6f);
            p.vowel = std::clamp((k == 0 ? 0.15f : 0.11f) / tempo * rng.range(0.85f, 1.15f), vs * 0.3f, vs * 3.0f);
            p.gain = k == 0 ? 1.0f : 0.85f;
            p.after = -0.012f;
            pieces.push_back(p);
            stressed.push_back(k == 0);
        }
        const float g = rng.uniform();
        if (g > 0.55f) {
            pieces.back().after = (g < 0.9f ? rng.range(0.03f, 0.09f) : rng.range(0.15f, 0.25f)) / tempo;
        }
    }
    pieces.back().vowel = std::min(pieces.back().vowel * 1.35f, vowel_seconds(*pieces.back().syllable) * 3.0f);
    pieces.back().after = 0.0f;
    // the tune: falling through the phrase, raised on each word's first syllable, now and then rising at the end
    const float base = bank.pitch * speaker.pitch * feeling.lift;
    const std::size_t n = pieces.size();
    std::vector<float> targets(n);
    for (std::size_t i = 0; i < n; ++i) {
        const float u = (fl(i) + 0.5f) / fl(n);
        const float fall = lerp(1.0f + 0.06f * range, 1.0f - feeling.fall, u);
        const float accent = stressed[i] ? 1.0f + 0.12f * range : 1.0f - 0.02f * range;
        targets[i] = base * fall * accent * (1.0f + 0.035f * range * rng.gauss());
    }
    const bool asks = rng.uniform() < 0.12f;
    if (asks) {
        targets[n - 1] = base * (1.0f + 0.2f * range);
    }
    for (std::size_t i = 0; i < n; ++i) {
        pieces[i].mid = targets[i];
        pieces[i].start = i > 0 ? (targets[i - 1] + targets[i]) / 2.0f : targets[i];
        pieces[i].end =
            i + 1 < n ? (targets[i] + targets[i + 1]) / 2.0f : targets[i] * (asks ? 1.08f : 1.0f - 0.3f * feeling.fall);
    }
    return speak(pieces, speaker, feeling.loud, rng);
}

std::vector<float> cry(const Bank& bank, Cry kind, const Speaker& speaker, std::uint32_t seed) {
    if (bank.syllables.empty() || bank.pitch <= 0.0f) {
        return {};
    }
    Rng rng(seed ^ 0xC2B2AE35u);
    auto pick = [&bank](std::initializer_list<const char*> names) {
        for (const char* name : names) {
            if (const Syllable* s = bank.find(name); s != nullptr && !s->marks.empty()) {
                return s;
            }
        }
        return &bank.syllables.front();
    };
    const float base = bank.pitch * speaker.pitch;
    Speaker voice = speaker;
    float loud = 1.0f;
    std::vector<Piece> pieces;
    auto add = [&pieces](const Syllable* s, float vowel, float start, float mid, float end, float after) {
        Piece p;
        p.syllable = s;
        p.cons = consonant_seconds(*s);
        p.vowel = vowel;
        p.start = start;
        p.mid = mid;
        p.end = end;
        p.after = after;
        pieces.push_back(p);
    };
    switch (kind) {
        case Cry::kLaugh: {
            const Syllable* ha = pick({"ha", "he", "a"});
            const int n = 4 + rng.below(4);
            for (int i = 0; i < n; ++i) {
                const float f = base * (1.5f - 0.05f * fl(i)) * rng.range(0.97f, 1.03f);
                add(ha, rng.range(0.06f, 0.09f), f * 1.05f, f, f * 0.92f, rng.range(0.03f, 0.06f));
                pieces.back().gain = 1.0f - 0.07f * fl(i);
            }
            loud = 1.3f;
            voice.breath += 0.25f;
            break;
        }
        case Cry::kCry: {
            const Syllable* a = pick({"a", "e", "u"});
            const int n = 2 + rng.below(2);
            for (int i = 0; i < n; ++i) {
                add(a, rng.range(0.5f, 0.9f), base * 1.5f, base * 1.75f, base * 1.15f, rng.range(0.25f, 0.45f));
            }
            voice.tremor = std::max(voice.tremor, 0.05f);
            voice.breath += 0.35f;
            loud = 1.1f;
            break;
        }
        case Cry::kCall: {
            add(pick({"he", "e"}), 0.22f, base * 1.3f, base * 1.4f, base * 1.45f, -0.012f);
            add(pick({"o", "yo", "lo"}), 0.45f, base * 1.5f, base * 1.45f, base * 1.15f, 0.0f);
            loud = 1.6f;
            break;
        }
        case Cry::kScream: {
            add(pick({"a", "e"}), rng.range(0.6f, 1.0f), base * 2.3f, base * 2.7f, base * 2.0f, 0.0f);
            loud = 2.0f;
            voice.breath += 0.3f;
            break;
        }
    }
    return speak(pieces, voice, loud, rng);
}

std::vector<float> blend(const std::vector<std::vector<float>>& sounds, float seconds, float per_second,
                         std::uint32_t seed) {
    const int n = std::max(1, samples(seconds));
    std::vector<float> out(static_cast<std::size_t>(n), 0.0f);
    if (sounds.empty()) {
        return out;
    }
    Rng rng(seed ^ 0x2545F491u);
    const int count = std::max(1, static_cast<int>(std::lround(seconds * per_second)));
    for (int k = 0; k < count; ++k) {
        const std::vector<float>& s = sounds[static_cast<std::size_t>(rng.below(static_cast<int>(sounds.size())))];
        const int at = rng.below(n);
        const float gain = rng.range(0.3f, 1.0f);
        for (std::size_t i = 0; i < s.size(); ++i) {
            out[static_cast<std::size_t>((at + static_cast<int>(i)) % n)] += s[i] * gain;
        }
    }
    normalize(out, 0.6f);
    return out;
}

std::vector<float> rhythm(const std::vector<std::vector<float>>& hits, float seconds, float beat, std::uint32_t seed) {
    const int n = std::max(1, samples(seconds));
    std::vector<float> out(static_cast<std::size_t>(n), 0.0f);
    if (hits.empty() || beat <= 0.0f) {
        return out;
    }
    Rng rng(seed ^ 0x9E3779B1u);
    // a bar of four beats, each beat whole or halved, repeated with small changes
    const int beats = std::max(1, static_cast<int>(seconds / beat));
    std::uint32_t pattern = rng.next() | 1u;
    for (int b = 0; b < beats * 2; ++b) {
        const int step = b % 8;
        if (step == 0 && rng.uniform() < 0.25f) {
            pattern ^= 1u << static_cast<unsigned>(1 + rng.below(7));
        }
        if (((pattern >> static_cast<unsigned>(step)) & 1u) == 0u) {
            continue;
        }
        const std::vector<float>& s = hits[static_cast<std::size_t>(rng.below(static_cast<int>(hits.size())))];
        const float gain = step == 0 ? 1.0f : (step % 2 == 0 ? 0.7f : 0.45f) * rng.range(0.85f, 1.0f);
        const int at = samples(fl(b) * beat / 2.0f);
        for (std::size_t i = 0; i < s.size(); ++i) {
            out[static_cast<std::size_t>((at + static_cast<int>(i)) % n)] += s[i] * gain;
        }
    }
    normalize(out, 0.7f);
    return out;
}

float brightness(const std::vector<float>& s) {
    double change = 0.0;
    double level = 0.0;
    for (std::size_t i = 1; i < s.size(); ++i) {
        const double d = static_cast<double>(s[i]) - static_cast<double>(s[i - 1]);
        change += d * d;
        level += static_cast<double>(s[i]) * static_cast<double>(s[i]);
    }
    if (level <= 0.0) {
        return 0.0f;
    }
    // a tone at f changes by 2 sin(pi f / rate) of its size each sample
    const double ratio = std::min(1.0, std::sqrt(change / level) / 2.0);
    return static_cast<float>(std::asin(ratio) * static_cast<double>(kRate) / static_cast<double>(kPi));
}

float loudness(const std::vector<float>& s) {
    if (s.empty()) {
        return 0.0f;
    }
    double sum = 0.0;
    for (const float v : s) {
        sum += static_cast<double>(v) * static_cast<double>(v);
    }
    return static_cast<float>(std::sqrt(sum / static_cast<double>(s.size())));
}

float pitch(const std::vector<float>& s, float lowest, float highest) {
    const int width = samples(0.04f);
    const int hop = samples(0.01f);
    const int lo = std::max(2, static_cast<int>(kRateF / highest));
    const int hi = static_cast<int>(kRateF / lowest);
    const int n = size_of(s);
    // the loudest frame sets which frames count as loud
    float top = 0.0f;
    std::vector<float> energy;
    for (int at = 0; at + width + hi < n; at += hop) {
        float e = 0.0f;
        for (int i = 0; i < width; ++i) {
            const float v = s[static_cast<std::size_t>(at) + static_cast<std::size_t>(i)];
            e += v * v;
        }
        energy.push_back(e);
        top = std::max(top, e);
    }
    std::vector<float> found;
    std::vector<float> r(static_cast<std::size_t>(hi + 1));
    for (std::size_t f = 0; f < energy.size(); ++f) {
        if (energy[f] < 0.3f * top) {
            continue;
        }
        const int at = static_cast<int>(f) * hop;
        float best = 0.0f;
        for (int lag = lo; lag <= hi; ++lag) {
            float xy = 0.0f;
            float xx = 0.0f;
            float yy = 0.0f;
            for (int i = 0; i < width; ++i) {
                const float x = s[static_cast<std::size_t>(at) + static_cast<std::size_t>(i)];
                const float y = s[static_cast<std::size_t>(at) + static_cast<std::size_t>(i + lag)];
                xy += x * y;
                xx += x * x;
                yy += y * y;
            }
            r[static_cast<std::size_t>(lag)] = xy / (std::sqrt(xx * yy) + 1e-9f);
            best = std::max(best, r[static_cast<std::size_t>(lag)]);
        }
        if (best < 0.5f) {
            continue;
        }
        // the shortest period nearly as alike as the best, so a pitch is not taken for its octave below
        for (int lag = lo; lag <= hi; ++lag) {
            const float here = r[static_cast<std::size_t>(lag)];
            const bool peak = (lag == lo || here >= r[static_cast<std::size_t>(lag - 1)]) &&
                              (lag == hi || here >= r[static_cast<std::size_t>(lag) + 1]);
            if (peak && here >= 0.85f * best) {
                found.push_back(kRateF / fl(lag));
                break;
            }
        }
    }
    if (found.empty()) {
        return 0.0f;
    }
    std::nth_element(found.begin(), found.begin() + static_cast<std::ptrdiff_t>(found.size() / 2), found.end());
    return found[found.size() / 2];
}

}  // namespace sound
