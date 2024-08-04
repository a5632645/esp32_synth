#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef enum : uint8_t {
    MK_KEY_UP = 0,
    MK_KEY_DOWN = 1
} MKKeyStateEnum;

typedef struct MatrixKeyboardConfigT {
    void(*callback)(void* data, int row, int col, MKKeyStateEnum state);
    void* data;
    const int* row_gpio;
    const size_t row_count;
    const int* col_gpio;
    const size_t col_count;
    const bool use_async_task;
} MatrixKeyboardConfigT;

void MatrixKeyboard_Init(const MatrixKeyboardConfigT* pconfig);
void MatrixKeyboard_Tick();

#ifdef __cplusplus
}
#endif