#include "x3d.h"
#include "x3d_app.h"
#include "encoding.h"

#if defined(ARDUINO)
extern int arduino_printf(const char *__restrict, ...);
#define PRINT_FCT arduino_printf
#else
#include <stdio.h>
#define PRINT_FCT printf
#endif

#if defined(TEST)

uint8_t data1[] = {0x96, 0x7e, 0xeb, 0x78, 0x33, 0x59, 0xb9, 0xf5, 0xac, 0x3c, 0x56, 0x5b, 0xd3, 0xbe, 0x8c, 0xe9, 0x80, 0x41, 0xd7, 0xc9, 0x4c, 0x09};
uint8_t data2[] = {0x26, 0xFF, 0x40, 0x02, 0x0C, 0x10, 0x02, 0x03, 0x84, 0x85, 0x98, 0x00, 0xAA, 0xBB, 0xFC, 0xE5, 0x04, 0x00,
                   0x00, 0x00, 0x00, 0x1F, 0xFF, 0x00, 0x00, 0x00, 0x00, 0xE0, 0x00, 0x00, 0x01, 0xFF, 0xFF, 0xFF, 0xF6, 0x3C, 0x30,
                   0x7D};
uint8_t data3[] = {0x1F, 0xFF, 0x32, 0x02, 0x0C, 0xAA, 0xBB, 0xCC, 0x00, 0x85, 0x98, 0x00, 0xAA, 0xBB, 0xFB, 0x4D, 0x40, 0x00, 0x00, 0x01, 0x00, 0x1F, 0xFF, 0x00, 0x00, 0x12, 0x34, 0xE0, 0x00, 0x82, 0x0D};
uint8_t data4[] = {0x96, 0x7E, 0xE1, 0x78, 0xB9, 0x5B, 0xBB, 0xF7, 0xF8, 0x36, 0x56, 0xDB, 0xD3, 0x9C, 0xEE, 0x9E, 0xA5, 0x02, 0xAB, 0xEB, 0x41, 0xBB, 0x63, 0xCA, 0x3D, 0x5A, 0x9E, 0xE3, 0x45, 0xAF, 0x87};
typedef struct
{
    uint8_t *data;
    uint8_t length;
    bool raw;
} data_struct;
data_struct data_table[] = {
    {data1, sizeof(data1), true},
    {data2, sizeof(data2), false},
    {data3, sizeof(data3), false},
    {data4, sizeof(data4), true},
};

int main(int argc, char *argv[])
{
    for (int i = 0; i < sizeof(data_table) / sizeof(data_table[0]); ++i)
    {
        data_struct *data = &data_table[i];
        if (data->raw)
        {
            buffer_t indata_buffer = {&data->data[2], data->length - 2, data->length - 2, buffer_type_byte};
            uint8_t buffer1[64];
            buffer_t out_buffer = {buffer1, sizeof(buffer1), 0, buffer_type_byte};
            ccitt_whitening_decoder_state_t cwd;
            ccitt_whitening_decoder_reset(&cwd);
            ccitt_whitening_decoder_process(&cwd, &indata_buffer, &out_buffer);
            PRINT_FCT("==================================================\n");
            x3d_print(&out_buffer.buffer[0], out_buffer.content_size);
            PRINT_FCT("==================================================\n");
        }
        else
        {
            PRINT_FCT("==================================================\n");
            x3d_print(&data->data[0], data->length);
            PRINT_FCT("==================================================\n");
        }
    }

    X3D_APP_STATE x3d_app_state;
    x3d_app_state_init(&x3d_app_state, 0x123456, 0x1);
    int slot = x3d_app_get_free_slot(&x3d_app_state);
    uint8_t buffer1[64];
    int buffer_length = x3d_app_create_pairing_message(&x3d_app_state, buffer1, sizeof(buffer1), 4, 0, x3d_app_state.slots, 0, slot, 0, X3D_PAIRING_PAYLOAD_STATUS_Open);
    x3d_print(buffer1, buffer_length);
    return 0;
}
#endif