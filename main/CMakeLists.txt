set (GUI_SOURCES
    "gui/component_peer.cpp"
    "gui/component.cpp"
    "gui/my_font.cpp"
    "gui/my_graphic.cpp"
    "gui/timer_queue.cpp"

    "ui/st7735_color.cpp"
    "ui/st7735_driver.cpp"

    "ui/debug_panel.cpp"
    "ui/keyboard_panel.cpp"
    "ui/setting_panel.cpp"
    "ui/table_panel.cpp"
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
    # "main2.cpp"
    # "main3.cpp"
    "model/global_model.c"
    "model/add_osc.cpp"
    "model/my_fp.s"
    # "model/dr.s"
    # "model/cordic.s"
    "model/phase_sin_table.c"
    "model/freq_sin_table.c"
    "model/sin2cos_table.c"
)

idf_component_register(SRCS ${GUI_SOURCES} ${DRIVER_SOURCES} ${USER_SOURCES}
                    INCLUDE_DIRS ".")