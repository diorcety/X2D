#ifndef X3D_ACTUATOR_H
#define X3D_ACTUATOR_H

#include "x3d_entity.h"

class X3DActuator : public X3DEntity
{
public:
    X3DActuator(CC1101_X3D *device, uint32_t device_id, uint8_t network_id);
    virtual ~X3DActuator();

    bool setData(X3D_MESSAGE_TYPE type, uint8_t *header, size_t header_length, uint8_t *payload, size_t payload_length, uint8_t send_count = 1, bool periodic = true);
    void resetData();

protected:
    void update() override;

private:
    bool trySend();

    X3D_MESSAGE_TYPE type;

    bool b_create_data;
    uint8_t send_count;

    uint8_t header[256];
    size_t header_length;

    uint8_t payload[256];
    size_t payload_length;

    uint8_t data[256];
    size_t data_length;
};

#endif