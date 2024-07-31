set (GUI_SOURCES
    "gui/component.cpp"
    "gui/component_peer.cpp"
    "gui/my_font.cpp"
    "gui/timer_queue.cpp"
    "gui/graphic.cpp"
)
set (DRIVER_SOURCES
    "lcd.c"
    "i2s_audio.c"
    "uart_midi.c"
    "my_adc.c"
    "keyboard.c"
)
set (USER_SOURCES
    "main.cpp"
    "st7735_ll_contex.cpp"
)

idf_component_register(SRCS ${GUI_SOURCES} ${DRIVER_SOURCES} ${USER_SOURCES}
                    INCLUDE_DIRS ".")