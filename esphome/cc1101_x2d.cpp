#include "cc1101_x2d.h"
#include "x2d_encoding.h"
#include "x2d.h"

using namespace esphome;

bool CC1101_X2D::receivedFlag;

CC1101_X2D::CC1101_X2D(int CSN, int GDO0, int GDO2) : module(CSN, GDO0, RADIOLIB_NC, GDO2, SPI), radio(&module)
{
  callback_registration_uid = 0;
  _GDO0 = GDO0;
  radioMode = RadioMode_Unknown;
}

float CC1101_X2D::get_setup_priority() const
{
  return esphome::setup_priority::HARDWARE;
}

void CC1101_X2D::setup()
{
  radioMode = RadioMode_Unknown;
  radioModeTimestamp = esphome::millis();
  receivedFlag = false;

  pinMode(_GDO0, INPUT);

  ESP_LOGD("CC1101_X2D", "initializing ... ");
  int state = radio.begin(868.439941, 4.82273, 39.55, 203.125000, 0, 32);
  if (state == RADIOLIB_ERR_NONE)
  {
    ESP_LOGD("CC1101_X2D", "success!");
  }
  else
  {
    ESP_LOGD("CC1101_X2D", "failed, code %d", state);
    assert(false);
  }

  if (radio.setDIOMapping(0, 0x2E) != RADIOLIB_ERR_NONE)
  {
    ESP_LOGD("CC1101_X2D", "setDIOMapping invalid for this module!");
    assert(false);
  }

  if (radio.setOOK(true) != RADIOLIB_ERR_NONE)
  {
    ESP_LOGD("CC1101_X2D", "setOOK invalid for this module!");
    assert(false);
  }

  if (radio.setCrcFiltering(false) != RADIOLIB_ERR_NONE)
  {
    ESP_LOGD("CC1101_X2D", "setCrcFiltering invalid for this module!");
    assert(false);
  }

  if (radio.setEncoding(0) != RADIOLIB_ERR_NONE)
  {
    ESP_LOGD("CC1101_X2D", "setEncoding invalid for this module!");
    assert(false);
  }

  if (radio.disableSyncWordFiltering(true) != RADIOLIB_ERR_NONE)
  {
    ESP_LOGD("CC1101_X2D", "setEncoding invalid for this module!");
    assert(false);
  }

  if (radio.disableAddressFiltering() != RADIOLIB_ERR_NONE)
  {
    ESP_LOGD("CC1101_X2D", "disableAddressFiltering invalid for this module!");
    assert(false);
  }

  if (radio.fixedPacketLengthMode(255) != RADIOLIB_ERR_NONE)
  {
    ESP_LOGD("CC1101_X2D", "setPacketMode invalid for this module!");
    assert(false);
  }

  receiveMode();
}

#if defined(ESP8266) || defined(ESP32)
ICACHE_RAM_ATTR
#endif
void CC1101_X2D::setFlag(void)
{
  // we got a packet, set the flag
  receivedFlag = true;
}

void CC1101_X2D::receiveMode()
{
  // 2 bytes can be set as sync word
  if (radio.setSyncWord(0x2A, 0xAB) == RADIOLIB_ERR_INVALID_SYNC_WORD)
  {
    ESP_LOGD("CC1101_X2D", "setSyncWord is invalid for this module!");
    return;
  }

  receivedFlag = false;

  // start listening for packets
  ESP_LOGD("CC1101_X2D", "Starting to listen ... ");
  int state = radio.startReceive();
  if (state == RADIOLIB_ERR_NONE)
  {
    ESP_LOGD("CC1101_X2D", "receive success!");
  }
  else
  {
    ESP_LOGD("CC1101_X2D", "reveive failed, code %d", state);
    return;
  }

  // set the function that will be called
  // when new packet is received
  radio.setGdo0Action(setFlag);

  radioMode = RadioMode_RX;
  radioModeTimestamp = esphome::millis();
}

void CC1101_X2D::transmitMode()
{
  if (radio.disableSyncWordFiltering(true) != RADIOLIB_ERR_NONE)
  {
    ESP_LOGD("CC1101_X2D", "disableSyncWordFiltering invalid for this module!");
    return;
  }

  radio.clearGdo0Action();

  radioMode = RadioMode_TX;
  radioModeTimestamp = esphome::millis();
}

