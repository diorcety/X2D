import sys

from enum import Enum
from itertools import repeat
from collections.abc import Iterable, Sized


def _set_or_extend(current, to_add):
    if current is None:
        current = to_add
    elif to_add is not None:
        current.extend(to_add)
    return current


#
# Processors
#

class Processor(object):
    class Status(Enum):
        CONTINUE = 0xAABBCCDD
        RESET = 0xDEADBEEF

    def __init__(self, throw=True, verbose=False):
        self._throw = throw
        self._verbose = verbose
        self._offset = 0
        self._buffered_data = None
        self.reset()

    def info(self, info):
        if self._verbose:
            print(f"{self} - {info}")

    def error(self, error):
        if self._throw:
            raise Exception(error)
        else:
            print(f"Process error: {error}", file=sys.stderr)

    def process(self, value):
        if value is None:
            return None
        if not isinstance(value, Iterable) or not isinstance(value, Sized):
            raise Exception(f"Invalid data type: {type(value)}")
        in_data = _set_or_extend(self._buffered_data, value)
        consumed, out_data, s = self.data(in_data)
        self._offset += consumed
        if s == Processor.Status.CONTINUE:
            self._buffered_data = in_data[consumed:]
            consumed = len(value)
        return consumed, out_data, s

    def data(self, data):
        raise NotImplementedError()

    def reset(self):
        if self._buffered_data is not None:
            self._offset += len(self._buffered_data)
            self._buffered_data = None


class Bitstream(object):
    class Decoder(Processor):
        def __init__(self, be=True, *args, **kwargs):
            super().__init__(*args, **kwargs)
            self._be = be

        def data(self, in_data):
            out_data = None
            b = 0
            idx = 0
            while idx < int(len(in_data)/8)*8:
                d = in_data[idx] & 0x1
                b = b << 1 | d << 0 if self._be else b >> 1 | d << 7
                idx += 1
                if idx % 8 == 0:
                    out_data = _set_or_extend(out_data, bytearray([b]))
                    b = 0

            return idx, out_data, Processor.Status.CONTINUE

    class Encoder(Processor):
        def __init__(self, be=True, *args, **kwargs):
            super().__init__(*args, **kwargs)
            self._be = be

        def data(self, in_data):
            out_data = None
            idx = 0
            while idx < len(in_data):
                d = in_data[idx]
                b = bytearray()
                for c in range(8):
                    b.append((d >> ((7 - c) if self._be else c)) & 0x1)
                out_data = _set_or_extend(out_data, b)
                idx += 1
            return idx, out_data, Processor.Status.CONTINUE


class OOK(object):
    class Decoder(Processor):
        UNDEFINED = 2

        def __init__(self, sample_rate, symbol_rate, error=0.3, *args, **kwargs):
            super().__init__(*args, **kwargs)
            self._threshold = sample_rate / symbol_rate
            self._error_threshold = self._threshold * error
            self._bit = None
            self._count = None
            self.reset()

        def reset(self):
            super().reset()
            self._bit = self.UNDEFINED

        def find_pulse_width(self, count):
            for i in range(1, 3):
                if (self._threshold - self._error_threshold) * i < count < (
                        self._threshold + self._error_threshold) * i:
                    return i
            return None

        def data(self, in_data):
            out_data = None
            idx = 0
            s = Processor.Status.CONTINUE
            while idx < len(in_data) and s == Processor.Status.CONTINUE:
                d = in_data[idx]
                if self._bit == self.UNDEFINED:
                    self._bit = d
                    self._count = 1
                    idx += 1
                else:
                    end = in_data.find(1 - self._bit, idx)
                    if end >= 0:
                        self._count += (end - idx)
                        idx += (end - idx)
                        d = self._bit
                        self._bit = self.UNDEFINED

                        width = self.find_pulse_width(self._count)

                        if width is None:
                            self.error(f"Invalid pulse \"{d}\" " +
                                       f"at offset {self._offset + idx - self._count} " +
                                       f"of size {self._count}")
                            s = Processor.Status.RESET
                        else:
                            self.info(
                                f"Pulse \"{d}\" at offset {self._offset + idx - self._count} of size {self._count}")
                            out_data = _set_or_extend(out_data, bytearray(repeat(d, width)))
                    else:
                        end = len(in_data)
                        self._count += (end - idx)
                        idx += (end - idx)
            return idx, out_data, s

    class Encoder(Processor):
        def __init__(self, sample_rate, symbol_rate, *args, **kwargs):
            super().__init__(*args, **kwargs)
            self._threshold = sample_rate / symbol_rate
            self.reset()

        def data(self, in_data):
            out_data = None
            idx = 0
            while idx < len(in_data):
                d = in_data[idx]
                out_data = _set_or_extend(out_data, bytearray(repeat(d, self._threshold)))
                idx += 1
            return idx, out_data, Processor.Status.CONTINUE


