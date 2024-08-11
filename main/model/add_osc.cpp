#include "add_osc.h"

#include <numbers>
#include <array>
#include <esp_log.h>
#include "synth_model.h"
#include "my_fp.h"
#include "my_math.h"

constexpr auto pi = std::numbers::pi_v<float>;
constexpr auto twopi = pi * 2.0f;

AddOsc::AddOsc() {
    // cordics_.resize(kNumCordic);
    // cordic_resets_.resize(kNumCordic);
    // freqs_.resize(kMaxNumHarmonics);
}

void AddOsc::Init(float sample_rate)
{
    inv_sample_rate_ = 2.0f / sample_rate;
}

void AddOsc::NoteOn(int note, float velocity) {
    note_curr_ = note;
    note_oned_ = true;
    note_freq_ = MyPitchToFreq(note) * inv_sample_rate_;
    output_gain_ = velocity;
}

void AddOsc::NoteOff(int note, float velocity) {
    note_curr_ = -1;
    output_gain_ = 0.0f;
}

void AddOsc::Process(int16_t* buffer, int len) {
    if (!IsPlaying())
        return;

    InternalCrTick(len);
    if (cordic_active_ == 0)
        return;

    for (int i = 0; i < len; ++i) {
        int32_t tmp = 0;
        Coridc_Tick(&cordics_[0], gains_, cordic_active_, &tmp);
        buffer[i] += tmp >> 12;
    }
}

bool AddOsc::IsPlaying() const {
    return note_curr_ != -1;
}

int AddOsc::GetNote() const {
    return note_curr_;
}

void AddOsc::InternalCrTick(int len) {
    InternalFreq(len);

    if (num_active_ != old_num_active_) {
        old_num_active_ = num_active_;
        auto plus = (num_active_ & 0b111u) == 0u ? 0u : 1u;
        cordic_active_ = (num_active_ >> 3u) + plus;
        // clear gain
        std::fill(gains_ + num_active_, gains_ + kMaxNumHarmonics, 0);
    }

    InternalGain(len);

    if (note_oned_) {
        Coridc_Reset(&cordics_[0], cordic_active_, &cordic_params_[0]);
        note_oned_ = false;
        freq_changed_ = false;
    }
    else if (freq_changed_) {
        Coridc_SetFreq(&cordics_[0], cordic_active_, &cordic_params_[0]);
        freq_changed_ = false;
    }
}

void AddOsc::InternalFreq(int len) {
    if (note_prev_ == note_curr_ && global_model.inharmonic_mode == model_.inharmonic_mode) 
        return;
    note_prev_ = note_curr_;
    model_.inharmonic_mode = global_model.inharmonic_mode;
    freq_changed_ = true;

    uint32_t freq_inc = static_cast<uint32_t>(note_freq_ * 32768);
    constexpr uint32_t kMaxFreq = 30000;

    num_active_ = 0;
    switch (model_.inharmonic_mode) {
    case kInharmonicMode_Linear:
        _InharmonicLinear(freq_inc, kMaxFreq);
        break;
    case kInharmonicMode_Octave:
        _InharmonicOctave(freq_inc, kMaxFreq);
        break;
    case kInharmonicMode_String:
        _InharmonicString(freq_inc, kMaxFreq);
        break;
    default:
        break;
    }
}

static constexpr auto kSaw = []{
    std::array<MyFpS0_15, 64> saw{};
    for (int i = 0; i < saw.size(); ++i)
        saw[i] = MyFpS0_15_FromFloat(std::numbers::inv_pi_v<float> / (i + 1.0f));
    return saw;
}();

void AddOsc::InternalGain(int len) {
    if (global_model.timber != model_.timber) {
        model_.timber = global_model.timber;

        if (model_.timber == 0) {
            std::copy(kSaw.begin(), kSaw.begin() + num_active_, gains_);
        }
    }
}

void AddOsc::_InharmonicLinear(uint32_t freq_inc, uint32_t max_freq) {
    uint32_t freq = freq_inc;
    for (int i = 0; i < kNumCordic; ++i) {
        for (int j = 0; j < 8; ++j) {
            if (freq > max_freq)
                break;

            ++num_active_;
            cordic_params_[i].freq[j] = static_cast<int16_t>(freq);
            freq += freq_inc;
        }

        if (freq > max_freq)
            break;
    }
}

void AddOsc::_InharmonicOctave(uint32_t freq_inc, uint32_t max_freq) {
    uint32_t freq = freq_inc;
    for (int i = 0; i < kNumCordic; ++i) {
        for (int j = 0; j < 8; ++j) {
            if (freq > max_freq)
                break;

            ++num_active_;
            cordic_params_[i].freq[j] = static_cast<int16_t>(freq);
            freq <<= 1;
        }

        if (freq > max_freq)
            break;
    }
}

void AddOsc::_InharmonicString(uint32_t freq_inc, uint32_t max_freq) {
    uint32_t freq = freq_inc;
    for (int i = 0; i < kNumCordic; ++i) {
        for (int j = 0; j < 8; ++j) {
            if (freq > max_freq)
                break;

            ++num_active_;
            cordic_params_[i].freq[j] = static_cast<int16_t>(freq);
            freq <<= 1;
        }

        if (freq > max_freq)
            break;
    }
}
