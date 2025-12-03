#include <Arduino.h>
#include <RadioLib.h>

#define X3D_APP

#ifdef X2D_APP
#include "x2d.h"
#include "x2d_encoding.h"
#endif
#ifdef X3D_APP
#include "x3d.h"
#include "x3d_app.h"
#include "encoding.h"
#endif

// Change LED
#ifdef LED_BUILTIN
#undef LED_BUILTIN
#endif
#define LED_BUILTIN PB12

#define SPI1_SCK PA5
#define SPI1_MOSI PA7
#define SPI1_MISO PA6
#define SPI1_CS PA4
#define GDO0 PA0
#define GDO2 PA1

#define WITH_TX
#define RADIO_INTERRUPT

// CC1101 has the following connections:
// CS pin:    PA4
// GDO0 pin:  PA1
// RST pin:   unused
// GDO2 pin:  PA0
CC1101 radio = new Module(SPI1_CS, GDO0, RADIOLIB_NC, GDO2, SPI);

// or using RadioShield
// https://github.com/jgromes/RadioShield
// CC1101 radio = RadioShield.ModuleA;

typedef enum
{
  RadioMode_Unknown,
  RadioMode_RX,
#ifdef WITH_TX
  RadioMode_TX,
#endif // WITH_TX
} RadioMode;

RadioMode radioMode = RadioMode_Unknown;

#ifdef RADIO_INTERRUPT
// flag to indicate that a packet was received
volatile bool receivedFlag = false;

// this function is called when a complete packet
// is received by the module
// IMPORTANT: this function MUST be 'void' type
//            and MUST NOT have any arguments!

#if defined(ESP8266) || defined(ESP32)
ICACHE_RAM_ATTR
#endif
void setFlag(void)
{
  // we got a packet, set the flag
  receivedFlag = true;
}
#endif

#if defined(GATE_433)
void bitstream2data(const char *bitstream, uint8_t *data, size_t *size)
{
  size_t in_size = *size;
  size_t out_size = 0;
  uint16_t current_byte = 0;
  uint8_t current_offset = 13;
  while (true)
  {
    if (current_offset <= 5 || ((*bitstream == '\0') && current_offset > 0))
    {
      *data = (current_byte >> 8);
      current_byte <<= 8;
      current_offset += 8;
      data++;
      out_size += 1;
      --in_size;
    }

    if (in_size == 0 || *bitstream == '\0')
    {
      break;
    }

    if (*bitstream == '1')
    {
      current_byte |= (0x3 << current_offset);
    }
    else
    {
      current_byte |= (0x1 << current_offset);
    }
    current_offset -= 3;

    ++bitstream;
  }

  *size = out_size;
}
#endif

extern "C" int arduino_printf(const char *format, ...)
{
  char buffer[256];
  va_list args;
  va_start(args, format);
  int ret = vsprintf(buffer, format, args);
  Serial.print(buffer);
  va_end(args);
  return ret;
}

