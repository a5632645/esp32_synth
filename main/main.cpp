#include <cmath>
#include <esp_log.h>
#include "i2s_audio.h"
#include "uart_midi.h"
#include "keyboard.h"

class TestGen {
public:
    void Init(float sample_rate) {
        inv_sample_rate_ = 1.0f / sample_rate;
    }
    void Process(float* buffer, int len) {
        if (!note_on_) {
            return;
        }

        for (int i = 0; i < len; i++) {
            phase_ += advance_;
            if (phase_ >= 1.0f) {
                phase_ -= 1.0f;
            }
            buffer[i] += (phase_ - 0.5f) * 0.5f;
        }
    }
    void NoteOn(int note, float velocity) {
        note_on_ = true;
        note_ = note;
        auto freq = 440.0f * std::pow(2.0f, (note - 69.0f) / 12.0f);
        phase_ = 0.0f;
        advance_ = freq * inv_sample_rate_;
    }
    void NoteOff(int note, float velocity) {
        note_on_ = false;
        note_ = -1;
    }
    bool IsPlaying() const { return note_on_; }
    int GetNote() const { return note_; }
private:
    bool note_on_{false};
    int note_{};
    float inv_sample_rate_{};
    float advance_{};
    float phase_{};
};

class PolyGen {
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
    void Process(float* buffer, int len) {
        std::fill(buffer, buffer + len, 0.0f);
        for (auto& osc : oscs_) {
            osc.Process(buffer, len);
        }
    }
private:
    static constexpr int kNumPolyNotes = 4;
    int rr_pos_{};
    TestGen oscs_[kNumPolyNotes];
};

static PolyGen poly_gen;

static void AudioCallback(float* buffer, int len) {
    // for (int i = 0; i < len; i++) {
    //     buffer[i] = rand() * 2.0f / RAND_MAX - 1.0f;
    // }
    poly_gen.Process(buffer, len);
}

static void UartMidiCallback(uint8_t* buffer, int len) {
    uint8_t head = buffer[0] >> 4;
    if (head == 9) {
        // rp303proc.NoteOn(buffer[1], buffer[2] / 127.0f);
        poly_gen.NoteOn(buffer[1], buffer[2] / 127.0f);
        ESP_LOGI("midi", "note on: %d %d", (int)buffer[1], (int)buffer[2]);
    }
    else if (head == 8) {
        // rp303proc.NoteOff(buffer[1], buffer[2] / 127.0f);
        poly_gen.NoteOff(buffer[1], buffer[2] / 127.0f);
        ESP_LOGI("midi", "note off: %d %d", (int)buffer[1], (int)buffer[2]);
    }
}

static void MKCallback(int row, int col, MKKeyStateEnum state) {
    static constexpr int kRemapTable[4][4] {
        { 0, 1, 2, 3 },
        { 4, 5, 7, 6 },
        { 9, 8, 11, 10 },
        { 15, 14, 13, 12 }
    };
    static int octave = 4;

    if(int i = kRemapTable[row][col]; i < 13) {
        auto note = octave * 12 + i;
        if(state == MKKeyStateEnum::MK_KEY_DOWN) {
            poly_gen.NoteOn(note, 1.0f);
            ESP_LOGI("midi", "note on: %d %f", note, 1.0f);
        } else if(state == MKKeyStateEnum::MK_KEY_UP) {
            poly_gen.NoteOff(note, 1.0f);
            ESP_LOGI("midi", "note off: %d %f", note, 1.0f);
        }
    } else {
        if (state == MKKeyStateEnum::MK_KEY_UP) {
            return;
        }

        switch (i) {
        case 12:
            break;
        case 13:
            octave = std::max(octave + 1, 8);
            ESP_LOGI("midi", "octave up %d", octave);
            break;
        case 14:
            break;
        case 15:
            octave = std::min(octave - 1, -2);
            ESP_LOGI("midi", "octave down %d", octave);
            break;
        default:
            break;
        }
    }
}

extern "C" void app_main(void) {
    // rp303proc.Init();

    // for (int i = 0; i < 400; ++i) {
    //     rp303proc.UpdataADC();
    //     vTaskDelay(1);
    // }

    I2sAudioConfigT i2s_config {
        .callback = AudioCallback,
        .i2s_port = I2S_NUM_0,
        .sample_rate = 48000,
        .bck_gpio = 40,
        .ws_gpio = 41,
        .out_gpio = 42,
        .channel_count = I2S_SLOT_MODE_MONO
    };
    poly_gen.Init(static_cast<float>(i2s_config.sample_rate));
    I2sAudioInit(&i2s_config);

    UartMidiConfigT midi_config = {
        .handler = UartMidiCallback,
        .uart_port = UART_NUM_0,
        .rx_gpio = 38,
        .tx_gpio = 39,
        .rts_gpio = UART_PIN_NO_CHANGE,
        .cts_gpio = UART_PIN_NO_CHANGE
    };
    UartMidi_Init(&midi_config);

    static const int row_gpio[] = {4, 5, 6, 7};
    static const int col_gpio[] = {10, 11, 12, 13};

    MatrixKeyboardConfigT matrix_config = {
        .callback = MKCallback,
        .row_gpio = row_gpio,
        .row_count = 4,
        .col_gpio = col_gpio,
        .col_count = 4
    };
    MatrixKeyboard_Init(&matrix_config);
}