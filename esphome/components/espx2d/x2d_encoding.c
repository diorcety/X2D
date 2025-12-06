#include "x2d_encoding.h"

// #define DEBUG
#if defined(DEBUG)
#if defined(ARDUINO)
extern int arduino_printf(const char *__restrict, ...);
#define PRINT_FCT(...) arduino_printf(__VA_ARGS__)
#else
#include <stdio.h>
#define PRINT_FCT(...) printf(__VA_ARGS__)
#endif

#define BUFFER_DUMP(B)                                                                                                \
    for (int i = 0; i < (B).content_size / ((B).type == buffer_type_leb || (B).type == buffer_type_beb ? 8 : 1); ++i) \
    {                                                                                                                 \
        PRINT_FCT("0x%02x, ", (B).buffer[i]);                                                                         \
    }                                                                                                                 \
    PRINT_FCT("\n");

#define DEBUG_DUMP(input, output)                                               \
    PRINT_FCT("Input(%d) Remaining: %d\n", (input).type, (input).content_size); \
    PRINT_FCT("Output(%d) Using: %d\n", (output).type, (output).content_size);  \
    PRINT_FCT("Content: ");                                                     \
    BUFFER_DUMP(output);                                                        \
    PRINT_FCT("\n");
#else
#define PRINT_FCT(...)
#define BUFFER_DUMP(B)
#define DEBUG_DUMP(input, output)
#endif

bool x2d_encode_state_reset(x2d_encode_state_t *state)
{
    state->buffer1 = (buffer_t){state->data1, sizeof(state->data1), 0, buffer_type_leb};
    state->buffer2 = (buffer_t){state->data2, sizeof(state->data2), 0, buffer_type_beb};
    state->buffer3 = (buffer_t){state->data3, sizeof(state->data3), 0, buffer_type_beb};
    buffer_transcoder_reset(&state->bt1_state);
    x2d_frame_encoder_reset(&state->x2d1_state, 9, 6);
    biphase_mark_encoder_reset(&state->bpe1_state);
    buffer_transcoder_reset(&state->bt2_state);
    return true;
}

int x2d_encode(x2d_encode_state_t *state, buffer_t *indata_buffer, buffer_t *outdata_buffer, bool flush)
{
    PROCESS_RESULT ret;

    PRINT_FCT("//////////////////////////////////////////////////\n");
    PRINT_FCT("X2D Encode: ");
    BUFFER_DUMP(*indata_buffer);
    PRINT_FCT("\n");

    PRINT_FCT("buffer_transcoder_process\n");
    buffer_transcoder_flush(&state->bt1_state, flush);
    if ((ret = buffer_transcoder_process(&state->bt1_state, indata_buffer, &state->buffer1)) != PROCESS_RESULT_OK)
    {
        PRINT_FCT("Error 1: %d\n", ret);
        return 1;
    }
    DEBUG_DUMP(*indata_buffer, state->buffer1)

    PRINT_FCT("x2d_frame_encoder_process\n");
    if ((ret = x2d_frame_encoder_process(&state->x2d1_state, &state->buffer1, &state->buffer2)) != PROCESS_RESULT_OK)
    {
        PRINT_FCT("Error 2: %d\n", ret);
        return 1;
    }
    DEBUG_DUMP(state->buffer3, state->buffer2);

    PRINT_FCT("biphase_mark_encoder_process\n");
    if ((ret = biphase_mark_encoder_process(&state->bpe1_state, &state->buffer2, &state->buffer3)) != PROCESS_RESULT_OK)
    {
        PRINT_FCT("Error 3: %d\n", ret);
        return 1;
    }
    DEBUG_DUMP(state->buffer2, state->buffer3);

    PRINT_FCT("buffer_transcoder_process\n");
    buffer_transcoder_flush(&state->bt2_state, flush);
    if ((ret = buffer_transcoder_process(&state->bt2_state, &state->buffer3, outdata_buffer)) != PROCESS_RESULT_OK)
    {
        PRINT_FCT("Error 4: %d\n", ret);
        return 1;
    }
    DEBUG_DUMP(state->buffer1, *outdata_buffer);
    PRINT_FCT("//////////////////////////////////////////////////\n");

    return 0;
}

bool x2d_decode_state_reset(x2d_decode_state_t *state)
{
    state->buffer1 = (buffer_t){state->data1, sizeof(state->data1), 0, buffer_type_beb};
    state->buffer2 = (buffer_t){state->data2, sizeof(state->data2), 0, buffer_type_beb};
    state->buffer3 = (buffer_t){state->data3, sizeof(state->data3), 0, buffer_type_leb};
    buffer_transcoder_reset(&state->bt1_state);
    biphase_mark_decoder_reset(&state->bpd1_state);
    x2d_frame_decoder_reset(&state->x2d1_state);
    buffer_transcoder_reset(&state->bt2_state);
    return true;
}

int x2d_decode(x2d_decode_state_t *state, buffer_t *indata_buffer, buffer_t *outdata_buffer, bool flush)
{
    PROCESS_RESULT ret;

    PRINT_FCT("//////////////////////////////////////////////////\n");
    PRINT_FCT("X2D Decode: ");
    BUFFER_DUMP(*indata_buffer);
    PRINT_FCT("\n");

    buffer_transcoder_flush(&state->bt1_state, flush);
    PRINT_FCT("buffer_transcoder_process\n");
    if ((ret = buffer_transcoder_process(&state->bt1_state, indata_buffer, &state->buffer1)) != PROCESS_RESULT_OK)
    {
        PRINT_FCT("Error 1: %d\n", ret);
        return 1;
    }
    DEBUG_DUMP(*indata_buffer, state->buffer1);

    PRINT_FCT("biphase_mark_decoder_process\n");
    if ((ret = biphase_mark_decoder_process(&state->bpd1_state, &state->buffer1, &state->buffer2)) != PROCESS_RESULT_OK)
    {
        PRINT_FCT("Error 2: %d\n", ret);
        return 1;
    }
    DEBUG_DUMP(state->buffer1, state->buffer2);

    PRINT_FCT("x2d_frame_decoder_process\n");
    if ((ret = x2d_frame_decoder_process(&state->x2d1_state, &state->buffer2, &state->buffer3)) != PROCESS_RESULT_OK)
    {
        PRINT_FCT("Error 3: %d\n", ret);
        return 1;
    }
    DEBUG_DUMP(state->buffer2, state->buffer3);

    buffer_transcoder_flush(&state->bt2_state, flush);
    PRINT_FCT("buffer_transcoder_process\n");
    if ((ret = buffer_transcoder_process(&state->bt2_state, &state->buffer3, outdata_buffer)) != PROCESS_RESULT_OK)
    {
        PRINT_FCT("Error 4: %d\n", ret);
        return 1;
    }
    DEBUG_DUMP(state->buffer3, *outdata_buffer);

    PRINT_FCT("//////////////////////////////////////////////////\n");
    return 0;
}