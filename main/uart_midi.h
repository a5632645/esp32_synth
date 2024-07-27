#pragma once

#include <driver/uart.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    void(*handler)(uint8_t* buffer, int len);
    int uart_port;
    int rx_gpio;
    int tx_gpio;
    int rts_gpio;
    int cts_gpio;
} UartMidiConfigT;

void UartMidi_Init(const UartMidiConfigT* pconfig);

#ifdef __cplusplus
}
#endif