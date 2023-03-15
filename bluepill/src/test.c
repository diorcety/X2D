#include "x2d.h"
#include "x2d_encoding.h"

#if defined(ARDUINO)
extern int arduino_printf(const char *__restrict, ...);
#define PRINT_FCT arduino_printf
#else
#include <stdio.h>
#define PRINT_FCT printf
#endif

#if defined(TEST)
#if 1
uint8_t data[] = {0x2f, 0x68, 0x83, 0x00, 0x05, 0x98, 0x0e, 0x03, 0x00, 0xb3, 0x7d, 0x00, 0x19, 0x6f, 0xfc, 0x80};
int main(int argc, char *argv[])
{
    x2d_encode_state_t encode_state;
    x2d_encode_state_reset(&encode_state);
    buffer_t indata_buffer = {data, sizeof(data), sizeof(data), buffer_type_byte};
    PRINT_FCT("==================================================\n");
    x2d_print(indata_buffer.buffer, indata_buffer.content_size);
    PRINT_FCT("==================================================\n");

    uint8_t buffer1[64];
    buffer_t out1_buffer = {buffer1, sizeof(buffer1), 0, buffer_type_byte};
    if (!x2d_encode(&encode_state, &indata_buffer, &out1_buffer, false))
    {
        x2d_decode_state_t decode_state;
        x2d_decode_state_reset(&decode_state);

        uint8_t buffer2[64];
        buffer_t out2_buffer = {buffer2, sizeof(buffer2), 0, buffer_type_byte};
        if (!x2d_decode(&decode_state, &out1_buffer, &out2_buffer, false))
        {
            PRINT_FCT("==================================================\n");
            x2d_print(out2_buffer.buffer, out2_buffer.content_size);
            PRINT_FCT("==================================================\n");
        }
    }
    return 0;
}


#else
uint8_t data[] = {0x33, 0x33, 0x2a, 0xab, 0x55, 0x2c, 0xcd, 0x2b, 0x53, 0x32, 0xb3, 0x33, 0x4b, 0x33, 0x32, 0xb2, 0xd2, 0xb3, 0x33, 0x54, 0xcc, 0xcc, 0xcc, 0xab, 0x34, 0xac, 0xb5, 0x54, 0xb4, 0xac, 0xd5, 0x55, };
int main(int argc, char *argv[])
{
    x2d_encode_state_t encode_state;
    x2d_encode_state_reset(&encode_state);
    buffer_t indata_buffer = {data, sizeof(data), sizeof(data)*8, buffer_type_beb};

    x2d_decode_state_t decode_state;
    x2d_decode_state_reset(&decode_state);

    uint8_t buffer2[64];
    buffer_t out2_buffer = {buffer2, sizeof(buffer2), 0, buffer_type_byte};
    if (!x2d_decode(&decode_state, &indata_buffer, &out2_buffer, false))
    {
        PRINT_FCT("==================================================\n");
        x2d_print(out2_buffer.buffer, out2_buffer.content_size);
        PRINT_FCT("==================================================\n");
    }
    return 0;
}


#endif
#endif