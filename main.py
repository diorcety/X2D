import random
import X2D as x2d
from encoding import Processor, OOK, BiphaseMark, Manchester, X2D, X2DMessage, Bitstream, process


#
# Helpers
#

def print_rflink_line(msg):
    id = msg.source.id << 16 | ((msg.house << 8) & 0xFF00) | ((msg.house >> 8) & 0x00FF)
    rc = (msg.data[-2] << 8 | msg.data[-1] << 0) if len(msg.data) >= 2 else 0
    s = (msg.data[-4] << 8 | msg.data[-3] << 0) if len(msg.data) >= 4 else 0
    print(f"X2D;ID={id:07x};SWITCH=?;CMD=?;EXT={msg.source.type};RC={rc:04x};S={s:04x};BAT=?;")


def print_message(name, msgs):
    print("#" * 80 + "\n" + name + "\n" + "#" * 80)
    assert len(msgs) > 0
    for msg in msgs:
        print(msg)
    print("")


class RFLinkDecoder(Processor):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self._bit = 0

    def data(self, in_data):
        out_data = bytearray()
        idx = 0
        s = Processor.Status.CONTINUE
        while idx < len(in_data) and s == Processor.Status.CONTINUE:
            d = in_data[idx]
            if 290 < d < 340:
                out_data.extend([self._bit, self._bit])
                self._bit = 1 - self._bit
            elif 80 < d < 130:
                out_data.append(self._bit)
                self._bit = 1 - self._bit
            else:
                self.error(f"Invalid pulse \"{d}\" end at offset {self._offset}")
                s = Processor.Status.RESET
            self._offset += 1
            idx += 1
        return idx, out_data, s


#
# Process different sources
#


def get_messages_from_baud_processors():
    return [BiphaseMark.Decoder(verbose=False), X2D.Decoder(verbose=False, throw=False),
            X2DMessage.Decoder(verbose=False)]


def get_messages_from_raw_processors(sample_rate, symbol_rate):
    return [OOK.Decoder(sample_rate, symbol_rate, verbose=False, throw=False)] + get_messages_from_baud_processors()


def get_messages_from_cc1101_manchester_processors():
    return [Bitstream.Encoder(), Manchester.Encoder(bytearray([0]))] + get_messages_from_baud_processors()


def get_messages_from_rflink_debug_processors():
    return [RFLinkDecoder(throw=False)] + get_messages_from_baud_processors()


