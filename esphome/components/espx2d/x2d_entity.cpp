#include "x2d_entity.h"

X2DEntity::X2DEntity(CC1101_X2D *device, uint16_t houseId, uint8_t zoneId, uint32_t update_interval) : esphome::PollingComponent(esphome::SCHEDULER_DONT_RUN),
                                                                                                       device(device), houseId(houseId), zoneId(zoneId), update_interval(update_interval) {}

X2DEntity::~X2DEntity()
{
    device->removeCallback(registrationId);
}

void X2DEntity::setup()
{
    registrationId = device->addCallback(std::bind(&X2DEntity::newData, this, std::placeholders::_1, std::placeholders::_2));
    set_timeout("update", update_interval, [this]()
                { update(); });
}

void X2DEntity::update()
{
}

void X2DEntity::newData(uint8_t *data, size_t size)
{
}

bool X2DEntity::sendX2DFrame(uint8_t *data, size_t data_length)
{
    return device->sendX2DFrame(data, data_length);
}