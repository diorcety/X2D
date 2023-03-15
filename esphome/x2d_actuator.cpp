#include "x2d_actuator.h"

using namespace esphome;

X2DActuator::X2DActuator(CC1101_X2D *device, uint16_t houseId, uint8_t zoneId) : X2DEntity(device, houseId, zoneId)
{
}

X2DActuator::~X2DActuator()
{
}

bool X2DActuator::setData(uint8_t *data, size_t size, bool periodic)
{
    if (size > sizeof(this->data) || size == 0 || data == NULL)
    {
        return false;
    }

    memcpy(this->data, data, size);
    this->data_length = size;
    if (periodic)
    {
        update();
    }
    else
    {
        trySend();
    }
    return true;
}

void X2DActuator::resetData()
{
    cancel_timeout("update");
}

bool X2DActuator::trySend()
{
    bool ret = sendX2DFrame(data, data_length);
    if (!ret)
    {
        set_timeout("resend", 1000, [this]()
                    { trySend(); });
    }
    return ret;
}

void X2DActuator::update()
{
    if (data_length > 0)
    {
        bool ret = sendX2DFrame(data, data_length);

        if (ret)
        {
            set_timeout("update", update_interval, [this]()
                        { update(); });
        }
        else
        {
            set_timeout("update", update_interval / 3, [this]()
                        { update(); });
        }
    }
}

void X2DActuator::associate()
{
    uint8_t data[32];
    size_t data_length = 0;
    X2D_BODY *body = (X2D_BODY *)&data[data_length];
    memset(body, 0, sizeof(X2D_BODY));
    body->house = UNS16_TO_BU16(houseId);
    BITFIELD_SET_RANGE(X2D_SOURCE, body->source, X2D_SOURCE_ID_1, X2D_SOURCE_ID_0, 0);
    BITFIELD_SET_RANGE(X2D_SOURCE, body->source, X2D_SOURCE_TYPE_5, X2D_SOURCE_TYPE_0, X2D_DEVICE_USB_Key);
    BITFIELD_SET_RANGE(X2D_RECIPIENT, body->recipient, X2D_RECIPIENT_ZONE_3, X2D_RECIPIENT_ZONE_0, zoneId);
    BITFIELD_SET_RANGE(X2D_TRANSMITER, body->transmitter, X2D_TRANSMITER_ATTRIBUTE_3, X2D_TRANSMITER_ATTRIBUTE_0, X2D_ATTRIBUTE_WithData);
    BITFIELD_BIT_SET(X2D_TRANSMITER, body->transmitter, X2D_TRANSMITER_ENROLLMENT_REQUESTED);
    BITFIELD_BIT_SET(X2D_CONTROL, body->control, X2D_CONTROL_F7);
    BITFIELD_BIT_SET(X2D_CONTROL, body->control, X2D_CONTROL_F4);
    data_length += sizeof(X2D_BODY);
    data[data_length] = X2D_MESSAGE_DATA_TYPE_Enrollment;
    data_length += 1;
    X2D_FOOTER *footer = (X2D_FOOTER *)&data[data_length];
    footer->checksum = UNS16_TO_BU16(x2d_compute_checksum(data, data_length));
    data_length += sizeof(X2D_FOOTER);
    setData(data, data_length, false);
}

X2DHeatingLevelActuator::X2DHeatingLevelActuator(CC1101_X2D *device, uint16_t houseId, uint8_t zoneId) : X2DActuator(device, houseId, zoneId)
{
}

X2DHeatingLevelActuator::~X2DHeatingLevelActuator()
{
}

void X2DHeatingLevelActuator::update()
{
    ESP_LOGD("X2DHeatingLevelActuator", "update");
    X2DActuator::update();
}

void X2DHeatingLevelActuator::setNone()
{
    ESP_LOGD("X2DHeatingLevelActuator", "reset");
    resetData();
}

void X2DHeatingLevelActuator::setReduced()
{
    setLevel(X2D_FUNCTIONING_MODE_Reduced);
}

void X2DHeatingLevelActuator::setModerato()
{
    setLevel(X2D_FUNCTIONING_MODE_Moderato);
}

void X2DHeatingLevelActuator::setMedio()
{
    setLevel(X2D_FUNCTIONING_MODE_Medio);
}

void X2DHeatingLevelActuator::setConfort()
{
    setLevel(X2D_FUNCTIONING_MODE_Confort);
}

void X2DHeatingLevelActuator::setStop()
{
    setLevel(X2D_FUNCTIONING_MODE_Stop);
}

void X2DHeatingLevelActuator::setAntiFrost()
{
    setLevel(X2D_FUNCTIONING_MODE_AntiFrost);
}

void X2DHeatingLevelActuator::setSpecial()
{
    setLevel(X2D_FUNCTIONING_MODE_Special);
}

void X2DHeatingLevelActuator::setAuto()
{
    setLevel(X2D_FUNCTIONING_MODE_Auto);
}

void X2DHeatingLevelActuator::setCentralized()
{
    setLevel(X2D_FUNCTIONING_MODE_Centralized);
}

void X2DHeatingLevelActuator::setLevel(X2D_FUNCTIONING_MODE mode)
{
    uint8_t data[32];
    size_t data_length = 0;
    X2D_BODY *body = (X2D_BODY *)&data[data_length];
    memset(body, 0, sizeof(X2D_BODY));
    body->house = UNS16_TO_BU16(houseId);
    BITFIELD_SET_RANGE(X2D_SOURCE, body->source, X2D_SOURCE_ID_1, X2D_SOURCE_ID_0, 0);
    BITFIELD_SET_RANGE(X2D_SOURCE, body->source, X2D_SOURCE_TYPE_5, X2D_SOURCE_TYPE_0, X2D_DEVICE_USB_Key);
    BITFIELD_SET_RANGE(X2D_RECIPIENT, body->recipient, X2D_RECIPIENT_ZONE_3, X2D_RECIPIENT_ZONE_0, zoneId);
    BITFIELD_SET_RANGE(X2D_TRANSMITER, body->transmitter, X2D_TRANSMITER_ATTRIBUTE_3, X2D_TRANSMITER_ATTRIBUTE_0, X2D_ATTRIBUTE_WithData);
    BITFIELD_BIT_SET(X2D_CONTROL, body->control, X2D_CONTROL_F7);
    BITFIELD_BIT_SET(X2D_CONTROL, body->control, X2D_CONTROL_F4);
    data_length += sizeof(X2D_BODY);
    data[data_length] = X2D_MESSAGE_DATA_TYPE_HeatingLevel;
    data_length += 1;
    X2D_FUNCTIONING_LEVEL_MESSAGE *message = (X2D_FUNCTIONING_LEVEL_MESSAGE *)&data[data_length];
    BITFIELD_SET_RANGE(X2D_FUNCTIONING_LEVEL_FLAG, message->flag, X2D_FUNCTIONING_LEVEL_FLAG_MODE_3, X2D_FUNCTIONING_LEVEL_FLAG_MODE_0, mode);
    data_length += 1; // Don't use optional duration
    X2D_FOOTER *footer = (X2D_FOOTER *)&data[data_length];
    footer->checksum = UNS16_TO_BU16(x2d_compute_checksum(data, data_length));
    data_length += sizeof(X2D_FOOTER);
    setData(data, data_length);
}