#
# Process the data
#
"""
with open("raw3.bin", 'rb') as file:
    data = bytearray(file.read())
    in_data_1 = process([OOK.Decoder(int(2000000 / 20), int(4820), verbose=False, throw=False)], data)
    # in_data_2 = process([BiphaseMark.Decoder(verbose=False)], in_data_1)
    # in_data_3 = process([X2D.Decoder(verbose=False, throw=False)], in_data_2)
    # msgs = process([X2DMessage.Decoder(verbose=False)], in_data_3)
    # print_message("raw.bin", msgs)
    msgs = process(get_messages_from_raw_processors(int(2000000 / 20), int(4820)), data)
    print_message("raw3.bin", msgs)
"""
"""
with open("raw4.bin", 'rb') as file:
    data = bytearray(file.read())
    #in_data_1 = process([OOK.Decoder(int(2000000 / 20), int(4820), verbose=False, throw=False)], data)
    #in_data_2 = process([BiphaseMark.Decoder(verbose=False)], in_data_1)
    #in_data_3 = process([X2D.Decoder(verbose=True, throw=False)], in_data_2)
    #msgs = process([X2DMessage.Decoder(verbose=True)], in_data_3)
    #print_message("raw.bin", msgs)
    msgs = process(get_messages_from_raw_processors(int(2000000 / 20), int(4820)), data)
    print_message("raw4.bin", msgs)

for d in [[0x33, 0x33, 0x2a, 0xab, 0x55, 0x2c, 0xcd, 0x2b, 0x53, 0x32, 0xb3, 0x33, 0x4b, 0x33, 0x34, 0xb2, 0xd2, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xd5, 0x52, 0xb4, 0xcd, 0x2a, 0xaa, 0xb5, 0x55]]:
    in_data_1 = process([Bitstream.Encoder(True, verbose=False, throw=False)], d)
    in_data_2 = process([BiphaseMark.Decoder(verbose=False)], in_data_1)
    in_data_3 = process([X2D.Decoder(verbose=False, throw=False)], in_data_2)
    msgs = process([X2DMessage.Decoder(verbose=False)], in_data_3)
    print_message("raw.bin", msgs)
    
"""
"""
with open("raw.bin", 'rb') as file:
    in_data_0 = bytearray(file.read())
    in_data_1 = process([OOK.Decoder(2000000, 4820, verbose=False, throw=False)], in_data_0)
    in_data_2 = process([BiphaseMark.Decoder(verbose=False)], in_data_1)
    in_data_3 = process([X2D.Decoder(verbose=False)], in_data_2)
    msgs = process([X2DMessage.Decoder(verbose=False)], in_data_3)
    print_message("raw.bin", msgs)
    out_data_3 = process([X2DMessage.Encoder()], msgs)
    # for m in out_data_3:
    #    print(''.join('0x{:02x}, '.format(x) for x in m))
    out_data_2 = process([X2D.Encoder()], out_data_3)
    assert out_data_2[:len(in_data_2)] == in_data_2
    out_data_1 = process([BiphaseMark.Encoder()], out_data_2)
    assert out_data_1[:len(in_data_1)] == in_data_1
    out_data_0 = process([Bitstream.Decoder(False, verbose=False)], out_data_1)
    # print(''.join('0x{:02x}, '.format(x) for x in out_data_0))

with open("raw2.bin", 'rb') as file:
    in_data_0 = bytearray(file.read())
    in_data_1 = process([OOK.Decoder(2000000, 4820, verbose=False, throw=False)], in_data_0)
    in_data_1_1 = process([Bitstream.Decoder(False, verbose=False)], in_data_1)
    # print(''.join('0x{:02x}, '.format(x) for x in in_data_1_1))
    in_data_2 = process([BiphaseMark.Decoder(verbose=False)], in_data_1)
    in_data_3 = process([X2D.Decoder(verbose=False)], in_data_2)
    msgs = process([X2DMessage.Decoder(verbose=False)], in_data_3)
    print_message("raw2.bin", msgs)

with open("baud.bin") as file:
    data = bytearray([1 if a == '1' else 0 for a in file.read()])
    msgs = process(get_messages_from_baud_processors(), data, lambda x: random.randint(1, min(len(x), 64)))
    print_message("baud.bin", msgs)
"""
AssoArea3 = [0x55, 0x7f, 0x5d, 0xa4, 0xca, 0x95, 0x32, 0x52, 0x55, 0x3f, 0x27, 0xff, 0xc0]
AssoArea2 = [0x55, 0x7f, 0x5d, 0xa4, 0xca, 0xD5, 0x32, 0x52, 0x55, 0x3f, 0x18, 0x00, 0x3f]
AssoArea1 = [0x55, 0x7f, 0x5d, 0xa4, 0xca, 0xaa, 0xcd, 0xad, 0xaa, 0xc0, 0xc7, 0xff, 0xc0, 0x51, 0x2d, 0x9a, 0xaa, 0x99,
             0x29, 0x2a, 0x9f, 0x9c, 0x0, 0x1f, 0xd7, 0x69, 0x32, 0xaa, 0xb3, 0x6b, 0x6a, 0xb0, 0x31, 0xff, 0xf0, 0x14,
             0x4b, 0x66, 0xaa, 0xa6, 0x4a, 0x4a, 0xa7, 0xe7, 0x0, 0x7, 0xf5, 0xda, 0x4c, 0xaa, 0xac, 0xda, 0xda, 0xac,
             0x0c, 0x7f, 0xfc, 0x5, 0x12, 0xd9, 0xaa, 0xa9, 0x92, 0x92, 0xa9, 0xf9, 0xc0, 0x0
             ]
OffArea3 = [0x55, 0x7f, 0x5d, 0xa4, 0xca, 0x95, 0x32, 0xad, 0x95, 0x4a, 0x81, 0xc3, 0x80, 0x3f]
OffArea2 = [0x55, 0x7f, 0x5d, 0xa4, 0xca, 0xd5, 0x32, 0xad, 0x95, 0x4a, 0x81, 0xf9, 0xc0, 0x1f]
OffArea1 = [0x55, 0x7f, 0x5d, 0xa4, 0xca, 0xaa, 0xcd, 0x52, 0x6a, 0xb5, 0x7e, 0x28, 0x7f, 0xc0]

OnArea3 = [0x55, 0x7f, 0x5d, 0xa4, 0xca, 0x95, 0x32, 0xad, 0x95, 0x0a, 0x81, 0xf3, 0x80, 0x3f]
OnArea2 = [0x55, 0x7f, 0x5d, 0xa4, 0xca, 0xd5, 0x32, 0xad, 0x95, 0x0a, 0x81, 0xdc, 0x7f, 0xc0]
OnArea1 = [0x55, 0x7f, 0x5d, 0xa4, 0xca, 0xaa, 0xcd, 0x52, 0x6a, 0xf5, 0x7e, 0x1c, 0x7f, 0xc0]

