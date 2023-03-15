#include "encoding.h"

#include <string.h>
#include <assert.h>
#include <stdlib.h>

#define ENCODING_ASSERT(__e) ((__e) ? (void)0 : abort())

static uint8_t _get_used_buffer_index(buffer_t *buffer, size_t index)
{
    size_t r_index = index;
    switch (buffer->type)
    {
    case buffer_type_byte:
        return buffer->buffer[r_index];
        break;
    case buffer_type_beb:
    case buffer_type_leb:
    {
        uint8_t shift = (buffer->type == buffer_type_beb) ? (7 - (index % 8)) : (index % 8);
        return (buffer->buffer[r_index / 8] >> shift) & 0x1;
        break;
    }
    default:
        ENCODING_ASSERT(false);
        break;
    }
}

static void _set_free_buffer_index(buffer_t *buffer, size_t index, uint8_t v)
{
    size_t r_index = buffer->content_size + index;
    switch (buffer->type)
    {
    case buffer_type_byte:
        buffer->buffer[r_index] = v;
        break;
    case buffer_type_beb:
    case buffer_type_leb:
    {
        uint8_t shift = (buffer->type == buffer_type_beb) ? (7 - (r_index % 8)) : (r_index % 8);
        buffer->buffer[r_index / 8] &= ~(0x1 << shift);
        buffer->buffer[r_index / 8] |= (v & 0x1) << shift;
        break;
    }
    default:
        ENCODING_ASSERT(false);
        break;
    }
}

static bool _buffer_have_n_available(buffer_t *buffer, size_t index, size_t n)
{
    size_t r_index = index + n;
    return r_index <= buffer->content_size;
}

static bool _buffer_have_n_space(buffer_t *buffer, size_t index, size_t n)
{
    size_t r_index = buffer->content_size + index + n;
    switch (buffer->type)
    {
    case buffer_type_byte:
        return r_index < buffer->allocated_size;
        break;
    case buffer_type_beb:
    case buffer_type_leb:
        return (r_index / 8) < buffer->allocated_size;
        break;
    default:
        ENCODING_ASSERT(false);
        break;
    }
}

static void _buffer_consume(buffer_t *data, size_t consumed_size)
{
    size_t remaining_size = data->content_size - consumed_size;
    data->content_size = 0;
    for (int i = 0; i < remaining_size; ++i)
    {
        _set_free_buffer_index(data, i, _get_used_buffer_index(data, consumed_size + i));
    }
    data->content_size = remaining_size;
}

static void _buffer_apply(buffer_t *data, size_t produced_size)
{
    data->content_size += produced_size;
}

#define ASSERT_IN_AVAIL(B, C, N)            \
    if (!_buffer_have_n_available(B, C, N)) \
    return PROCESS_RESULT_NEED_INPUT_DATA
#define ASSERT_OUT_SPACE(B, C, N)       \
    if (!_buffer_have_n_space(B, C, N)) \
    return PROCESS_RESULT_NEED_OUTPUT_SPACE

#define ASSERT_IN_TYPE(C) \
    if (!(C))             \
    return PROCESS_RESULT_INVALID_INPUT_TYPE
#define ASSERT_OUT_TYPE(C) \
    if (!(C))              \
    return PROCESS_RESULT_INVALID_OUTPUT_TYPE

bool biphase_mark_decoder_reset(biphase_mark_decoder_state_t *state)
{
    return true;
}

PROCESS_RESULT biphase_mark_decoder_process(biphase_mark_decoder_state_t *state, buffer_t *in_data, buffer_t *out_data)
{
    size_t in_index = 0;
    size_t out_index = 0;
    ASSERT_IN_TYPE(in_data->type == buffer_type_beb || in_data->type == buffer_type_leb);
    ASSERT_IN_AVAIL(in_data, in_index, 2);
    ASSERT_OUT_TYPE(out_data->type == buffer_type_beb || out_data->type == buffer_type_leb);
    ASSERT_OUT_SPACE(out_data, out_index, 1);
    do
    {
        _set_free_buffer_index(out_data, out_index, _get_used_buffer_index(in_data, in_index) ^ _get_used_buffer_index(in_data, in_index + 1));
        in_index += 2;
        out_index += 1;
    } while (_buffer_have_n_available(in_data, in_index, 2) && _buffer_have_n_space(out_data, out_index, 1));
    _buffer_consume(in_data, in_index);
    _buffer_apply(out_data, out_index);
    return PROCESS_RESULT_OK;
}

