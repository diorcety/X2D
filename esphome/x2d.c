#include "x2d.h"

#if defined(ARDUINO)
extern int arduino_printf(const char *__restrict, ...);
#define PRINT_FCT(...) arduino_printf(__VA_ARGS__)
#else
#include <stdio.h>
#define PRINT_FCT(...) printf(__VA_ARGS__)
#endif

#define VNS(x, y)   \
    {               \
        x##_##y, #y \
    }
#define ARRAY_SIZE(T) (sizeof(T) / sizeof(T[0]))

static struct
{
    X2D_DEVICE device;
    const char *name;
} device_str[] = {
    VNS(X2D_DEVICE, Tyxia_ZZAA),
    VNS(X2D_DEVICE, Calybox),
    VNS(X2D_DEVICE, Deltia_Emitter),
    VNS(X2D_DEVICE, Tydom_Panel_Controller),
    VNS(X2D_DEVICE, Tyxia_XXYY),
    VNS(X2D_DEVICE, Two_Buttons_Alarm_Remote),
    VNS(X2D_DEVICE, Four_Buttons_Alarm_Remote),
    VNS(X2D_DEVICE, Volumetric),
    VNS(X2D_DEVICE, Volumetric_Infrared),
    VNS(X2D_DEVICE, Volumetric_Pressure),
    VNS(X2D_DEVICE, Volumetric_Dual),
    VNS(X2D_DEVICE, Perimetric),
    VNS(X2D_DEVICE, Perimetric_Contact),
    VNS(X2D_DEVICE, Perimetric_Glass_Breakage),
    VNS(X2D_DEVICE, Perimetric_Sound),
    VNS(X2D_DEVICE, Perimetric_Roller_Shutter),
    VNS(X2D_DEVICE, Technical),
    VNS(X2D_DEVICE, Technical_Water),
    VNS(X2D_DEVICE, Technical_Gas),
    VNS(X2D_DEVICE, Technical_Fire),
    VNS(X2D_DEVICE, Technical_Smoke),
    VNS(X2D_DEVICE, Technical_Frost),
    VNS(X2D_DEVICE, Technical_Power_Outage),
    VNS(X2D_DEVICE, Technical_Phone_Outage),
    VNS(X2D_DEVICE, Technical_Freezer_Outage),

    VNS(X2D_DEVICE, USB_Key),
};

static const char *get_device_str(X2D_DEVICE device)
{
    for (int i = 0; i < ARRAY_SIZE(device_str); ++i)
    {
        if (device_str[i].device == device)
        {
            return device_str[i].name;
        }
    }
    return "(NULL)";
}

static struct
{
    X2D_ATTRIBUTE attribute;
    const char *name;
} attribute_str[] = {
    VNS(X2D_ATTRIBUTE, Simple),
    VNS(X2D_ATTRIBUTE, WithData),
};

static const char *get_attribute_str(X2D_ATTRIBUTE attribute)
{
    for (int i = 0; i < ARRAY_SIZE(attribute_str); ++i)
    {
        if (attribute_str[i].attribute == attribute)
        {
            return attribute_str[i].name;
        }
    }
    return "(NULL)";
}

static struct
{
    X2D_MESSAGE_DATA_TYPE message_data_type;
    const char *name;
} message_data_type_str[] = {
    VNS(X2D_MESSAGE_DATA_TYPE, Enrollment),
    VNS(X2D_MESSAGE_DATA_TYPE, HeatingLevel),
    VNS(X2D_MESSAGE_DATA_TYPE, FunctioningLevel),
    VNS(X2D_MESSAGE_DATA_TYPE, InternalTemperature),
    VNS(X2D_MESSAGE_DATA_TYPE, ExternalTemperature),
    VNS(X2D_MESSAGE_DATA_TYPE, MeterReading),
    VNS(X2D_MESSAGE_DATA_TYPE, CurrentLevel),
    VNS(X2D_MESSAGE_DATA_TYPE, BasicCommand),
    VNS(X2D_MESSAGE_DATA_TYPE, VariationCommand),
    VNS(X2D_MESSAGE_DATA_TYPE, ScenarioCommand),
    VNS(X2D_MESSAGE_DATA_TYPE, Variation),
    VNS(X2D_MESSAGE_DATA_TYPE, CurrentTransformerDefinition),
};

