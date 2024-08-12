import math
import numpy as np

with open('./main/model/phase_sin_table.c', 'w') as f:
    table_size = 1 << 14
    scale = table_size
    t = np.arange(table_size, dtype=np.float32)
    t = t / table_size
    t = np.sin(t * np.pi) * scale
    t = np.round(t).astype(np.int32)
    t = np.clip(t, a_min=-32767, a_max=32767)
    f.write('#include <stdint.h>\n')
    f.write(f'const int16_t phase_sin_table[{table_size}] = {{\n')
    for i in range(table_size):
        f.write(f'    {t[i]},\n')
    f.write('};\n')

with open('./main/model/freq_sin_table.c', 'w') as f:
    table_size = 1 << 15
    scale = table_size
    t = np.arange(table_size, dtype=np.float32)
    t = t / table_size
    t = np.sin(t * np.pi) * scale
    t = np.round(t).astype(np.int32)
    t = np.clip(t, a_min=-32767, a_max=32767)
    f.write('#include <stdint.h>\n')
    f.write(f'const int16_t freq_sin_table[{table_size}] = {{\n')
    for i in range(table_size):
        f.write(f'    {t[i]},\n')
    f.write('};\n')

with open('./main/model/sin2cos_table.c', 'w') as f:
    table_size = 1 << 14
    scale = table_size
    t = np.arange(table_size, dtype=np.float32)
    t = np.sqrt(scale**2 - t**2)
    t = np.round(t).astype(np.int32)
    t = np.clip(t, a_min=-32767, a_max=32767)
    f.write('#include <stdint.h>\n')
    f.write(f'const int16_t sin2cos_table[{table_size}] = {{\n')
    for i in range(table_size):
        f.write(f'    {t[i]},\n')
    f.write('};\n')

with open('./main/model/table.h', 'w') as f:
    f.write('#pragma once\n')
    f.write('#include <stdint.h>\n')
    f.write('#ifdef __cplusplus\n')
    f.write('extern "C" {\n')
    f.write('#endif\n')
    f.write(f'extern const int16_t phase_sin_table[{1<<14}];\n')
    f.write(f'extern const int16_t freq_sin_table[{1<<15}];\n')
    f.write(f'extern const int16_t sin2cos_table[{1<<14}];\n')
    f.write('#ifdef __cplusplus\n')
    f.write('}\n')
    f.write('#endif\n')