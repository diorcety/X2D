#ifndef CC1101_X2D_H
#define CC1101_X2D_H

#include "esphome.h"

#pragma push_macro("yield")
#pragma push_macro("millis")
#pragma push_macro("delay")
#pragma push_macro("delayMicroseconds")
#pragma push_macro("micros")
#undef yield
#undef millis
#undef delay
#undef delayMicroseconds
#undef micros
#include <RadioLib.h>
#pragma pop_macro("micros")
#pragma pop_macro("delayMicroseconds")
#pragma pop_macro("delay")
#pragma pop_macro("millis")
#pragma pop_macro("yield")

#include <functional>
#include <map>

class CC1101_X2D : public esphome::Component
{
public:
  typedef std::function<void(uint8_t *, size_t)> callback_t;
  typedef uint32_t callback_registration;

private:
  static constexpr uint32_t MIN_RX_TIME = 2000;

  typedef enum
  {
    RadioMode_Unknown,
    RadioMode_RX,
    RadioMode_TX,
  } RadioMode;
  RadioMode radioMode;
  uint32_t radioModeTimestamp;
  static bool receivedFlag;

  int _GDO0;
  Module module;
  CC1101 radio;
  std::map<callback_registration, callback_t> callbacks;
  callback_registration callback_registration_uid;

public:
  CC1101_X2D(int CSN, int GDO0, int GDO2 = RADIOLIB_NC);

protected:
  float get_setup_priority() const override;
  void setup() override;
  void loop() override;

private:
#if defined(ESP8266) || defined(ESP32)
  ICACHE_RAM_ATTR
#endif
  static void setFlag(void);

  void receiveMode();

  void transmitMode();

public:
  bool sendX2DFrame(uint8_t *data, size_t data_length);
  callback_registration addCallback(callback_t callback);
  bool removeCallback(callback_registration callbackId);
};

#define get_cc1101_x2d(id) (*((CC1101_X2D *)id))

#endif
