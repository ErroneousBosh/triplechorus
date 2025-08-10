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

#include "triplechorus.hpp"

START_NAMESPACE_DISTRHO

TripleChorus::TripleChorus() : Plugin(0, 0, 0) {  // no parameters, programs, or states
    lowpass = new float[getBufferSize()];
    ram = new float[DELAYSIZE];  // probably needs to be calculated based on sample rate

    sampleRate = getSampleRate();

    // lfo values taken from a rough simulation
    fastOmega = 6.283 * 5.7 / sampleRate;  // approximate, can be adjusted
    slowOmega = 6.283 * 0.5 / sampleRate;  // again approximate

    fastPhase = 0;
    slowPhase = 0;

    // calculate SVF params
    // hardcoded values for now
    // this is the pre-chorus filter based around TR2
    // It's actually a Sallen-Key filter which is easy to realise in hardware
    // however a State Variable Filter is far easier to realise in software
    // simple is good, and using a little maths we can work out that for
    // R55 = R56 = 22k, C54 = 1.5nF, C76 = 220pF then the filter is at
    // 12.6kHz and a Q of about 1.3
    //
    // Here is the best writeup ever on SVFs
    // https://kokkinizita.linuxaudio.org/papers/digsvfilt.pdf
    float fc = 12600;
    float F = fc / sampleRate;  // FIXME assume 48kHz
    float w = 2 * tan(3.14159 * F);
    float a = w / 1.3;
    float b = w * w;

    // "corrected" SVF params, per Fons Adriaensen
    c1 = (a + b) / (1 + a / 2 + b / 4);
    c2 = b / (a + b);
    d0 = c1 * c2 / 4;
}

// Initialisation function
void TripleChorus::initAudioPort(bool input, uint32_t index, AudioPort &port) {
    Plugin::initAudioPort(input, index, port);
}

// Processing functions
void TripleChorus::activate() {
    // zero out filter state
    in_z1 = in_z2 = 0;
    memset(ram, 0, sizeof(float) * DELAYSIZE);
}

void TripleChorus::deactivate() {
    // zero out the outputs, maybe
    printf("called deactivate()\n");
}

void TripleChorus::run(const float **inputs, float **outputs, uint32_t frames) {
    // actual effects here

    // now run the DSP
    float input, out0 = 0, out120 = 0, out240 = 0, s0 = 0, s1 = 0, x;
    float lfoMod, dly1, frac;
    uint16_t tap, delay;

    for (uint32_t i = 0; i < frames; i++) {
        // run a step of LFO
        fastPhase += fastOmega;
        if (fastPhase > 6.283) fastPhase -= 6.283;
        slowPhase += slowOmega;
        if (slowPhase > 6.283) slowPhase -= 6.283;

        input = inputs[0][i];

        // lowpass filter
        x = input - in_z1 - in_z2;
        in_z2 += c2 * in_z1;
        in_z1 += c1 * x;
        ram[delayptr] = (d0 * x + in_z2);  // store the filtered audio in the buffer

        // now we need to calculate the delay
        // I don't know how long the Solina's delay lines are so I'm guessing 2-4ms for now
        // normalised mod depths, from a quick simulation of the LFO block:
        //   0deg 0.203 slow 0.635 fast
        // 120deg 0.248 slow 0.745 fast
        // 240deg 0.252 slow 0.609 fast

        // 0 degree delay line
        lfoMod = 0.203 * sin(fastPhase) + 0.635 * sin(slowPhase);
        dly1 = (0.002 + (0.002 * lfoMod)) * sampleRate;
        delay = (int)dly1;
        frac = dly1 - delay;

        tap = delayptr - delay;
        s1 = ram[(tap - 1) & 0x3ff];
        s0 = ram[tap & 0x3ff];
        out0 = ((s1 - s0) * frac) + s0;

        // 120 degree delay line
        lfoMod = 0.248 * sin(fastPhase + 2.09) + 0.745 * sin(slowPhase + 2.09);
        dly1 = (0.002 + (0.002 * lfoMod)) * sampleRate;
        delay = (int)dly1;
        frac = dly1 - delay;

        tap = delayptr - delay;
        s1 = ram[(tap - 1) & 0x3ff];
        s0 = ram[tap & 0x3ff];
        out120 = ((s1 - s0) * frac) + s0;

        // 240 degree delay line
        lfoMod = 0.252 * sin(fastPhase + 4.18) + 0.609 * sin(slowPhase + 4.18);
        dly1 = (0.002 + (0.002 * lfoMod)) * sampleRate;
        delay = (int)dly1;
        frac = dly1 - delay;

        tap = delayptr - delay;
        s1 = ram[(tap - 1) & 0x3ff];
        s0 = ram[tap & 0x3ff];
        out240 = ((s1 - s0) * frac) + s0;

        outputs[0][i] = (out0 + out120 + out240) / 3;

        delayptr++;
        delayptr &= 0x3ff;
    }
}

// create the plugin
Plugin *createPlugin() { return new TripleChorus(); }

END_NAMESPACE_DISTRHO
