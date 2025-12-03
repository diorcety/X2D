#include "x3d_app.h"

#include <string.h>

void x3d_app_state_init(X3D_APP_STATE *pstate, uint32_t device_id, uint16_t network_id)
{
  pstate->device_id = device_id;
  pstate->network_id = network_id;
  pstate->msg_id = 0;
  pstate->slots = 0;
}

int x3d_app_get_free_slot(X3D_APP_STATE *pstate)
{
  int slot = __builtin_ctz(~pstate->slots);
  if (slot >= 16)
  {
    return -1;
  }
  return slot;
}

void x3d_app_set_slot(X3D_APP_STATE *pstate, int slot)
{
  pstate->slots |= (1 << slot);
}

void x3d_app_reset_slot(X3D_APP_STATE *pstate, int slot)
{
  pstate->slots &= ~(1 << slot);
}

#define X3D_APP_GET_SECTION(STRUCT)          \
  (STRUCT *)&data[offset];                   \
  if ((offset + sizeof(STRUCT)) > data_size) \
  {                                          \
    return -1;                               \
  }

int x3d_app_prepare_message(X3D_APP_STATE *pstate, uint8_t *data, size_t data_size, X3D_MESSAGE_TYPE type)
{
  if (pstate->msg_id == 0)
  {
    pstate->msg_id = 1;
  }

  int offset = 0;

  // Header
  X3D_FRAME_HEADER *frame_header = X3D_APP_GET_SECTION(X3D_FRAME_HEADER);
  offset += sizeof(X3D_FRAME_HEADER);

  // Body start
  X3D_BODY_START *body_start = X3D_APP_GET_SECTION(X3D_BODY_START);
  body_start->seq = UNS8_TO_LU8(pstate->msg_id);
  body_start->const_ff = UNS8_TO_LU8(0xff);
  body_start->type = type;
  BITFIELD_SET_RANGE(X3D_CONTROL, body_start->control, X3D_CONTROL_HEADER_F7, X3D_CONTROL_HEADER_F5, 0);
  offset += sizeof(X3D_BODY_START);

  // Header start
  X3D_HEADER_START *header_start = X3D_APP_GET_SECTION(X3D_HEADER_START);
  header_start->device_id = UNS24_TO_LU24(pstate->device_id);
  header_start->network_id = UNS8_TO_LU8(pstate->network_id);
  offset += sizeof(X3D_HEADER_START);

  return offset;
}

int x3d_app_finalize_header(X3D_APP_STATE *pstate, uint8_t *data, size_t data_size, size_t header_size)
{
  int offset = 0;

  // Header
  X3D_FRAME_HEADER *frame_header = X3D_APP_GET_SECTION(X3D_FRAME_HEADER);
  offset += sizeof(X3D_FRAME_HEADER);

  // Body start
  X3D_BODY_START *body_start = X3D_APP_GET_SECTION(X3D_BODY_START);
  size_t control_header_size = header_size + sizeof(body_start->control) + sizeof(X3D_HEADER_START) + sizeof(X3D_HEADER_END) + sizeof(X3D_HEADER_FOOTER);
  BITFIELD_SET_RANGE(X3D_CONTROL, body_start->control, X3D_CONTROL_HEADER_LENGTH_4, X3D_CONTROL_HEADER_LENGTH_0, control_header_size);
  offset += sizeof(X3D_BODY_START);

  // Header start
  X3D_HEADER_START *header_start = X3D_APP_GET_SECTION(X3D_HEADER_START);
  offset += sizeof(X3D_HEADER_START);

  // Header
  offset += header_size;

  // Header end
  X3D_HEADER_END *header_end = X3D_APP_GET_SECTION(X3D_HEADER_END);
  header_end->msg_id = UNS16_TO_LU16(x3d_enc_msg_id(pstate->msg_id, pstate->device_id));
  offset += sizeof(X3D_HEADER_END);

  // Header footer
  X3D_HEADER_FOOTER *header_footer = X3D_APP_GET_SECTION(X3D_HEADER_FOOTER);
  size_t header_checksum_size = (size_t)((uint8_t *)header_footer - (uint8_t *)header_start);
  header_footer->checksum = UNS16_TO_BU16(x3d_compute_header_checksum((uint8_t *)header_start, header_checksum_size));
  offset += sizeof(X3D_HEADER_FOOTER);

  return offset;
}

