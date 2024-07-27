#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    MK_KEY_UP = 0,
    MK_KEY_DOWN = 1
} MKKeyStateEnum;

typedef struct MatrixKeyboardConfigT {
    void(*callback)(int row, int col, MKKeyStateEnum state);
    const int* row_gpio;
    size_t row_count;
    const int* col_gpio;
    size_t col_count;
} MatrixKeyboardConfigT;

void MatrixKeyboard_Init(const MatrixKeyboardConfigT* pconfig);

#ifdef __cplusplus
}
#endif