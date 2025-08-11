/*
   TripleChorus chorus plugin

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

#ifndef TRIPLECHORUS_HPP
#define TRIPLECHORUS_HPP

#include "DistrhoPlugin.hpp"
#include "svf.hpp"

// total size of delay line buffer
#define DELAYSIZE 1028

START_NAMESPACE_DISTRHO

class TripleChorus : public Plugin {
   public:
    TripleChorus();
    ~TripleChorus();

   protected:
    const char *getLabel() const override { return "TripleChorus"; }
    const char *getDescription() const override {
        return "Triple Chorus";
    }
    const char *getMaker() const override { return "Gordonjcp"; }
    const char *getLicense() const override { return "ISC"; }
    uint32_t getVersion() const override { return d_version(1, 0, 0); }
    int64_t getUniqueId() const override { return d_cconst('3', 'C', 'H', 'O'); }

    // Initialisation
    void initAudioPort(bool input, uint32_t index, AudioPort &port) override;

    // Processing
    void activate() override;
    void deactivate() override;
    void run(const float **inputs, float **outputs, uint32_t frames) override;

   private:
    double sampleRate;
    double fastPhase, fastOmega;
    double slowPhase, slowOmega;
    double fastLfo, slowLfo;

    uint16_t delayptr;

    float *ram;
    float *lpfIn;
    float *lpfOut1, *lpfOut2;

    SVF *preFilter, *postFilter1, *postFilter2;

    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TripleChorus);
};

END_NAMESPACE_DISTRHO

#endif
