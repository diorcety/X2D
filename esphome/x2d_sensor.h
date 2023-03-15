#ifndef X2D_SENSOR_H
#define X2D_SENSOR_H

#include "x2d_entity.h"

class X2DSensor : public X2DEntity, public esphome::text_sensor::TextSensor
{
public:
    X2DSensor(CC1101_X2D *device, uint16_t houseId, uint8_t zoneId, uint32_t update_interval = 30000);
    virtual ~X2DSensor();

protected:
    bool askData(X2D_MESSAGE_DATA_TYPE dataType);
};

class X2DZoneLevelSensor : public X2DSensor
{
public:
    X2DZoneLevelSensor(CC1101_X2D *device, uint16_t houseId, uint8_t zoneId, uint32_t update_interval = 30000);

protected:
    void update() override;

    void newData(uint8_t *data, size_t size) override;

public:
    void ask();
};

#endif