static const char *get_message_data_type_str(X2D_MESSAGE_DATA_TYPE message_data_type)
{
    for (int i = 0; i < ARRAY_SIZE(message_data_type_str); ++i)
    {
        if (message_data_type_str[i].message_data_type == message_data_type)
        {
            return message_data_type_str[i].name;
        }
    }
    return "(NULL)";
}

static struct
{
    X2D_FUNCTIONING_MODE functioning_mode;
    const char *name;
} functioning_mode_str[] = {
    VNS(X2D_FUNCTIONING_MODE, Reduced),
    VNS(X2D_FUNCTIONING_MODE, Moderato),
    VNS(X2D_FUNCTIONING_MODE, Medio),
    VNS(X2D_FUNCTIONING_MODE, Confort),
    VNS(X2D_FUNCTIONING_MODE, Stop),
    VNS(X2D_FUNCTIONING_MODE, AntiFrost),
    VNS(X2D_FUNCTIONING_MODE, Special),
    VNS(X2D_FUNCTIONING_MODE, Auto),
    VNS(X2D_FUNCTIONING_MODE, Centralized),
};

static const char *get_functioning_mode_str(X2D_FUNCTIONING_MODE functioning_mode)
{
    for (int i = 0; i < ARRAY_SIZE(functioning_mode_str); ++i)
    {
        if (functioning_mode_str[i].functioning_mode == functioning_mode)
        {
            return functioning_mode_str[i].name;
        }
    }
    return "(NULL)";
}

/*

TYPEDEF_ENUM_BEG(X2D_ELECTRICITY_TARIFF)
    X2D_ELECTRICITY_TARIFF_Base = 0,
    X2D_ELECTRICITY_TARIFF_EjpOffPeakDay = 32,
    X2D_ELECTRICITY_TARIFF_EjpPeakDay = 33,
    X2D_ELECTRICITY_TARIFF_DoubleOffPeakHour = 64,
    X2D_ELECTRICITY_TARIFF_DoublePeakHour = 65,
    X2D_ELECTRICITY_TARIFF_TempoBlueDayOffPeakHour = 96,
    X2D_ELECTRICITY_TARIFF_TempoBlueDayPeakHour = 97,
    X2D_ELECTRICITY_TARIFF_TempoWhiteDayOffPeakHour = 98,
    X2D_ELECTRICITY_TARIFF_TempoWhiteDayPeakHour = 99,
    X2D_ELECTRICITY_TARIFF_TempoRedDayOffPeakHour = 100,
    X2D_ELECTRICITY_TARIFF_TempoRedDayPeakHour = 101,
TYPEDEF_ENUM_END(X2D_ELECTRICITY_TARIFF)


TYPEDEF_ENUM_BEG(X2D_REGISTER_SELECTION)
    X2D_REGISTER_SELECTION_CurrentTransformer1 = 0,
    X2D_REGISTER_SELECTION_CurrentTransformer2 = 1,
    X2D_REGISTER_SELECTION_CurrentTransformer3 = 2,
    X2D_REGISTER_SELECTION_Heating = 3,
    X2D_REGISTER_SELECTION_HotWater = 4,
    X2D_REGISTER_SELECTION_Total = 5,
    X2D_REGISTER_SELECTION_Cooling = 7,
    X2D_REGISTER_SELECTION_HeatingAndCooling = 8,
    X2D_REGISTER_SELECTION_ElectricityProduction = 9,
TYPEDEF_ENUM_END(X2D_REGISTER_SELECTION)

*/

