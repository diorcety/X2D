import sys

from construct import *
from enum import IntEnum
from itertools import repeat


def _count_leading(data, d=None):
    for i in range(len(data)):
        if d is None:
            d = data[i]
        if d is not None and data[i] != d:
            return d, i
    return d, 0


#
# Stream
#

def bytes_to_bitstream(data, le=True):
    new_data = []
    for i in data:
        for c in range(8):
            new_data.append((i >> ((7 - c) if le else c)) & 0x1)
    return new_data


def bitstream_to_bytes(data, le=True):
    if len(data) % 8 != 0:
        raise Exception("Invalid data")
    new_data = bytearray()
    b = 0
    c = 0
    for i in data:
        if le:
            b = b << 1 | i << 0
        else:
            b = b >> 1 | i << 7
        c += 1
        if c == 8:
            new_data.append(b)
            b = 0
            c = 0
    if c != 0:
        new_data.append(b)
    return new_data


#
# Processors
#

class Processor(object):
    def __init__(self, throw=True, verbose=False):
        self._throw = throw
        self._verbose = verbose

    def info(self, info):
        if self._verbose:
            print(info)

    def error(self, error):
        if self._throw:
            raise Exception(error)
        else:
            print(f"Process error: {error}", file=sys.stderr)

    def process(self, data):
        raise NotImplementedError()


class OOK(object):
    class Decoder(Processor):
        def __init__(self, sample_rate, symbol_rate, error=0.25, *args, **kwargs):
            super().__init__(*args, **kwargs)
            self._threshold = sample_rate / symbol_rate
            self._error_threshold = self._threshold * error
            self._buffered_data = bytearray()

        def process(self, in_data):
            out_data = bytearray()
            self._buffered_data.extend(in_data)
            while True:
                bit, count = _count_leading(self._buffered_data)
                if count == 0:
                    # Not enough data
                    break
                for i in range(1, 3):
                    if (self._threshold - self._error_threshold) * i < count < (
                            self._threshold + self._error_threshold) * i:
                        out_data.extend(repeat(bit, i))
                self._buffered_data = self._buffered_data[count:]
            return out_data

    class Encoder(Processor):
        def __init__(self, sample_rate, symbol_rate, *args, **kwargs):
            super().__init__(*args, **kwargs)
            self._threshold = sample_rate / symbol_rate

        def process(self, in_data):
            out_data = bytearray()
            for d in in_data:
                out_data.extend(repeat(d, self._threshold))


class Manchester(object):
    #      _  __  _   _...
    # ..._||_| |_||__|
    #     0  0 1  1  0
    #
    zero_pulse = bytes([0, 1])
    one_pulse = bytes([1, 0])

    class Encoder(Processor):
        def process(self, in_data):
            out_data = bytearray()
            for i in in_data:
                if i == 0:
                    out_data.extend(Manchester.zero_pulse)
                elif i == 1:
                    out_data.extend(Manchester.one_pulse)
                else:
                    self.error(f"Invalid value: {i}")
            return out_data

    class Decoder(Processor):
        def __init__(self, *args, **kwargs):
            super().__init__(*args, **kwargs)
            self._buffered_data = bytearray()

        def process(self, in_data):
            self._buffered_data.extend(in_data)

            out_data = bytearray()
            while len(self._buffered_data) > 1:
                part = self._buffered_data[0:2]
                if part == Manchester.zero_pulse:
                    out_data.append(0)
                elif part == Manchester.one_pulse:
                    out_data.append(1)
                else:
                    self.error(f"Invalid value: {part}")
            return out_data


class BiphaseMark(object):
    #     __   _  __  _
    # ...| |__||_| |_||_...
    #    0  0  1  0   1
    #
    zero_pulses = [bytearray([0, 0]), bytearray([1, 1])]
    one_pulses = [bytearray([0, 1]), bytearray([1, 0])]

    class Encoder(Processor):
        def __init__(self, *args, **kwargs):
            super().__init__(*args, **kwargs)
            self._flip = 1

        def process(self, in_data):
            out_data = bytearray()
            for i in in_data:
                if i == 0:
                    out_data.extend(BiphaseMark.zero_pulses[1 - self._flip])
                    self._flip = out_data[-1]
                elif i == 1:
                    out_data.extend(BiphaseMark.one_pulses[1 - self._flip])
                    self._flip = out_data[-1]
                else:
                    self.error(f"Invalid value: {i}")
            return out_data

    class Decoder(Processor):
        def __init__(self, *args, **kwargs):
            super().__init__(*args, **kwargs)
            self._buffered_data = bytearray()

        def process(self, in_data):
            self._buffered_data.extend(in_data)

            out_data = bytearray()
            i = 0
            for i in range(0, len(self._buffered_data), 2):
                if self._buffered_data[i:i + 2] in BiphaseMark.zero_pulses:
                    out_data.append(0)
                elif self._buffered_data[i:i + 2] in BiphaseMark.one_pulses:
                    out_data.append(1)
            self._buffered_data = self._buffered_data[i:]
            return out_data