void CC1101_X2D::loop()
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
      // reset flag
      receivedFlag = false;
      if (state == RADIOLIB_ERR_NONE)
      {
        ESP_LOGD("CC1101_X2D", "received packet!");
        size_t length = radio.getPacketLength(false);
        ESP_LOGD("CC1101_X2D", "received length: %d", length);
        ESP_LOGD("CC1101_X2D", "RSSI: %f dBm", radio.getRSSI());
        ESP_LOGD("CC1101_X2D", "LQI: %u", radio.getLQI());

        x2d_decode_state_t decode_state;
        x2d_decode_state_reset(&decode_state);

        uint8_t buffer[256];
        size_t buffer_length = 0;

        // Recreate preambule/syncword
        static uint8_t forced_header[] = {0x33, 0x33, 0x2a, 0xab};
        memcpy(&buffer[buffer_length], forced_header, sizeof(forced_header));
        buffer_length += sizeof(forced_header);
        if (length > sizeof(buffer) - buffer_length)
        {
          length = sizeof(buffer) - buffer_length;
        }
        memcpy(&buffer[buffer_length], data, length);
        buffer_length += length;

        buffer_t out1_buffer = {buffer, sizeof(buffer), buffer_length * 8, buffer_type_beb};

        uint8_t buffer2[256];
        buffer_t out2_buffer = {buffer2, sizeof(buffer2), 0, buffer_type_byte};
        if (!x2d_decode(&decode_state, &out1_buffer, &out2_buffer, false))
        {
          ESP_LOGD("CC1101_X2D", "Decode Success");
          x2d_print(out2_buffer.buffer, out2_buffer.content_size);
          size_t size = x2d_get_frame_size(out2_buffer.buffer, out2_buffer.content_size);
          if (size > 0)
          {
            for (auto callback_entry : callbacks)
            {
              callback_entry.second(out2_buffer.buffer, size);
            }
          }
        }
        else
        {
          ESP_LOGD("CC1101_X2D", "Decode failure");
        }
      }
    }
  }
}

bool CC1101_X2D::sendX2DFrame(uint8_t *data, size_t data_length)
{
  if (radioMode == RadioMode_TX || (esphome::millis() - radioModeTimestamp) < MIN_RX_TIME)
  {
    ESP_LOGD("CC1101_X2D", "busy, can't transmit");
    return false;
  }

  x2d_encode_state_t encode_state;
  x2d_encode_state_reset(&encode_state);
  buffer_t indata_buffer = {data, data_length, data_length, buffer_type_byte};
  uint8_t buffer1[256] = {0xFF, 0xFF}; // Preambule
  size_t buffer_length = 2 * 8;
  buffer_t out1_buffer = {buffer1, sizeof(buffer1), buffer_length, buffer_type_beb};

  while (!x2d_encode(&encode_state, &indata_buffer, &out1_buffer, false))
  {
    indata_buffer.content_size = data_length;
    if (((out1_buffer.content_size - 1) / 8) + 1 >= sizeof(buffer1)/2)
      break;
    buffer_length = out1_buffer.content_size;
  }
  buffer_length = ((buffer_length - 1) / 8) + 1;
  if (buffer_length > 2)
  {
    transmitMode();

    int state = radio.transmit(buffer1, buffer_length);
    if (state == RADIOLIB_ERR_NONE)
    {
      // the packet was successfully transmitted
      ESP_LOGD("CC1101_X2D", "transmit success!");
    }
    else if (state == RADIOLIB_ERR_PACKET_TOO_LONG)
    {
      // the supplied packet was longer than 64 bytes
      ESP_LOGD("CC1101_X2D", "transmit too long!");
    }
    else
    {
      // some other error occurred
      ESP_LOGD("CC1101_X2D", "transmit failed, code %d", state);
    }

    receiveMode();
    return state == RADIOLIB_ERR_NONE;
  }
  return false;
}

CC1101_X2D::callback_registration CC1101_X2D::addCallback(callback_t callback)
{
  callbacks.insert(std::make_pair(++callback_registration_uid, callback));
  return callback_registration_uid;
}

bool CC1101_X2D::removeCallback(callback_registration callbackId)
{
  return callbacks.erase(callback_registration_uid) == 1;
}