static struct
{
    X2D_ELECTRICITY_TARIFF electricity_tariff;
    const char *name;
} electricity_tariff_str[] = {
    VNS(X2D_ELECTRICITY_TARIFF, Base),
    VNS(X2D_ELECTRICITY_TARIFF, EjpOffPeakDay),
    VNS(X2D_ELECTRICITY_TARIFF, EjpPeakDay),
    VNS(X2D_ELECTRICITY_TARIFF, DoubleOffPeakHour),
    VNS(X2D_ELECTRICITY_TARIFF, DoublePeakHour),
    VNS(X2D_ELECTRICITY_TARIFF, TempoBlueDayOffPeakHour),
    VNS(X2D_ELECTRICITY_TARIFF, TempoBlueDayPeakHour),
    VNS(X2D_ELECTRICITY_TARIFF, TempoWhiteDayOffPeakHour),
    VNS(X2D_ELECTRICITY_TARIFF, TempoWhiteDayPeakHour),
    VNS(X2D_ELECTRICITY_TARIFF, TempoRedDayOffPeakHour),
    VNS(X2D_ELECTRICITY_TARIFF, TempoRedDayPeakHour),
};

static const char *get_electricity_tariff_str(X2D_ELECTRICITY_TARIFF electricity_tariff)
{
    for (int i = 0; i < ARRAY_SIZE(electricity_tariff_str); ++i)
    {
        if (electricity_tariff_str[i].electricity_tariff == electricity_tariff)
        {
            return electricity_tariff_str[i].name;
        }
    }
    return "(NULL)";
}

static struct
{
    X2D_REGISTER_SELECTION register_selection;
    const char *name;
} register_selection_str[] = {
    VNS(X2D_REGISTER_SELECTION, CurrentTransformer1),
    VNS(X2D_REGISTER_SELECTION, CurrentTransformer2),
    VNS(X2D_REGISTER_SELECTION, CurrentTransformer3),
    VNS(X2D_REGISTER_SELECTION, Heating),
    VNS(X2D_REGISTER_SELECTION, HotWater),
    VNS(X2D_REGISTER_SELECTION, Total),
    VNS(X2D_REGISTER_SELECTION, Cooling),
    VNS(X2D_REGISTER_SELECTION, HeatingAndCooling),
    VNS(X2D_REGISTER_SELECTION, ElectricityProduction),
};

static const char *get_register_selection_str(X2D_REGISTER_SELECTION register_selection)
{
    for (int i = 0; i < ARRAY_SIZE(register_selection_str); ++i)
    {
        if (register_selection_str[i].register_selection == register_selection)
        {
            return register_selection_str[i].name;
        }
    }
    return "(NULL)";
}

static const char *get_bool_str(bool b)
{
    return b ? "true" : "false";
}

