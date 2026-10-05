// P7's Fourier transform, for the linear model of rain over mountains (A7.2, WLD-16): our own, radix 2, its turning
// factors from P5's sine and cosine, so it gives the same bits everywhere (A3.4). The torus is periodic, as the
// transform assumes, so the land needs no padding. Pre-production code (research 00).
#pragma once

#include <vector>

#include "pool.hpp"

namespace worldgen {

struct Complex {
    double re = 0.0;
    double im = 0.0;
};

// Transforms a grid of width × height, both powers of two, in place, rows then columns, each row or column on one
// thread; the inverse divides by the count.
void fft2(std::vector<Complex>* grid, int width, int height, bool inverse, minds::Pool* pool);

}  // namespace worldgen
