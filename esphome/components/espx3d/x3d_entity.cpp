#include "x3d_entity.h"
#include "encoding.h"

X3DEntity::X3DEntity(CC1101_X3D *device, uint32_t device_id, uint8_t network_id) : esphome::PollingComponent(esphome::SCHEDULER_DONT_RUN),
                                                                                   device(device)
{

    x3d_app_state_init(&state, device_id, network_id);
    state.msg_id = 2;
}

X3DEntity::~X3DEntity()
{
    device->removeCallback(registrationId);
}

void X3DEntity::setup()
{
    registrationId = device->addCallback(std::bind(&X3DEntity::newData, this, std::placeholders::_1, std::placeholders::_2));
    set_timeout("update", update_interval, [this]()
                { update(); });
}

void X3DEntity::update()
{
}

void X3DEntity::newData(uint8_t *data, size_t size)
{
}

size_t X3DEntity::createX3DMessage(X3D_MESSAGE_TYPE type, uint8_t *data, size_t data_length, uint8_t *header, size_t header_length, uint8_t *payload, size_t payload_length)
{
    if (data_length < 2 || data == NULL)
    {
        return 0;
    }

    uint8_t buffer[256];
    int buffer_length = x3d_app_create_message(&state, buffer, sizeof(buffer), type, header, header_length, payload, payload_length);
    if (buffer_length < 0)
    {
        return 0;
    }
    x3d_print(buffer, buffer_length);

    buffer_t indata_buffer = {buffer, sizeof(buffer), buffer_length, buffer_type_byte};
    // 16 ending bits of the syncword
    data[0] = 0x96;
    data[1] = 0x7e;
    buffer_t out_buffer = {data, data_length, 2, buffer_type_byte};

    ccitt_whitening_encoder_state_t cwd;
    ccitt_whitening_encoder_reset(&cwd);
    if (ccitt_whitening_encoder_process(&cwd, &indata_buffer, &out_buffer) != PROCESS_RESULT_OK)
    {
        return 0;
    }
    return out_buffer.content_size;
}

bool X3DEntity::sendX3DFrame(uint8_t *data, size_t data_length, bool force)
{
    return device->sendX3DFrame(data, data_length, force);
}