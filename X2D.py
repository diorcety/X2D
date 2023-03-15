import construct.core
import io

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
    CurrentLevel = 26
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
        "manual" / Default(Flag, False),
        "duration" / Default(Flag, False),
        "f5" / Default(Flag, False),
        "f4" / Default(Flag, False),
        "mode" / Enum(BitsInteger(4), FunctioningMode)
    )),
    "duration" / Optional(Int16ub)
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
    "temperature" / TemperatureAdapter(Int16ul)
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
    "dummy1" / Default(Int8ub, 0),
    "dummy2" / Default(Int8ub, 0),
)

#
# Enrollment
#

EnrollmentMessage = Struct(
)


#
# Meter Reading Message
#


class ElectricityTariff(IntEnum):
    Base = 0
    EjpOffPeakDay = 32
    EjpPeakDay = 33
    DoubleOffPeakHour = 64
    DoublePeakHour = 65
    TempoBlueDayOffPeakHour = 96
    TempoBlueDayPeakHour = 97
    TempoWhiteDayOffPeakHour = 98
    TempoWhiteDayPeakHour = 99
    TempoRedDayOffPeakHour = 100
    TempoRedDayPeakHour = 101


class RegisterSelection(IntEnum):
    CurrentTransformer1 = 0
    CurrentTransformer2 = 1
    CurrentTransformer3 = 2
    Heating = 3
    HotWater = 4
    Total = 5
    Cooling = 7
    HeatingAndCooling = 8
    ElectricityProduction = 9


MeterReadingMessage = Struct(
    "selection" / Enum(Int8ub, RegisterSelection),
    "currentTariff" / Bitwise(Struct(
        "f7" / Default(Flag, False),
        "double" / Default(Flag, False),
        "ejp" / Default(Flag, False),
        "euro" / Default(Flag, False),
        "f3" / Default(Flag, False),
        "f2" / Default(Flag, False),
        "f1" / Default(Flag, False),
        "f0" / Default(Flag, False),
    )),
    "register" / BytesInteger(3, swapped=True)
)


#
# Functions
#

def x2d_crc(data):
    crc = 0
    for i in data:
        crc += int(i)
    return int((~crc) + 1) & 0xFFFF


class OffsettedEnd(Subconstruct):
    r"""
    Parses all bytes in the stream till EOF plus endoffset is reached.

    This is useful when GreedyBytes (or an other greedy construct) is followed by a fixed-size footer.

    Parsing determines the length of the stream and reads all bytes till EOF plus `endoffset` is reached, then defers to subcon using new BytesIO with said bytes. Building defers to subcon as-is. Size is undefined.

    :param endoffset: integer or context lambda, only negative offsets or 0 are allowed.
    :param subcon: Construct instance

    :raises StreamError: could not read enough bytes
    :raises StreamError: reads behind the stream (if endoffset is positive)

    Example::

        >>> d = Struct("data"/OffsettedEnd(-2, GreedyBytes), "footer"/Bytes(2))
        >>> d.parse(b"\x01\x02\x03\x04\x05")
        Container(data=b'\x01\x02\x03', footer=b'\x04\x05')
        >>> d.build(Container(data=b"\x01\x02\x03", footer=b"\x04\x05"))
        b'\x01\x02\x03\x04\x05'
    """

    def __init__(self, endoffset, subcon):
        super().__init__(subcon)
        self.endoffset = endoffset

    def _parse(self, stream, context, path):
        endoffset = construct.core.evaluate(self.endoffset, context)
        curpos = stream_tell(stream, path)
        stream_seek(stream, 0, 2, path)
        endpos = stream_tell(stream, path)
        stream_seek(stream, curpos, 0, path)
        length = endpos + endoffset - curpos
        data = stream_read(stream, length, path)
        if self.subcon is GreedyBytes:
            return data
        if type(self.subcon) is GreedyString:
            return data.decode(self.subcon.encoding)
        return self.subcon._parsereport(io.BytesIO(data), context, path)

    def _build(self, obj, stream, context, path):
        return self.subcon._build(obj, stream, context, path)


def _offset(context):
    return -2 if context.control.rolling_code else -0

_x2d_struct = Struct(
    "body" / OffsettedEnd(-2, RawCopy(Struct(
        "house" / Int16ub,
        "source" / Bitwise(Struct(
            "id" / BitsInteger(2),
            "type" / Enum(BitsInteger(6), Device)
        )),
        "recipient" / Bitwise(Struct(
            "f7" / Default(Flag, False),
            "f6" / Default(Flag, False),
            "f5" / Default(Flag, False),
            "f4" / Default(Flag, False),
            "zone" / BitsInteger(4)
        )),
        "transmitter" / Bitwise(Struct(
            "enrollment_requested" / Default(Flag, False),
            "internal_fault_detected" / Default(Flag, False),
            "box_opened" / Default(Flag, False),
            "battery_failing" / Default(Flag, False),
            "attribute" / Enum(BitsInteger(4), Attribute)
        )),
        "control" / Bitwise(Struct(
            "f7" / Default(Flag, False),
            "f6" / Default(Flag, False),
            "f5" / Default(Flag, False),
            "f4" / Default(Flag, False),
            "rolling_code" / Default(Flag, False),
            "answer_request" / Default(Flag, False),
            "f1" / Default(Flag, False),
            "f0" / Default(Flag, False)
        )),
        "data" / OffsettedEnd(_offset, RawCopy(Switch(lambda this: int(this.transmitter.attribute), {
            Attribute.WithData: Struct(
                "type" / Enum(Int8ub, MessageDataType),
                "content" / Optional(Switch(lambda this: int(this.type), {
                    MessageDataType.Enrollment: EnrollmentMessage,
                    MessageDataType.BasicCommand: BasicCommandMessage,
                    MessageDataType.HeatingLevel: HeatingLevelMessage,
                    MessageDataType.FunctioningLevel: FunctioningLevelMessage,
                    MessageDataType.VariationCommand: VariationCommandMessage,
                    MessageDataType.InternalTemperature: TemperatureMessage,
                    MessageDataType.MeterReading: MeterReadingMessage,
                    MessageDataType.CurrentLevel: HeatingLevelMessage,
                }, default=GreedyBytes))
            )
        }))),
        "rollingCode" / If(this.control.rolling_code, Int16ub),
    ))),
    "checksum" / Checksum(Int16ub, x2d_crc, this.body.data)
)


def parse_x2d_message(data):
    return _x2d_struct.parse(bytearray(data)).body.value


def format_x2d_message(msg):
    return _x2d_struct.build(dict(body=dict(value=msg)))