class X2D(object):
    MIN_LEADING_ZEROS = 7
    MIN_LEADING_ONES = 6
    MAX_SUCCESSIVE_ONES = 5
    END_OF_FRAME = 0x1FE
    END_OF_FRAME_MASK = 0x1FF
    TRAILING_LENGTH = 7

    class Encoder(Processor):
        def __init__(self, preamble_0_count=9, preamble_1_count=6, separator=None, *args, **kwargs):
            super().__init__(*args, **kwargs)
            self._preamble_0_count = preamble_0_count
            self._preamble_1_count = preamble_1_count
            self._separator = separator or [1, 1, 1, 1, 1, 1, 0]
            self._is_init = False

        def insert_0_after_successive_1(self, data):
            count = 0
            new_data = bytearray()
            for i in data:
                if i == 1:
                    if count == X2D.MAX_SUCCESSIVE_ONES:
                        new_data.append(0)
                        count = 0
                    new_data.append(i)
                    count += 1
                else:
                    new_data.append(i)
                    count = 0
            return new_data

        def insert_end_of_frame(self):
            out_data = bytearray()
            for i in range(0, X2D.END_OF_FRAME_MASK.bit_length()):
                o = X2D.END_OF_FRAME_MASK.bit_length() - i - 1
                if X2D.END_OF_FRAME_MASK & (1 << o):
                    out_data.append((X2D.END_OF_FRAME >> o) & 0x1)
            return out_data

        def process(self, in_data):
            out_data = bytearray()
            for d in in_data:
                if not self._is_init:
                    out_data.extend(repeat(0, self._preamble_0_count))
                    out_data.extend(repeat(1, self._preamble_1_count))
                    out_data.append(0)
                    self._is_init = True
                bitstream = bytes_to_bitstream(d, False)
                out_data.extend(self.insert_0_after_successive_1(bitstream))
                out_data.extend(self.insert_end_of_frame())
                out_data.extend(self._separator)
            return out_data

    class Decoder(Processor):
        class State(Enum):
            INIT = 0
            LEAD_0 = 1
            LEAD_1 = 2
            EXTRA_0 = 3
            DATA = 4
            TRAILING = 5

        def __init__(self, *args, **kwargs):
            super().__init__(*args, **kwargs)
            self._buffered_data = bytearray()
            self._state = self.State.INIT
            self._offset = 0

        def strip_0_after_successive_1(self, data):
            count = 0
            new_data = bytearray()
            for i in data:
                if i == 1:
                    new_data.append(i)
                    count += 1
                else:
                    if count < X2D.MAX_SUCCESSIVE_ONES:
                        new_data.append(i)
                    elif count != X2D.MAX_SUCCESSIVE_ONES:
                        self.error("Invalid data: 5 \"1\" should be fallowed by one \"0\"")
                    count = 0
            return new_data

        def find_end_frame(self, data, pattern, mask):
            value = 0
            for idx, v in enumerate(data):
                value = (value << 1 | v) & mask
                if value == pattern:
                    return idx - mask.bit_length() + 1
            return None

        def pop(self, count):
            self._offset += count
            self._buffered_data = self._buffered_data[count:]

        def process(self, in_data):
            self._buffered_data.extend(in_data)
            out_data = []
            while True:
                if self._state == self.State.INIT:
                    v, count = _count_leading(self._buffered_data)
                    if count <= 0:
                        # Not enough data
                        break
                    if count >= X2D.MIN_LEADING_ZEROS and v == 0:
                        self._state = self.State.LEAD_0
                    else:
                        self.info(f"Ignoring data at offset {self._offset} of size {count}")
                        self.pop(count)
                elif self._state == self.State.LEAD_0:
                    _, count = _count_leading(self._buffered_data, 0)
                    if count <= 0:
                        # Not enough data
                        break

                    if count < X2D.MIN_LEADING_ZEROS:
                        self.error(f"Invalid lead 0 size: {count}")
                        self._state = self.State.INIT
                        break

                    # Update state
                    self.info(f"Leading 0 at offset {self._offset} of size {count}")
                    self.pop(count)
                    self._state = self.State.LEAD_1
                elif self._state == self.State.LEAD_1:
                    _, count = _count_leading(self._buffered_data, 1)
                    if count <= 0:
                        # Not enough data
                        break

                    if count < X2D.MIN_LEADING_ONES:
                        self.error(f"Invalid lead 1 size: {count}")
                        self._state = self.State.INIT
                        break

                    # Update state
                    self.info(f"Leading 1 at offset {self._offset} of size {count}")
                    self.pop(count)
                    self._state = self.State.EXTRA_0
                elif self._state == self.State.EXTRA_0:
                    if len(self._buffered_data) <= 0:
                        # Not enough data
                        break

                    # Update state
                    self.info(f"Extra 0 at offset {self._offset}")
                    self.pop(1)
                    self._state = self.State.DATA
                elif self._state == self.State.DATA:
                    end = self.find_end_frame(self._buffered_data, X2D.END_OF_FRAME, X2D.END_OF_FRAME_MASK)
                    if end is None:
                        # Not enough data
                        break

                    # Extract data
                    part = self._buffered_data[0:end]
                    frame_bitstream = self.strip_0_after_successive_1(part)
                    out_data.append(bitstream_to_bytes(frame_bitstream, False))

                    # Update state
                    self.info(f"Data offset {self._offset} of size {end}")
                    self.pop(end)
                    self.info(f"End of frame offset {self._offset} of size {X2D.END_OF_FRAME_MASK.bit_length()}")
                    self.pop(X2D.END_OF_FRAME_MASK.bit_length())
                    self._state = self.State.TRAILING
                elif self._state == self.State.TRAILING:
                    if len(self._buffered_data) < X2D.TRAILING_LENGTH:
                        # Not enough data
                        break

                    # Update state
                    self.info(f"Trailing at offset {self._offset} of size {X2D.TRAILING_LENGTH}")
                    self.pop(X2D.TRAILING_LENGTH)
                    self._state = self.State.DATA
            return out_data


