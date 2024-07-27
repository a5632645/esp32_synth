#pragma once
#include <esp_adc/adc_oneshot.h>

#ifdef __cplusplus
extern "C" {
#endif

void SimpleAdcInit(adc_unit_t unit, adc_channel_t channel);
int SimpleAdcRead(adc_unit_t unit, adc_channel_t channel);
float SimpleAdcReadFloat(adc_unit_t unit, adc_channel_t channel);

#ifdef __cplusplus
}
#endif