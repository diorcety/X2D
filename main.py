from X2D import OOK, BiphaseMark, Manchester, X2D, parse_x2d_message, format_x2d_message, bytes_to_bitstream


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
    for msg in msgs:
        print(msg)
    print("")


#
# Process different sources
#

def get_messages_from_baud(in_data_1, check=False):
    in_data_2 = BiphaseMark.Decoder().process(in_data_1)
    in_data_3 = X2D.Decoder().process(in_data_2)
    msgs = [parse_x2d_message(d) for d in in_data_3]
    if check:
        out_data_3 = [format_x2d_message(m) for m in msgs]
        out_data_2 = X2D.Encoder().process(out_data_3)
        assert out_data_2[:len(in_data_2)] == in_data_2
        out_data_1 = BiphaseMark.Encoder().process(out_data_2)
        assert out_data_1[:len(in_data_1)] == in_data_1
    return msgs


def get_messages_from_raw(in_data_1, sample_rate, symbol_rate, check=False):
    in_data_2 = OOK.Decoder(sample_rate, symbol_rate).process(in_data_1)
    return get_messages_from_baud(in_data_2, check)


def get_messages_from_cc1101_manchester(in_data_1, check=False):
    in_data_2 = bytearray([0])
    in_data_2.extend(Manchester.Encoder().process(bytes_to_bitstream(in_data_1)))
    return get_messages_from_baud(in_data_2, check)


def get_messages_from_rflink_debug(in_data_1, check=False):
    in_data_2 = bytearray()
    bit = 0
    for i in in_data_1:
        if 290 < i < 340:
            in_data_2.extend([bit, bit])
            bit = 1 - bit
        elif 80 < i < 130:
            in_data_2.append(bit)
            bit = 1 - bit
        else:
            print(f"Ignored pulse: {i}")
    return get_messages_from_baud(in_data_2, check)


#
# Process sources
#

with open("raw.bin", 'rb') as file:
    in_data_0 = bytearray(file.read())
    msgs = get_messages_from_raw(in_data_0, 2000000, 4820, True)
    print_message("raw.bin", msgs)

with open("baud.bin") as file:
    data = bytearray([1 if a == '1' else 0 for a in file.read()])
    msgs = get_messages_from_baud(data)
    print_message("baud.bin", msgs)

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
    msgs = get_messages_from_cc1101_manchester(data)
    print_message(name, msgs)

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
    msgs = get_messages_from_rflink_debug(data)
    print_message(name, msgs)
