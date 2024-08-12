set (GUI_SOURCES
    "gui/component.cpp"
    "gui/component_peer.cpp"
    "gui/my_font.cpp"
    "gui/timer_queue.cpp"
    "gui/graphic.cpp"
    "gui/my_frame.cpp"

    "ui/table_panel.cpp"
    "ui/keyboard_panel.cpp"
    "ui/setting_panel.cpp"
    "ui/debug_panel.cpp"
)
set (DRIVER_SOURCES
    "lcd.c"
    "i2s_audio.c"
    "uart_midi.c"
    "my_adc.c"
    "keyboard.c"
)
set (USER_SOURCES
    # "main.cpp"
    # "main2.cpp"
    "main3.cpp"
    "st7735_ll_contex.cpp"
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