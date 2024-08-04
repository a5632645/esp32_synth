#include "esp_log.h"
#include "my_adc.h"

static adc_oneshot_unit_handle_t adc_handle[2] = {};

void SimpleAdcInit(adc_unit_t unit, adc_channel_t channel) {
    if(adc_handle[unit] == NULL ) {
        adc_oneshot_unit_init_cfg_t adc_cfg = {
            .clk_src = ADC_RTC_CLK_SRC_DEFAULT,
            .ulp_mode = ADC_ULP_MODE_DISABLE,
            .unit_id = unit
        };
        ESP_ERROR_CHECK(adc_oneshot_new_unit(&adc_cfg, adc_handle + unit));
    }

    adc_oneshot_chan_cfg_t channel_cfg = {
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_12
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc_handle[unit], channel, &channel_cfg));
}

int SimpleAdcRead(adc_unit_t unit, adc_channel_t channel) {
    int value = 0;
    ESP_ERROR_CHECK(adc_oneshot_read(adc_handle[unit], channel, &value));
    return value;
}

#define ADC_READ_SCALE (1.0f / 0.9f / (1 << 12))
float SimpleAdcReadFloat(adc_unit_t unit, adc_channel_t channel) {
    int value = SimpleAdcRead(unit, channel);
    return value * ADC_READ_SCALE;
}