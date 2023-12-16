#pragma once

#include <array>
#include <complex>

using std::complex;

constexpr double PI = 3.14159265358979323846;


// fft is pretty much always a shit ton faster but hey, i'll keep this anyway
// at least this is something i can easily comprehend in contrast to the fft algorithm
template <size_t N>
void dft(std::array<complex<float>, N>& a) {
    for (int k = 0; k < N; k++) {
        const complex i(0.0f, 1.0f);
        complex sum(0.0f, 0.0f);
        for (int n = 0; n < N; n++) {
            complex y = a[n];
            complex factor = std::exp(-i * 2.0 * (float) PI * (k * n / (float) N));
            sum += y * factor;
        }
        a[k] = sum;
    }
}

template<size_t N>
void fftInPlace(std::array<complex<float>, N>& a) {
    if (N == 1)
        return;

    // might want to heap allocate these if a greater sample size is used in case of stack overflow
    // at the moment these should be bit faster than vectors
    std::array<complex<float>, N / 2> evens;
    std::array<complex<float>, N / 2> odds;
    for (int k = 0; k < N / 2; k++) {
        evens[k] = a[2 * k];
        odds[k] = a[2 * k + 1];
    }
    fftInPlace(evens);
    fftInPlace(odds);

    const complex i(0.0f, 1.0f);
    complex W = std::exp(-i * 2.0f * (float) PI / (float) N);
    complex w(1.0f);
    for (int k = 0; k < N / 2; k++) {
        a[k] = evens[k] + w * odds[k];
        a[k + N / 2] = evens[k] - w * odds[k];
        w *= W;
    }
}

template<size_t N>
std::array<complex<float>, N> fft(std::array<complex<float>, N>&& a) {
    fftInPlace(a);
    return a;
}