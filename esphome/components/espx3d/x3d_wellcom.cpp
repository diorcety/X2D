#include "x3d_wellcom.h"

using namespace esphome;

X3DWellcomRemote::X3DWellcomRemote(CC1101_X3D *device, uint32_t device_id, uint8_t network_id) : X3DActuator(device, device_id, network_id)
{
}

X3DWellcomRemote::~X3DWellcomRemote()
{
}

void X3DWellcomRemote::associate()
{
    uint8_t header[] = {0x85, 0x98, 0x22};
    setData(X3D_MESSAGE_TYPE_Sensor, header, sizeof(header), NULL, 0, 5, false);
}

void X3DWellcomRemote::down()
{
    // 4th byte
    // Bit7 | Bit6 | Bit5 | Bit4 | Bit3 | Bit2 | Bit1 | Bit0
    //   1  |  0   |  0   |  0   |   0  | STOP | DOWN |  UP
    uint8_t header[] = {0x05, 0x98, 0x22, 0x82, 0x00};
    setData(X3D_MESSAGE_TYPE_Sensor, header, sizeof(header), NULL, 0, 5, false);
}

void X3DWellcomRemote::up()
{
    uint8_t header[] = {0x05, 0x98, 0x22, 0x81, 0x00};
    setData(X3D_MESSAGE_TYPE_Sensor, header, sizeof(header), NULL, 0, 5, false);
}

void X3DWellcomRemote::stop()
{
    uint8_t header[] = {0x05, 0x98, 0x22, 0x84, 0x00};
    setData(X3D_MESSAGE_TYPE_Sensor, header, sizeof(header), NULL, 0, 5, false);
}

void X3DWellcomRemote::stop_down()
{
    uint8_t header[] = {0x05, 0x98, 0x22, 0x86, 0x00};
    setData(X3D_MESSAGE_TYPE_Sensor, header, sizeof(header), NULL, 0, 5, false);
}

void X3DWellcomRemote::stop_up()
{
    uint8_t header[] = {0x05, 0x98, 0x22, 0x85, 0x00};
    setData(X3D_MESSAGE_TYPE_Sensor, header, sizeof(header), NULL, 0, 5, false);
}