HgArea3 = [0x55, 0x7f, 0x5d, 0xa4, 0xca, 0x95, 0x32, 0xad, 0x95, 0x35, 0x7e, 0x1c, 0x7f, 0xc0]
HgArea2 = [0x55, 0x7f, 0x5d, 0xa4, 0xca, 0xd5, 0x32, 0xad, 0x95, 0x35, 0x7e, 0x3c, 0x7f, 0xc0]
HgArea1 = [0x55, 0x7f, 0x5d, 0xa4, 0xca, 0xaa, 0xcd, 0xad, 0x52, 0x6a, 0xca, 0x81, 0xf9, 0xc0]

SunArea1 = [0x55, 0x7f, 0x5d, 0xa4, 0xca, 0xaa, 0xcd, 0x52, 0x2a, 0xea, 0x81, 0xc8, 0x7f, 0xc0]
MoonArea1 = [0x55, 0x7f, 0x5d, 0xa4, 0xca, 0xaa, 0xcd, 0x52, 0x2a, 0xaa, 0x81, 0xe7, 0x80, 0x3f]
SunArea2 = [0b01010101, 0b01111111, 0b01011101, 0b10100100, 0b11001010, 0b11010101, 0b00110010, 0b10101101, 0b11010101,
            0b00010101, 0b01111110, 0b00010111, 0b10000000, 0b00111111]
MoonArea2 = [0b01010101, 0b01111111, 0b01011101, 0b10100100, 0b11001010, 0b11010101, 0b00110010, 0b10101101, 0b11010101,
             0b01010101, 0b01111110, 0b00100111, 0b10000000, 0b00111111]
SunArea3 = [0b01010101, 0b01111111, 0b01011101, 0b10100100, 0b11001010, 0b10010101, 0b00110010, 0b10101101, 0b11010101,
            0b00010101, 0b01111110, 0b00101000, 0b01111111, 0b11000000]
MoonArea3 = [0b01010101, 0b01111111, 0b01011101, 0b10100100, 0b11001010, 0b10010101, 0b00110010, 0b10101101, 0b11010101,
             0b01010101, 0b01111110, 0b00001000, 0b01111111, 0b11000000]

for name, data in [('OnArea1', OnArea1), ('OnArea3', OnArea3), ('OffArea3', OffArea3), ('HgArea3', HgArea3),
                   ('SunArea3', SunArea3),
                   ('MoonArea3', MoonArea3), ('AssoArea3', AssoArea3), ('MoonArea1', MoonArea1)]:
    msgs = process(get_messages_from_cc1101_manchester_processors(), data)
    print_message(name, msgs)