bool biphase_mark_encoder_reset(biphase_mark_encoder_state_t *state)
{
    state->flip = 1;
    return true;
}

PROCESS_RESULT biphase_mark_encoder_process(biphase_mark_encoder_state_t *state, buffer_t *in_data, buffer_t *out_data)
{
    size_t in_index = 0;
    size_t out_index = 0;
    ASSERT_IN_TYPE(in_data->type == buffer_type_beb || in_data->type == buffer_type_leb);
    ASSERT_IN_AVAIL(in_data, in_index, 1);
    ASSERT_OUT_TYPE(out_data->type == buffer_type_beb || out_data->type == buffer_type_leb);
    ASSERT_OUT_SPACE(out_data, out_index, 2);
    do
    {
        state->flip = 1 - state->flip;
        _set_free_buffer_index(out_data, out_index, state->flip);
        state->flip = (state->flip + _get_used_buffer_index(in_data, in_index)) & 0x1;
        _set_free_buffer_index(out_data, out_index + 1, state->flip);
        in_index += 1;
        out_index += 2;
    } while (_buffer_have_n_available(in_data, in_index, 1) && _buffer_have_n_space(out_data, out_index, 2));
    _buffer_consume(in_data, in_index);
    _buffer_apply(out_data, out_index);
    return PROCESS_RESULT_OK;
}

bool buffer_transcoder_reset(buffer_transcoder_state_t *state)
{
    return true;
}

bool buffer_transcoder_flush(buffer_transcoder_state_t *state, bool flush)
{
    state->flush = flush;
    return true;
}

PROCESS_RESULT buffer_transcoder_process(buffer_transcoder_state_t *state, buffer_t *in_data, buffer_t *out_data)
{
    size_t in_index = 0;
    size_t out_index = 0;
    if (in_data->type == buffer_type_beb || in_data->type == buffer_type_leb)
    {
        if (out_data->type == buffer_type_beb || out_data->type == buffer_type_leb)
        {
            ASSERT_IN_AVAIL(in_data, in_index, 1);
            ASSERT_OUT_SPACE(out_data, out_index, 1);
            do
            {
                _set_free_buffer_index(out_data, out_index, _get_used_buffer_index(in_data, in_index));
                in_index += 1;
                out_index += 1;
            } while (_buffer_have_n_available(in_data, in_index, 1) && _buffer_have_n_space(out_data, out_index, 1));
        }
        else
        {
            ASSERT_IN_AVAIL(in_data, in_index, state->flush ? 1 : 8);
            ASSERT_OUT_SPACE(out_data, out_index, 1);
            do
            {
                uint8_t v = 0;
                for (int i = 0; i < 8; ++i)
                {
                    if (!_buffer_have_n_available(in_data, in_index, 1))
                    {
                        break;
                    }
                    v |= _get_used_buffer_index(in_data, in_index++) << i;
                }
                _set_free_buffer_index(out_data, out_index++, v);
            } while (_buffer_have_n_available(in_data, in_index, state->flush ? 1 : 8) && _buffer_have_n_space(out_data, out_index, 1));
        }
    }
    else
    {
        ASSERT_IN_AVAIL(in_data, in_index, 1);
        ASSERT_OUT_SPACE(out_data, out_index, state->flush ? 1 : 8);

        do
        {
            for (int i = 0; i < 8; ++i)
            {
                if (!_buffer_have_n_space(out_data, out_index, 1))
                {
                    break;
                }
                _set_free_buffer_index(out_data, out_index++, _get_used_buffer_index(in_data, in_index) >> i);
            }
            in_index += 1;
        } while (_buffer_have_n_available(in_data, in_index, 1) && _buffer_have_n_space(out_data, out_index, state->flush ? 1 : 8));
    }

    _buffer_consume(in_data, in_index);
    _buffer_apply(out_data, out_index);
    return PROCESS_RESULT_OK;
}

#define x2d_frame_MIN_LEADING_ZEROS 7
#define x2d_frame_MIN_LEADING_ONES 6
#define x2d_frame_MAX_SUCCESSIVE_ONES 5
#define x2d_frame_END_OF_FRAME    \
    {                             \
        1, 1, 1, 1, 1, 1, 1, 1, 0 \
    }
#define x2d_frame_SEPARATOR \
    {                       \
        1, 1, 1, 1, 1, 1, 0 \
    }
#define x2d_frame_TRAILING_LENGTH 7
#define x2d_frame_EXTRA_0_LENGTH 1
#define x2d_frame_MAX_BIT_LENGTH 16 * 8