static void print_data(uint8_t *data, size_t size)
{
    if (size < 1)
    {
        return;
    }

    X2D_MESSAGE_DATA_TYPE data_type = (X2D_MESSAGE_DATA_TYPE)data[0];
    PRINT_FCT("Data|Type: %s\n", get_message_data_type_str(data_type));
    size -= 1;
    data += 1;
    switch (data_type)
    {
    case X2D_MESSAGE_DATA_TYPE_Enrollment:
    {
        break;
    }
    case X2D_MESSAGE_DATA_TYPE_InternalTemperature:
    {
        if (size < sizeof(X2D_TEMPERATURE_MESSAGE))
        {
            return;
        }

        X2D_TEMPERATURE_MESSAGE *message = (X2D_TEMPERATURE_MESSAGE *)data;
        float temperature = (float)LU16_TO_UNS16(message->temperature) / 512.0f;
        float s_temperature = (temperature - (float)(int)temperature) * 10;
        PRINT_FCT("InternalTemperature|Temperature: %d.%d°\n", (int)temperature, (int)s_temperature);
        size -= sizeof(X2D_TEMPERATURE_MESSAGE);
        data += sizeof(X2D_TEMPERATURE_MESSAGE);
        break;
    }
    case X2D_MESSAGE_DATA_TYPE_ExternalTemperature:
    {
        if (size < sizeof(X2D_TEMPERATURE_MESSAGE))
        {
            return;
        }
        X2D_TEMPERATURE_MESSAGE *message = (X2D_TEMPERATURE_MESSAGE *)data;
        float temperature = (float)LU16_TO_UNS16(message->temperature) / 512.0f;
        float s_temperature = (temperature - (float)(int)temperature) * 10;
        PRINT_FCT("ExternalTemperature|Temperature: %d.%d°\n", (int)temperature, (int)s_temperature);
        size -= sizeof(X2D_TEMPERATURE_MESSAGE);
        data += sizeof(X2D_TEMPERATURE_MESSAGE);
        break;
    }
    case X2D_MESSAGE_DATA_TYPE_FunctioningLevel:
    {
        if (size < 1)
        {
            return;
        }
        X2D_FUNCTIONING_LEVEL_MESSAGE *message = (X2D_FUNCTIONING_LEVEL_MESSAGE *)data;
        PRINT_FCT("FunctioningLevel|FunctioningMode: %s\n", get_functioning_mode_str((X2D_FUNCTIONING_MODE)BITFIELD_GET_RANGE(X2D_FUNCTIONING_LEVEL_FLAG, message->flag, X2D_FUNCTIONING_LEVEL_FLAG_MODE_3, X2D_FUNCTIONING_LEVEL_FLAG_MODE_0)));
        PRINT_FCT("FunctioningLevel|Manual: %s\n", get_bool_str(BITFIELD_BIT_IS_SET(X2D_FUNCTIONING_LEVEL_FLAG, message->flag, X2D_FUNCTIONING_LEVEL_FLAG_MANUAL)));
        if (BITFIELD_BIT_IS_SET(X2D_FUNCTIONING_LEVEL_FLAG, message->flag, X2D_FUNCTIONING_LEVEL_FLAG_DURATION))
        {
            PRINT_FCT("FunctioningLevel|Duration %u\n", BU16_TO_UNS16(message->duration));
            size -= sizeof(X2D_FUNCTIONING_LEVEL_MESSAGE);
            data += sizeof(X2D_FUNCTIONING_LEVEL_MESSAGE);
        }
        else
        {
            size -= 1;
            data += 1;
        }

        break;
    }
    case X2D_MESSAGE_DATA_TYPE_HeatingLevel:
    {
        if (size < 1)
        {
            return;
        }
        X2D_FUNCTIONING_LEVEL_MESSAGE *message = (X2D_FUNCTIONING_LEVEL_MESSAGE *)data;
        PRINT_FCT("HeatingLevel|FunctioningMode: %s\n", get_functioning_mode_str((X2D_FUNCTIONING_MODE)BITFIELD_GET_RANGE(X2D_FUNCTIONING_LEVEL_FLAG, message->flag, X2D_FUNCTIONING_LEVEL_FLAG_MODE_3, X2D_FUNCTIONING_LEVEL_FLAG_MODE_0)));
        PRINT_FCT("HeatingLevel|Manual: %s\n", get_bool_str(BITFIELD_BIT_IS_SET(X2D_FUNCTIONING_LEVEL_FLAG, message->flag, X2D_FUNCTIONING_LEVEL_FLAG_MANUAL)));
        if (size >= sizeof(X2D_FUNCTIONING_LEVEL_MESSAGE) && BITFIELD_BIT_IS_SET(X2D_FUNCTIONING_LEVEL_FLAG, message->flag, X2D_FUNCTIONING_LEVEL_FLAG_DURATION))
        {
            PRINT_FCT("HeatingLevel|Duration %u\n", BU16_TO_UNS16(message->duration));
            size -= sizeof(X2D_FUNCTIONING_LEVEL_MESSAGE);
            data += sizeof(X2D_FUNCTIONING_LEVEL_MESSAGE);
        }
        else
        {
            size -= 1;
            data += 1;
        }

        break;
    }
    case X2D_MESSAGE_DATA_TYPE_CurrentLevel:
    {
        if (size < 1)
        {
            return;
        }
        X2D_FUNCTIONING_LEVEL_MESSAGE *message = (X2D_FUNCTIONING_LEVEL_MESSAGE *)data;
        PRINT_FCT("CurrentLevel|FunctioningMode: %s\n", get_functioning_mode_str((X2D_FUNCTIONING_MODE)BITFIELD_GET_RANGE(X2D_FUNCTIONING_LEVEL_FLAG, message->flag, X2D_FUNCTIONING_LEVEL_FLAG_MODE_3, X2D_FUNCTIONING_LEVEL_FLAG_MODE_0)));
        PRINT_FCT("CurrentLevel|Manual: %s\n", get_bool_str(BITFIELD_BIT_IS_SET(X2D_FUNCTIONING_LEVEL_FLAG, message->flag, X2D_FUNCTIONING_LEVEL_FLAG_MANUAL)));
        if (size >= sizeof(X2D_FUNCTIONING_LEVEL_MESSAGE) && BITFIELD_BIT_IS_SET(X2D_FUNCTIONING_LEVEL_FLAG, message->flag, X2D_FUNCTIONING_LEVEL_FLAG_DURATION))
        {
            PRINT_FCT("CurrentLevel|Duration %u\n", BU16_TO_UNS16(message->duration));
            size -= sizeof(X2D_FUNCTIONING_LEVEL_MESSAGE);
            data += sizeof(X2D_FUNCTIONING_LEVEL_MESSAGE);
        }
        else
        {
            size -= 1;
            data += 1;
        }

        break;
    }
    case X2D_MESSAGE_DATA_TYPE_MeterReading:
    {
        if (size < sizeof(X2D_METER_READING_MESSAGE))
        {
            return;
        }
        X2D_METER_READING_MESSAGE *message = (X2D_METER_READING_MESSAGE *)data;
        BITFIELD_VAR(X2D_CURRENT_TARIFF, flag);
        flag = message->flag;
        bool euro = BITFIELD_BIT_IS_SET(X2D_CURRENT_TARIFF, flag, X2D_CURRENT_TARIFF_EURO);
        BITFIELD_BIT_CLR(X2D_CURRENT_TARIFF, flag, X2D_CURRENT_TARIFF_EURO);
        uint32_t regValue = LU8_TO_UNS8(message->registerHigh) << 16 | LU16_TO_UNS16(message->registerLow);

        PRINT_FCT("MeterReading|Selection: %s\n", get_functioning_mode_str(message->selection));
        PRINT_FCT("MeterReading|Tariff: %s\n", get_electricity_tariff_str((X2D_ELECTRICITY_TARIFF)BITFIELD_GET(X2D_CURRENT_TARIFF, flag)));
        PRINT_FCT("MeterReading|Register: %u\n", regValue);
        PRINT_FCT("MeterReading|Unit: %s\n", euro ? "Euro" : "Kwh");
        size -= sizeof(X2D_METER_READING_MESSAGE);
        data += sizeof(X2D_METER_READING_MESSAGE);
        break;
    }
    }
}

