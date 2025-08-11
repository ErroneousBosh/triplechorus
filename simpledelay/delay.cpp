/*
   Simple Delay plugin

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

#include "delay.hpp"

START_NAMESPACE_DISTRHO

SimpleDelay::SimpleDelay() : Plugin(0, 0, 0) {  // no parameters, programs, or states
    lpfIn = new float[getBufferSize()];
    lpfOut1 = new float[getBufferSize()];
    lpfOut2 = new float[getBufferSize()];
    ram = new float[DELAYSIZE];  // probably needs to be calculated based on sample rate

    sampleRate = getSampleRate();

    // lfo values taken from a rough simulation

    fastPhase = 0;
    slowPhase = 0;

    preFilter = new SVF();
    postFilter1 = new SVF();
    postFilter2 = new SVF();

}

SimpleDelay::~SimpleDelay() {
    delete lpfIn;
    delete lpfOut1;
    delete lpfOut2;
    delete ram;
    delete preFilter;
    delete postFilter1;
    delete postFilter2;
}

// Initialisation function
void SimpleDelay::initAudioPort(bool input, uint32_t index, AudioPort &port) {
    Plugin::initAudioPort(input, index, port);
}

// Processing functions
void SimpleDelay::activate() {
    fastOmega = 6.283 * 6.8 / sampleRate;  // approximate, can be adjusted
    slowOmega = 6.283 * 0.7 / sampleRate;  // again approximate

    // zero out the delay buffer
    memset(ram, 0, sizeof(float) * DELAYSIZE);
    memset(lpfIn, 0, sizeof(float) * getBufferSize());
    memset(lpfOut1, 0, sizeof(float) * getBufferSize());
    memset(lpfOut2, 0, sizeof(float) * getBufferSize());

    preFilter->setCutoff(12600, 1.3, sampleRate);
    postFilter1->setCutoff(5000, 3, sampleRate);
    postFilter2->setCutoff(12000, 0.7, sampleRate);
    
}


void SimpleDelay::deactivate() {
    // zero out the outputs, maybe
    printf("called deactivate()\n");
}

void SimpleDelay::run(const float **inputs, float **outputs, uint32_t frames) {
    // actual effects here

    // now run the DSP
    float input, out0 = 0, out120 = 0, out240 = 0, s0 = 0, s1 = 0, x;
    float lfoMod, dly1, frac;
    uint16_t tap, delay;

    // filter the input
    preFilter->runSVF(inputs[0], lpfIn, frames);
    // printf("after filter lpfIn[0] = %6f\n", lpfIn[0]);

    for (uint32_t i = 0; i < frames; i++) {
        // run a step of LFO
        fastPhase += fastOmega;
        if (fastPhase > 6.283) fastPhase -= 6.283;
        slowPhase += slowOmega;
        if (slowPhase > 6.283) slowPhase -= 6.283;

        ram[delayptr] = lpfIn[i] + 0.975 * lpfOut1[i];

        // lowpass filter

        // now we need to calculate the delay
        // I don't know how long the Solina's delay lines are so I'm guessing 2-4ms for now
        // normalised mod depths, from a quick simulation of the LFO block:
        //   0deg 0.203 slow 0.635 fast
        // 120deg 0.248 slow 0.745 fast
        // 240deg 0.252 slow 0.609 fast

#define BASE 0.65
#define AMT 0.005

        // 0 degree delay line
        lfoMod = 0.5 * sin(fastPhase) + 0.0 * sin(slowPhase);
        dly1 = (BASE + (AMT * lfoMod)) * sampleRate;
        delay = (int)dly1;
        frac = dly1 - delay;

        tap = delayptr - delay;
        s1 = ram[(tap - 1) & 0xffff];
        s0 = ram[tap & 0xffff];
        out0 = ((s1 - s0) * frac) + s0;

        lpfOut1[i] = (out0 ) ;

        delayptr++;
        delayptr &= 0xffff;
    }
    postFilter2->runSVF(lpfOut1, outputs[0], frames);
    // outputs[0][0]=0.5;
}

// create the plugin
Plugin *createPlugin() { return new SimpleDelay(); }

END_NAMESPACE_DISTRHO