class Manchester(object):
    #      _  __  _   _...
    # ..._||_| |_||__|
    #     0  0 1  1  0
    #
    zero_pulse = bytes([0, 1])
    one_pulse = bytes([1, 0])

    class Encoder(Processor):
        def __init__(self, initial, *args, **kwargs):
            super().__init__(*args, **kwargs)
            self._initial = initial or bytearray()
            self._initialized = None
            self.reset()

        def reset(self):
            super().reset()
            self._initialized = False

        def data(self, in_data):
            out_data = None
            if not self._initialized:
                self._initialized = True
                out_data = self._initial
            s = Processor.Status.CONTINUE
            idx = 0
            while idx < len(in_data):
                d = in_data[idx]
                if d == 0:
                    out_data = _set_or_extend(out_data, bytearray(Manchester.zero_pulse))
                elif d == 1:
                    out_data = _set_or_extend(out_data, bytearray(Manchester.one_pulse))
                else:
                    self.error(f"Invalid value: {d} at offset {self._offset + idx}")
                    s = Processor.Status.RESET
                idx += 1
            return idx, out_data, s

    class Decoder(Processor):
        def __init__(self, *args, **kwargs):
            super().__init__(*args, **kwargs)
            self.reset()

        def data(self, in_data):
            out_data = None
            s = Processor.Status.CONTINUE
            idx = 0
            while idx + 2 <= len(in_data) and s == Processor.Status.CONTINUE:
                part = in_data[idx:idx + 2]
                if part == Manchester.zero_pulse:
                    out_data = _set_or_extend(out_data, bytearray([0]))
                elif part == Manchester.one_pulse:
                    out_data = _set_or_extend(out_data, bytearray([1]))
                else:
                    self.error(f"Invalid value \"{part}\" at offset {self._offset + idx}")
                    s = Processor.Status.RESET
                idx += 2
            return idx, out_data, s


class BiphaseMark(object):
    #     __   _  __  _
    # ...| |__||_| |_||_...
    #    0  0  1  0   1
    #
    zero_pulses = [bytes([0, 0]), bytes([1, 1])]
    one_pulses = [bytes([0, 1]), bytes([1, 0])]

    class Encoder(Processor):
        def __init__(self, *args, **kwargs):
            super().__init__(*args, **kwargs)
            self._flip = None
            self.reset()

        def reset(self):
            super().reset()
            self._flip = 1

        def data(self, in_data):
            out_data = None
            idx = 0
            s = Processor.Status.CONTINUE
            while idx < len(in_data) and s == Processor.Status.CONTINUE:
                d = in_data[idx]
                if d == 0:
                    out_data = _set_or_extend(out_data, bytearray(BiphaseMark.zero_pulses[1 - self._flip]))
                    self._flip = out_data[-1]
                elif d == 1:
                    out_data = _set_or_extend(out_data, bytearray(BiphaseMark.one_pulses[1 - self._flip]))
                    self._flip = out_data[-1]
                else:
                    self.error(f"Invalid value \"{d}\" at offset {self._offset + idx}")
                    s = Processor.Status.RESET
                idx += 1
            return idx, out_data, s

    class Decoder(Processor):
        def __init__(self, *args, **kwargs):
            super().__init__(*args, **kwargs)
            self.reset()

        def data(self, in_data):
            out_data = None
            idx = 0
            while idx + 2 <= len(in_data):
                chunk = in_data[idx:idx + 2]
                if chunk in BiphaseMark.zero_pulses:
                    out_data = _set_or_extend(out_data, bytearray([0]))
                elif chunk in BiphaseMark.one_pulses:
                    out_data = _set_or_extend(out_data, bytearray([1]))
                self.info(f"Detect \"{out_data[-1]}\" at offset {self._offset + idx}")
                idx += 2
            return idx, out_data, Processor.Status.CONTINUE


