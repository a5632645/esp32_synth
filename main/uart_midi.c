#include <driver/uart.h>
#include <freertos/FreeRTOS.h>
#include <string.h>
#include <esp_log.h>
#include "uart_midi.h"

typedef enum {
    kMidi_Empty,
    kMidi_Still
} MidiStateEnum;

static void UartMidi_DefaultHandler(uint8_t* buffer, int size) {
    uint8_t head = buffer[0] >> 4;
    if(head == 9) {
        ESP_LOGI("uart", "note on: %d %d", (int)buffer[1], (int)buffer[2]);
    }
    else if(head == 8) {
        ESP_LOGI("uart", "note off: %d %d", (int)buffer[1], (int)buffer[2]);
    }
}

static void(*uartmidi_handler)(uint8_t* buffer, int size) = UartMidi_DefaultHandler;
static int uartmidi_port = UART_NUM_MAX;

static bool IsSupportMsg(uint8_t type) {
    uint8_t head = type >> 4;
    return head == 9 || head == 8;
}

static uint32_t GetMsgNeed(uint8_t type) {
    return 2;
}

static void UartMidiProcessStream(uint8_t* buffer, size_t size) {
    static MidiStateEnum state = kMidi_Empty;
    static uint8_t read_buffer[16] = {};
    static uint32_t read_count = 0;
    static uint32_t msg_need = 0;

    ESP_LOG_BUFFER_HEX("uart", buffer, size);

    for (size_t offset = 0; offset < size;) {
        if (state == kMidi_Empty) {
            // seek for support midi msg
            while (true) {
                if (IsSupportMsg(buffer[offset])) {
                    read_buffer[0] = buffer[offset];
                    state = kMidi_Still;
                    read_count = 1;
                    msg_need = GetMsgNeed(read_buffer[0]);
                    ++offset;
                    break;
                }
                if (++offset >= size)
                    return;
            }
        }

        if (state == kMidi_Still) {
            if(offset == size)
                return;

            uint32_t can_read = size - offset;
            if (can_read < msg_need) {
                memcpy(read_buffer + read_count, buffer + offset, can_read);
                msg_need -= can_read;
                read_count += can_read;
                return;
            }
            else {
                memcpy(read_buffer + read_count, buffer + offset, msg_need);
                offset += msg_need;
                uartmidi_handler(read_buffer, read_count);
                state = kMidi_Empty;
                read_count = 0;
                msg_need = 0;
            }
        }
    }
}

static void UartTask(void *arg)
{
    uint8_t buffer[64] = {0};
    for (;;)
    {
        int buffered_size = uart_read_bytes(uartmidi_port, buffer, 64, pdMS_TO_TICKS(20));
        if(buffered_size > 0) {
            UartMidiProcessStream(buffer, buffered_size);
        } 
    }
    vTaskDelete(NULL);
}

// ================================================================================
// public
// ================================================================================
void UartMidi_Init(const UartMidiConfigT* pconfig) {
    if(pconfig->handler != NULL) {
        uartmidi_handler = pconfig->handler;
    }
    uartmidi_port = pconfig->uart_port;

    ESP_ERROR_CHECK(uart_driver_install(pconfig->uart_port, 1024 * 2, 0, 0, NULL, 0));

    uart_config_t cfg = {};
    cfg.baud_rate = 31250;
    cfg.data_bits = UART_DATA_8_BITS;
    cfg.parity = UART_PARITY_DISABLE;
    cfg.stop_bits = UART_STOP_BITS_1;
    cfg.flow_ctrl = UART_HW_FLOWCTRL_DISABLE;
    cfg.source_clk  = UART_SCLK_DEFAULT;
    cfg.rx_flow_ctrl_thresh = 7;
    ESP_ERROR_CHECK(uart_param_config(pconfig->uart_port, &cfg));

    ESP_ERROR_CHECK(uart_set_pin(pconfig->uart_port, pconfig->tx_gpio, pconfig->rx_gpio,
                                     pconfig->rts_gpio, pconfig->cts_gpio));

    xTaskCreate(UartTask, "uart midi", 8192, NULL, 5, NULL);
}