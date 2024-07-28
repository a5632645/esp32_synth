#pragma once

#ifndef __cplusplus
#error only c++ support
#endif

#include "synth_model.h"

class AddOsc {
public:
    AddOsc(SynthModel& model) : model_(model) {}

    void Init(float sample_rate) {

    }

    void NoteOn(int note, float velocity) {

    }

    void NoteOff(int note, float velocity) {

    }

    void Process(float* buffer, int len) {

    }
private:
    SynthModel& model_;
};