# Champ Adresse Maison       2 octets    0 a FFFFh
# Champ Adresse Source       1 octet     0 a FFh
# Champ Adresse Destinataire 1 octet     0 a FFh
# Champ Adresse Transmetteur 1 octet     0 a FFh
# Champ de controle          1 octet
# Champ d information        N octet(s)  N compris entre 0 et 8
# Champ de verification      2 octets
def get_x2d_struct(data_length):
    def crc_func(data):
        crc = 0
        for i in data:
            crc += int(i)
        return int((~crc) + 1) & 0xFFFF

    class ControlType(IntEnum):
        A = 2
        B = 144

    class DeviceType(IntEnum):
        Tyxia_ZZAA = 0
        Calybox = 3
        Deltia_Emitter = 5
        Tyxia_XXYY = 18
        Tydom_Panel_Controller = 13
        Two_Buttons_Alarm_Remote = 21
        Four_Buttons_Alarm_Remote = 22
        Volumetric_Infrared = 33
        Perimeter_Contact = 41

    class OverrideOrder(IntEnum):
        MOON = 0
        SUN = 3

    class HeatOrder(IntEnum):
        OFF = 4
        HG = 5
        ON = 7

    return Struct(
        "body" / RawCopy(Struct(
            "house" / Int16ub,
            "source" / Bitwise(Struct(
                "id" / BitsInteger(2),
                "type" / Enum(BitsInteger(6), DeviceType)
            )),
            "recipient" / Int8ub,  # Area
            "transmitter" / Bitwise(Struct(
                "paring_requested" / Flag,
                "f6" / Flag,
                "f5" / Flag,
                "f4" / Flag,
                "f3" / Flag,
                "f2" / Flag,
                "f1" / Flag,
                "f0" / Flag
            )),
            "control" / Int8ub,
            "data" / Array(data_length, Byte),
        )),
        "checksum" / Checksum(Int16ub, crc_func, this.body.data)
    )


def parse_x2d_message(data):
    return get_x2d_struct(len(data) - 8).parse(bytearray(data)).body.value


def format_x2d_message(msg):
    return get_x2d_struct(len(msg.data)).build(dict(body=dict(value=msg)))