#ifndef X3D_DEVICE_H
#define X3D_DEVICE_H

#include "esphome/core/component.h"
#include "cc1101_x3d.h"
#include "x3d.h"
#include "x3d_app.h"

class X3DEntity : public esphome::PollingComponent
{
private:
    CC1101_X3D *device;
    CC1101_X3D::callback_registration registrationId;

protected:
    X3D_APP_STATE state;
    uint16_t houseId;
    uint8_t zoneId;
    uint32_t update_interval;

public:
    X3DEntity(CC1101_X3D *device, uint32_t device_id, uint8_t network_id);
    virtual ~X3DEntity();

protected:
    void setup() override;
    void update() override;

    virtual void newData(uint8_t *data, size_t size);

    size_t createX3DMessage(X3D_MESSAGE_TYPE type, uint8_t *data, size_t data_length, uint8_t *header, size_t header_length, uint8_t *payload, size_t payload_length);
    bool sendX3DFrame(uint8_t *data, size_t data_length, bool force=false);
};

#endif