static uint8_t x2d_frame_END_OF_FRAME_ARRAY[] = x2d_frame_END_OF_FRAME;
static uint8_t x2d_frame_SEPARATOR_ARRAY[] = x2d_frame_SEPARATOR;

bool x2d_frame_encoder_reset(x2d_frame_encoder_state_t *state, uint8_t preamble_0_count, uint8_t preamble_1_count)
{
    state->initialized = 0;
    state->preamble_0_count = preamble_0_count;
    state->preamble_1_count = preamble_1_count;
    return true;
}

PROCESS_RESULT x2d_frame_encoder_process(x2d_frame_encoder_state_t *state, buffer_t *in_data, buffer_t *out_data)
{
    size_t in_index = 0;
    size_t out_index = 0;

    ASSERT_IN_AVAIL(in_data, in_index, 8 * 8);
    ASSERT_IN_TYPE(in_data->type == buffer_type_beb || in_data->type == buffer_type_leb);
    ASSERT_IN_TYPE(out_data->type == buffer_type_beb || out_data->type == buffer_type_leb);

    if (state->initialized == 0)
    {
        // Leading zeros
        ASSERT_OUT_SPACE(out_data, out_index, state->preamble_0_count);
        for (int i = 0; i < state->preamble_0_count; ++i)
        {
            _set_free_buffer_index(out_data, out_index++, 0);
        }

        // Leading Ones
        ASSERT_OUT_SPACE(out_data, out_index, state->preamble_1_count);
        for (int i = 0; i < state->preamble_1_count; ++i)
        {
            _set_free_buffer_index(out_data, out_index++, 1);
        }

        // Extra 0
        ASSERT_OUT_SPACE(out_data, out_index, 1);
        _set_free_buffer_index(out_data, out_index++, 0);
    }

    // Data
    int successive_one = 0;
    for (int i = 0; i < in_data->content_size; ++i)
    {
        ASSERT_OUT_SPACE(out_data, out_index, 1);
        uint8_t v = _get_used_buffer_index(in_data, in_index++);
        _set_free_buffer_index(out_data, out_index++, v);

        if (v != 0)
        {
            successive_one++;
            if (successive_one >= x2d_frame_MAX_SUCCESSIVE_ONES)
            {
                ASSERT_OUT_SPACE(out_data, out_index, 1);
                _set_free_buffer_index(out_data, out_index++, 0);
                successive_one = 0;
            }
        }
        else
        {
            successive_one = 0;
        }
    }

    // End of frame
    ASSERT_OUT_SPACE(out_data, out_index, sizeof(x2d_frame_END_OF_FRAME_ARRAY));
    for (int i = 0; i < sizeof(x2d_frame_END_OF_FRAME_ARRAY); ++i)
    {
        _set_free_buffer_index(out_data, out_index++, x2d_frame_END_OF_FRAME_ARRAY[i]);
    }

    // Separator
    ASSERT_OUT_SPACE(out_data, out_index, sizeof(x2d_frame_SEPARATOR_ARRAY));
    for (int i = 0; i < sizeof(x2d_frame_SEPARATOR_ARRAY); ++i)
    {
        _set_free_buffer_index(out_data, out_index++, x2d_frame_SEPARATOR_ARRAY[i]);
    }

    _buffer_consume(in_data, in_index);
    _buffer_apply(out_data, out_index);
    state->initialized = 1;
    return PROCESS_RESULT_OK;
}

bool x2d_frame_decoder_reset(x2d_frame_decoder_state_t *state)
{
    state->machine_state = x2d_frame_DECODE_MACHINE_STATE_INIT;
    return true;
}

static uint8_t count_leading(buffer_t *data, size_t *psize)
{
    size_t index = 0;
    uint8_t value = 2;
    while (_buffer_have_n_available(data, index, 1))
    {
        if (value != 0 && value != 1)
        {
            value = _get_used_buffer_index(data, index);
        }
        else
        {
            if (value != _get_used_buffer_index(data, index))
            {
                *psize = index;
                return value;
            }
        }
        index++;
    }
    *psize = index > 0 ? index - 1 : 0;
    return value;
}

static bool find_end_frame(buffer_t *data, size_t *pindex)
{
    size_t index = 0;
    while (_buffer_have_n_available(data, index, sizeof(x2d_frame_END_OF_FRAME_ARRAY)))
    {
        bool found = true;
        for (int i = 0; i < sizeof(x2d_frame_END_OF_FRAME_ARRAY) && found == true; ++i)
        {
            found &= _get_used_buffer_index(data, index + i) == x2d_frame_END_OF_FRAME_ARRAY[i];
        }
        if (found)
        {
            *pindex = index;
            return true;
        }
        index++;
    }
    return false;
}

