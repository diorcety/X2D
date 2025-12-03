#include "cc1101_x3d.h"
#include "encoding.h"
#include "x3d_app.h"
#include "x3d.h"
#include "esphome/core/hal.h"
#include "esphome/core/helpers.h"

using namespace esphome;

bool CC1101_X3D::receivedFlag;

CC1101_X3D::CC1101_X3D(int CSN, int GDO0, int GDO2) : module(CSN, GDO0, RADIOLIB_NC, GDO2, SPI), radio(&module)
{
  callback_registration_uid = 0;
  _GDO0 = GDO0;
  radioMode = RadioMode_Unknown;
}

float CC1101_X3D::get_setup_priority() const
{
  return esphome::setup_priority::HARDWARE;
}

void CC1101_X3D::setup()
{
  radioMode = RadioMode_Unknown;
  radioModeTimestamp = esphome::millis();
  receivedFlag = false;

  pinMode(_GDO0, INPUT);

  ESP_LOGD("CC1101_X3D", "initializing ... ");
  int state = radio.begin(869.034, 40.0, 80.0, 270.0, 0, 32);
  if (state == RADIOLIB_ERR_NONE)
  {
    ESP_LOGD("CC1101_X3D", "success!");
  }
  else
  {
    ESP_LOGD("CC1101_X3D", "failed, code %d", state);
    assert(false);
  }

  if (radio.setDIOMapping(0, 0x2E) != RADIOLIB_ERR_NONE)
  {
    ESP_LOGD("CC1101_X3D", "setDIOMapping invalid for this module!");
    assert(false);
  }

  if (radio.setOOK(false) != RADIOLIB_ERR_NONE)
  {
    ESP_LOGD("CC1101_X3D", "setOOK invalid for this module!");
    assert(false);
  }

  if (radio.setDataShaping(RADIOLIB_SHAPING_NONE) != RADIOLIB_ERR_NONE)
  {
    ESP_LOGD("CC1101_X3D", "setDataShaping invalid for this module!");
    assert(false);
  }

  if (radio.setCrcFiltering(false) != RADIOLIB_ERR_NONE)
  {
    ESP_LOGD("CC1101_X3D", "setCrcFiltering invalid for this module!");
    assert(false);
  }

  if (radio.setEncoding(0) != RADIOLIB_ERR_NONE)
  {
    ESP_LOGD("CC1101_X3D", "setEncoding invalid for this module!");
    assert(false);
  }

  static uint8_t syncword[] = {0x81, 0x69};
  if (radio.setSyncWord(syncword, (uint8_t)sizeof(syncword), 0, true) != RADIOLIB_ERR_NONE)
  {
    ESP_LOGD("CC1101_X3D", "setEncoding invalid for this module!");
    assert(false);
  }

  if (radio.disableAddressFiltering() != RADIOLIB_ERR_NONE)
  {
    ESP_LOGD("CC1101_X3D", "disableAddressFiltering invalid for this module!");
    assert(false);
  }

  if (radio.fixedPacketLengthMode(255) != RADIOLIB_ERR_NONE)
  {
    ESP_LOGD("CC1101_X3D", "setPacketMode invalid for this module!");
    assert(false);
  }

  if (radio.setOutputPower(5) != RADIOLIB_ERR_NONE)
  {
    ESP_LOGD("CC1101_X3D", "setOutputPower invalid for this module!");
    assert(false);
  }

  receiveMode();
}

#if defined(ESP8266) || defined(ESP32)
ICACHE_RAM_ATTR
#endif
void CC1101_X3D::setFlag(void)
{
  // we got a packet, set the flag
  receivedFlag = true;
}

void CC1101_X3D::receiveMode()
{
  receivedFlag = false;

  // start listening for packets
  ESP_LOGD("CC1101_X3D", "Starting to listen ... ");
  int state = radio.startReceive();
  if (state == RADIOLIB_ERR_NONE)
  {
    ESP_LOGD("CC1101_X3D", "receive success!");
  }
  else
  {
    ESP_LOGD("CC1101_X3D", "reveive failed, code %d", state);
    return;
  }

  // set the function that will be called
  // when new packet is received
  radio.setGdo0Action(setFlag, RISING);

  radioMode = RadioMode_RX;
  radioModeTimestamp = esphome::millis();
}