"""
# X2D;ID=0014881;SWITCH=12;CMD=ON;EXT=TYXIA;RC=2093;S=2281;BAT=OK;
Pulse1 = [2550, 300, 330, 300, 330, 300, 330, 300, 330, 90, 120, 90, 120, 90, 120, 90, 120, 90, 120, 90, 120, 300, 120,
          90, 330, 300, 330, 300, 330, 300, 120, 90, 330, 300, 330, 90, 120, 300, 330, 90, 120, 300, 330, 120, 120, 300,
          330, 90, 120, 300, 120, 120, 330, 300, 330, 300, 330, 300, 330, 300, 330, 120, 120, 300, 120, 90, 330, 300,
          330, 300, 120, 90, 330, 300, 330, 300, 120, 120, 330, 300, 120, 90, 330, 120, 120, 300, 330, 300, 120, 90,
          330, 300, 120, 90, 330, 300, 330, 300, 330, 300, 120, 90, 330, 300, 330, 300, 330, 90, 120, 300, 330, 90, 120,
          90, 120, 300, 330, 90, 120, 300, 330, 120, 120, 300, 330, 120, 120, 90, 120, 90, 120, 90, 120, 90, 120, 300,
          120, 90, 330, 90, 120, 300, 120, 90, 120, 90, 120, 90, 120, 90, 330, 90, 120, 90, 120, 120, 120, 90, 120, 90,
          120, 90, 120, 90, 120, 90, 120, 300, 120, 90, 120, 90, 120, 90, 120, 90, 120, 90, 120, 90, 330, 120, 120, 300,
          330, 300, 330, 300, 330, 90, 120, 330, 330, 300, 120, 90, 330, 330, 120, 90, 330, 300, 120, 90, 330, 300, 120,
          90, 330, 90, 120, 300, 330, 300, 330, 300, 330, 300, 330, 300, 120, 90, 330, 120, 120, 300, 330, 300, 330, 90,
          120, 300, 330, 300, 330, 120, 120, 300, 330, 90, 120, 300, 120, 90, 330, 300, 330, 120, 120, 300, 330, 90,
          120, 300, 330, 330, 330, 300, 330, 90, 120, 300, 330, 300, 330, 300, 120, 90, 330, 300, 120, 90, 120, 120,
          330, 300, 120, 90, 330, 300, 120, 120, 330, 300, 120, 120, 120, 90, 120, 120, 120, 90, 120, 90, 330, 90, 120,
          300, 120, 120, 330, 90, 120, 90, 120, 90, 120, 90, 120, 300, 120, 90, 120, 90, 120, 120, 120, 90, 120, 90,
          120, 90, 120, 120, 120, 90, 330, 90, 120, 90, 120, 90, 120, 90, 120, 90, 120, 90, 120, 300, 120, 120, 330,
          330, 330, 300, 330, 300, 120, 90, 330, 300, 360, 60, 120, 300, 330, 90, 120, 300, 330, 120, 120, 300, 330, 90,
          120, 300, 120, 120, 330, 300, 330, 300, 330, 300, 330, 300, 330, 120, 120, 300, 120, 90, 330, 300, 330, 300,
          120, 90, 330, 300, 330, 300, 120, 90, 330, 300, 120, 90, 330, 90, 120, 300, 330, 300, 120, 90, 330, 300, 120,
          90, 330, 300, 330, 300, 330, 300, 120, 90, 330, 300, 330, 300, 330, 90, 120, 300, 330, 120, 120, 90, 120, 300,
          330, 90, 120, 300, 330, 90, 120, 300, 330, 90, 120, 90, 120, 90, 120, 90, 120, 90, 120, 300, 120, 90, 330, 90,
          120, 300, 120, 90, 120, 90, 120, 90, 120, 90, 330, 90, 120, 90, 120, 120, 120, 90, 120, 90, 120, 90, 120, 90,
          120, 90, 120, 300, 120, 90, 120, 90, 120, 90, 120, 90, 120, 90, 120, 90, 330, 120, 120, 300, 330, 300, 330,
          300, 330, 90, 120, 300, 330]
# [0x81, 0x48, 0x52, 0x0, 0x85, 0x90, 0x22, 0x81, 0x20, 0x93, 0xfc, 0x7a]
for name, data in [('Pulse1', Pulse1)]:
    msgs = process(get_messages_from_rflink_debug_processors(), data)
    print_message(name, msgs)
"""
data = [
    # [0x2f, 0x68, 0x83, 0x02, 0x05, 0x98, 0x01, 0x00, 0x00, 0x00, 0x47, 0x05, 0xfd, 0xfa],
    # [0x2f, 0x68, 0x83, 0x02, 0x05, 0x98, 0x01, 0x03, 0x00, 0x00, 0x0a, 0x09, 0xfe, 0x30],
    # [0x2f, 0x68, 0x83, 0x01, 0x05, 0x98, 0x01, 0x03, 0x00, 0x00, 0x43, 0x05, 0xfd, 0xfc],
    # [0x2f, 0x68, 0x83, 0x03, 0x05, 0x98, 0x01, 0x03, 0x00, 0x00, 0xac, 0x04, 0xfd, 0x92],
    # [0x2f, 0x68, 0x83, 0x02, 0x05, 0x98, 0x01, 0x00, 0x00, 0x00, 0xad, 0x04, 0xfd, 0x95],
    # [0x2f, 0x68, 0x83, 0x03, 0x05, 0x98, 0x01, 0x00, 0x00, 0x00, 0xd6, 0x08, 0xfd, 0x67],
    # [0x2f, 0x68, 0x83, 0x03, 0x05, 0x98, 0x02, 0x04, 0x00, 0x00, 0x59, 0x09, 0xfd, 0xde],
    # [0x2f, 0x68, 0x83, 0x00, 0x05, 0x98, 0xfe, 0x04, 0x01, 0x08, 0x01, 0x00, 0x00, 0xb7, 0x05, 0xfc, 0x81],
    # [0x2f, 0x68, 0x83, 0x00, 0x05, 0x98, 0xfe, 0x04, 0x00, 0x24, 0x03, 0x00, 0x00, 0x1a, 0x09, 0xfc, 0xfd],
    # [0x2f, 0x68, 0x83, 0x00, 0x05, 0x98, 0x0e, 0x03, 0x00, 0xb3, 0x7d, 0x00, 0x19, 0x6f, 0xfc, 0x80],
    # [0x2f, 0x68, 0x83, 0x00, 0x05, 0x98, 0x11, 0x00, 0xfe, 0x05, 0xfd, 0x35],
    # [0x2f, 0x68, 0x83, 0x00, 0x05, 0x98, 0x0a, 0x66, 0x25, 0xfd, 0x6e, 0xfc, 0x49],
    [0x2f, 0x68, 0x83, 0x00, 0x05, 0x98, 0x0d, 0x00, 0x00, 0x00, 0x1e, 0xe7, 0x6e, 0xfc, 0xc9],
    [0x2f, 0x68, 0x83, 0x00, 0x05, 0x98, 0x10, 0x15, 0x25, 0x20, 0x1f, 0xff, 0xff, 0xd4, 0x0e, 0xfa, 0xe0],
    [0x2f, 0x68, 0x83, 0x00, 0x05, 0x98, 0x11, 0x00, 0xf5, 0x27, 0xfd, 0x1c],
    [0x2f, 0x68, 0x83, 0x01, 0x05, 0x98, 0x1a, 0x70, 0x00, 0xfe, 0x6e, 0xfc, 0x52],
    [0x2f, 0x68, 0x83, 0x02, 0x05, 0x98, 0x1a, 0x70, 0x00, 0x64, 0x0d, 0xfd, 0x4c],
    [0x2f, 0x68, 0x83, 0x03, 0x05, 0x98, 0x1a, 0x70, 0x00, 0xde, 0x0e, 0xfc, 0xd0],
    [0x2f, 0x68, 0x83, 0x00, 0x05, 0x98, 0x1b, 0x00, 0x80, 0xd5, 0x0e, 0xfc, 0xcb],
    [0x2f, 0x68, 0x83, 0x00, 0x05, 0x98, 0x0e, 0x05, 0x00, 0x45, 0x35, 0x00, 0x06, 0x21, 0xfd, 0x95]
]
msgs = process([X2DMessage.Decoder(verbose=False)], data)
print_message("data", msgs)

