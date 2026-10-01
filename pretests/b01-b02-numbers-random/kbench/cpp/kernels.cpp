// B01/B02 pre-test kernels in C++ (throwaway). Same algorithms, same order of operations
// and same key layout as the Rust versions in src/kernels.rs and src/rng.rs, so the
// checksums can be compared bit for bit. Compiled twice (see build.rs): with the
// compiler's default float settings (prefix cpp_) and with -ffp-contract=off (cppnc_).
#include <cstddef>
#include <cstdint>
#include <type_traits>

#ifndef KB_PREFIX
#define KB_PREFIX cpp_
#endif
#define KB_CAT2(a, b) a##b
#define KB_CAT(a, b) KB_CAT2(a, b)
#define KB_FN(name) KB_CAT(KB_PREFIX, name)

// Same layout as rng::Stream (#[repr(C)]).
struct KbStream {
  uint64_t k0, k1, sq_key, wy_seed;
  uint32_t chacha_key[8];
};

namespace {

// ------------------------------------------------------------ B02 generators
constexpr uint64_t GOLDEN = 0x9e3779b97f4a7c15ULL;

inline uint64_t mix64(uint64_t z) {
  z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
  z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
  return z ^ (z >> 31);
}
inline uint64_t rotr64(uint64_t x, int r) { return (x >> r) | (x << (64 - r)); }
inline uint32_t rotl32(uint32_t x, int r) { return (x << r) | (x >> (32 - r)); }

struct GSplit {
  uint64_t k0;
  explicit GSplit(const KbStream& s) : k0(s.k0) {}
  inline uint64_t draw(uint64_t b, uint64_t m) const {
    uint64_t seed = mix64(k0 ^ b);
    return mix64(seed + (m + 1) * GOLDEN);
  }
};

struct GPhilox {
  uint32_t key0, key1;
  explicit GPhilox(const KbStream& s) : key0((uint32_t)s.k0), key1((uint32_t)(s.k0 >> 32)) {}
  inline uint64_t draw(uint64_t b, uint64_t m) const {
    uint32_t c0 = (uint32_t)m, c1 = (uint32_t)(m >> 32), c2 = (uint32_t)b, c3 = (uint32_t)(b >> 32);
    uint32_t k0 = key0, k1 = key1;
    for (int r = 0; r < 10; ++r) {
      if (r > 0) {
        k0 += 0x9E3779B9u;
        k1 += 0xBB67AE85u;
      }
      uint64_t p0 = (uint64_t)0xD2511F53u * c0;
      uint64_t p1 = (uint64_t)0xCD9E8D57u * c2;
      uint32_t n0 = (uint32_t)(p1 >> 32) ^ c1 ^ k0;
      uint32_t n1 = (uint32_t)p1;
      uint32_t n2 = (uint32_t)(p0 >> 32) ^ c3 ^ k1;
      uint32_t n3 = (uint32_t)p0;
      c0 = n0; c1 = n1; c2 = n2; c3 = n3;
    }
    return (uint64_t)c0 | ((uint64_t)c1 << 32);
  }
};

inline uint64_t squares64(uint64_t ctr, uint64_t key) {
  uint64_t t, x, y, z;
  y = x = ctr * key;
  z = y + key;
  x = x * x + y; x = rotr64(x, 32);
  x = x * x + z; x = rotr64(x, 32);
  x = x * x + y; x = rotr64(x, 32);
  t = x = x * x + z; x = rotr64(x, 32);
  return t ^ ((x * x + y) >> 32);
}
struct GSquares {
  uint64_t key, k1;
  explicit GSquares(const KbStream& s) : key(s.sq_key), k1(s.k1) {}
  inline uint64_t draw(uint64_t b, uint64_t m) const { return squares64(mix64(k1 ^ b) + m, key); }
};

inline uint64_t pcg_mix(uint64_t x) {
  uint64_t s = x * 6364136223846793005ULL + 1442695040888963407ULL;
  uint64_t w = ((s >> ((s >> 59) + 5)) ^ s) * 12605985483714917081ULL;
  return (w >> 43) ^ w;
}
struct GPcg {
  uint64_t k0;
  explicit GPcg(const KbStream& s) : k0(s.k0) {}
  inline uint64_t draw(uint64_t b, uint64_t m) const { return pcg_mix(pcg_mix(k0 ^ b) + m); }
};

constexpr uint64_t WY_P0 = 0xa0761d6478bd642fULL;
constexpr uint64_t WY_P1 = 0xe7037ed1a0b428dbULL;
struct GWy {
  uint64_t seed;
  explicit GWy(const KbStream& s) : seed(s.wy_seed) {}
  inline uint64_t draw(uint64_t b, uint64_t m) const {
    unsigned __int128 r = (unsigned __int128)(b ^ WY_P1) * (m ^ seed);
    uint64_t a = (uint64_t)r, bb = (uint64_t)(r >> 64);
    unsigned __int128 r2 = (unsigned __int128)(a ^ WY_P0 ^ 16) * (bb ^ WY_P1);
    return (uint64_t)r2 ^ (uint64_t)(r2 >> 64);
  }
};

#define KB_QR(a, b, c, d)                    \
  x[a] += x[b]; x[d] = rotl32(x[d] ^ x[a], 16); \
  x[c] += x[d]; x[b] = rotl32(x[b] ^ x[c], 12); \
  x[a] += x[b]; x[d] = rotl32(x[d] ^ x[a], 8);  \
  x[c] += x[d]; x[b] = rotl32(x[b] ^ x[c], 7);

struct GChaCha {
  uint32_t key[8];
  explicit GChaCha(const KbStream& s) {
    for (int i = 0; i < 8; ++i) key[i] = s.chacha_key[i];
  }
  inline uint64_t draw(uint64_t b, uint64_t m) const {
    const uint32_t s[16] = {0x61707865u, 0x3320646eu, 0x79622d32u, 0x6b206574u,
                            key[0], key[1], key[2], key[3], key[4], key[5], key[6], key[7],
                            (uint32_t)m, (uint32_t)(m >> 32), (uint32_t)b, (uint32_t)(b >> 32)};
    uint32_t x[16];
    for (int i = 0; i < 16; ++i) x[i] = s[i];
    for (int i = 0; i < 4; ++i) {
      KB_QR(0, 4, 8, 12) KB_QR(1, 5, 9, 13) KB_QR(2, 6, 10, 14) KB_QR(3, 7, 11, 15)
      KB_QR(0, 5, 10, 15) KB_QR(1, 6, 11, 12) KB_QR(2, 7, 8, 13) KB_QR(3, 4, 9, 14)
    }
    return (uint64_t)(x[0] + s[0]) | ((uint64_t)(x[1] + s[1]) << 32);
  }
};

template <class F>
inline auto with_gen(int gen, const KbStream& s, F&& f) -> decltype(f(GSplit(s))) {
  switch (gen) {
    case 0: return f(GSplit(s));
    case 1: return f(GPhilox(s));
    case 2: return f(GSquares(s));
    case 3: return f(GPcg(s));
    case 4: return f(GWy(s));
    default: return f(GChaCha(s));
  }
}

// ------------------------------------------------------------ B01 number formats
// Wrapping integer adds go through unsigned types (signed overflow is undefined in C++).
inline float add(float a, float b) { return a + b; }
inline double add(double a, double b) { return a + b; }
inline int32_t add(int32_t a, int32_t b) { return (int32_t)((uint32_t)a + (uint32_t)b); }
inline int64_t add(int64_t a, int64_t b) { return (int64_t)((uint64_t)a + (uint64_t)b); }

template <class T> struct Tag { using type = T; };
template <class F>
inline void with_fmt(int fmt, F&& f) {
  switch (fmt) {
    case 0: f(Tag<float>()); break;
    case 1: f(Tag<double>()); break;
    case 2: f(Tag<int32_t>()); break;
    default: f(Tag<int64_t>()); break;
  }
}

// ------------------------------------------------------------ heat
constexpr int HN = 512;
inline float upd(float c, float n, float s, float e, float w, float a) {
  return c + a * (((n + s) + (e + w)) - 4.0f * c);
}
inline double upd(double c, double n, double s, double e, double w, double a) {
  return c + a * (((n + s) + (e + w)) - 4.0 * c);
}
// Fixed point uses wrapping 32/64-bit arithmetic, exactly like the Rust version.
inline int32_t upd(int32_t c, int32_t n, int32_t s, int32_t e, int32_t w, int32_t a) {
  uint32_t lap = (((uint32_t)n + (uint32_t)s) + ((uint32_t)e + (uint32_t)w)) - 4u * (uint32_t)c;
  return (int32_t)((uint32_t)c + (uint32_t)(((int64_t)a * (int64_t)(int32_t)lap) >> 16));
}
inline int64_t upd(int64_t c, int64_t n, int64_t s, int64_t e, int64_t w, int64_t a) {
  uint64_t lap = (((uint64_t)n + (uint64_t)s) + ((uint64_t)e + (uint64_t)w)) - 4u * (uint64_t)c;
  return (int64_t)((uint64_t)c + (uint64_t)(((__int128)a * (__int128)(int64_t)lap) >> 32));
}

template <class T>
inline void heat_rows(const T* __restrict src, T* __restrict dst, int r0, int r1, T a) {
  const int N = HN;
  for (int y = r0; y < r1; ++y) {
    int ym = (y + N - 1) % N, yp = (y + 1) % N;
    const T* row = src + (size_t)y * N;
    const T* up = src + (size_t)ym * N;
    const T* dn = src + (size_t)yp * N;
    T* o = dst + (size_t)y * N;
    o[0] = upd(row[0], up[0], dn[0], row[1], row[N - 1], a);
    for (int x = 1; x < N - 1; ++x) o[x] = upd(row[x], up[x], dn[x], row[x + 1], row[x - 1], a);
    o[N - 1] = upd(row[N - 1], up[N - 1], dn[N - 1], row[0], row[N - 2], a);
  }
}

// ------------------------------------------------------------ walk
constexpr uint32_t WMASK = 1023;
template <class T, class G>
inline void walk_agents(const G& g, const T* __restrict grid, uint32_t* __restrict px, uint32_t* __restrict py,
                        T* __restrict tally, uint64_t a0, uint64_t a1, uint64_t moment) {
  static const uint32_t DX[4] = {1, WMASK, 0, 0};
  static const uint32_t DY[4] = {0, 0, 1, WMASK};
  for (uint64_t i = a0; i < a1; ++i) {
    uint64_t d = g.draw(i, moment);
    unsigned dir = (unsigned)(d >> 62);
    uint32_t x = (px[i] + DX[dir]) & WMASK, y = (py[i] + DY[dir]) & WMASK;
    px[i] = x;
    py[i] = y;
    tally[i] = add(tally[i], grid[((size_t)y << 10) | x]);
  }
}

// ------------------------------------------------------------ sum
constexpr size_t SUM_B = 4096;
template <class T> struct AccOf { using type = T; };
template <> struct AccOf<int32_t> { using type = int64_t; };

template <class A>
inline A tree_in_place(A* t, size_t n) {
  for (size_t h = n / 2; h >= 1; h /= 2)
    for (size_t i = 0; i < h; ++i) t[i] = add(t[i], t[i + h]);
  return t[0];
}

template <class T>
inline typename AccOf<T>::type block_sum(const T* x) {
  using A = typename AccOf<T>::type;
  A tmp[SUM_B / 2];
  for (size_t i = 0; i < SUM_B / 2; ++i) tmp[i] = add((A)x[i], (A)x[i + SUM_B / 2]);
  return tree_in_place(tmp, SUM_B / 2);
}

template <class A>
inline uint64_t bits_of(A v) {
  uint64_t out = 0;
  __builtin_memcpy(&out, &v, sizeof(A));
  return out;
}

// ------------------------------------------------------------ learn
constexpr int WEIGHTS = 64;
constexpr int FEATS = 256;
template <class T>
inline void learn_update(T* __restrict w, const T* __restrict x, T target) {
  T acc[8] = {0, 0, 0, 0, 0, 0, 0, 0};
  for (int c = 0; c < WEIGHTS / 8; ++c)
    for (int k = 0; k < 8; ++k) acc[k] = acc[k] + w[c * 8 + k] * x[c * 8 + k];
  T y = ((acc[0] + acc[1]) + (acc[2] + acc[3])) + ((acc[4] + acc[5]) + (acc[6] + acc[7]));
  T g = (T)(1.0 / 64.0) * (target - y);
  for (int j = 0; j < WEIGHTS; ++j) w[j] = w[j] + g * x[j];
}
template <>
inline void learn_update<int32_t>(int32_t* __restrict w, const int32_t* __restrict x, int32_t target) {
  int64_t acc = 0;
  for (int j = 0; j < WEIGHTS; ++j) acc += (int64_t)w[j] * (int64_t)x[j];
  int32_t y = (int32_t)(acc >> 16);
  int32_t err = add(target, (int32_t)(0u - (uint32_t)y));
  int32_t g = (int32_t)((1024 * (int64_t)err) >> 16);
  for (int j = 0; j < WEIGHTS; ++j) w[j] = add(w[j], (int32_t)(((int64_t)g * (int64_t)x[j]) >> 16));
}

}  // namespace