void CC1101_X3D::transmitMode()
{
  radio.clearGdo0Action();

  radioMode = RadioMode_TX;
  radioModeTimestamp = esphome::millis();
}

void CC1101_X3D::loop()
{
  // Avoid overflow
  if (esphome::millis() - radioModeTimestamp > MIN_RX_TIME)
  {
    radioModeTimestamp = esphome::millis() - MIN_RX_TIME;
  }

  if (radioMode == RadioMode_RX)
  {
    // check if the flag is set
    if (receivedFlag)
    {
      uint8_t data[256];
      // you can read received data as an Arduino String
      int state = radio.readData(data, 256);
      if (state == RADIOLIB_ERR_NONE)
      {
        ESP_LOGD("CC1101_X3D", "received packet!");
        size_t length = radio.getPacketLength(true);
        ESP_LOGD("CC1101_X3D", "received length: %d", length);
        ESP_LOGD("CC1101_X3D", "RSSI: %f dBm", radio.getRSSI());
        ESP_LOGD("CC1101_X3D", "LQI: %u", radio.getLQI());

        // 16 ending bits of the syncword
        if (data[0] == 0x96 && data[1] == 0x7e)
        {
          buffer_t indata_buffer = {&data[2], sizeof(data) - 2, length - 2, buffer_type_byte};
          uint8_t buffer[256];
          buffer_t out_buffer = {buffer, sizeof(buffer), 0, buffer_type_byte};

          ccitt_whitening_decoder_state_t cwd;
          ccitt_whitening_decoder_reset(&cwd);
          if (ccitt_whitening_decoder_process(&cwd, &indata_buffer, &out_buffer) == PROCESS_RESULT_OK)
          {
            ESP_LOGD("CC1101_X3D", "Decode Success");
            x3d_print(out_buffer.buffer, out_buffer.content_size);
            for (auto callback_entry : callbacks)
            {
              callback_entry.second(out_buffer.buffer, out_buffer.content_size);
            }
          }
        }
      }
      receiveMode();
    }
  }
}

bool CC1101_X3D::sendX3DFrame(uint8_t *data, size_t data_length, bool force)
{
  if (radioMode == RadioMode_TX || ((esphome::millis() - radioModeTimestamp) < MIN_RX_TIME && !force))
  {
    ESP_LOGD("CC1101_X3D", "busy, can't transmit");
    return false;
  }

  if (data != NULL && data_length > 2)
  {
    transmitMode();

    ESP_LOGD("CC1101_X3D", "Transmitting X3D Frame (%u bytes): %s",
             data_length,
             format_hex_pretty(data, data_length).c_str(), ' ');

    int state = radio.transmit(data, data_length);
    if (state == RADIOLIB_ERR_NONE)
    {
      // the packet was successfully transmitted
      ESP_LOGD("CC1101_X3D", "transmit success!");
    }
    else if (state == RADIOLIB_ERR_PACKET_TOO_LONG)
    {
      // the supplied packet was longer than 64 bytes
      ESP_LOGD("CC1101_X3D", "transmit too long!");
    }
    else
    {
      // some other error occurred
      ESP_LOGD("CC1101_X3D", "transmit failed, code %d", state);
    }

    receiveMode();
    return state == RADIOLIB_ERR_NONE;
  }
  return false;
}

CC1101_X3D::callback_registration CC1101_X3D::addCallback(callback_t callback)
{
  callbacks.insert(std::make_pair(++callback_registration_uid, callback));
  return callback_registration_uid;
}

bool CC1101_X3D::removeCallback(callback_registration callbackId)
{
  return callbacks.erase(callback_registration_uid) == 1;
}