int x3d_app_finalize_message(X3D_APP_STATE *pstate, uint8_t *data, size_t data_size, size_t payload_size)
{
  int offset = 0;

  // Header
  X3D_FRAME_HEADER *frame_header = X3D_APP_GET_SECTION(X3D_FRAME_HEADER);
  offset += sizeof(X3D_FRAME_HEADER);

  // Body start
  X3D_BODY_START *body_start = X3D_APP_GET_SECTION(X3D_BODY_START);
  uint8_t header_length = BITFIELD_GET_RANGE(X3D_CONTROL, body_start->control, X3D_CONTROL_HEADER_LENGTH_4, X3D_CONTROL_HEADER_LENGTH_0);
  header_length -= (sizeof(body_start->control) + sizeof(X3D_HEADER_START) + sizeof(X3D_HEADER_END) + sizeof(X3D_HEADER_FOOTER));
  offset += sizeof(X3D_BODY_START);

  // Header start
  X3D_HEADER_START *header_start = X3D_APP_GET_SECTION(X3D_HEADER_START);
  offset += sizeof(X3D_HEADER_START);

  // Header
  offset += header_length;

  // Header end
  X3D_HEADER_END *header_end = X3D_APP_GET_SECTION(X3D_HEADER_END);
  offset += sizeof(X3D_HEADER_END);

  // Header footer
  X3D_HEADER_FOOTER *header_footer = X3D_APP_GET_SECTION(X3D_HEADER_FOOTER);
  offset += sizeof(X3D_HEADER_FOOTER);

  offset += payload_size;

  // Update frame header with full frame length
  frame_header->length = UNS8_TO_BU8(offset + sizeof(X3D_FRAME_FOOTER));

  X3D_FRAME_FOOTER *frame_footer = X3D_APP_GET_SECTION(X3D_FRAME_FOOTER);
  size_t checksum_size = (size_t)((uint8_t *)frame_footer - (uint8_t *)frame_header);
  frame_footer->checksum = UNS16_TO_BU16(x3d_compute_checksum((uint8_t *)frame_header, checksum_size));
  offset += sizeof(X3D_FRAME_FOOTER);

  // Increment state msg_id
  pstate->msg_id += 1;

  return offset;
}

int x3d_app_create_message(X3D_APP_STATE *pstate, uint8_t *data, size_t data_size, X3D_MESSAGE_TYPE type, uint8_t *header, size_t header_size, uint8_t *payload, size_t payload_size)
{
  int data_offset = x3d_app_prepare_message(pstate, data, data_size, type);
  if (data_offset < 0)
  {
    return data_offset;
  }

  if (header != NULL && header_size > 0)
  {
    memcpy(&data[data_offset], header, header_size);
  }

  data_offset = x3d_app_finalize_header(pstate, data, data_size, header_size);
  if (data_offset < 0)
  {
    return data_offset;
  }

  if (payload != NULL && payload_size > 0)
  {
    memcpy(&data[data_offset], payload, payload_size);
  }

  data_offset = x3d_app_finalize_message(pstate, data, data_size, payload_size);
  return data_offset;
}

int x3d_app_create_pairing_message(X3D_APP_STATE *pstate, uint8_t *data, size_t data_size, uint8_t request_counter, uint8_t reply_counter, uint16_t transfer_slot, uint16_t transfered_slot, uint8_t slot, uint16_t rnd, X3D_PAIRING_PAYLOAD_STATUS status)
{
  X3D_HEADER_PAIRING_PAYLOAD pairing_header_payload;
  pairing_header_payload.const_85 = UNS8_TO_LU8(0x85);
  pairing_header_payload.const_98 = UNS8_TO_LU8(0x98);
  pairing_header_payload.const_00 = UNS8_TO_LU8(0x00);

  X3D_PAIRING_PAYLOAD pairing_payload;
  BITFIELD_SET_RANGE(X3D_HEADER_PAIRING_PAYLOAD_COUNTERS, pairing_payload.counters, X3D_HEADER_PAIRING_PAYLOAD_COUNTERS_REQUEST_3, X3D_HEADER_PAIRING_PAYLOAD_COUNTERS_REQUEST_0, request_counter);
  BITFIELD_SET_RANGE(X3D_HEADER_PAIRING_PAYLOAD_COUNTERS, pairing_payload.counters, X3D_HEADER_PAIRING_PAYLOAD_COUNTERS_REPLY_3, X3D_HEADER_PAIRING_PAYLOAD_COUNTERS_REPLY_0, reply_counter);
  pairing_payload.transfer_slot = UNS16_TO_LU16(transfer_slot);
  pairing_payload.transfered_slot = UNS16_TO_LU16(transfered_slot);
  pairing_payload.const_1f = UNS8_TO_LU8(0x1f);
  pairing_payload.const_ff = UNS8_TO_LU8(0xff);
  pairing_payload.slot = UNS8_TO_LU8(slot);
  pairing_payload.const_00 = UNS8_TO_LU8(0x00);
  pairing_payload.rnd = UNS16_TO_LU16(rnd);
  pairing_payload.status = status;

  return x3d_app_create_message(pstate, data, data_size, X3D_MESSAGE_TYPE_Pairing, (uint8_t *)&pairing_header_payload, sizeof(X3D_HEADER_PAIRING_PAYLOAD), (uint8_t *)&pairing_payload, sizeof(X3D_PAIRING_PAYLOAD));
}