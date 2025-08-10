Triple Chorus
=============

Triple Chorus is a crude implementation of a Solina-style "ensemble" chorus. It has three delay paths (very short!) which are modulated with a fast and slow LFO. The three LFO drive signals are 120° out of phase with each other, giving a complex and swirling rich chorus.

Building
--------

Triple Chorus is written using [Distrho Plugin Framework](https://github.com/DISTRHO/DPF). Once you have cloned this repository, you must pull down the DPF submodule:

    cd triplechorus   
    git submodule update --init --recursive
    make

You'll find the finished binaries (LV2, VST2/3, standalone Jack) in `bin/` where they can be copied to wherever you want, probably `~/.lv2/`.

Technical
---------

The DSP code in this plugin is not very good, and makes no claims to performance or accuracy. Even so, it does sound quite good.

The input signal is lowpass filtered and fed to a delay line, with three output taps that are modulated by the two LFOs. They are then mixed together and fed to the output. Unlike a "chorus pedal" type effect, there is no dry signal at all - it's all chorused.

The LFO implementation calls sin() six times per sample which is clearly stupid.

The output lowpass filter is not present and this contributes greatly to the tone.

The LFO rates and depths are based on a quick simulation of the real LFO circuit, and the delay times are a total guess.

I don't know what the BBD circuit is clocked at but the TCA350Y has only 184 stages, so the delay time is given by `t = 184/(2*fc)`. I don't know what the clock speed is, I couldn't get a stable simulation, but similar ensemble effects seem to be in the single-digit milliseconds.

More work is required.