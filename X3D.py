from construct import *
from enum import IntEnum

from XxD import OffsettedEnd


class MessageType(IntEnum):
    Sensor = 0
    Standard = 1
    Pairing = 2
    Beacon = 3


class StatusType(IntEnum):
    Open = 0xe0
    Pinned = 0xe5


_sbox_table = [0x1, 0x0, 0xC, 0x8, 0xA, 0x9, 0xE, 0x7, 0x3, 0x5, 0x4, 0xB, 0x2, 0xF, 0x6, 0xD]


def _apply_sbox(value: int, count: int) -> int:
    nibble = (value >> count) & 0xF
    new_nibble = _sbox_table[nibble]
    result = (value & ~(0xF << count)) | (new_nibble << count)
    return result & 0xFFFF


def x3d_enc_msg_id(p_msg_id: int, device_id: int) -> int:
    result = p_msg_id
    xor_key = ((device_id & 0xFF) ^ ((device_id >> 16) & 0xFF)) | (device_id & 0xFF00)
    for i in range(32):
        result = _apply_sbox(result, i % 13) ^ xor_key
    return result & 0xFFFF


def x3d_dec_msg_id(enc_msg_id: int, device_id: int) -> int:
    result = enc_msg_id
    xor_key = ((device_id & 0xFF) ^ ((device_id >> 16) & 0xFF)) | (device_id & 0xFF00)
    for i in reversed(range(32)):
        result = _apply_sbox(result ^ xor_key, i % 13)
    return result & 0xFFFF


class MsgIdAdapter(Adapter):
    def _decode(self, obj, context, path):
        device_id = context.device_id
        return x3d_dec_msg_id(obj, device_id)

    def _encode(self, obj, context, path):
        device_id = context.device_id
        return x3d_enc_msg_id(obj, device_id)


def x3d_body_crc(data):
    res = 0
    for d in data:
        res -= d
    return res & 0xFFFF


def x3d_crc(data):
    from crc import Calculator, Crc16
    calculator = Calculator(Crc16.XMODEM)
    return calculator.checksum(data)


def _body_length(ctx):
    if getattr(ctx, 'length', None) is not None:
        return ctx.length.value - 3
    elif hasattr(ctx, 'body'):
        body_bytes = _x3d_struct.body.subcon.subcon.build(ctx.body)
        return len(body_bytes) + 3
    else:
        raise RuntimeError()


def _header_length(ctx):
    if getattr(ctx.control, 'header_length', None) is not None:
        return ctx.control.header_length - 3
    elif hasattr(ctx, 'header'):
        data_bytes = _x3d_struct.body.subcon.subcon.subcon.header.subcon.subcon.build(ctx.header)
        return len(data_bytes) + 3
    else:
        raise RuntimeError()


PairingPayload = Struct(
    "counters" / Bitwise(Struct(
        "reply" / BitsInteger(4),
        "request" / BitsInteger(4),
    )),
    "transfer_slot" / Int16ul,
    "transferred_slot" / Int16ul,
    Const(b"\x1F"),
    Const(b"\xFF"),
    "slot" / Int8ul,
    Const(b"\x00"),
    "rnd" / Int16ul,
    "status" / Enum(Int8ub, StatusType),
    "dummy" / GreedyBytes
)

HeaderPairingPayload = Struct(
    Const(b"\x85"),
    Const(b"\x98"),
    Const(b"\x00"),
)

HeaderBeaconPayload = Struct(
    Const(b"\x05"),
    Const(b"\x98"),
    Const(b"\x00"),
)

_x3d_struct = Struct(
    "length" / RawCopy(Rebuild(Int8ub, _body_length)),
    "body" / FixedSized(_body_length, RawCopy(Struct(
        Const(b"\xFF"),
        "seq" / Int8ul,
        "type" / Enum(Int8ul, MessageType),
        "control" / Bitwise(Struct(
            "f7" / Default(Flag, False),
            "f6" / Default(Flag, False),
            "f5" / Default(Flag, False),
            "header_length" / Rebuild(BitsInteger(5), lambda ctx: _header_length(ctx._)),
        )),
        "header" / FixedSized(_header_length, RawCopy(Struct(
            "device_id" / Int24ul,
            "network_id" / Int8ul,
            "payload" / OffsettedEnd(-2, RawCopy(Switch(lambda this: int(this._.type), {
                MessageType.Pairing: HeaderPairingPayload,
                MessageType.Beacon: HeaderBeaconPayload,
            }, default=GreedyBytes))),
            "msg_id" / MsgIdAdapter(Int16ul),
            "_msg_id" / Check(lambda ctx: ctx.msg_id != 0)
        ))),
        "checksum" / Checksum(Int16ub, x3d_body_crc, lambda ctx: ctx.header.data),
        "payload" / Optional(RawCopy(Switch(lambda this: int(this.type), {
            MessageType.Pairing: PairingPayload,
        }, default=GreedyBytes))),
    ))),
    "checksum" / Checksum(Int16ub, x3d_crc, lambda ctx: ctx.length.data + ctx.body.data)
)


def parse_x3d_message(data):
    return _x3d_struct.parse(bytearray(data)).body.value


def format_x3d_message(msg):
    return _x3d_struct.build(dict(body=dict(value=msg)))
