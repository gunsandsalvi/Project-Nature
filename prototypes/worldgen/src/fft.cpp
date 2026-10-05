// P7's Fourier transform: see fft.hpp. Pre-production code (research 00).
#include "fft.hpp"

#include "maths.hpp"

namespace worldgen {

namespace {

constexpr double kTwoPi = 6.28318530717958623200e+00;

// The turning factors of a transform of length n, for the forward direction.
std::vector<Complex> turns(int n) {
    std::vector<Complex> w(static_cast<std::size_t>(n / 2));
    for (int k = 0; k < n / 2; ++k) {
        const double angle = kTwoPi * k / n;
        w[static_cast<std::size_t>(k)] = {samebits::cosine(angle), -samebits::sine(angle)};
    }
    return w;
}

// One transform of length n (a power of two) in place: bit-reversed order, then butterflies.
void fft(Complex* a, int n, const std::vector<Complex>& w, bool inverse) {
    for (int i = 1, j = 0; i < n; ++i) {
        int bit = n >> 1;
        for (; (j & bit) != 0; bit >>= 1) {
            j ^= bit;
        }
        j ^= bit;
        if (i < j) {
            const Complex t = a[i];
            a[i] = a[j];
            a[j] = t;
        }
    }
    for (int len = 2; len <= n; len <<= 1) {
        const int step = n / len;
        for (int i = 0; i < n; i += len) {
            for (int k = 0; k < len / 2; ++k) {
                const int turn = k * step;
                const Complex& t = w[static_cast<std::size_t>(turn)];
                const double wr = t.re;
                const double wi = inverse ? -t.im : t.im;
                Complex& u = a[i + k];
                Complex& v = a[i + k + (len / 2)];
                const double vr = (v.re * wr) - (v.im * wi);
                const double vi = (v.re * wi) + (v.im * wr);
                v.re = u.re - vr;
                v.im = u.im - vi;
                u.re += vr;
                u.im += vi;
            }
        }
    }
}

}  // namespace

void fft2(std::vector<Complex>* grid, int width, int height, bool inverse, minds::Pool* pool) {
    const std::vector<Complex> wx = turns(width);
    const std::vector<Complex> wy = turns(height);
    std::vector<Complex>& g = *grid;
    pool->run(height, [&](int y, int /*thread*/) {
        fft(&g[static_cast<std::size_t>(y) * static_cast<std::size_t>(width)], width, wx, inverse);
    });
    pool->run(width, [&](int x, int /*thread*/) {
        std::vector<Complex> column(static_cast<std::size_t>(height));
        for (int y = 0; y < height; ++y) {
            column[static_cast<std::size_t>(y)] = g[(static_cast<std::size_t>(y) * width) + x];
        }
        fft(column.data(), height, wy, inverse);
        for (int y = 0; y < height; ++y) {
            g[(static_cast<std::size_t>(y) * width) + x] = column[static_cast<std::size_t>(y)];
        }
    });
    if (inverse) {
        const double scale = 1.0 / (static_cast<double>(width) * height);
        for (Complex& c : g) {
            c.re *= scale;
            c.im *= scale;
        }
    }
}

}  // namespace worldgen
