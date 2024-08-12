#include <cmath>
#include <array>
#include <string>
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
#include "gui/timer_queue.h"
#include "gui/component_peer.h"
#include "st7735_ll_contex.h"
#include "ui/top_window.h"
#include "ui/my_events.h"
#include <string.h>
#include "model/synth_model.h"
#include "model/poly_synth.h"
#include "model/add_osc.h"

static PolySynth<AddOsc> poly_gen;
static TopWindow top_window;

// ================================================================================
// audio
// ================================================================================
static void AudioCallback(void* buf, int len) {
    int16_t* buffer = (int16_t*)buf;
    poly_gen.Process(buffer, len);
    auto* osc = (OscPanel*)top_window.GetChildUncheck(1);
    osc->PushSample(buffer, len);
}

// ================================================================================
// midi
// ================================================================================
static void UartMidiCallback(uint8_t* buffer, int len) {
    uint8_t head = buffer[0] >> 4;
    if (head == 9) {
        poly_gen.NoteOn(buffer[1], buffer[2] / 127.0f);
        ESP_LOGI("midi", "note on: %d %d", (int)buffer[1], (int)buffer[2]);
    }
    else if (head == 8) {
        poly_gen.NoteOff(buffer[1], buffer[2] / 127.0f);
        ESP_LOGI("midi", "note off: %d %d", (int)buffer[1], (int)buffer[2]);
    }
}

static void NoteOn(int note, float velocity) {
    poly_gen.NoteOn(note, velocity);
    top_window.GetKeyboardPanel().NoteOn(note);
}

static void NoteOff(int note, float velocity) {
    poly_gen.NoteOff(note, velocity);
    top_window.GetKeyboardPanel().NoteOff(note);
}

