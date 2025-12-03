#include "x3d.h"

#if defined(ARDUINO)
extern int arduino_printf(const char *__restrict, ...);
#define PRINT_FCT(...) arduino_printf(__VA_ARGS__)
#else
#include <stdio.h>
#define PRINT_FCT(...) printf(__VA_ARGS__)
#endif

#define VNS(x, y) \
    {             \
        x##_##y, #y}
#define ARRAY_SIZE(T) (sizeof(T) / sizeof(T[0]))

static struct
{
    X3D_MESSAGE_TYPE type;
    const char *name;
} message_type_str[] = {
    VNS(X3D_MESSAGE_TYPE, Sensor),
    VNS(X3D_MESSAGE_TYPE, Standard),
    VNS(X3D_MESSAGE_TYPE, Pairing),
    VNS(X3D_MESSAGE_TYPE, Beacon),
};

static const char *get_message_type_str(X3D_MESSAGE_TYPE type)
{
    for (int i = 0; i < ARRAY_SIZE(message_type_str); ++i)
    {
        if (message_type_str[i].type == type)
        {
            return message_type_str[i].name;
        }
    }
    return "(NULL)";
}

static struct
{
    X3D_PAIRING_PAYLOAD_STATUS status;
    const char *name;
} pairing_payload_status_str[] = {
    VNS(X3D_PAIRING_PAYLOAD_STATUS, Open),
    VNS(X3D_PAIRING_PAYLOAD_STATUS, Pinned),
};

static const char *get_pairing_payload_status_str(X3D_PAIRING_PAYLOAD_STATUS status)
{
    for (int i = 0; i < ARRAY_SIZE(pairing_payload_status_str); ++i)
    {
        if (pairing_payload_status_str[i].status == status)
        {
            return pairing_payload_status_str[i].name;
        }
    }
    return "(NULL)";
}

// from https://github.com/mr-sven/x3d-rfm-esp32/blob/main/x3d-lib/x3d.c
static uint8_t sbox[] = {0x1, 0x0, 0xC, 0x8, 0xA, 0x9, 0xE, 0x7, 0x3, 0x5, 0x4, 0xB, 0x2, 0xF, 0x6, 0xD};
static uint16_t apply_sbox(uint16_t value, uint8_t count)
{
    return (value & ((0xf << count) ^ 0xffff)) | (sbox[(value >> count) & 0xf] << count);
}

uint16_t x3d_enc_msg_id(uint16_t msgId, uint32_t deviceId)
{
    uint16_t result = msgId;
    uint16_t xor_key = ((deviceId & 0xff) ^ ((deviceId >> 16) & 0xff)) | (deviceId & 0xff00);
    for (int i = 0; i < 32; i++)
    {
        result = apply_sbox(result, i % 13) ^ xor_key;
    }

    return result;
}

uint16_t x3d_dec_msg_id(uint16_t encMsgId, uint32_t deviceId)
{
    uint16_t result = encMsgId;
    uint16_t xor_key = ((deviceId & 0xff) ^ ((deviceId >> 16) & 0xff)) | (deviceId & 0xff00);
    for (int i = 31; i >= 0; i--)
    {
        result = apply_sbox(result ^ xor_key, i % 13);
    }
    return result;
}

uint16_t x3d_compute_header_checksum(uint8_t *data, size_t size)
{
    uint16_t res = 0;
    for (int i = 0; i < size; i++)
    {
        res -= data[i];
    }
    return res;
}

uint16_t x3d_compute_checksum(uint8_t *data, size_t size)
{
    uint16_t crc = 0;
    for (int i = 0; i < size; i++)
    {
        crc = crc ^ ((uint16_t)data[i] << 8);
        for (int j = 0; j < 8; j++)
        {
            if (crc & 0x8000)
            {
                crc = (crc << 1) ^ 0x1021;
            }
            else
            {
                crc <<= 1;
            }
        }
    }
    return crc;
}

static void print_binary(uint8_t *data, size_t size)
{
    if (size == 0)
    {
        PRINT_FCT("(None)");
        return;
    }

    for (int i = 0; i < size; ++i)
    {
        if (i == 0)
        {
            PRINT_FCT("%02X", data[i]);
        }
        else
        {
            PRINT_FCT(" %02X", data[i]);
        }
    }
}

static void print_header_payload(X3D_MESSAGE_TYPE type, uint8_t *data, size_t size)
{
    PRINT_FCT("Header payload:\n");
    switch (type)
    {
    case X3D_MESSAGE_TYPE_Sensor:
    {
        print_binary(data, size);
        PRINT_FCT("\n");
        break;
    }
    case X3D_MESSAGE_TYPE_Standard:
    {

        print_binary(data, size);
        PRINT_FCT("\n");
        break;
    }
    case X3D_MESSAGE_TYPE_Pairing:
    {
        print_binary(data, size);
        PRINT_FCT("\n");
        break;
    }
    case X3D_MESSAGE_TYPE_Beacon:
    {
        print_binary(data, size);
        PRINT_FCT("\n");
        break;
    }
    }
    PRINT_FCT("\n");
}

static void print_payload(X3D_MESSAGE_TYPE type, uint8_t *data, size_t size)
{
    PRINT_FCT("Payload:\n");
    switch (type)
    {
    case X3D_MESSAGE_TYPE_Pairing:
    {
        X3D_PAIRING_PAYLOAD *payload = (X3D_PAIRING_PAYLOAD *)data;
        uint8_t counters_request = BITFIELD_GET_RANGE(X3D_HEADER_PAIRING_PAYLOAD_COUNTERS, payload->counters, X3D_HEADER_PAIRING_PAYLOAD_COUNTERS_REQUEST_3, X3D_HEADER_PAIRING_PAYLOAD_COUNTERS_REQUEST_0);
        uint8_t counters_reply = BITFIELD_GET_RANGE(X3D_HEADER_PAIRING_PAYLOAD_COUNTERS, payload->counters, X3D_HEADER_PAIRING_PAYLOAD_COUNTERS_REPLY_3, X3D_HEADER_PAIRING_PAYLOAD_COUNTERS_REPLY_0);
        PRINT_FCT("Request counter: %u\n", counters_request);
        PRINT_FCT("Reply counter: %u\n", counters_reply);
        PRINT_FCT("Transfer slot: %u\n", LU16_TO_UNS16(payload->transfer_slot));
        PRINT_FCT("Transfered slot: %u\n", LU16_TO_UNS16(payload->transfered_slot));
        PRINT_FCT("Slot: %u\n", LU8_TO_UNS8(payload->slot));
        PRINT_FCT("Rnd: %u\n", LU16_TO_UNS16(payload->rnd));
        PRINT_FCT("Status: %s\n", get_pairing_payload_status_str(payload->status));
        break;
    }
    default:
    {
        print_binary(data, size);
        PRINT_FCT("\n");
        break;
    }
    }
    PRINT_FCT("\n");
}

void x3d_body_print(uint8_t *data, size_t size)
{
    size_t payload_length = size;
    if (size < sizeof(X3D_BODY_START))
    {
        PRINT_FCT("Invalid X3D data size\n");
        return;
    }

    X3D_BODY_START *body_start = (X3D_BODY_START *)&data[0];
    PRINT_FCT("Sequence: %u\n", LU8_TO_UNS8(body_start->seq));
    X3D_MESSAGE_TYPE type = (X3D_MESSAGE_TYPE)body_start->type;
    PRINT_FCT("Message Type: %s\n", get_message_type_str(type));
    uint8_t header_length = BITFIELD_GET_RANGE(X3D_CONTROL, body_start->control, X3D_CONTROL_HEADER_LENGTH_4, X3D_CONTROL_HEADER_LENGTH_0);
    PRINT_FCT("Header length: %u\n", header_length);

    size -= sizeof(X3D_BODY_START);
    data += sizeof(X3D_BODY_START);
    payload_length -= sizeof(X3D_BODY_START);

    if (size < sizeof(X3D_HEADER_START))
    {
        PRINT_FCT("Invalid X3D data size\n");
        return;
    }

    X3D_HEADER_START *header_start = (X3D_HEADER_START *)&data[0];
    uint32_t device_id = LU24_TO_UNS24(header_start->device_id);
    PRINT_FCT("Device ID: %u\n", device_id);
    PRINT_FCT("Network ID: %u\n", LU8_TO_UNS8(header_start->network_id));

    size -= sizeof(X3D_HEADER_START);
    data += sizeof(X3D_HEADER_START);
    payload_length -= sizeof(X3D_HEADER_START);
    header_length -= (sizeof(body_start->control) + sizeof(X3D_HEADER_START) + sizeof(X3D_HEADER_END) + sizeof(X3D_HEADER_FOOTER));

    if (size < header_length)
    {
        PRINT_FCT("Invalid X3D data size\n");
        return;
    }

    print_header_payload(type, data, header_length);

    size -= header_length;
    data += header_length;
    payload_length -= header_length;

    if (size < sizeof(X3D_HEADER_END))
    {
        PRINT_FCT("Invalid X3D data size\n");
        return;
    }

    X3D_HEADER_END *header_end = (X3D_HEADER_END *)&data[0];
    uint16_t msg_id = x3d_dec_msg_id(LU16_TO_UNS16(header_end->msg_id), device_id);
    PRINT_FCT("Msg ID: %u\n", msg_id);

    size -= sizeof(X3D_HEADER_END);
    data += sizeof(X3D_HEADER_END);
    payload_length -= sizeof(X3D_HEADER_END);

    if (size < sizeof(X3D_HEADER_FOOTER))
    {
        PRINT_FCT("Invalid X3D data size\n");
        return;
    }

    X3D_HEADER_FOOTER *header_footer = (X3D_HEADER_FOOTER *)&data[0];
    size_t header_checksum_size = (size_t)((void *)header_footer - (void *)header_start);
    uint16_t computed_header_checkum = x3d_compute_header_checksum((uint8_t *)header_start, header_checksum_size);
    uint16_t given_header_checksum = BU16_TO_UNS16(header_footer->checksum);
    PRINT_FCT("Header checksum: %04x / %04x : %s\n", computed_header_checkum, given_header_checksum, ((computed_header_checkum == given_header_checksum) ? "VALID" : "INVALID"));

    size -= sizeof(X3D_HEADER_FOOTER);
    data += sizeof(X3D_HEADER_FOOTER);
    payload_length -= sizeof(X3D_HEADER_FOOTER);

    print_payload(type, data, payload_length);
}

void x3d_print(uint8_t *data, size_t size)
{
    if (size < sizeof(X3D_FRAME_HEADER) + sizeof(X3D_FRAME_FOOTER))
    {
        PRINT_FCT("Invalid X3D data size\n");
        return;
    }

    X3D_FRAME_HEADER *frame_header = (X3D_FRAME_HEADER *)&data[0];
    uint8_t length = BU8_TO_UNS8(frame_header->length);
    uint8_t payload_length = length - sizeof(X3D_FRAME_FOOTER);

    PRINT_FCT("Length: %u\n", length);

    size -= sizeof(X3D_FRAME_HEADER);
    data += sizeof(X3D_FRAME_HEADER);
    payload_length -= sizeof(X3D_FRAME_HEADER);

    x3d_body_print(data, payload_length);
    size -= payload_length;
    data += payload_length;

    if (size < sizeof(X3D_FRAME_FOOTER))
    {
        PRINT_FCT("Invalid X3D data size\n");
        return;
    }

    X3D_FRAME_FOOTER *frame_footer = (X3D_FRAME_FOOTER *)&data[0];
    size_t checksum_size = (size_t)((void *)frame_footer - (void *)frame_header);
    uint16_t computed_checkum = x3d_compute_checksum((uint8_t *)frame_header, checksum_size);
    uint16_t given_checksum = BU16_TO_UNS16(frame_footer->checksum);
    PRINT_FCT("Checksum: %04x / %04x : %s\n", computed_checkum, given_checksum, ((computed_checkum == given_checksum) ? "VALID" : "INVALID"));
}