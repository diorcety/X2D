#ifndef X2D_ENTITY_H
#define X2D_ENTITY_H

#include "esphome.h"
#include "cc1101_x2d.h"
#include "x2d.h"

class X2DEntity : public esphome::PollingComponent
{
private:
    CC1101_X2D *device;
    CC1101_X2D::callback_registration registrationId;

protected:
    uint16_t houseId;
    uint8_t zoneId;
    uint32_t update_interval;

public:
    X2DEntity(CC1101_X2D *device, uint16_t houseId, uint8_t zoneId, uint32_t update_interval = 30000);
    virtual ~X2DEntity();

protected:
    void setup() override;
    void update() override;

    virtual void newData(uint8_t *data, size_t size);
    bool sendX2DFrame(uint8_t *data, size_t data_length);
};

#endif