// ================================================================================
// keyboard
// ================================================================================
static void MKCallback(void* data, int row, int col, MKKeyStateEnum state) {
    static constexpr int kRemapTable[4][4] {
        { 0, 1, 2, 3 },
        { 4, 5, 7, 6 },
        { 9, 8, 11, 10 },
        { 15, 14, 13, 12 }
    };
    
    int i = kRemapTable[row][col];
    MsgQueue::GetInstance().Push({
        .handler = [ii = i, st = state]() {
            MyEvent ee {
                .event_type = st == MKKeyStateEnum::MK_KEY_UP ? events::kButtonUp : events::kButtonDown,
                .sub_type = (uint16_t)ii
            };
            top_window.OnEventGet(ee);
        }
    });

    if(i < 12) {
        auto note = global_model.curr_octave * 12 + i;
        if(state == MKKeyStateEnum::MK_KEY_DOWN) {
            NoteOn(note, 1.0f);
        } else if(state == MKKeyStateEnum::MK_KEY_UP) {
            NoteOff(note, 1.0f);
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

    Vec128Struct lag_vals {};
    Vec128Struct curr_vals {};
    for (;;) {
        curr_vals.s16[0] = SimpleAdcRead(ADC_UNIT_1, ADC_CHANNEL_3); // 12bit, 4096
        curr_vals.s16[1] = SimpleAdcRead(ADC_UNIT_1, ADC_CHANNEL_4);
        curr_vals.s16[2] = SimpleAdcRead(ADC_UNIT_1, ADC_CHANNEL_6);
        curr_vals.s16[3] = SimpleAdcRead(ADC_UNIT_1, ADC_CHANNEL_5);

        constexpr int16_t lag = MyFpS0_15_FromFloat(0.9f);
        lag_vals.s16[0] = (lag * (lag_vals.s16[0] - curr_vals.s16[0]) >> 15) + curr_vals.s16[0];
        lag_vals.s16[1] = (lag * (lag_vals.s16[1] - curr_vals.s16[1]) >> 15) + curr_vals.s16[1];
        lag_vals.s16[2] = (lag * (lag_vals.s16[2] - curr_vals.s16[2]) >> 15) + curr_vals.s16[2];
        lag_vals.s16[3] = (lag * (lag_vals.s16[3] - curr_vals.s16[3]) >> 15) + curr_vals.s16[3];

        for (int i = 0; i < 4; ++i)
            global_model.adc_vals[i] = (lag_vals.s16[i] >> 4) & 255; // 8bit, 256

        vTaskDelay(pdMS_TO_TICKS(100));
    }
    vTaskDelete(nullptr);
}

// ================================================================================
// lcd
// ================================================================================
static void TimerQueueTask(void*) {
    auto last_tick = xTaskGetTickCount();
    auto& tq = TimerQueue::GetInstance();
    for (;;) {
        auto new_tick = xTaskGetTickCount();
        tq.Tick(pdTICKS_TO_MS(new_tick - last_tick));
        last_tick = new_tick;
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    vTaskDelete(nullptr);
}

class StaticsComponent : public Component {
public:
    void DrawSelf(MyGraphic& g) override {
        g.SetColor(colors::kRed);
        g.DrawSingleLineText(std::to_string(drawed_per_sec), 0, 0);
    }

    int drawed_per_sec = 0;
};

static void MyLcdTask(void*) {
    // timer queue init
    xTaskCreate(TimerQueueTask, "TimerQueueTask", 2048, NULL, 5, NULL);
    
    // ll driver init
    static St7735LLContext ll_context;
    spi_master_init(&ll_context.dev, 2, 1, 41, 40, 42);
    lcdInit(&ll_context.dev, 128, 160, 0, 0);
    lcdDisplayOn(&ll_context.dev);

    // gui init
    ComponentPeer peer1 {&ll_context};
    peer1.SetComponent(&top_window);

    // timer task init
    StaticsComponent statics;
    peer1.GetComponent()->AddChild(&statics);
    statics.SetBound(peer1.GetComponent()->GetLocalBound().RemoveFromBottom(8));
    TimerTask statics_task {
        [&statics]() {
            MsgQueue::GetInstance().Push({
                .handler = [&statics]() { statics.Repaint(); }
            });
            return false;
        },
        1000
    };
    TimerQueue::GetInstance().AddTimer(&statics_task, false);

    { // keyboard init
        // static const int row_gpio[] = {46, 3, 8, 18};
        static const int row_gpio[] = {18, 8, 3, 46};
        static const int col_gpio[] = {9, 10, 11, 12};
            MatrixKeyboardConfigT matrix_config = {
            .callback = MKCallback,
            .data = &peer1,
            .row_gpio = row_gpio,
            .row_count = 4,
            .col_gpio = col_gpio,
            .col_count = 4,
            .use_async_task = true
        };
        MatrixKeyboard_Init(&matrix_config);
    }

    auto& mq = MsgQueue::GetInstance();
    auto tick = xTaskGetTickCount();
    constexpr auto kTicksPerSec = pdMS_TO_TICKS(1000);
    int drawed = 0;
    for (;;) {
        if (!mq.CollectMessageIf())
            mq.WaitMessage();
        mq.DispatchMessage();

        if (peer1.HasInvalidRects())
            ++drawed;
        peer1.FlushInvalidRects();

        auto new_tick = xTaskGetTickCount();
        if (new_tick - tick > kTicksPerSec) {
            statics.drawed_per_sec = drawed;
            drawed = 0;
            tick = new_tick;
        }
        vTaskDelay(pdMS_TO_TICKS(20));
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
        .rx_gpio = UART_PIN_NO_CHANGE,
        .tx_gpio = UART_PIN_NO_CHANGE,
        .rts_gpio = UART_PIN_NO_CHANGE,
        .cts_gpio = UART_PIN_NO_CHANGE
    };
    UartMidi_Init(&midi_config);

    xTaskCreate(AdcTask, "AdcTask", 4096, NULL, 5, NULL);
    xTaskCreate(MyLcdTask, "LcdTask", 8192, NULL, 4, NULL);
}