void x2d_print(uint8_t *data, size_t size)
{
    if (size < sizeof(X2D_BODY) + sizeof(X2D_FOOTER))
    {
        PRINT_FCT("Invalid X2D data size\n");
        return;
    }

    size -= sizeof(X2D_FOOTER);
    X2D_FOOTER *footer = (X2D_FOOTER *)&data[size];
    uint16_t checksum = x2d_compute_checksum(data, size);
    if (checksum != BU16_TO_UNS16(footer->checksum))
    {
        PRINT_FCT("Invalid checksum\n");
        return;
    }

    X2D_BODY *body = (X2D_BODY *)data;
    PRINT_FCT("House: %u\n", BU16_TO_UNS16(body->house));
    PRINT_FCT("Source|Id: %u\n", BITFIELD_GET_RANGE(X2D_SOURCE, body->source, X2D_SOURCE_ID_1, X2D_SOURCE_ID_0));
    PRINT_FCT("Source|Type: %s\n", get_device_str((X2D_DEVICE)BITFIELD_GET_RANGE(X2D_SOURCE, body->source, X2D_SOURCE_TYPE_5, X2D_SOURCE_TYPE_0)));
    PRINT_FCT("Recipient|Zone: %u\n", BITFIELD_GET_RANGE(X2D_RECIPIENT, body->recipient, X2D_RECIPIENT_ZONE_3, X2D_RECIPIENT_ZONE_0));
    PRINT_FCT("Transmitter|Attribute: %s\n", get_attribute_str((X2D_ATTRIBUTE)BITFIELD_GET_RANGE(X2D_TRANSMITER, body->transmitter, X2D_TRANSMITER_ATTRIBUTE_3, X2D_TRANSMITER_ATTRIBUTE_0)));
    PRINT_FCT("Transmitter|BatteryFailing: %s\n", get_bool_str(BITFIELD_BIT_IS_SET(X2D_TRANSMITER, body->transmitter, X2D_TRANSMITER_BATTERY_FAILLING)));
    PRINT_FCT("Transmitter|BoxOpened: %s\n", get_bool_str(BITFIELD_BIT_IS_SET(X2D_TRANSMITER, body->transmitter, X2D_TRANSMITER_BOX_OPENED)));
    PRINT_FCT("Transmitter|InternalFaultDetected: %s\n", get_bool_str(BITFIELD_BIT_IS_SET(X2D_TRANSMITER, body->transmitter, X2D_TRANSMITER_INTERNAL_FAULT_DETECTED)));
    PRINT_FCT("Transmitter|EnrollmentRequested: %s\n", get_bool_str(BITFIELD_BIT_IS_SET(X2D_TRANSMITER, body->transmitter, X2D_TRANSMITER_ENROLLMENT_REQUESTED)));
    PRINT_FCT("Control|AnswerRequested: %s\n", get_bool_str(BITFIELD_BIT_IS_SET(X2D_CONTROL, body->control, X2D_CONTROL_ANSWER_REQUESTED)));

    size -= sizeof(X2D_BODY);
    data += sizeof(X2D_BODY);

    if (BITFIELD_BIT_IS_SET(X2D_CONTROL, body->control, X2D_CONTROL_ROLLING_CODE))
    {
        size -= sizeof(X2D_ROLLING_CODE_FOOTER);
        X2D_ROLLING_CODE_FOOTER *message = (X2D_ROLLING_CODE_FOOTER *)&data[size];
        PRINT_FCT("RollingCode: %u\n", BU16_TO_UNS16(message->rollingCode));
    }

    print_data(data, size);
}

size_t x2d_get_frame_size(uint8_t *data, size_t size)
{
    if (size < sizeof(X2D_BODY) + sizeof(X2D_FOOTER))
    {
        return 0;
    }

    size -= sizeof(X2D_FOOTER);
    X2D_FOOTER *footer = (X2D_FOOTER *)&data[size];
    uint16_t checksum = x2d_compute_checksum(data, size);
    if (checksum != BU16_TO_UNS16(footer->checksum))
    {
        return 0;
    }
    return size;
}

uint16_t x2d_compute_checksum(uint8_t *data, size_t size)
{
    uint16_t checksum = 0;
    for (int i = 0; i < size; ++i)
    {
        checksum += data[i];
    }
    return ~checksum + 1;
}