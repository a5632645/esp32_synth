#include <stdlib.h>
#include <driver/i2s_std.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include "i2s_audio.h"

static void I2sAudio_DefaultCallback(void* buffer, int len) {
    static const float scale = 1.0f / RAND_MAX;
    float *buf = (float*)buffer;
    for(int i = 0; i < len; i++) {
        buf[i] = rand() * scale * 2.0f - 1.0f;
    }
}

static void(*i2s_audio_callback)(void* buffer, int len) = I2sAudio_DefaultCallback;

typedef int16_t BufferTypeT;
#define BUFFER_TYPE_MAX 0x2000
#define BUFFER_LENGTH 1024
#define BUFFER_SIZE (sizeof(BufferTypeT) * BUFFER_LENGTH)

static i2s_chan_handle_t tx_chan; // I2S tx channel handler
static i2s_chan_handle_t rx_chan; // I2S rx channel handler

// static SemaphoreHandle_t send_comp_sem = {0};
// BufferTypeT buffer0[BUFFER_LENGTH] = {0};
// BufferTypeT buffer1[BUFFER_LENGTH] = {0};
// BufferTypeT* write_ptr = buffer0;
// BufferTypeT* read_ptr = buffer1;
BufferTypeT buffer[BUFFER_LENGTH] = {0};

static void I2sWriteTask(void* args) {
    while (true) {
        // i2s_channel_write(tx_chan, read_ptr, sizeof(buffer0), NULL, portMAX_DELAY);
        // BufferTypeT* tmp = write_ptr;
        // write_ptr = read_ptr;
        // read_ptr = tmp;
        // xSemaphoreGive(send_comp_sem);
        i2s_audio_callback(buffer, BUFFER_LENGTH);
        i2s_channel_write(tx_chan, buffer, sizeof(buffer), NULL, portMAX_DELAY);
        vTaskDelay(1);
    }
    vTaskDelete(NULL);
}

// static void AudioProduceTask(void *args) {
//     float* buffer = malloc(sizeof(float) * BUFFER_LENGTH);
//     while (true) {
//         i2s_audio_callback(buffer, BUFFER_LENGTH);
//         for (int i = 0; i < BUFFER_LENGTH; i++) {
//             int32_t t = buffer[i] * BUFFER_TYPE_MAX;
//             t = t > BUFFER_TYPE_MAX ? BUFFER_TYPE_MAX : t;
//             t = t < -BUFFER_TYPE_MAX ? -BUFFER_TYPE_MAX : t;
//             write_ptr[i] = t;
//         }
//         xSemaphoreTake(send_comp_sem, portMAX_DELAY);
//     }
//     free(buffer);
//     vTaskDelete(NULL);
// }

void I2sAudioInit(const I2sAudioConfigT* pconfig) {
    if(pconfig->callback != NULL) {
        i2s_audio_callback = pconfig->callback;
    }

    // vSemaphoreCreateBinary(send_comp_sem);

    i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_AUTO, I2S_ROLE_MASTER);
    ESP_ERROR_CHECK(i2s_new_channel(&chan_cfg, &tx_chan, &rx_chan));

    i2s_std_config_t std_cfg = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(pconfig->sample_rate),
        .slot_cfg = I2S_STD_MSB_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, pconfig->channel_count),
        .gpio_cfg = {
            .mclk = GPIO_NUM_NC, // some codecs may require mclk signal, this example doesn't need it
            .bclk = pconfig->bck_gpio,
            .ws = pconfig->ws_gpio,
            .dout = pconfig->out_gpio,
            .din = GPIO_NUM_NC, // In duplex mode, bind output and input to a same gpio can loopback internally
            .invert_flags = {
                .mclk_inv = false,
                .bclk_inv = false,
                .ws_inv = false,
            },
        },
    };
    /* Initialize the channels */
    ESP_ERROR_CHECK(i2s_channel_init_std_mode(tx_chan, &std_cfg));
    ESP_ERROR_CHECK(i2s_channel_init_std_mode(rx_chan, &std_cfg));
    ESP_ERROR_CHECK(i2s_channel_enable(tx_chan));

    xTaskCreatePinnedToCore(I2sWriteTask, "I2sWriteTask", 2048, NULL, 5, NULL, 1);
    // xTaskCreatePinnedToCore(AudioProduceTask, "AudioProduceTask", 2048, NULL, 5, NULL, 1);
}
