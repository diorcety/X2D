#ifndef X2D_ENCODING_H_
#define X2D_ENCODING_H_

#include "encoding.h"

typedef struct
{
    uint8_t data1[64];
    buffer_t buffer1;
    buffer_transcoder_state_t bt1_state;

    uint8_t data2[64];
    buffer_t buffer2;
    x2d_frame_encoder_state_t x2d1_state;

    uint8_t data3[64];
    buffer_t buffer3;
    biphase_mark_encoder_state_t bpe1_state;

    buffer_transcoder_state_t bt2_state;
} x2d_encode_state_t;

typedef struct
{
    uint8_t data1[64];
    buffer_t buffer1;
    buffer_transcoder_state_t bt1_state;

    uint8_t data2[64];
    buffer_t buffer2;
    biphase_mark_decoder_state_t bpd1_state;

    uint8_t data3[64];
    buffer_t buffer3;
    x2d_frame_decoder_state_t x2d1_state;

    buffer_transcoder_state_t bt2_state;
} x2d_decode_state_t;

#ifdef __cplusplus
extern "C"
{
#endif
    bool x2d_encode_state_reset(x2d_encode_state_t *state);
    int x2d_encode(x2d_encode_state_t *state, buffer_t *indata_buffer, buffer_t *outdata_buffer, bool flush);

    bool x2d_decode_state_reset(x2d_decode_state_t *state);
    int x2d_decode(x2d_decode_state_t *state, buffer_t *indata_buffer, buffer_t *outdata_buffer, bool flush);
#ifdef __cplusplus
}
#endif

#endif