PROCESS_RESULT x2d_frame_decoder_process(x2d_frame_decoder_state_t *state, buffer_t *in_data, buffer_t *out_data)
{
    size_t in_index = 0;
    size_t out_index = 0;

    ASSERT_IN_AVAIL(in_data, in_index, 8 * 8);
    ASSERT_IN_TYPE(in_data->type == buffer_type_beb || in_data->type == buffer_type_leb);
    ASSERT_IN_TYPE(out_data->type == buffer_type_beb || out_data->type == buffer_type_leb);

    while (true)
    {
        switch (state->machine_state)
        {
        case x2d_frame_DECODE_MACHINE_STATE_INIT:
        {
            size_t prefix_size;
            uint8_t value = count_leading(in_data, &prefix_size);
            if (prefix_size == 0)
            {
                return PROCESS_RESULT_NEED_INPUT_DATA;
            }
            if (prefix_size >= x2d_frame_MIN_LEADING_ZEROS && value == 0)
            {
                _buffer_consume(in_data, prefix_size);
                state->machine_state = x2d_frame_DECODE_MACHINE_STATE_LEAD_1;
            }
            else
            {
                _buffer_consume(in_data, prefix_size);
                state->machine_state = x2d_frame_DECODE_MACHINE_STATE_INIT;
            }
            break;
        }
        case x2d_frame_DECODE_MACHINE_STATE_LEAD_1:
        {
            size_t prefix_size;
            uint8_t value = count_leading(in_data, &prefix_size);
            if (prefix_size == 0)
            {
                return PROCESS_RESULT_NEED_INPUT_DATA;
            }
            if (prefix_size >= x2d_frame_MIN_LEADING_ONES && value == 1)
            {
                _buffer_consume(in_data, prefix_size);
                state->machine_state = x2d_frame_DECODE_MACHINE_STATE_EXTRA_0;
            }
            else
            {
                _buffer_consume(in_data, prefix_size);
                state->machine_state = x2d_frame_DECODE_MACHINE_STATE_INIT;
            }
            break;
        }
        case x2d_frame_DECODE_MACHINE_STATE_EXTRA_0:
        {
            ASSERT_IN_AVAIL(in_data, in_index, x2d_frame_EXTRA_0_LENGTH);
            _buffer_consume(in_data, x2d_frame_EXTRA_0_LENGTH);
            state->machine_state = x2d_frame_DECODE_MACHINE_STATE_DATA;
            break;
        }
        case x2d_frame_DECODE_MACHINE_STATE_DATA:
        {
            size_t end_of_frame_index;
            if (!find_end_frame(in_data, &end_of_frame_index))
            {
                if (in_data->content_size >= x2d_frame_MAX_BIT_LENGTH)
                {
                    _buffer_consume(in_data, in_data->content_size);
                    state->machine_state = x2d_frame_DECODE_MACHINE_STATE_INIT;
                }
                return PROCESS_RESULT_NEED_INPUT_DATA;
            }

            // Data
            int successive_one = 0;
            while (in_index < end_of_frame_index)
            {
                uint8_t v = _get_used_buffer_index(in_data, in_index++);

                if (v != 0)
                {
                    successive_one++;
                    ASSERT_OUT_SPACE(out_data, out_index, 1);
                    _set_free_buffer_index(out_data, out_index++, v);
                }
                else
                {
                    if (successive_one < x2d_frame_MAX_SUCCESSIVE_ONES)
                    {
                        ASSERT_OUT_SPACE(out_data, out_index, 1);
                        _set_free_buffer_index(out_data, out_index++, v);
                    }

                    successive_one = 0;
                }
            }

            _buffer_consume(in_data, end_of_frame_index + sizeof(x2d_frame_END_OF_FRAME_ARRAY));
            _buffer_apply(out_data, end_of_frame_index);
            state->machine_state = x2d_frame_DECODE_MACHINE_STATE_TRAILING;
            return PROCESS_RESULT_OK;
            break;
        }
        case x2d_frame_DECODE_MACHINE_STATE_TRAILING:
        {
            ASSERT_IN_AVAIL(in_data, in_index, x2d_frame_TRAILING_LENGTH);
            _buffer_consume(in_data, x2d_frame_TRAILING_LENGTH);
            state->machine_state = x2d_frame_DECODE_MACHINE_STATE_DATA;
            break;
        }
        }
    }

    return PROCESS_RESULT_OK;
}