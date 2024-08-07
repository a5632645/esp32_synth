#pragma once

#ifndef __cplusplus
#error only c++ support
#endif

template<typename T>
concept CanBeOsc = requires(T t, float a, int b, int16_t* c) {
    t.Init(a);
    t.NoteOn(b, a);
    t.NoteOff(b, a);
    t.Process(c, b);
    {t.GetNote()} -> std::same_as<int>;
    {t.IsPlaying()} -> std::same_as<bool>;
};

template<CanBeOsc MonoOsc>
class PolySynth {
public:
    void Init(float sample_rate) {
        for(auto& osc : oscs_) {
            osc.Init(sample_rate);
        }
    }
    void NoteOn(int note, float velocity) {
        for (int i = 0; i < kNumPolyNotes; ++i) {
            if(!oscs_[rr_pos_].IsPlaying()) {
                oscs_[rr_pos_].NoteOn(note, velocity);
                rr_pos_ = (rr_pos_ + 1) & (kNumPolyNotes - 1);
                return;
            }
            rr_pos_ = (rr_pos_ + 1) & (kNumPolyNotes - 1);
        }
        oscs_[rr_pos_].NoteOn(note, velocity);
        rr_pos_ = (rr_pos_ + 1) & (kNumPolyNotes - 1);
    }
    void NoteOff(int note, float velocity) {
        for (auto& osc : oscs_) {
            if (osc.GetNote() == note) {
                osc.NoteOff(note, velocity);
            }
        }
    }
    void Process(int16_t* buffer, int len) {
        std::fill(buffer, buffer + len, 0.0f);
        for (auto& osc : oscs_) {
            osc.Process(buffer, len);
        }
    }
private:
    static constexpr int kNumPolyNotes = 4;
    int rr_pos_{};
    MonoOsc oscs_[kNumPolyNotes];
};