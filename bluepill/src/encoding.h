#ifndef ENCODING_H_
#define ENCODING_H_

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef enum
{
    buffer_type_byte,
    buffer_type_leb,
    buffer_type_beb
} buffer_type;

typedef struct
{
    uint8_t *buffer;
    size_t allocated_size;
    size_t content_size;
    buffer_type type;
} buffer_t;

typedef enum
{
    PROCESS_RESULT_OK,
    PROCESS_RESULT_RESET,
    PROCESS_RESULT_INVALID_INPUT_TYPE,
    PROCESS_RESULT_INVALID_OUTPUT_TYPE,
    PROCESS_RESULT_NEED_INPUT_DATA,
    PROCESS_RESULT_NEED_OUTPUT_SPACE,
} PROCESS_RESULT;

typedef struct
{
} biphase_mark_decoder_state_t;

typedef struct
{
    uint8_t flip;
} biphase_mark_encoder_state_t;

typedef struct
{
    bool flush;
} buffer_transcoder_state_t;

typedef struct
{
    uint8_t initialized;
    uint8_t preamble_0_count;
    uint8_t preamble_1_count;
} x2d_frame_encoder_state_t;

typedef enum
{
    x2d_frame_DECODE_MACHINE_STATE_INIT,
    x2d_frame_DECODE_MACHINE_STATE_LEAD_1,
    x2d_frame_DECODE_MACHINE_STATE_EXTRA_0,
    x2d_frame_DECODE_MACHINE_STATE_DATA,
    x2d_frame_DECODE_MACHINE_STATE_TRAILING,
} x2d_frame_decoder_machine_state_t;
typedef struct
{
    x2d_frame_decoder_machine_state_t machine_state;
} x2d_frame_decoder_state_t;

#ifdef __cplusplus
extern "C"
{
#endif
    bool biphase_mark_decoder_reset(biphase_mark_decoder_state_t *state);
    PROCESS_RESULT biphase_mark_decoder_process(biphase_mark_decoder_state_t *state, buffer_t *in_data, buffer_t *out_data);

    bool biphase_mark_encoder_reset(biphase_mark_encoder_state_t *state);
    PROCESS_RESULT biphase_mark_encoder_process(biphase_mark_encoder_state_t *state, buffer_t *in_data, buffer_t *out_data);

    bool buffer_transcoder_reset(buffer_transcoder_state_t *state);
    bool buffer_transcoder_flush(buffer_transcoder_state_t *state, bool flush);
    PROCESS_RESULT buffer_transcoder_process(buffer_transcoder_state_t *state, buffer_t *in_data, buffer_t *out_data);

    bool x2d_frame_encoder_reset(x2d_frame_encoder_state_t *state, uint8_t preamble_0_count, uint8_t preamble_1_count);
    PROCESS_RESULT x2d_frame_encoder_process(x2d_frame_encoder_state_t *state, buffer_t *in_data, buffer_t *out_data);

    bool x2d_frame_decoder_reset(x2d_frame_decoder_state_t *state);
    PROCESS_RESULT x2d_frame_decoder_process(x2d_frame_decoder_state_t *state, buffer_t *in_data, buffer_t *out_data);
#ifdef __cplusplus
}
#endif

#endif