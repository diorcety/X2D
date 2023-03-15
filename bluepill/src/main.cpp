#include <Arduino.h>
#include <RadioLib.h>
#include "x2d.h"
#include "x2d_encoding.h"

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

#define RX
#define RADIO_INTERRUPT

// CC1101 has the following connections:
// CS pin:    10
// GDO0 pin:  2
// RST pin:   unused
// GDO2 pin:  3 (optional)
CC1101 radio = new Module(SPI1_CS, GDO0, RADIOLIB_NC, GDO2, SPI);

// or using RadioShield
// https://github.com/jgromes/RadioShield
// CC1101 radio = RadioShield.ModuleA;
#ifdef RADIO_INTERRUPT
// flag to indicate that a packet was received
volatile bool receivedFlag = false;

typedef enum
{
  RadioMode_Unknown,
  RadioMode_RX,
  RadioMode_TX,
} RadioMode;

RadioMode radioMode = RadioMode_Unknown;
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
  int state = radio.begin(868.439941, 4.82273, 39.55, 203.125000, 0, 32);
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
  if (radio.setOOK(true) != RADIOLIB_ERR_NONE)
  {
    Serial.println(F("[CC1101] setOOK invalid for this module!"));
    while (true)
      ;
  }

  if (radio.setCrcFiltering(false) != RADIOLIB_ERR_NONE)
  {
    Serial.println(F("[CC1101] setCrcFiltering invalid for this module!"));
    while (true)
      ;
  }

  if (radio.setEncoding(0) != RADIOLIB_ERR_NONE)
  {
    Serial.println(F("[CC1101] setEncoding invalid for this module!"));
    while (true)
      ;
  }

  if (radio.disableSyncWordFiltering(true) != RADIOLIB_ERR_NONE)
  {
    Serial.println(F("[CC1101] setEncoding invalid for this module!"));
    while (true)
      ;
  }

  if (radio.disableAddressFiltering() != RADIOLIB_ERR_NONE)
  {
    Serial.println(F("[CC1101] disableAddressFiltering invalid for this module!"));
    while (true)
      ;
  }

  if (radio.fixedPacketLengthMode(255) != RADIOLIB_ERR_NONE)
  {
    Serial.println(F("[CC1101] setPacketMode invalid for this module!"));
    while (true)
      ;
  }

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
        // 2 bytes can be set as sync word
        if (radio.setSyncWord(0x2A, 0xAB) == RADIOLIB_ERR_INVALID_SYNC_WORD)
        {
          Serial.println(F("[CC1101] Selected sync word is invalid for this module!"));
          while (true)
            ;
        }

#ifdef RADIO_INTERRUPT
        receivedFlag = 0;

        // start listening for packets
        //Serial.print(F("[CC1101] Starting to listen ... "));
        state = radio.startReceive();
        if (state == RADIOLIB_ERR_NONE)
        {
          //Serial.println(F("success!"));
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
#endif
      }
      else
      {
        if (radio.disableSyncWordFiltering(true) != RADIOLIB_ERR_NONE)
        {
          Serial.println(F("[CC1101] disableSyncWordFiltering invalid for this module!"));
          while (true)
            ;
        }
#ifdef RADIO_INTERRUPT
        radio.clearGdo0Action();
#endif
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
      // you can also read received data as byte array
      /*
        byte byteArr[8];
        int state = radio.readData(byteArr, 8);
      */

      if (state == RADIOLIB_ERR_NONE)
      {
        length = radio.getPacketLength(false);

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
    if (!deadline_valid)
    {
      deadline = millis() + 4000;
      deadline_valid = true;
      newRadioMode = RadioMode_TX;
    }
  }
  else if (radioMode == RadioMode_TX)
  {
    if (!deadline_valid)
    {
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
      //uint8_t data[] = {0x2f, 0x68, 0x3e, 0x01, 0x05, 0x90, 0x01, 0x03, 0xfe, 0x91}; // set
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
      if (buffer_length > 2)
      {
        #if 0
        for (int i = 0; i < buffer_length; ++i)
        {
          arduino_printf("0x%02x, ", out1_buffer.buffer[i]);
        }

        Serial.print(F("[CC1101] Data Length:\t\t"));
        Serial.println(buffer_length);
        for (int i = 0; i < buffer_length; ++i)
        {
          Serial.print(buffer1[i], 16);
        }
        Serial.print("\n");
        #endif
        for (int i = 0; i < 1; ++i)
        {
          int state = radio.transmit(buffer1, buffer_length);
          if (state == RADIOLIB_ERR_NONE)
          {
            // the packet was successfully transmitted
            //Serial.println(F("success!"));
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
}

extern "C" void HardFault_Handler(void)
{
  while (1)
    ;
}