/*
   Triple Chorus plugin

   Copyright 2025 Gordon JC Pearce <gordonjcp@gjcp.net>

   Permission to use, copy, modify, and/or distribute this software for any
   purpose with or without fee is hereby granted, provided that the above
   copyright notice and this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
   WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
   MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
   SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
   WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
   OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
   CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/
#include "svf.hpp"

#include <math.h>
#include <cstdio>

SVF::SVF() {
    // zero out all values
    z1 = 0;
    z2 = 0;
    c1 = 0;
    c2 = 0;
    d0 = 0;
}

void SVF::setCutoff(float cutoff, float Q, float sampleRate) {
    float F = cutoff / sampleRate;
    float w = 2 * tan(3.14159 * F);
    float a = w / Q;
    float b = w * w;

    // "corrected" SVF params, per Fons Adriaensen
    c1 = (a + b) / (1 + a / 2 + b / 4);
    c2 = b / (a + b);
    d0 = c1 * c2 / 4;
}

void SVF::runSVF(const float *input, float *output, uint32_t frames) {
    float x;
    for (uint32_t i = 0; i < frames; i++) {
        // lowpass filter
        x = input[i] - z1 - z2;
        z2 += c2 * z1;
        z1 += c1 * x;
        output[i] = d0 * x + z2;
    }
    //printf("%f\n", x);
}