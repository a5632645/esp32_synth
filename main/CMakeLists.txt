set(CODE_SOURCE
    "lcd.c"
    "i2s_audio.c"
    "uart_midi.c"
    "main.cpp"
    "my_adc.c"
    "keyboard.c"
    "st7735_ll_contex.cpp")

idf_component_register(SRCS ${CODE_SOURCE}
                    INCLUDE_DIRS ".")