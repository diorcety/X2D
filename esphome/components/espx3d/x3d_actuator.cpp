#include "x3d_actuator.h"

using namespace esphome;

X3DActuator::X3DActuator(CC1101_X3D *device, uint32_t device_id, uint8_t network_id) : X3DEntity(device, device_id, network_id), b_create_data(false), send_count(0)
{
}

X3DActuator::~X3DActuator()
{
}

bool X3DActuator::setData(X3D_MESSAGE_TYPE type, uint8_t *header, size_t header_length, uint8_t *payload, size_t payload_length, uint8_t send_count, bool periodic)
{
    if (header_length > sizeof(this->header) || payload_length > sizeof(this->payload))
    {
        return false;
    }

    if (payload_length == 0 && header_length == 0)
    {
        return false;
    }

    if (header == NULL && payload == NULL)
    {
        return false;
    }

    this->type = type;
    this->send_count = send_count;

    if (header != NULL && header_length != 0)
    {
        memcpy(this->header, header, header_length);
        this->header_length = header_length;
    }
    else
    {
        this->header_length = 0;
    }

    if (payload != NULL && payload_length != 0)
    {
        memcpy(this->payload, payload, payload_length);
        this->payload_length = payload_length;
    }
    else
    {
        this->payload_length = 0;
    }

    b_create_data = true;
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

void X3DActuator::resetData()
{
    cancel_timeout("update");
}

bool X3DActuator::trySend()
{
    if (b_create_data)
    {
        this->data_length = createX3DMessage(this->type, this->data, sizeof(this->data), this->header, this->header_length, this->payload, this->payload_length);
        b_create_data = false;
    }
    unsigned long deadline = millis() + 50;
    bool ret = sendX3DFrame(this->data, this->data_length, true);
    if (!ret)
    {
        set_timeout("resend", 1000, [this]()
                    { trySend(); });
    }
    else
    {
        if (--send_count > 0)
        {
            set_timeout("resend", deadline - millis(), [this]()
                        { trySend(); });
        }
    }
    return ret;
}

void X3DActuator::update()
{

    if (b_create_data)
    {
        this->data_length = createX3DMessage(this->type, this->data, sizeof(this->data), this->header, this->header_length, this->payload, this->payload_length);
        b_create_data = false;
    }

    if (data_length > 0)
    {
        bool ret = sendX3DFrame(data, data_length);

        if (ret)
        {
            b_create_data = true; // Continue sequence
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