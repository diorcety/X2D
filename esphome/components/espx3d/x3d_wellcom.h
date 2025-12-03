#ifndef X3D_WELLCOM_H
#define X3D_WELLCOM_H

#include "x3d_actuator.h"

class X3DWellcomRemote : public X3DActuator
{
public:
    X3DWellcomRemote(CC1101_X3D *device, uint32_t device_id, uint8_t network_id);
    virtual ~X3DWellcomRemote();

    void associate();

    void up();
    void stop();
    void down();
    void stop_down();
    void stop_up();
};

#endif