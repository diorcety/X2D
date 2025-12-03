#ifndef X3D_APP_H_
#define X3D_APP_H_

#include "x3d.h"

typedef struct
{
    uint32_t device_id;
    uint16_t network_id;
    uint16_t msg_id;
    uint8_t slots;
} X3D_APP_STATE;

#ifdef __cplusplus
extern "C"
{
#endif

    void x3d_app_state_init(X3D_APP_STATE *pstate, uint32_t device_id, uint16_t network_id);

    int x3d_app_get_free_slot(X3D_APP_STATE *pstate);
    void x3d_app_set_slot(X3D_APP_STATE *pstate, int slot);
    void x3d_app_reset_slot(X3D_APP_STATE *pstate, int slot);

    int x3d_app_prepare_message(X3D_APP_STATE *pstate, uint8_t *data, size_t data_size, X3D_MESSAGE_TYPE type);

    int x3d_app_finalize_header(X3D_APP_STATE *pstate, uint8_t *data, size_t data_size, size_t header_size);

    int x3d_app_finalize_message(X3D_APP_STATE *pstate, uint8_t *data, size_t data_size, size_t payload_size);

    int x3d_app_create_pairing_message(X3D_APP_STATE *pstate, uint8_t *data, size_t data_size, uint8_t request_counter, uint8_t reply_counter, uint16_t transfer_slot, uint16_t transfered_slot, uint8_t slot, uint16_t rnd, X3D_PAIRING_PAYLOAD_STATUS status);

#ifdef __cplusplus
}
#endif

#endif // X3D_APP_H_