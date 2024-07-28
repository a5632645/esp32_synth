#include <cmath>
#include <array>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include "i2s_audio.h"
#include "uart_midi.h"
#include "keyboard.h"
#include "my_adc.h"
#include "lcd.h"
#include "gui/component.h"
#include "gui/msg_queue.h"
#include "gui/timer_task.h"
#include "gui/timer_queue.h"

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

// ================================================================================
// audio
// ================================================================================
static void AudioCallback(float* buffer, int len) {
    // for (int i = 0; i < len; i++) {
    //     buffer[i] = rand() * 2.0f / RAND_MAX - 1.0f;
    // }
    poly_gen.Process(buffer, len);
}

// ================================================================================
// midi
// ================================================================================
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

// ================================================================================
// keyboard
// ================================================================================
static void MKCallback(int row, int col, MKKeyStateEnum state) {
    static constexpr int kRemapTable[4][4] {
        { 0, 1, 2, 3 },
        { 4, 5, 7, 6 },
        { 9, 8, 11, 10 },
        { 15, 14, 13, 12 }
    };
    static int octave = 4;

    if(int i = kRemapTable[row][col]; i < 12) {
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
            octave = std::min(octave + 1, 8);
            ESP_LOGI("midi", "octave up %d", octave);
            break;
        case 14:
            break;
        case 15:
            octave = std::max(octave - 1, -2);
            ESP_LOGI("midi", "octave down %d", octave);
            break;
        default:
            break;
        }
    }
}