class X2D(object):
    MIN_LEADING_ZEROS = 7
    MIN_LEADING_ONES = 6
    MAX_SUCCESSIVE_ONES = 5
    END_OF_FRAME = [1, 1, 1, 1, 1, 1, 1, 1, 0]
    TRAILING_LENGTH = 7
    EXTRA_0_LENGTH = 1
    MAX_BIT_LENGTH = 16 * 8

    class Encoder(Processor):
        def __init__(self, preamble_0_count=9, preamble_1_count=6, separator=None, *args, **kwargs):
            super().__init__(*args, **kwargs)
            self._preamble_0_count = preamble_0_count
            self._preamble_1_count = preamble_1_count
            self._separator = separator or [1, 1, 1, 1, 1, 1, 0]
            self._is_initialized = None
            self.reset()

        def reset(self):
            super().reset()
            self._is_initialized = False

        @staticmethod
        def insert_0_after_successive_1(data):
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

        def data(self, in_data):
            out_data = None
            s = Processor.Status.CONTINUE
            idx = 0
            while idx < len(in_data):
                if not self._is_initialized:
                    out_data = _set_or_extend(out_data, bytearray(repeat(0, self._preamble_0_count)))
                    out_data = _set_or_extend(out_data, bytearray(repeat(1, self._preamble_1_count)))
                    out_data = _set_or_extend(out_data, bytearray([0]))
                    self._is_initialized = True
                d = in_data[idx]
                length, data, f = Bitstream.Encoder(False).process(d)
                if length != len(d) or f != Processor.Status.CONTINUE:
                    self.info(f"Invalid data at offset {self._offset + idx}")
                    s = Processor.Status.RESET
                    break
                out_data = _set_or_extend(out_data, self.insert_0_after_successive_1(data))
                out_data = _set_or_extend(out_data, X2D.END_OF_FRAME)
                out_data = _set_or_extend(out_data, self._separator)
                idx += 1
            return idx, out_data, s

    class Decoder(Processor):
        class State(Enum):
            INIT = 0
            LEAD_1 = 1
            EXTRA_0 = 2
            DATA = 3
            TRAILING = 4

        def __init__(self, *args, **kwargs):
            super().__init__(*args, **kwargs)
            self._state = None
            self.reset()

        def reset(self):
            super().reset()
            self._state = self.State.INIT

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

        @staticmethod
        def count_leading(data):
            d = None
            offset = 0
            while offset < len(data):
                if d is None:
                    d = data[offset]
                if d is not None and data[offset] != d:
                    return d, offset, 0
                offset += 1
            return d, 0, max(offset - 1, 0)

        @staticmethod
        def find_end_frame(data, pattern):
            n = len(data)
            m = len(pattern)
            for i in range(n - m + 1):
                found = True
                for j in range(m):
                    if data[i + j] != pattern[j]:
                        found = False
                        break
                if found:
                    return i
            return None

        def data(self, in_data):
            out_data = None
            idx = 0
            s = Processor.Status.CONTINUE
            while s == Processor.Status.CONTINUE:
                if self._state == self.State.INIT:
                    v, count, _ = self.count_leading(in_data[idx:])
                    if count <= 0:
                        # Not enough data
                        break

                    if count >= X2D.MIN_LEADING_ZEROS and v == 0:
                        self.info(f"Leading \"0\" at offset {self._offset + idx} of size {count}")
                        idx += count
                        self._state = self.State.LEAD_1
                    else:
                        self.info(f"Ignoring data at offset {self._offset + idx} of size {count}")
                        idx += count
                        self._state = self.State.INIT
                        break
                elif self._state == self.State.LEAD_1:
                    v, count, _ = self.count_leading(in_data[idx:])
                    if count <= 0:
                        # Not enough data
                        break

                    if count < X2D.MIN_LEADING_ONES or v != 1:
                        self.error(f"Invalid lead 1 size: {count}")
                        s = Processor.Status.RESET
                        self._state = self.State.INIT
                        break

                    # Update state
                    self.info(f"Leading \"1\" at offset {self._offset + idx} of size {count}")
                    idx += count
                    self._state = self.State.EXTRA_0
                elif self._state == self.State.EXTRA_0:
                    if idx + X2D.EXTRA_0_LENGTH > len(in_data):
                        # Not enough data
                        break

                    # Update state
                    self.info(f"Extra 0 at offset {self._offset + idx}")
                    idx += X2D.EXTRA_0_LENGTH
                    self._state = self.State.DATA
                elif self._state == self.State.DATA:
                    end = self.find_end_frame(in_data[idx:], X2D.END_OF_FRAME)
                    if end is None:
                        # Not enough data or frame too long?
                        if idx + X2D.MAX_BIT_LENGTH <= len(in_data):
                            self.info(f"Missing end of frame data after offset {self._offset + idx}")
                            s = Processor.Status.RESET
                            self._state = self.State.INIT
                        break

                    # Extract data
                    part = in_data[idx:idx + end]
                    frame_bitstream = self.strip_0_after_successive_1(part)
                    length, data, f = Bitstream.Decoder(False).process(frame_bitstream)
                    if length != len(frame_bitstream) or f != Processor.Status.CONTINUE:
                        self.info(f"Invalid data at offset {self._offset + idx}")
                        s = Processor.Status.RESET
                        self._state = self.State.INIT
                        break
                    out_data = _set_or_extend(out_data, [data])

                    # Update state
                    self.info(f"Data offset {self._offset + idx} of size {end}")
                    idx += end
                    self.info(f"End of frame offset {self._offset + idx} of size {len(X2D.END_OF_FRAME)}")
                    idx += len(X2D.END_OF_FRAME)
                    self._state = self.State.TRAILING
                elif self._state == self.State.TRAILING:
                    if idx + X2D.TRAILING_LENGTH > len(in_data):
                        # Not enough data
                        break

                    # Update state
                    self.info(f"Trailing at offset {self._offset + idx} of size {X2D.TRAILING_LENGTH}")
                    idx += X2D.TRAILING_LENGTH
                    self._state = self.State.DATA
            return idx, out_data, s


class X2DMessage(object):
    class Decoder(Processor):
        def data(self, in_data):
            from X2D import parse_x2d_message
            out_data = []
            idx = 0
            while idx < len(in_data):
                m = in_data[idx]
                out_data.append(parse_x2d_message(m))
                idx += 1
            return idx, out_data, Processor.Status.CONTINUE

    class Encoder(Processor):
        def data(self, in_data):
            from X2D import format_x2d_message
            out_data = []
            idx = 0
            while idx < len(in_data):
                m = in_data[idx]
                out_data.append(format_x2d_message(m))
                idx += 1
            return idx, out_data, Processor.Status.CONTINUE


def process(processors, in_data, count_fct=None):
    if len(processors) == 0 or in_data is None:
        return in_data
    idx = 0
    out_data = None
    while idx < len(in_data):
        length = len(in_data) if count_fct is None else count_fct(in_data)
        data = in_data[idx:idx + length]

        length, data, status = processors[0].process(data)
        idx += length

        data = process(processors[1:], data)

        out_data = _set_or_extend(out_data, data)

        if status == Processor.Status.RESET:
            for p in processors:
                p.reset()
    return out_data