void setup()
{
  // initialize LED digital pin as an output.
  pinMode(LED_BUILTIN, OUTPUT);

  Serial.begin(115200);

  delay(2000);

  // initialize CC1101 with default settings
  Serial.print(F("[CC1101] Initializing ... "));
#ifdef X2D_APP
  int state = radio.begin(868.439941, 4.82273, 39.55, 203.125000, 0, 32);
#endif
#ifdef X3D_APP
  int state = radio.begin(869.034, 40.0, 80.0, 270.0, 0, 32);
#endif
  if (state == RADIOLIB_ERR_NONE)
  {
    Serial.println(F("success!"));
  }
  else
  {
    Serial.print(F("failed, code "));
    Serial.println(state);
    while (true)
      ;
  }
#ifdef X2D_APP
  if (radio.setOOK(true) != RADIOLIB_ERR_NONE)
  {
    Serial.println(F("[CC1101] setOOK invalid for this module!"));
    while (true)
      ;
  }
#endif
#ifdef X3D_APP
  if (radio.setOOK(false) != RADIOLIB_ERR_NONE)
  {
    Serial.println(F("[CC1101] setOOK invalid for this module!"));
    while (true)
      ;
  }
  if (radio.setDataShaping(RADIOLIB_SHAPING_NONE) != RADIOLIB_ERR_NONE)
  {
    Serial.println(F("[CC1101] setDataShaping invalid for this module!"));
    while (true)
      ;
  }
#endif

  if (radio.setCrcFiltering(false) != RADIOLIB_ERR_NONE)
  {
    Serial.println(F("[CC1101] setCrcFiltering invalid for this module!"));
    while (true)
      ;
  }

#ifdef X2D_APP
  if (radio.setEncoding(0) != RADIOLIB_ERR_NONE)
#endif
#ifdef X3D_APP
    if (radio.setEncoding(0) != RADIOLIB_ERR_NONE)
#endif
    {
      Serial.println(F("[CC1101] setEncoding invalid for this module!"));
      while (true)
        ;
    }

#ifdef X2D_APP
  if (radio.disableSyncWordFiltering(true) != RADIOLIB_ERR_NONE)
  {
    Serial.println(F("[CC1101] setEncoding invalid for this module!"));
    while (true)
      ;
  }
#endif // X2D_APP
#ifdef X3D_APP
#if 1
  static uint8_t syncword[] = {0x81, 0x69};
  if (radio.setSyncWord(syncword, (uint8_t)sizeof(syncword), 0, true) != RADIOLIB_ERR_NONE)
  {
    Serial.println(F("[CC1101] setSyncWord invalid for this module!"));
    while (true)
      ;
  }
#else
  if (radio.disableSyncWordFiltering(true) != RADIOLIB_ERR_NONE)
  {
    Serial.println(F("[CC1101] setEncoding invalid for this module!"));
    while (true)
      ;
  }
#endif
#endif // X3D_APP
  if (radio.disableAddressFiltering() != RADIOLIB_ERR_NONE)
  {
    Serial.println(F("[CC1101] disableAddressFiltering invalid for this module!"));
    while (true)
      ;
  }

#ifdef X2D_APP
  if (radio.fixedPacketLengthMode(255) != RADIOLIB_ERR_NONE)
  {
    Serial.println(F("[CC1101] setPacketMode invalid for this module!"));
    while (true)
      ;
  }
#endif // X2D_APP
#ifdef X3D_APP
#if 0
  if (radio.variablePacketLengthMode(255) != RADIOLIB_ERR_NONE)
  {
    Serial.println(F("[CC1101] setPacketMode invalid for this module!"));
    while (true)
      ;
  }
#else
  if (radio.fixedPacketLengthMode(255) != RADIOLIB_ERR_NONE)
  {
    Serial.println(F("[CC1101] setPacketMode invalid for this module!"));
    while (true)
      ;
  }
#endif
#endif // X3D_APP

  if (radio.setOutputPower(5) != RADIOLIB_ERR_NONE)
  {
    Serial.println(F("[CC1101] setOutputPower invalid for this module!"));
    while (true)
      ;
  }
}