// ------------------------------------------------------------ exported entry points
extern "C" {

void KB_FN(heat_step_f32)(const float* s, float* d, int32_t r0, int32_t r1, float a) { heat_rows(s, d, r0, r1, a); }
void KB_FN(heat_step_f64)(const double* s, double* d, int32_t r0, int32_t r1, double a) { heat_rows(s, d, r0, r1, a); }
void KB_FN(heat_step_fx32)(const int32_t* s, int32_t* d, int32_t r0, int32_t r1, int32_t a) { heat_rows(s, d, r0, r1, a); }
void KB_FN(heat_step_fx64)(const int64_t* s, int64_t* d, int32_t r0, int32_t r1, int64_t a) { heat_rows(s, d, r0, r1, a); }

void KB_FN(walk_step)(int32_t fmt, int32_t gen, const KbStream* st, const void* grid, uint32_t* px, uint32_t* py,
                      void* tally, uint64_t a0, uint64_t a1, uint64_t moment) {
  with_fmt(fmt, [&](auto tag) {
    using T = typename decltype(tag)::type;
    with_gen(gen, *st, [&](auto g) {
      walk_agents<T>(g, (const T*)grid, px, py, (T*)tally, a0, a1, moment);
      return 0;
    });
  });
}

void KB_FN(sum_blocks)(int32_t fmt, const void* data, void* partials, uint64_t b0, uint64_t b1) {
  with_fmt(fmt, [&](auto tag) {
    using T = typename decltype(tag)::type;
    using A = typename AccOf<T>::type;
    const T* x = (const T*)data;
    A* p = (A*)partials;
    for (uint64_t b = b0; b < b1; ++b) p[b] = block_sum(x + b * SUM_B);
  });
}

void KB_FN(sum_tree)(int32_t fmt, void* partials, uint64_t n) {
  with_fmt(fmt, [&](auto tag) {
    using T = typename decltype(tag)::type;
    using A = typename AccOf<T>::type;
    tree_in_place((A*)partials, (size_t)n);
  });
}

uint64_t KB_FN(sum_naive)(int32_t fmt, const void* data, uint64_t n) {
  uint64_t out = 0;
  with_fmt(fmt, [&](auto tag) {
    using T = typename decltype(tag)::type;
    using A = typename AccOf<T>::type;
    const T* x = (const T*)data;
    A s = 0;
    for (uint64_t i = 0; i < n; ++i) s = add(s, (A)x[i]);
    out = bits_of(s);
  });
  return out;
}

void KB_FN(learn_step)(int32_t fmt, void* w, const void* feats, const void* targets, uint64_t l0, uint64_t l1,
                       uint64_t step) {
  with_fmt(fmt, [&](auto tag) {
    using T = typename decltype(tag)::type;
    if constexpr (std::is_same_v<T, int64_t>) {
      return;  // fx64 learn is not part of the test
    } else {
      T* W = (T*)w;
      const T* X = (const T*)feats;
      const T* Y = (const T*)targets;
      for (uint64_t l = l0; l < l1; ++l) {
        uint64_t f = (l * 7 + step * 13) & (FEATS - 1);
        learn_update<T>(W + l * WEIGHTS, X + f * WEIGHTS, Y[f]);
      }
    }
  });
}

uint64_t KB_FN(rng_sum)(int32_t gen, const KbStream* st, uint64_t b0, uint64_t b1, uint64_t moment) {
  return with_gen(gen, *st, [&](auto g) {
    uint64_t acc = 0;
    for (uint64_t b = b0; b < b1; ++b) acc += g.draw(b, moment);
    return acc;
  });
}

uint64_t KB_FN(rng_one)(int32_t gen, const KbStream* st, uint64_t being, uint64_t moment) {
  return with_gen(gen, *st, [&](auto g) { return g.draw(being, moment); });
}

}  // extern "C"