// ================================================================================
// adc
// ================================================================================
static void AdcTask(void*) {
    SimpleAdcInit(ADC_UNIT_1, ADC_CHANNEL_3);
    SimpleAdcInit(ADC_UNIT_1, ADC_CHANNEL_4);
    SimpleAdcInit(ADC_UNIT_1, ADC_CHANNEL_5);
    SimpleAdcInit(ADC_UNIT_1, ADC_CHANNEL_6);

    for (;;) {
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    vTaskDelete(nullptr);
}

// ================================================================================
// lcd
// ================================================================================
class Component1 : public Component, public TimerTask {
public:
    Component1() {
        StartTimerHz(10.0f);
    }

    void TimerCallback() override {
        Repaint(GetLocalBound());
    }

    void PaintSelf(Graphic& g) override {
        auto b = GetLocalBound();
        g.Fill(colors::kGreen);
        g.SetColor(colors::kWhite);
        g.DrawLine(rand() % b.w_, rand() % b.h_, rand() % b.w_, rand() % b.h_);
        g.DrawRect(GetLocalBound());
    }
    void Resized() override {
    }
};

class Component2 : public Component, public TimerTask {
public:
    Component2() {
        StartTimerHz(5.0f);
    }

    void TimerCallback() override {
        Repaint(GetLocalBound());
    }

    void PaintSelf(Graphic& g) override {
        auto b = GetLocalBound();
        g.Fill(colors::kBlack);
        g.SetColor(colors::kWhite);
        g.FillRect(rand() % b.w_, rand() % b.h_, rand() % b.w_, rand() % b.h_);
        g.SetColor(colors::kRed);
        g.DrawRect(GetLocalBound());
    }
    void Resized() override {
    }
};

class MainComponent : public Component, public TimerTask {
public:
    MainComponent(LLContext* ctx)
        : Component(ctx) {
        AddChild(&c1);
        AddChild(&c2);
        StartTimerHz(1.0f);
    }

    void TimerCallback() override {
        slice = (slice + 1) & 3;
        auto b = GetLocalBound();
        b.w_ /= 4;
        b.x_ = b.w_ * slice;
        Repaint(b);
    }

    void PaintSelf(Graphic& g) override {
        g.Fill(Color{uint32_t(rand() & 0xffffffu)});
    }
    void Resized() override {
        c1.SetBound(0, 0, 20, 20);
        c2.SetBound(40, 40, 20, 20);
    }
private:
    int slice{};
    int slice2{};
    Component1 c1;
    Component2 c2;
};

static void MyQueueLock(void* arg) {
    SemaphoreHandle_t* sema = (SemaphoreHandle_t*)arg;
    xSemaphoreTake(*sema, portMAX_DELAY);
}

static void MyQueueUnlock(void* arg) {
    SemaphoreHandle_t* sema = (SemaphoreHandle_t*)arg;
    xSemaphoreGive(*sema);
}

static void MyMessageWait(void* arg) {
    SemaphoreHandle_t* sema = (SemaphoreHandle_t*)arg;
    xSemaphoreTake(*sema, portMAX_DELAY);
}

static void MyQueueNotify(void* arg) {
    SemaphoreHandle_t* sema = (SemaphoreHandle_t*)arg;
    xSemaphoreGive(*sema);
}

static void MyLcdTask(void*) {
    // ll init
    ST7735_t dev;
    spi_master_init(&dev, 2, 1, 41, 40, 42);
    lcdInit(&dev, 128, 160, 0, 0);
    lcdDisplayOn(&dev);
    alignas(32) static uint16_t screen_buffer[128][160] = {};
    LLContext context {screen_buffer};

    // msg queue init
    SemaphoreHandle_t queue_sema = xSemaphoreCreateBinary();
    SemaphoreHandle_t lock_sema = xSemaphoreCreateBinary();
    xSemaphoreGive(lock_sema);

    MsgQueue::lock_obj = &lock_sema;
    MsgQueue::msg_notify_obj = &queue_sema;
    MsgQueue::get_lock = &MyQueueLock;
    MsgQueue::release_lock = &MyQueueUnlock;
    MsgQueue::notify = &MyQueueNotify;
    MsgQueue::wait = &MyMessageWait;

    // gui init
    static MainComponent w{&context};
    w.SetBound(Bound{0,0,128,160});

    auto& mq = MsgQueue::GetInstance();
    for (;;) {
        auto msg = mq.Pop();
        if (msg.command == cmds::kPaint) {
            msg.handler();
            LcdDrawScreen(&dev, (uint16_t*)screen_buffer, 0, 0, 128, 160);
        }
        else {  
            msg.handler();
        }
        // vTaskDelay(pdMS_TO_TICKS(100));
    }
    vTaskDelete(nullptr);
}

static void TimerQueueTask(void*) {
    TimerQueue::min_ms = 10;
    auto last_tick = xTaskGetTickCount();
    for (;;) {
        TimerQueue::GetInstance().Tick();
        vTaskDelay(pdMS_TO_TICKS(TimerQueue::min_ms));
        xTaskDelayUntil(&last_tick, pdMS_TO_TICKS(TimerQueue::min_ms));
    }
    vTaskDelete(nullptr);
}

// ================================================================================
// app
// ================================================================================
extern "C" void app_main(void) {
    I2sAudioConfigT i2s_config {
        .callback = AudioCallback,
        .i2s_port = I2S_NUM_0,
        .sample_rate = 48000,
        .bck_gpio = 16,
        .ws_gpio = 15,
        .out_gpio = 17,
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

    static const int row_gpio[] = {46, 3, 8, 18};
    static const int col_gpio[] = {9, 10, 11, 12};

    MatrixKeyboardConfigT matrix_config = {
        .callback = MKCallback,
        .row_gpio = row_gpio,
        .row_count = 4,
        .col_gpio = col_gpio,
        .col_count = 4
    };
    MatrixKeyboard_Init(&matrix_config);

    xTaskCreate(TimerQueueTask, "TimerQueueTask", 2048, NULL, 5, NULL);
    xTaskCreate(AdcTask, "AdcTask", 4096, NULL, 5, NULL);
    xTaskCreate(MyLcdTask, "LcdTask", 4096, NULL, 5, NULL);
}