static uint16_t v = 0;
static unsigned long deadline;
static bool deadline_valid = false;
static RadioMode newRadioMode;
void loop()
{
  int state;

  X3D_APP_STATE x3d_app_state;
  x3d_app_state_init(&x3d_app_state, 0x123456, 0x1);

  if (radioMode == RadioMode_Unknown)
  {
    newRadioMode = RadioMode_RX;
  }

  if (radioMode == RadioMode_Unknown || (deadline_valid && (long)(deadline - millis()) <= 0))
  {
    deadline_valid = false;
    if (newRadioMode != radioMode)
    {
      if (newRadioMode == RadioMode_RX)
      {

#ifdef X2D_APP
        // 2 bytes can be set as sync word
        if (radio.setSyncWord(0x2A, 0xAB) == RADIOLIB_ERR_INVALID_SYNC_WORD)
        {
          Serial.println(F("[CC1101] Selected sync word is invalid for this module!"));
          while (true)
            ;
        }
#endif // X2D_APP

#ifdef RADIO_INTERRUPT
        receivedFlag = 0;

        // start listening for packets
        // Serial.print(F("[CC1101] Starting to listen ... "));
        state = radio.startReceive();
        if (state == RADIOLIB_ERR_NONE)
        {
          // Serial.println(F("success!"));
        }
        else
        {
          Serial.print(F("failed, code "));
          Serial.println(state);
          while (true)
            ;
        }

        // set the function that will be called
        // when new packet is received
        radio.setGdo0Action(setFlag);
#endif // RADIO_INTERRUPT
      }
      else
      {
#ifdef X2D_APP
        if (radio.disableSyncWordFiltering(true) != RADIOLIB_ERR_NONE)
        {
          Serial.println(F("[CC1101] disableSyncWordFiltering invalid for this module!"));
          while (true)
            ;
        }
#endif // X2D_APP
#ifdef RADIO_INTERRUPT
        radio.clearGdo0Action();
#endif // RADIO_INTERRUPT
      }
      radioMode = newRadioMode;
    }
  }
  if (radioMode == RadioMode_RX)
  {
    uint8_t data[256];
    size_t length;
#if defined(RADIO_INTERRUPT)
    // check if the flag is set
    if (receivedFlag)
    {
      // you can read received data as an Arduino String
      state = radio.readData(data, 256);
      // reset flag
      receivedFlag = false;
#else
    state = radio.receive(data, 256);
#endif

      if (state == RADIOLIB_ERR_NONE)
      {
#ifdef X2D_APP
        length = radio.getPacketLength(false);
#endif // X2D_APP
#ifdef X3D_APP
        length = radio.getPacketLength(true);
#endif // X3D_APP

        // packet was successfully received
        Serial.println(F("[CC1101] Received packet!"));
        Serial.println(v);

        Serial.print(F("[CC1101] Data Length:\t\t"));
        Serial.println(length);
        for (int i = 0; i < length; ++i)
        {
          Serial.print(data[i], 16);
        }

        // print RSSI (Received Signal Strength Indicator)
        // of the last received packet
        Serial.print(F("[CC1101] RSSI:\t\t"));
        Serial.print(radio.getRSSI());
        Serial.println(F(" dBm"));

        // print LQI (Link Quality Indicator)
        // of the last received packet, lower is better
        Serial.print(F("[CC1101] LQI:\t\t"));
        Serial.println(radio.getLQI());

#ifdef X2D_APP
        x2d_decode_state_t decode_state;
        x2d_decode_state_reset(&decode_state);

        uint8_t buffer[256];
        size_t buffer_length = 0;

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
          Serial.print("==================================================\n");
          for (int i = 0; i < out2_buffer.content_size; ++i)
          {
            arduino_printf("0x%02x, ", out2_buffer.buffer[i]);
          }
          Serial.print("\n==================================================\n");
          x2d_print(out2_buffer.buffer, out2_buffer.content_size);
          Serial.print("==================================================\n");
        }
#endif
#ifdef X3D_APP
        // 16 ending bits of the syncword
        if (data[0] == 0x96 && data[1] == 0x7e)
        {
          buffer_t indata_buffer = {&data[2], sizeof(data) - 2, length - 2, buffer_type_byte};
          uint8_t buffer[256];
          buffer_t out_buffer = {buffer, sizeof(buffer), 0, buffer_type_byte};
          ccitt_whitening_decoder_state_t cwd;
          ccitt_whitening_decoder_reset(&cwd);
          ccitt_whitening_decoder_process(&cwd, &indata_buffer, &out_buffer);

          Serial.print("\n==================================================\n");
          x3d_print(&out_buffer.buffer[0], out_buffer.content_size);
          Serial.print("==================================================\n");
        }
#endif
      }
      else if (state == RADIOLIB_ERR_RX_TIMEOUT)
      {
        // timeout occurred while waiting for a packet
        Serial.println(F("timeout!"));
      }
      else if (state == RADIOLIB_ERR_CRC_MISMATCH)
      {
        // packet was received, but is malformed
        Serial.println(F("CRC error!"));
      }
      else
      {
        // some other error occurred
        Serial.print(F("failed, code "));
        Serial.println(state);
      }
#if defined(RADIO_INTERRUPT)
      // put module back to listen mode
      radio.startReceive();
    }
#endif
#ifdef WITH_TX
    if (!deadline_valid)
    {
      deadline = millis() + 4000;
      deadline_valid = true;
      newRadioMode = RadioMode_TX;
    }
#endif
  }
