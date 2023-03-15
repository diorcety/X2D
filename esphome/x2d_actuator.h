#ifndef X2D_ACTUATOR_H
#define X2D_ACTUATOR_H

#include "x2d_entity.h"

class X2DActuator : public X2DEntity
{
public:
    X2DActuator(CC1101_X2D *device, uint16_t houseId, uint8_t zoneId);
    virtual ~X2DActuator();

    void associate();

    bool setData(uint8_t *data, size_t size, bool periodic = true);
    void resetData();

protected:
    void update() override;

private:
    bool trySend();

    uint8_t data[256];
    size_t data_length;
};

class X2DHeatingLevelActuator : public X2DActuator
{
public:
    X2DHeatingLevelActuator(CC1101_X2D *device, uint16_t houseId, uint8_t zoneId);
    virtual ~X2DHeatingLevelActuator();

    void setNone();
    void setReduced();
    void setModerato();
    void setMedio();
    void setConfort();
    void setStop();
    void setAntiFrost();
    void setSpecial();
    void setAuto();
    void setCentralized();

protected:
    void update() override;

private:
    void setLevel(X2D_FUNCTIONING_MODE mode);
};

#define get_x2d_heating_level_actuator(id) (*((X2DHeatingLevelActuator *)id))
#endif