"""
House: 12136
Source|Id: 2
Source|Type: Calybox
Recipient|Zone: 1
Transmitter|Attribute: WithData
Transmitter|BatteryFailing: false
Transmitter|BoxOpened: false
Transmitter|InternalFaultDetected: false
Transmitter|EnrollmentRequested: false
Control|AnswerRequested: false
Data|Type: HeatingLevel
HeatingLevel|FunctioningMode: Reduced
HeatingLevel|Manual: false
RollingCode: 62496
"""

d = {"house": 12136,
     "source": {"id": 0, "type": x2d.Device.USB_Key},
     "recipient": {"zone": 0},
     "transmitter": {"attribute": x2d.Attribute.WithData},
     "control": {"answer_request": True, "f7": True, "f4": True},
     "data": {"value": {"type": x2d.MessageDataType.CurrentLevel}},
     "rollingCode": None
     }
"""
d = {"house": 12136,
     "source": {"id": 0, "type": x2d.Device.USB_Key},
     "recipient": {"zone": 1},
     "transmitter": {"attribute": x2d.Attribute.WithData},
     "control": {"answer_request": False, "f7": True, "f4": True},
     "data": {"value": {"type": x2d.MessageDataType.HeatingLevel, "content": {"flags": {"manual": False, "mode": x2d.FunctioningMode.Confort}}}},
     "rollingCode": None
     }
"""
"""
Data|Type: HeatingLevel
HeatingLevel|FunctioningMode: Reduced
HeatingLevel|Manual: false
"""

#
"""
d = {"house": 12136,
     "source": {"id": 0, "type": x2d.Device.USB_Key},
     "recipient": {"zone": 0},
     "transmitter": {"attribute": x2d.Attribute.WithData, "enrollment_requested": True},
     "control": {"answer_request": False, "f7": True, "f4": True},
     "data": {"value": {"type": x2d.MessageDataType.Enrollment}},
     "rollingCode": None
     }
"""

# x = dict(width=3, height=2, pixels=[7, 8, 9, 11, 12, 13])
msgs = process([X2DMessage.Encoder(verbose=False)], [d])
for msg in msgs:
    print(''.join('0x{:02x}, '.format(x) for x in msg))
# msgs = process([X2DMessage.Decoder(verbose=False)], [d])