#ifdef WITH_TX
  else if (radioMode == RadioMode_TX)
  {
    Serial.println(F("IN TX"));
    if (!deadline_valid)
    {
#ifdef X2D_APP
#if 0
    uint8_t data[256];
    size_t data_length = 0;
    // Body
    X2D_BODY *body = (X2D_BODY *)&data[data_length];
    memset(body, 0, sizeof(X2D_BODY));
    body->house = UNS16_TO_BU16(12136);
    BITFIELD_SET_RANGE(X2D_SOURCE, body->source, X2D_SOURCE_ID_1, X2D_SOURCE_ID_0, 0);
    BITFIELD_SET_RANGE(X2D_SOURCE, body->source, X2D_SOURCE_TYPE_5, X2D_SOURCE_TYPE_0, X2D_DEVICE_Tydom_Panel_Controller);
    BITFIELD_SET_RANGE(X2D_RECIPIENT, body->recipient, X2D_RECIPIENT_SUBINDEX_3, X2D_RECIPIENT_SUBINDEX_0, 0);
    BITFIELD_SET_RANGE(X2D_TRANSMITER, body->transmitter, X2D_TRANSMITER_ATTRIBUTE_3, X2D_TRANSMITER_ATTRIBUTE_0, X2D_ATTRIBUTE_WithData);
    BITFIELD_BIT_SET(X2D_TRANSMITER, body->transmitter, X2D_TRANSMITER_ENROLLMENT_REQUESTED);
    data_length += sizeof(X2D_BODY);

    // Data
    data[data_length] = X2D_MESSAGE_DATA_TYPE_Enrollment;
    data_length += 1;

    // Footer
    X2D_FOOTER *footer = (X2D_FOOTER *)&data[data_length];
    footer->checksum = UNS16_TO_BU16(x2d_compute_checksum(data, data_length));
    data_length += sizeof(X2D_FOOTER);
#else
      // uint8_t data[] = {0x2f, 0x68, 0x12, 0x00, 0x85, 0x90, 0x00, 0xfe, 0x42}; // assoc
      // uint8_t data[] = {0x2f, 0x68, 0x3e, 0x01, 0x05, 0x90, 0x01, 0x03, 0xfe, 0x91}; // set
      uint8_t data[] = {0x2f, 0x68, 0x3e, 0x00, 0x05, 0x94, 0x1a, 0xfe, 0x78};
      size_t data_length = sizeof(data) - 2;
      X2D_FOOTER *footer = (X2D_FOOTER *)&data[data_length];
      footer->checksum = UNS16_TO_BU16(x2d_compute_checksum(data, data_length));
      data_length += sizeof(X2D_FOOTER);
#endif
#if 0
      Serial.print("==================================================\n");
      x2d_print(data, data_length);
      Serial.print("==================================================\n");
#endif

      x2d_encode_state_t encode_state;
      x2d_encode_state_reset(&encode_state);
      buffer_t indata_buffer = {data, sizeof(data), data_length, buffer_type_byte};
      uint8_t buffer1[256] = {0xFF, 0xFF}; // Preambule
      size_t buffer_length = 2 * 8;
      buffer_t out1_buffer = {buffer1, sizeof(buffer1), buffer_length, buffer_type_beb};

      while (!x2d_encode(&encode_state, &indata_buffer, &out1_buffer, false))
      {
        indata_buffer.content_size = data_length;
        if (((out1_buffer.content_size - 1) / 8) + 1 >= sizeof(buffer1))
          break;
        buffer_length = out1_buffer.content_size;
      }
      buffer_length = ((buffer_length - 1) / 8) + 1;
#endif
#ifdef X3D_APP
      uint8_t buffer1[256];
      size_t buffer1_offset = 0;
      memset(buffer1, 0, sizeof(buffer1));

      int slot = x3d_app_get_free_slot(&x3d_app_state);

      int buffer_length = x3d_app_create_pairing_message(&x3d_app_state, buffer1, sizeof(buffer1), 4, 0, x3d_app_state.slots, 0, slot, 0, X3D_PAIRING_PAYLOAD_STATUS_Open);

#endif
      if (buffer_length > 2)
      {
        buffer_t indata_buffer = {buffer1, buffer_length, buffer_length, buffer_type_byte};

        // 16 ending bits of the syncword
        uint8_t buffer[256];
        buffer[0] = 0x96;
        buffer[1] = 0x7e;
        buffer_t out_buffer = {buffer, sizeof(buffer), 2, buffer_type_byte};

        ccitt_whitening_encoder_state_t cwd;
        ccitt_whitening_encoder_reset(&cwd);
        ccitt_whitening_encoder_process(&cwd, &indata_buffer, &out_buffer);

#if 1
        Serial.print(F("[CC1101] Data Length:\t\t"));
        Serial.println(out_buffer.content_size);
        for (int i = 0; i < out_buffer.content_size; ++i)
        {
          Serial.print(buffer[i], 16);
        }
        Serial.print("\n");
#endif
        for (int i = 0; i < 5; ++i)
        {
          int state = radio.transmit(buffer, out_buffer.content_size);
          if (state == RADIOLIB_ERR_NONE)
          {
            // the packet was successfully transmitted
            // Serial.println(F("success!"));
          }
          else if (state == RADIOLIB_ERR_PACKET_TOO_LONG)
          {
            // the supplied packet was longer than 64 bytes
            Serial.println(F("too long!"));
          }
          else
          {
            // some other error occurred
            Serial.print(F("failed, code "));
            Serial.println(state);
          }
        }
      }
    }
    if (!deadline_valid)
    {
      deadline = millis() + 0;
      deadline_valid = true;
      newRadioMode = RadioMode_RX;
    }
  }
#endif
}

extern "C" void HardFault_Handler(void)
{
  while (1)
    ;
}