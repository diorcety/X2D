from construct import *
from enum import IntEnum


class Device(IntEnum):
    Tyxia_ZZAA = 0
    Calybox = 3
    Deltia_Emitter = 5
    Tydom_Panel_Controller = 13
    Tyxia_XXYY = 18
    Two_Buttons_Alarm_Remote = 21
    Four_Buttons_Alarm_Remote = 22

    # Detector/Simple
    Volumetric = 32
    Volumetric_Infrared = 33
    Volumetric_Pressure = 34
    Volumetric_Dual = 35
    Perimetric = 40
    Perimetric_Contact = 41
    Perimetric_Glass_Breakage = 42
    Perimetric_Sound = 43
    Perimetric_Roller_Shutter = 44
    Technical = 48
    Technical_Water = 49
    Technical_Gas = 50
    Technical_Fire = 51
    Technical_Smoke = 52
    Technical_Frost = 53
    Technical_Power_Outage = 54
    Technical_Phone_Outage = 55
    Technical_Freezer_Outage = 56

    USB_Key = 62


class Family(IntEnum):
    Regulation = 1
    Sensor = 2
    Metering = 3
    Actuator = 4


Affiliations = {
    1: Family.Regulation,
    2: Family.Regulation,
    10: Family.Sensor,
    12: Family.Sensor,
    14: Family.Metering,
    33: Family.Actuator,
    34: Family.Actuator,
    35: Family.Actuator,
    37: Family.Actuator,
    252: Family.Metering
}


class Attribute(IntEnum):
    Simple = 1
    WithData = 5


#
# WithData
#

class MessageDataType(IntEnum):
    Enrollment = 0  # g EnrolmentMessage
    HeatingLevel = 1  # m regulation Confort heating level
    FunctioningLevel = 2  # n regulation Auto functioning mode
    InternalTemperature = 10  # p Internal temperature command
    ExternalTemperature = 11  # o External temperature command
    MeterReading = 14  # bit[9] == 6 -> j(BasicMeterReading) else c (MeterReading)
    BasicCommand = 33  # q BasicCommand
    VariationCommand = 34  # d VariationCommand
    ScenarioCommand = 35  # k Scenario
    Variation = 37  # e Variation
    CurrentTransformerDefinition = 252  # l Current Transformer


#
# Functioning Mode / Heating Level
#

class FunctioningMode(IntEnum):
    Reduced = 0
    Moderato = 1
    Medio = 2
    Confort = 3
    Stop = 4
    AntiFrost = 5
    Special = 6
    Auto = 7
    Centralized = 8


FunctioningLevelMessage = Struct(
    "flags" / Bitwise(Struct(
        "manual" / Flag,
        "duration" / Flag,
        "f5" / Flag,
        "f4" / Flag,
        "mode" / Enum(BitsInteger(4), FunctioningMode)
    )),
    "duration" / IfThenElse(this.flags.duration, Int16ub, Pass)
)
HeatingLevelMessage = FunctioningLevelMessage


#
# Temperature
#

class TemperatureAdapter(Adapter):
    def _decode(self, obj, context, path):
        return obj / 512

    def _encode(self, obj, context, path):
        return int(obj * 512)


TemperatureMessage = Struct(
    "temperature" / TemperatureAdapter(Int16ub)
)

#
# Basic Command
#

class BasicCommand(IntEnum):
    Off = 0
    On = 1
    Toggle = 2


BasicCommandMessage = Struct(
    "command" / Enum(Int8ub, BasicCommand)
)


#
# Variation
#

class VariationValueAdapter(Adapter):
    def _decode(self, obj, context, path):
        return obj & 0x7F

    def _encode(self, obj, context, path):
        return obj & 0x7F


VariationMessage = Struct(
    "value" / VariationValueAdapter(Int8ub)
)


#
# Variation Command
#

class VariationCommand(IntEnum):
    More = 1
    ShortReleasedMore = 129
    LongReleasedMore = 65
    Less = 2
    ShortReleasedLess = 130
    LongReleasedLess = 66
    Stop = 4


VariationCommandMessage = Struct(
    "command" / Enum(Int8ub, VariationCommand),
    "dummy1" / Int8ub,
    "dummy2" / Int8ub,
)


#
# Enrollment
#

EnrollmentMessage = Struct(
    "rollingCode" / Int16ub
)

def x2d_crc(data):
    crc = 0
    for i in data:
        crc += int(i)
    return int((~crc) + 1) & 0xFFFF


def get_x2d_struct(data_length):
    return Struct(
        "body" / RawCopy(Struct(
            "house" / Int16ub,
            "source" / Bitwise(Struct(
                "id" / BitsInteger(2),
                "type" / Enum(BitsInteger(6), Device)
            )),
            "recipient" / Bitwise(Struct(
                "f7" / Flag,
                "f6" / Flag,
                "f5" / Flag,
                "f4" / Flag,
                "sub_index" / BitsInteger(4)
            )),
            "transmitter" / Bitwise(Struct(
                "enrollment_requested" / Flag,
                "internal_fault_detected" / Flag,
                "box_opened" / Flag,
                "battery_failing" / Flag,
                "attribute" / Enum(BitsInteger(4), Attribute)
            )),
            "control" / Bitwise(Struct(
                "f7" / Flag,
                "f6" / Flag,
                "f5" / Flag,
                "f4" / Flag,
                "f3" / Flag,
                "answer_request" / Flag,
                "f1" / Flag,
                "f0" / Flag
            )),
            "data" / Switch(lambda this: int(this.transmitter.attribute), {
                Attribute.WithData: Struct(
                    "type" / Enum(Int8ub, MessageDataType),
                    "content" / Switch(lambda this: int(this.type), {
                        MessageDataType.Enrollment: If(data_length - 2 > 0, EnrollmentMessage),
                        MessageDataType.BasicCommand: BasicCommandMessage,
                        MessageDataType.HeatingLevel: HeatingLevelMessage,
                        MessageDataType.FunctioningLevel: FunctioningLevelMessage,
                        MessageDataType.VariationCommand: VariationCommandMessage,
                    }, default=Array(data_length - 1, Byte))
                )
            }, default=Array(data_length, Byte)),
        )),
        "checksum" / Checksum(Int16ub, x2d_crc, this.body.data)
    )


def parse_x2d_message(data):
    return get_x2d_struct(len(data) - 8).parse(bytearray(data)).body.value


def format_x2d_message(msg):
    return get_x2d_struct(len(msg.data)).build(dict(body=dict(value=msg)))
