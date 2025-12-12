from construct import *
from enum import IntEnum


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
    """
    Apply S-box substitution to a specific nibble of the value.

    Args:
        value: The integer value to apply S-box to
        count: The bit position (0-3) of the nibble to substitute

    Returns:
        The value with the specified nibble substituted using the S-box table
    """
    # 1. Extract the nibble at the specified bit position
    nibble = (value >> count) & 0xF

    # 2. Substitute the nibble using the S-box table
    new_nibble = _sbox_table[nibble]

    # 3. Replace the nibble in the value and return
    result = (value & ~(0xF << count)) | (new_nibble << count)

    return result & 0xFFFF


def x3d_enc_msg_id(p_msg_id: int, device_id: int) -> int:
    """
    Encrypt a message ID using device-specific XOR key and S-box substitution.

    Args:
        p_msg_id: The plain message ID to encrypt
        device_id: The device identifier used for XOR key generation

    Returns:
        The encrypted message ID
    """
    # 1. Initialize result with plain message ID
    result = p_msg_id

    # 2. Generate XOR key from device ID
    xor_key = ((device_id & 0xFF) ^ ((device_id >> 16) & 0xFF)) | (device_id & 0xFF00)

    # 3. Apply S-box substitution and XOR with key for 32 iterations
    for i in range(32):
        result = _apply_sbox(result, i % 13) ^ xor_key

    return result & 0xFFFF


def x3d_dec_msg_id(enc_msg_id: int, device_id: int) -> int:
    """
    Decrypt a message ID using device-specific XOR key and reverse S-box substitution.

    Args:
        enc_msg_id: The encrypted message ID to decrypt
        device_id: The device identifier used for XOR key generation

    Returns:
        The decrypted message ID
    """
    # 1. Initialize result with encrypted message ID
    result = enc_msg_id

    # 2. Generate XOR key from device ID (same as encryption)
    xor_key = ((device_id & 0xFF) ^ ((device_id >> 16) & 0xFF)) | (device_id & 0xFF00)

    # 3. Apply reverse S-box substitution and XOR with key for 32 iterations (in reverse)
    for i in reversed(range(32)):
        result = _apply_sbox(result ^ xor_key, i % 13)

    return result & 0xFFFF


class MsgIdAdapter(Adapter):
    """
    Adapter to automatically encrypt/decrypt message IDs based on device context.
    """
    def _decode(self, obj, context, path):
        """
        Decrypt message ID when parsing data.

        Args:
            obj: The encrypted message ID from the data
            context: Parsing context containing device_id
            path: Parsing path

        Returns:
            The decrypted message ID
        """
        device_id = context.device_id
        return x3d_dec_msg_id(obj, device_id)

    def _encode(self, obj, context, path):
        """
        Encrypt message ID when building data.

        Args:
            obj: The plain message ID to encrypt
            context: Building context containing device_id
            path: Building path

        Returns:
            The encrypted message ID
        """
        device_id = context.device_id
        return x3d_enc_msg_id(obj, device_id)


def x3d_body_crc(data):
    """
    Calculate CRC for X3D message body.

    Args:
        data: The data to calculate CRC for

    Returns:
        The calculated CRC value
    """
    res = 0
    for d in data:
        res -= d
    return res & 0xFFFF


def x3d_crc(data):
    """
    Calculate CRC using XMODEM algorithm.

    Args:
        data: The data to calculate CRC for

    Returns:
        The calculated CRC value using XMODEM algorithm
    """
    from crc import Calculator, Crc16
    calculator = Calculator(Crc16.XMODEM)
    return calculator.checksum(data)


def _body_length(ctx):
    """
    Calculate the length of the body section.

    Args:
        ctx: Context object containing message structure

    Returns:
        The calculated body length

    Raises:
        RuntimeError: If body length cannot be determined
    """
    # 1. If length value is already known, return it minus 3
    if getattr(ctx, 'length', None) is not None:
        return ctx.length.value - 3

    # 2. If body is available, calculate its length
    elif hasattr(ctx, 'body'):
        body_bytes = _x3d_struct.body.subcon.subcon.build(ctx.body)
        return len(body_bytes) + 3

    # 3. Raise error if neither length nor body is available
    else:
        raise RuntimeError()


def _header_length(ctx):
    """
    Calculate the length of the header section.

    Args:
        ctx: Context object containing message structure

    Returns:
        The calculated header length

    Raises:
        RuntimeError: If header length cannot be determined
    """
    # 1. If header_length is already known, return it minus 3
    if getattr(ctx.control, 'header_length', None) is not None:
        return ctx.control.header_length - 3

    # 2. If header is available, calculate its length
    elif hasattr(ctx, 'header'):
        data_bytes = _x3d_struct.body.subcon.subcon.subcon.header.subcon.subcon.build(ctx.header)
        return len(data_bytes) + 3

    # 3. Raise error if neither header_length nor header is available
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


def parse_x3d_message(data: bytes) -> dict:
    """
    Parse X3D message data into structured format.

    Args:
        data: The raw X3D message data to parse

    Returns:
        The parsed message structure
    """
    return _x3d_struct.parse(bytearray(data)).body.value


def format_x3d_message(msg: dict) -> bytes:
    """
    Format message structure into X3D message data.

    Args:
        msg: The message structure to format

    Returns:
        The formatted X3D message data
    """
    return _x3d_struct.build(dict(body=dict(value=msg)))
