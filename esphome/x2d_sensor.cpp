#include "x2d_sensor.h"

using namespace esphome;

X2DSensor::X2DSensor(CC1101_X2D *device, uint16_t houseId, uint8_t zoneId, uint32_t update_interval) : X2DEntity(device, houseId, zoneId, update_interval)
{
}

X2DSensor::~X2DSensor()
{
}

bool X2DSensor::askData(X2D_MESSAGE_DATA_TYPE dataType)
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
    BITFIELD_BIT_SET(X2D_CONTROL, body->control, X2D_CONTROL_ANSWER_REQUESTED);
    data_length += sizeof(X2D_BODY);
    data[data_length] = dataType;
    data_length += 1;
    X2D_FOOTER *footer = (X2D_FOOTER *)&data[data_length];
    footer->checksum = UNS16_TO_BU16(x2d_compute_checksum(data, data_length));
    data_length += sizeof(X2D_FOOTER);
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
    return ret;
}

X2DZoneLevelSensor::X2DZoneLevelSensor(CC1101_X2D *device, uint16_t houseId, uint8_t zoneId, uint32_t update_interval) : X2DSensor(device, houseId, zoneId, update_interval)
{
}

void X2DZoneLevelSensor::update()
{
    ESP_LOGD("X2DZoneLevelSensor", "update");
    ask();
}

void X2DZoneLevelSensor::ask()
{
    askData(X2D_MESSAGE_DATA_TYPE_CurrentLevel);
}

void X2DZoneLevelSensor::newData(uint8_t *data, size_t size)
{
    X2D_BODY *body = (X2D_BODY *)data;
    uint16_t receivedHouseId = BU16_TO_UNS16(body->house);
    if (receivedHouseId != houseId)
    {
        ESP_LOGD("X2DZoneLevelSensor", "not matching house id  %u != %u", receivedHouseId, houseId);
        return;
    }
    uint8_t receivedZoneId = BITFIELD_GET_RANGE(X2D_RECIPIENT, body->recipient, X2D_RECIPIENT_ZONE_3, X2D_RECIPIENT_ZONE_0);
    if (receivedZoneId != zoneId)
    {
        ESP_LOGV("X2DZoneLevelSensor", "not matching zone id  %u != %u", receivedZoneId, zoneId);
        return;
    }

    X2D_ATTRIBUTE receivedAttribute = (X2D_ATTRIBUTE)BITFIELD_GET_RANGE(X2D_TRANSMITER, body->transmitter, X2D_TRANSMITER_ATTRIBUTE_3, X2D_TRANSMITER_ATTRIBUTE_0);
    if (receivedAttribute != X2D_ATTRIBUTE_WithData)
    {
        ESP_LOGD("X2DZoneLevelSensor", "not with data");
        return;
    }
    size -= sizeof(X2D_BODY);
    data += sizeof(X2D_BODY);

    if (size < sizeof(X2D_MESSAGE_DATA_TYPE))
    {
        ESP_LOGD("X2DZoneLevelSensor", "no data type");
        return;
    }
    X2D_MESSAGE_DATA_TYPE data_type = (X2D_MESSAGE_DATA_TYPE)data[0];
    if (data_type != X2D_MESSAGE_DATA_TYPE_FunctioningLevel && data_type != X2D_MESSAGE_DATA_TYPE_HeatingLevel && data_type != X2D_MESSAGE_DATA_TYPE_CurrentLevel)
    {
        ESP_LOGD("X2DZoneLevelSensor", "not level data type");
        return;
    }
    size -= sizeof(X2D_MESSAGE_DATA_TYPE);
    data += sizeof(X2D_MESSAGE_DATA_TYPE);

    if (size < sizeof(X2D_FUNCTIONING_LEVEL_MESSAGE))
    {
        ESP_LOGD("X2DZoneLevelSensor", "not level message");
        return;
    }

    X2D_FUNCTIONING_LEVEL_MESSAGE *message = (X2D_FUNCTIONING_LEVEL_MESSAGE *)data;
    X2D_FUNCTIONING_MODE mode = (X2D_FUNCTIONING_MODE)BITFIELD_GET_RANGE(X2D_FUNCTIONING_LEVEL_FLAG, message->flag, X2D_FUNCTIONING_LEVEL_FLAG_MODE_3, X2D_FUNCTIONING_LEVEL_FLAG_MODE_0);
    switch (mode)
    {
    case X2D_FUNCTIONING_MODE_Reduced:
        publish_state("Reduced");
        break;
    case X2D_FUNCTIONING_MODE_Moderato:
        publish_state("Moderato");
        break;
    case X2D_FUNCTIONING_MODE_Medio:
        publish_state("Medio");
        break;
    case X2D_FUNCTIONING_MODE_Confort:
        publish_state("Confort");
        break;
    case X2D_FUNCTIONING_MODE_Stop:
        publish_state("Stop");
        break;
    case X2D_FUNCTIONING_MODE_AntiFrost:
        publish_state("AntiFrost");
        break;
    case X2D_FUNCTIONING_MODE_Special:
        publish_state("Special");
        break;
    case X2D_FUNCTIONING_MODE_Auto:
        publish_state("Auto");
        break;
    case X2D_FUNCTIONING_MODE_Centralized:
        publish_state("Centralized");
        break;
    }
}