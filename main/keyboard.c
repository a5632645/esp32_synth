#include <driver/dedic_gpio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/gpio.h>
#include <esp_log.h>
#include <string.h>
#include "keyboard.h"

static void EmptyCallback(int row, int col, MKKeyStateEnum state) {
    ESP_LOGI("keyboard", "row %d col %d state %d", row, col, state);
}

static dedic_gpio_bundle_handle_t row_handle;
static void(*callback)(int row, int col, MKKeyStateEnum state) = EmptyCallback;
static bool* state = NULL;
static gpio_num_t* col_gpios = NULL;
static int col_count = 0;
static int row_count = 0;

static void KeyboardInit(const MatrixKeyboardConfigT* pconfig) {
    state = malloc(sizeof(bool) * pconfig->col_count * pconfig->row_count);
    memset(state, 0, sizeof(bool) * pconfig->col_count * pconfig->row_count);
    col_gpios = malloc(sizeof(gpio_num_t) * pconfig->col_count);
    col_count = pconfig->col_count;
    row_count = pconfig->row_count;
    for (int i = 0; i < pconfig->col_count; i++) {
        col_gpios[i] = pconfig->col_gpio[i];
    }
    // row
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_INPUT,
        .pull_down_en = 1
    };
    for (int i = 0; i < pconfig->row_count; i++) {
        io_conf.pin_bit_mask |= 1 << pconfig->row_gpio[i];
    }
    gpio_config(&io_conf);

    dedic_gpio_bundle_config_t row_cfg = {
        .array_size = pconfig->row_count,
        .gpio_array = pconfig->row_gpio,
        .flags = {
            .in_en = 1,
            .out_en = 0
        }
    };
    dedic_gpio_new_bundle(&row_cfg, &row_handle);

    // col
    gpio_config_t io_conf2 = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_OUTPUT
    };
    for (int i = 0; i < 4; i++) {
        io_conf2.pin_bit_mask |= 1 << pconfig->col_gpio[i];
    }
    gpio_config(&io_conf2);
}

static void KeyboardTask(void *args) {
    for(;;) {
        for(int i = 0; i < col_count; i++) {
            gpio_set_level(col_gpios[i], 1);
            uint32_t val = dedic_gpio_bundle_read_in(row_handle);
            for (int j = 0; j < row_count; j++) {
                bool on = val & (1 << j);
                int offset = i * row_count + j;
                if(on && !state[offset]) {
                    state[offset] = true;
                    callback(j, i, MK_KEY_DOWN);
                }
                else if(!on && state[offset]) {
                    state[offset] = false;
                    callback(j, i, MK_KEY_UP);
                }
            }
            gpio_set_level(col_gpios[i], 0);
        }

        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

void MatrixKeyboard_Init(const MatrixKeyboardConfigT* pconfig) {
    if (pconfig->callback != NULL) {
        callback = pconfig->callback;
    }
    KeyboardInit(pconfig);

    xTaskCreate(KeyboardTask, "KeyboardTask", 4096, NULL, 5, NULL);
}