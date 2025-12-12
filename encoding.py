from __future__ import annotations

import sys

from enum import Enum
from itertools import repeat
from typing import Optional, MutableSequence, Sequence, TypeVar, Tuple, Callable, List, Iterator

T = TypeVar("T")
U = TypeVar("U")


def _to_mutable(seq: Sequence[T], copy: bool = True) -> MutableSequence[T]:
    """
    Convert a sequence to a mutable sequence type.

    Args:
        seq: Input sequence to convert (can be bytes, bytearray, or other sequence)
        copy: If True, creates a copy of the input; if False, returns the original when possible

    Returns:
        MutableSequence: bytearray for bytes input, bytearray for bytearray input,
        or list for other sequence types
    """
    # 1. Handle bytes input by converting to bytearray
    if isinstance(seq, bytes):
        return bytearray(seq)

    # 2. Handle bytearray input - return copy or original based on copy parameter
    if isinstance(seq, bytearray):
        return bytearray(seq) if copy else seq

    # 3. Handle all other sequence types by converting to list
    return list(seq)


def _set_or_extend(current: MutableSequence[T] | None,
                   to_add: Sequence[T] | None, copy: bool = True) -> MutableSequence[T] | None:
    """
    Set or extend a mutable sequence with additional data.

    Args:
        current: Current mutable sequence or None (if None, will be initialized)
        to_add: Sequence to add to current sequence or None
        copy: If True, creates copies when initializing new sequences

    Returns:
        MutableSequence or None: Updated sequence or None if both inputs are None
    """
    # 1. If current is None, initialize it with to_add data
    if current is None:
        current = _to_mutable(to_add, copy) if to_add is not None else None
    elif to_add is not None:
        # 2. If current exists and to_add is provided, extend current with to_add
        current.extend(to_add)

    # 3. Return the (possibly modified) current sequence
    return current


#
# Processors
#

class Processor(object):
    """
    Base processor class for handling sequential data processing with buffering capabilities.
    Provides framework for processing data in chunks while maintaining state between calls.
    """

    class Status(Enum):
        """
        Enumeration of processor status values indicating processing state.
        """
        CONTINUE = 0xAABBCCDD  # Continue processing normally
        OUT_OF_DATA = 0xBADCAFE  # No more data available
        RESET = 0xDEADBEEF  # Reset processing state

    def __init__(self, throw=True, verbose=False):
        """
        Initialize the processor with configuration options.

        Args:
            throw: If True, raises exceptions on errors; if False, prints errors and continues
            verbose: If True, enables verbose logging output
        """
        # 1. Store configuration parameters
        self._throw = throw
        self._verbose = verbose

        # 2. Initialize processing state variables
        self._offset = 0
        self._buffered_data = None
        self.reset()

    def info(self, info):
        """
        Log informational message if verbose mode is enabled.

        Args:
            info: Informational message to log
        """
        if self._verbose:
            print(f"{self} - {info}")

    def error(self, error):
        """
        Handle error conditions based on throw configuration.

        Args:
            error: Error message to handle
        """
        # 1. If throw is enabled, raise exception; otherwise print to stderr
        if self._throw:
            raise Exception(error)
        else:
            print(f"Process error: {error}", file=sys.stderr)

    def process(self, value: Sequence[T] | None) -> Tuple[int, Optional[Sequence[U]], Processor.Status]:
        """
        Process incoming data and return results with status information.

        Args:
            value: Input data sequence to process or None to signal end of data

        Returns:
            Tuple containing:
            - int: Number of input items consumed
            - Optional[Sequence[U]]: Processed output data
            - Processor.Status: Current processing status
        """
        # 1. Handle None input by returning OUT_OF_DATA status
        if value is None:
            return 0, None, Processor.Status.OUT_OF_DATA

        # 2. Combine buffered data with new input data
        in_data = _set_or_extend(self._buffered_data, value, True)

        # 3. Process the combined data
        consumed, out_data, s = self.data(in_data)

        # 4. Update offset counter with consumed data
        self._offset += consumed

        # 5. Handle reset status - if not reset, update buffered data for next iteration
        if s != Processor.Status.RESET:
            self._buffered_data = in_data[consumed:]
            consumed = len(value)

        return consumed, out_data, s

    def data(self, in_data: Sequence[T]) -> Tuple[int, Optional[Sequence[U]], Processor.Status]:
        """
        Abstract method to be implemented by subclasses for actual data processing.

        Args:
            in_data: Input data sequence to process

        Returns:
            Tuple containing:
            - int: Number of input items consumed
            - Optional[Sequence[U]]: Processed output data
            - Processor.Status: Processing status
        """
        raise NotImplementedError()

    def reset(self):
        """
        Reset processor state and clear any buffered data.
        """
        # 1. If there's buffered data, add its length to offset and clear buffer
        if self._buffered_data is not None:
            self._offset += len(self._buffered_data)
            self._buffered_data = None


class Bitstream(object):
    """
    Bitstream class that handles bit-level encoding and decoding operations.
    Provides functionality for converting between byte sequences and bit streams,
    supporting both big-endian and little-endian bit ordering.
    """

    class Decoder(Processor):
        """
        Decoder class for converting bit streams back to byte sequences.
        Handles bit reordering based on endianness configuration.
        """

        def __init__(self, be=True, *args, **kwargs):
            """
            Initialize the bitstream decoder.

            Args:
                be: If True, uses big-endian bit ordering; if False, uses little-endian
                *args: Additional arguments passed to parent Processor class
                **kwargs: Additional keyword arguments passed to parent Processor class
            """
            super().__init__(*args, **kwargs)
            self._be = be

        def data(self, in_data: bytes) -> Tuple[int, Optional[bytes], Processor.Status]:
            """
            Process incoming bit stream data and convert to byte sequences.

            Args:
                in_data: Input bytes representing bit stream data

            Returns:
                Tuple containing:
                - int: Number of input bytes consumed
                - Optional[bytes]: Processed output bytes
                - Processor.Status: Processing status
            """
            # 1. Initialize output data and bit accumulator
            out_data = None
            b = 0
            idx = 0

            # 2. Process input data in 8-byte chunks
            while idx < int(len(in_data) / 8) * 8:
                # 3. Extract least significant bit from current byte
                d = in_data[idx] & 0x1

                # 4. Shift and insert bit based on endianness
                if self._be:
                    # Big-endian: shift left and insert bit at LSB
                    b = b << 1 | d << 0
                else:
                    # Little-endian: shift right and insert bit at MSB
                    b = b >> 1 | d << 7

                idx += 1

                # 5. When 8 bits are processed, add to output and reset accumulator
                if idx % 8 == 0:
                    out_data = _set_or_extend(out_data, bytearray([b]))
                    b = 0

            return idx, out_data, Processor.Status.CONTINUE

    class Encoder(Processor):
        """
        Encoder class for converting byte sequences to bit streams.
        Handles bit reordering based on endianness configuration.
        """

        def __init__(self, be=True, *args, **kwargs):
            """
            Initialize the bitstream encoder.

            Args:
                be: If True, uses big-endian bit ordering; if False, uses little-endian
                *args: Additional arguments passed to parent Processor class
                **kwargs: Additional keyword arguments passed to parent Processor class
            """
            super().__init__(*args, **kwargs)
            self._be = be

        def data(self, in_data: bytes) -> Tuple[int, Optional[bytes], Processor.Status]:
            """
            Process incoming byte data and convert to bit stream.

            Args:
                in_data: Input bytes to encode as bit stream

            Returns:
                Tuple containing:
                - int: Number of input bytes consumed
                - Optional[bytes]: Processed output bit stream
                - Processor.Status: Processing status
            """
            # 1. Initialize output data
            out_data = None
            idx = 0

            # 2. Process each byte in input data
            while idx < len(in_data):
                d = in_data[idx]
                b = bytearray()

                # 3. Extract each bit and append to output based on endianness
                for c in range(8):
                    if self._be:
                        # Big-endian: extract bits from MSB to LSB
                        bit = (d >> (7 - c)) & 0x1
                    else:
                        # Little-endian: extract bits from LSB to MSB
                        bit = (d >> c) & 0x1
                    b.append(bit)

                # 4. Extend output with processed bits
                out_data = _set_or_extend(out_data, b)
                idx += 1

            return idx, out_data, Processor.Status.CONTINUE


class OOK(object):
    """
    On-Off Keying (OOK) modulation processor for digital communication.
    Handles encoding and decoding of OOK signals with pulse width detection.
    """

    class Decoder(Processor):
        """
        Decoder class for OOK signal decoding.
        Converts pulse width modulated signals back to original bit sequences.
        """
        UNDEFINED = 2

        def __init__(self, sample_rate, symbol_rate, error=0.3, *args, **kwargs):
            """
            Initialize the OOK decoder.

            Args:
                sample_rate: Sampling rate of the input signal
                symbol_rate: Rate at which symbols are transmitted
                error: Error tolerance for pulse width detection (as fraction)
                *args: Additional arguments passed to parent Processor class
                **kwargs: Additional keyword arguments passed to parent Processor class
            """
            super().__init__(*args, **kwargs)
            # 1. Calculate threshold based on sample rate and symbol rate
            self._threshold = sample_rate / symbol_rate
            # 2. Calculate error threshold for pulse width validation
            self._error_threshold = self._threshold * error
            # 3. Initialize decoder state variables
            self._bit = self.UNDEFINED
            self._count = None

        def reset(self):
            """
            Reset decoder state to initial values.
            """
            super().reset()
            self._bit = self.UNDEFINED
            self._count = None

        def find_pulse_width(self, count):
            """
            Determine pulse width based on threshold and error tolerance.

            Args:
                count: Measured pulse duration

            Returns:
                int: Detected pulse width (number of symbols) or None if invalid
            """
            # 1. Check if pulse width matches expected thresholds (1x, 2x, 3x)
            for i in range(1, 3):
                if (self._threshold - self._error_threshold) * i < count < (
                        self._threshold + self._error_threshold) * i:
                    return i
            return None

        def data(self, in_data: bytes) -> Tuple[int, Optional[bytes], Processor.Status]:
            """
            Process incoming OOK signal data and decode pulse widths to bits.

            Args:
                in_data: Input bytes representing OOK signal

            Returns:
                Tuple containing:
                - int: Number of input bytes consumed
                - Optional[bytes]: Decoded output bytes
                - Processor.Status: Processing status
            """
            # 1. Initialize output data and processing state
            out_data = None
            idx = 0
            s = Processor.Status.CONTINUE

            # 2. Process input data while still in valid state
            while idx < len(in_data) and s == Processor.Status.CONTINUE:
                d = in_data[idx]

                # 3. Handle undefined bit state (first bit)
                if self._bit == self.UNDEFINED:
                    self._bit = d
                    self._count = 1
                    idx += 1
                else:
                    # 4. Find next transition (bit change)
                    try:
                        end = in_data.index(1 - self._bit, idx)
                        # 5. Accumulate count of current pulse
                        self._count += (end - idx)
                        idx += (end - idx)
                        d = self._bit
                        self._bit = self.UNDEFINED

                        # 6. Determine pulse width
                        width = self.find_pulse_width(self._count)

                        if width is None:
                            # 7. Invalid pulse width - error condition
                            self.error(f"Invalid pulse \"{d}\" " +
                                       f"at offset {self._offset + idx - self._count} " +
                                       f"of size {self._count}")
                            s = Processor.Status.RESET
                        else:
                            # 8. Valid pulse width - add to output
                            self.info(
                                f"Pulse \"{d}\" at offset {self._offset + idx - self._count} of size {self._count}")
                            out_data = _set_or_extend(out_data, bytearray(repeat(d, width)))
                    except ValueError:
                        # 9. No more transitions found - end of data
                        s = Processor.Status.OUT_OF_DATA

            return idx, out_data, s

    class Encoder(Processor):
        """
        Encoder class for OOK signal encoding.
        Converts bit sequences to pulse width modulated signals.
        """

        def __init__(self, sample_rate, symbol_rate, *args, **kwargs):
            """
            Initialize the OOK encoder.

            Args:
                sample_rate: Sampling rate of the output signal
                symbol_rate: Rate at which symbols are transmitted
                *args: Additional arguments passed to parent Processor class
                **kwargs: Additional keyword arguments passed to parent Processor class
            """
            super().__init__(*args, **kwargs)
            # 1. Calculate threshold based on sample rate and symbol rate
            self._threshold = sample_rate / symbol_rate

        def data(self, in_data: bytes) -> Tuple[int, Optional[bytes], Processor.Status]:
            """
            Process incoming bit data and encode as OOK pulse width modulated signal.

            Args:
                in_data: Input bytes representing bit sequence to encode

            Returns:
                Tuple containing:
                - int: Number of input bytes consumed
                - Optional[bytes]: Encoded output signal
                - Processor.Status: Processing status
            """
            # 1. Initialize output data
            out_data = None
            idx = 0

            # 2. Process each bit in input data
            while idx < len(in_data):
                d = in_data[idx]
                # 3. Repeat each bit for the threshold duration
                out_data = _set_or_extend(out_data, bytearray(repeat(d, self._threshold)))
                idx += 1

            return idx, out_data, Processor.Status.CONTINUE


class Manchester(object):
    """
    Manchester encoding/decoding processor for digital communication.
    Implements Manchester encoding where each bit is represented by a transition
    at the center of the bit period.

    Encoding:
    - 0 bit: 01 transition (low to high)
    - 1 bit: 10 transition (high to low)

          _   __   _    _...
     ..._| |_|  |_| |__|
         0   0  1   1  0

    """

    # Manchester encoding patterns
    zero_pulse = bytes([0, 1])  # Low-to-high transition for 0 bit
    one_pulse = bytes([1, 0])  # High-to-low transition for 1 bit

    class Encoder(Processor):
        """
        Encoder class for Manchester encoding.
        Converts bit sequences to Manchester encoded bit streams.
        """

        def __init__(self, initial: bytes, *args, **kwargs):
            """
            Initialize the Manchester encoder.

            Args:
                initial: Initial bytes to prepend to output
                *args: Additional arguments passed to parent Processor class
                **kwargs: Additional keyword arguments passed to parent Processor class
            """
            super().__init__(*args, **kwargs)
            self._initial = initial or bytes()
            self._initialized = False

        def reset(self):
            """
            Reset encoder state to initial values.
            """
            super().reset()
            self._initialized = False

        def data(self, in_data: bytes) -> Tuple[int, Optional[bytes], Processor.Status]:
            """
            Process incoming bit data and encode as Manchester bit stream.

            Args:
                in_data: Input bytes representing bit sequence to encode

            Returns:
                Tuple containing:
                - int: Number of input bytes consumed
                - Optional[bytes]: Encoded Manchester bit stream
                - Processor.Status: Processing status
            """
            # 1. Initialize output data
            out_data = None

            # 2. Handle initial bytes (only once)
            if not self._initialized:
                self._initialized = True
                out_data = _to_mutable(self._initial, True)

            # 3. Initialize processing state
            s = Processor.Status.CONTINUE
            idx = 0

            # 4. Process each bit in input data
            while idx < len(in_data):
                d = in_data[idx]
                if d == 0:
                    # 5. Encode 0 bit as 01 transition
                    out_data = _set_or_extend(out_data, Manchester.zero_pulse)
                elif d == 1:
                    # 6. Encode 1 bit as 10 transition
                    out_data = _set_or_extend(out_data, Manchester.one_pulse)
                else:
                    # 7. Invalid bit value - error condition
                    self.error(f"Invalid value: {d} at offset {self._offset + idx}")
                    s = Processor.Status.RESET
                idx += 1

            return idx, out_data, s

    class Decoder(Processor):
        """
        Decoder class for Manchester decoding.
        Converts Manchester encoded bit streams back to original bit sequences.
        """

        def data(self, in_data: bytes) -> Tuple[int, Optional[bytes], Processor.Status]:
            """
            Process incoming Manchester bit stream and decode to original bits.

            Args:
                in_data: Input bytes representing Manchester encoded bit stream

            Returns:
                Tuple containing:
                - int: Number of input bytes consumed
                - Optional[bytes]: Decoded original bit sequence
                - Processor.Status: Processing status
            """
            # 1. Initialize output data and processing state
            out_data = None
            s = Processor.Status.CONTINUE
            idx = 0

            # 2. Process input data in pairs (2 bytes per bit)
            while idx + 2 <= len(in_data) and s == Processor.Status.CONTINUE:
                part = in_data[idx:idx + 2]
                if part == Manchester.zero_pulse:
                    # 3. Decode 01 transition as 0 bit
                    out_data = _set_or_extend(out_data, bytearray([0]))
                elif part == Manchester.one_pulse:
                    # 4. Decode 10 transition as 1 bit
                    out_data = _set_or_extend(out_data, bytearray([1]))
                else:
                    # 5. Invalid Manchester pattern - error condition
                    self.error(f"Invalid value \"{part}\" at offset {self._offset + idx}")
                    s = Processor.Status.RESET
                idx += 2

            return idx, out_data, s


class BiphaseMark(object):
    """
    Biphase Mark encoding/decoding processor for digital communication.
    Implements Biphase Mark encoding where each bit is represented by transitions
    at both the beginning and center of the bit period.

    Encoding:
    - 0 bit: 00 or 11 transition (no transition at bit center)
    - 1 bit: 01 or 10 transition (transition at bit center)

         __    _   __   _
     ...|  |__| |_|  |_| |_...
        0  0  1   0  1

    """

    # Biphase Mark encoding patterns
    zero_pulses = [bytes([0, 0]), bytes([1, 1])]  # No transition at bit center
    one_pulses = [bytes([0, 1]), bytes([1, 0])]  # Transition at bit center

    class Encoder(Processor):
        """
        Encoder class for Biphase Mark encoding.
        Converts bit sequences to Biphase Mark encoded bit streams.
        """

        def __init__(self, *args, **kwargs):
            """
            Initialize the Biphase Mark encoder.

            Args:
                *args: Additional arguments passed to parent Processor class
                **kwargs: Additional keyword arguments passed to parent Processor class
            """
            super().__init__(*args, **kwargs)
            self._flip = 1

        def reset(self):
            """
            Reset encoder state to initial values.
            """
            super().reset()
            self._flip = 1

        def data(self, in_data: bytes) -> Tuple[int, Optional[bytes], Processor.Status]:
            """
            Process incoming bit data and encode as Biphase Mark bit stream.

            Args:
                in_data: Input bytes representing bit sequence to encode

            Returns:
                Tuple containing:
                - int: Number of input bytes consumed
                - Optional[bytes]: Encoded Biphase Mark bit stream
                - Processor.Status: Processing status
            """
            # 1. Initialize output data and processing state
            out_data = None
            idx = 0
            s = Processor.Status.CONTINUE

            # 2. Process each bit in input data
            while idx < len(in_data) and s == Processor.Status.CONTINUE:
                d = in_data[idx]
                if d == 0:
                    # 3. Encode 0 bit using current flip state
                    out_data = _set_or_extend(out_data, BiphaseMark.zero_pulses[1 - self._flip])
                    self._flip = out_data[-1]  # Update flip state based on last bit
                elif d == 1:
                    # 4. Encode 1 bit using current flip state
                    out_data = _set_or_extend(out_data, BiphaseMark.one_pulses[1 - self._flip])
                    self._flip = out_data[-1]  # Update flip state based on last bit
                else:
                    # 5. Invalid bit value - error condition
                    self.error(f"Invalid value \"{d}\" at offset {self._offset + idx}")
                    s = Processor.Status.RESET
                idx += 1

            return idx, out_data, s

    class Decoder(Processor):
        """
        Decoder class for Biphase Mark decoding.
        Converts Biphase Mark encoded bit streams back to original bit sequences.
        """

        def data(self, in_data: bytes) -> Tuple[int, Optional[bytes], Processor.Status]:
            """
            Process incoming Biphase Mark bit stream and decode to original bits.

            Args:
                in_data: Input bytes representing Biphase Mark encoded bit stream

            Returns:
                Tuple containing:
                - int: Number of input bytes consumed
                - Optional[bytes]: Decoded original bit sequence
                - Processor.Status: Processing status
            """
            # 1. Initialize output data and processing state
            out_data = None
            idx = 0

            # 2. Process input data in pairs (2 bytes per bit)
            while idx + 2 <= len(in_data):
                chunk = in_data[idx:idx + 2]
                if chunk in BiphaseMark.zero_pulses:
                    # 3. Decode 00 or 11 pattern as 0 bit
                    out_data = _set_or_extend(out_data, bytearray([0]))
                elif chunk in BiphaseMark.one_pulses:
                    # 4. Decode 01 or 10 pattern as 1 bit
                    out_data = _set_or_extend(out_data, bytearray([1]))

                self.info(f"Detect \"{out_data[-1]}\" at offset {self._offset + idx}")
                idx += 2

            return idx, out_data, Processor.Status.CONTINUE


class Packetizer(object):
    """
    Packetizer for extracting packets from bitstream data.
    Detects silent periods and extracts packets based on preamble and syncword patterns.
    """

    class Decoder(Processor):
        """
        Decoder class for packet extraction from bitstream.
        """
        UNDEFINED = 2

        def __init__(self, sample_rate, symbol_rate, silent_length=10, preamble=None, syncword=None, *args, **kwargs):
            """
            Initialize the Packetizer decoder.

            Args:
                sample_rate: Sampling rate of the input data
                symbol_rate: Rate at which symbols are transmitted
                silent_length: Length of silent period to detect (in symbol periods)
                preamble: Expected preamble pattern for packet detection
                syncword: Expected syncword pattern for packet detection
                *args: Additional arguments passed to parent Processor class
                **kwargs: Additional keyword arguments passed to parent Processor class
            """
            super().__init__(*args, **kwargs)
            # Number of samples representing one symbol
            self._bit_count = sample_rate // symbol_rate
            # Number of samples for silent period (silent_length * _bit_count)
            self._silent_count = int(self._bit_count * silent_length)
            self._preamble = preamble
            self._syncword = syncword

            # Initialize output data and processing state
            self._bit = self.UNDEFINED
            self._current = None
            self._count = None

        def reset(self):
            """
            Reset packetizer state to initial values.
            """
            super().reset()
            self._bit = self.UNDEFINED
            self._current = None
            self._count = None

        def data(self, in_data: bytes) -> Tuple[int, Optional[Sequence[bytes]], Processor.Status]:
            """
            Process incoming bitstream data and extract packets.

            Args:
                in_data: Input bytes representing bitstream data

            Returns:
                Tuple containing:
                - int: Number of input bytes consumed
                - Optional[Sequence[bytes]]: List of extracted packets
                - Processor.Status: Processing status
            """
            # 1. Initialize output data and processing state
            out_data = None
            idx = 0
            s = Processor.Status.CONTINUE

            # 2. Process each byte in input data
            while idx < len(in_data) and s == Processor.Status.CONTINUE:
                d = in_data[idx]

                if self._bit == self.UNDEFINED:
                    # 3. Initialize bit detection
                    self._bit = d
                    self._count = 1
                    idx += 1
                else:
                    # 4. Find next transition (bit change)
                    try:
                        end = in_data.index(1 - self._bit, idx)
                    except ValueError:
                        end = len(in_data)

                    # 5. Accumulate count of consecutive bits
                    self._count += (end - idx)

                    # 6. Check if silent period detected
                    if self._count >= self._silent_count:
                        idx += (end - idx)

                        # 7. Process silent period only if there is data
                        if end != len(in_data) or self._current is not None:
                            d = self._bit
                            self._bit = self.UNDEFINED

                            self.info(
                                f"Silent detected \"{d}\" at offset {self._offset + idx - self._count} of size {self._count}")

                        # 8. Process current packet if exists
                        if self._current is not None:
                            forward = True

                            # 9. Add missing bits to complete byte alignment
                            extend_length = ((((len(self._current) - 1) // 8) + 1) * 8) - len(self._current)
                            _set_or_extend(self._current, bytearray(repeat(d, extend_length)))

                            # 10. Decode bitstream to bytes
                            length, data, f = Bitstream.Decoder(True).process(self._current)

                            # 11. Validate preamble
                            if forward and self._preamble is not None:
                                preamble = data[0:len(self._preamble)]
                                if preamble != self._preamble:
                                    forward = False
                                    self.info(
                                        f"Preamble \"{preamble}\" doesn't match \"{self._preamble}\" exclude the packet")
                                else:
                                    data = data[len(self._preamble):]

                            # 12. Validate syncword
                            if forward and self._syncword is not None:
                                syncword = data[0:len(self._syncword)]
                                if syncword != self._syncword:
                                    forward = False
                                    self.info(
                                        f"Syncword \"{syncword}\" doesn't match \"{self._syncword}\" exclude the packet")
                                else:
                                    data = data[len(self._syncword):]

                            # 13. Add packet to output if valid
                            if forward:
                                self.info(f"New packet")
                                out_data = _set_or_extend(out_data, [data])

                            self._current = None
                    else:
                        # 14. Only handle the data if this not the end of the data
                        if end != len(in_data):
                            idx += (end - idx)
                            d = self._bit
                            self._bit = self.UNDEFINED

                            width = round(self._count / self._bit_count)
                            if width < 1:
                                self.info(
                                    f"Glitch \"{d}\" at offset {self._offset + idx - self._count} of size {self._count}")
                            else:
                                self.info(
                                    f"Pulse \"{d}\" at offset {self._offset + idx - self._count} of size {self._count} ({width} symbols)")
                                self._current = _set_or_extend(self._current, bytearray(repeat(d, width)))
                        else:
                            # 15. Keep data for next processing round
                            self._count -= (end - idx)
                            s = Processor.Status.OUT_OF_DATA

            return idx, out_data, s

    class Encoder(Processor):
        """
        Encoder class for packet assembly into bitstream.
        """

        def __init__(self, sample_rate, symbol_rate, silent_length=10, preamble=None, syncword=None, *args, **kwargs):
            """
            Initialize the Packetizer encoder.

            Args:
                sample_rate: Sampling rate of the output data
                symbol_rate: Rate at which symbols are transmitted
                silent_length: Length of silent period to insert (in symbol periods)
                preamble: Preamble pattern to insert
                syncword: Syncword pattern to insert
                *args: Additional arguments passed to parent Processor class
                **kwargs: Additional keyword arguments passed to parent Processor class
            """
            super().__init__(*args, **kwargs)
            # Number of samples representing one symbol
            self._bit_count = sample_rate // symbol_rate
            # Number of samples for silent period (silent_length * _bit_count)
            self._silent_count = int(self._bit_count * silent_length)
            self._preamble = preamble
            self._syncword = syncword

        @classmethod
        def repeat_bits(cls, out_data: Optional[bytearray], data: Sequence[int], count: int) -> bytearray:
            """
            Repeat each bit a specified number of times.

            Args:
                out_data: Output data to append to
                data: Data to repeat
                count: Number of times to repeat

            Returns:
                Updated output data
            """
            for b in data:
                out_data = _set_or_extend(out_data, bytearray(repeat(b, count)))
            return out_data

        def data(self, in_data: Sequence[bytes]) -> Tuple[int, Optional[bytes], Processor.Status]:
            """
            Process input packets and encode them into bitstream.

            Args:
                in_data: Input packets (sequence of bytes)

            Returns:
                Tuple containing:
                - int: Number of input packets consumed
                - Optional[bytes]: Encoded bitstream data
                - Processor.Status: Processing status
            """
            # 1. Initialize output data
            out_data = None
            idx = 0

            # 2. Process each packet in input data
            while idx < len(in_data):
                d = in_data[idx]

                # 3. Encode packet to bitstream (array of bytes to array of bits)
                length, data, f = Bitstream.Encoder(True).process(d)

                # 4. Insert silent period before packet
                out_data = self.repeat_bits(out_data, [0], self._silent_count)

                # 5. Insert preamble
                if self._preamble is not None:
                    preamble_length, preamble_data, preamble_f = Bitstream.Encoder(True).process(self._preamble)
                    out_data = self.repeat_bits(out_data, preamble_data, self._bit_count)

                # 6. Insert syncword
                if self._syncword is not None:
                    syncword_length, syncword_data, syncword_f = Bitstream.Encoder(True).process(self._syncword)
                    out_data = self.repeat_bits(out_data, syncword_data, self._bit_count)

                # 7. Insert packet data
                out_data = self.repeat_bits(out_data, data, self._bit_count)

                # 8. Insert silent period after packet
                out_data = self.repeat_bits(out_data, [0], self._silent_count)

                idx += 1

            return idx, out_data, Processor.Status.CONTINUE


class CcittWhitening(object):
    @classmethod
    def reverse_bits(cls, x):
        """
        Reverse the bit order of an 8-bit value.

        This function performs bit reversal using bitwise operations:
        - First swaps nibbles (4-bit groups)
        - Then swaps 2-bit groups
        - Finally swaps individual bits

        Args:
            x: 8-bit integer to reverse

        Returns:
            Integer with reversed bit order
        """
        x = ((x & 0xF0) >> 4) | ((x & 0x0F) << 4)
        x = ((x & 0xCC) >> 2) | ((x & 0x33) << 2)
        x = ((x & 0xAA) >> 1) | ((x & 0x55) << 1)
        return x

    @classmethod
    def ccitt_whitening_cstyle(cls, data: bytes):
        """
        Apply CCITT whitening algorithm to input data.

        CCITT whitening is a pseudo-random bit sequence generator used for
        data scrambling. This implementation follows the CCITT standard
        (ITU-T Recommendation V.42) for error detection and prevention.

        Args:
            data: Input bytes to be whitened

        Returns:
            Bytes with CCITT whitening applied
        """
        key_msb = 0x01
        key_lsb = 0xFF
        out = _to_mutable(data)

        for i in range(len(out)):
            whitening_byte = cls.reverse_bits(key_lsb)
            out[i] ^= whitening_byte

            for _ in range(8):
                key_msb_prev = key_msb
                key_msb = (key_lsb & 0x01) ^ ((key_lsb >> 5) & 1)
                key_lsb = ((key_msb_prev << 7) & 0x80) | ((key_lsb >> 1) & 0xFF)

        return bytes(out)

    class DecoderEncoder(Processor):
        """
        CCITT Whitening Decoder/Encoder processor.

        This processor applies the CCITT whitening algorithm to decode
        whitened data back to its original form. Since the CCITT whitening
        algorithm is its own inverse, the same function is used for both
        encoding and decoding.
        """

        def data(self, in_data: Sequence[bytes]) -> Tuple[int, Optional[Sequence[bytes]], Processor.Status]:
            """
            Process input data through CCITT whitening decoder/encoder.

            Args:
                in_data: Sequence of bytes to decode/encode

            Returns:
                Tuple of (number_of_items_processed, decoded_data, processing_status)
            """
            out_data = []
            idx = 0
            while idx < len(in_data):
                m = in_data[idx]
                out_data.append(CcittWhitening.ccitt_whitening_cstyle(m))
                idx += 1
            return idx, out_data, Processor.Status.CONTINUE

    Decoder = DecoderEncoder
    Encoder = DecoderEncoder


class Duplicator(object):
    class Decoder(Processor):
        """
        Duplicator Decoder processor.

        This processor removes consecutive duplicate values from the input stream.
        It maintains a history of the previously processed value and only passes
        through values that are different from the previous one.
        """

        def __init__(self):
            super().__init__()
            self._previous = None

        def reset(self):
            """
            Reset the decoder state.

            Clears the previous value tracking, allowing the decoder to start
            fresh processing from the next input.
            """
            self._previous = None

        def data(self, in_data: Sequence[T]) -> Sequence[T]:
            """
            Process input data through the duplicator decoder.

            Args:
                in_data: Sequence of input values to process

            Returns:
                Tuple of (number_of_items_processed, filtered_data, processing_status)
            """
            out_data = []
            idx = 0
            while idx < len(in_data):
                m = in_data[idx]
                if m != self._previous:
                    out_data.append(m)
                self._previous = m
                idx += 1
            return idx, out_data, Processor.Status.CONTINUE

    class Encoder(Processor):
        """
        Duplicator Encoder processor.

        This processor duplicates each input value a specified number of times.
        """

        def __init__(self, duplicate_count=5):
            """
            Initialize the Duplicator encoder.

            Args:
                duplicate_count: Number of times each input value should be duplicated
            """
            super().__init__()
            self._duplicate_count = duplicate_count

        def data(self, in_data: Sequence[T]) -> Sequence[T]:
            """
            Process input data through the duplicator encoder.

            Args:
                in_data: Sequence of input values to process

            Returns:
                Tuple of (number_of_items_processed, duplicated_data, processing_status)
            """
            out_data = []
            idx = 0
            while idx < len(in_data):
                m = in_data[idx]
                out_data = _set_or_extend(out_data, list(repeat(self._duplicate_count, m)))
                idx += 1
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
        """
        X2D Encoder processor.

        Encodes input data using the X2D scheme with preamble, frame separation,
        and bitstream encoding.

        Args:
            preamble_0_count: Number of leading zeros in preamble
            preamble_1_count: Number of leading ones in preamble
            separator: Frame separator pattern (defaults to [1,1,1,1,1,1,0])
        """

        def __init__(self, preamble_0_count=9, preamble_1_count=6, separator=None, *args, **kwargs):
            super().__init__(*args, **kwargs)
            self._preamble_0_count = preamble_0_count
            self._preamble_1_count = preamble_1_count
            self._separator = separator or [1, 1, 1, 1, 1, 1, 0]
            self._is_initialized = False

        def reset(self):
            """
            Reset the encoder state.

            Clears the initialization flag to allow re-initialization of preamble.
            """
            super().reset()
            self._is_initialized = False

        @staticmethod
        def insert_0_after_successive_1(data: bytes) -> bytes:
            """
            Insert a zero after every MAX_SUCCESSIVE_ONES consecutive ones.

            Args:
                data: Input byte sequence

            Returns:
                Modified byte sequence with zeros inserted after successive ones
            """
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

        def data(self, in_data: Sequence[bytes]) -> Tuple[int, Optional[bytes], Processor.Status]:
            """
            Process input data through X2D encoder.

            Args:
                in_data: Sequence of input bytes to encode

            Returns:
                Tuple of (number_of_items_processed, encoded_data, processing_status)
            """
            out_data = None
            s = Processor.Status.CONTINUE
            idx = 0

            if not self._is_initialized:
                out_data = _set_or_extend(out_data, bytearray(repeat(0, self._preamble_0_count)))
                out_data = _set_or_extend(out_data, bytearray(repeat(1, self._preamble_1_count)))
                out_data = _set_or_extend(out_data, [0])  # Extra zero
                self._is_initialized = True

            while idx < len(in_data):
                # Process bitstream encoding
                length, data, f = Bitstream.Encoder(False).process(in_data[idx])

                # Validate bitstream processing result
                if length != len(in_data[idx]) or data is None or f != Processor.Status.CONTINUE:
                    self.info(f"Invalid data at offset {self._offset + idx}")
                    s = Processor.Status.RESET
                    break

                # Apply X2D encoding rules
                out_data = _set_or_extend(out_data, self.insert_0_after_successive_1(data))

                # Add frame separator
                out_data = _set_or_extend(out_data, X2D.END_OF_FRAME)
                out_data = _set_or_extend(out_data, self._separator)
                idx += 1
            return idx, out_data, s

    class Decoder(Processor):
        """
        X2D Decoder processor.

        Decodes X2D encoded data by recognizing preamble patterns, frame boundaries,
        and bitstream data.
        """

        class State(Enum):
            INIT = 0
            LEAD_1 = 1
            EXTRA_0 = 2
            DATA = 3
            TRAILING = 4

        def __init__(self, *args, **kwargs):
            """
            Initialize X2D decoder.
            """
            super().__init__(*args, **kwargs)
            self._state = self.State.INIT

        def reset(self):
            """
            Reset the decoder state.

            Resets to initial state for processing new data.
            """
            super().reset()
            self._state = self.State.INIT

        def strip_0_after_successive_1(self, data: bytes) -> bytes:
            """
            Remove zeros that were inserted after MAX_SUCCESSIVE_ONES consecutive ones.

            Args:
                data: Input byte sequence

            Returns:
                Modified byte sequence with inserted zeros removed
            """
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
        def count_leading(data: bytes) -> Tuple[int, int, int]:
            """
            Count leading identical elements in data.

            Args:
                data: Input byte sequence

            Returns:
                Tuple of (value, count, offset)
            """
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
            """
            Find the end frame pattern in data.

            Args:
                data: Input byte sequence
                pattern: Pattern to search for

            Returns:
                Index of pattern start or None if not found
            """
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

        def data(self, in_data: bytes) -> Tuple[int, Optional[Sequence[bytes]], Processor.Status]:
            """
            Process input data through X2D decoder.

            Args:
                in_data: Input byte sequence to decode

            Returns:
                Tuple of (number_of_items_processed, decoded_data, processing_status)

            State Machine Flow:
                INIT -> LEAD_0 -> LEAD_1 -> EXTRA_0 -> DATA -> TRAILING -> INIT
                - INIT: Look for leading zeros to start frame detection
                - LEAD_1: Look for leading ones to validate frame start
                - EXTRA_0: Consume extra zero bit after leading ones
                - DATA: Extract and decode frame data
                - TRAILING: Handle trailing separator bits
            """
            out_data = None
            idx = 0
            s = Processor.Status.CONTINUE

            while s == Processor.Status.CONTINUE:
                if self._state == self.State.INIT:
                    # Look for leading zeros to identify frame start
                    v, count, _ = self.count_leading(in_data[idx:])
                    if count <= 0:
                        # Not enough data available to process
                        break

                    if count >= X2D.MIN_LEADING_ZEROS and v == 0:
                        # Found valid leading zeros, advance index and transition to LEAD_1 state
                        self.info(f"Leading \"0\" at offset {self._offset + idx} of size {count}")
                        idx += count
                        self._state = self.State.LEAD_1
                    else:
                        # Invalid leading pattern, skip and return to INIT state
                        self.info(f"Ignoring data at offset {self._offset + idx} of size {count}")
                        idx += count
                        self._state = self.State.INIT

                elif self._state == self.State.LEAD_1:
                    # Look for leading ones to validate frame start
                    v, count, _ = self.count_leading(in_data[idx:])
                    if count <= 0:
                        # Not enough data available to process
                        break

                    if count < X2D.MIN_LEADING_ONES or v != 1:
                        # Invalid leading ones pattern, reset and return to INIT state
                        self.error(f"Invalid lead 1 size: {count}")
                        s = Processor.Status.RESET
                        self._state = self.State.INIT
                        break

                    # Found valid leading ones, advance index and transition to EXTRA_0 state
                    self.info(f"Leading \"1\" at offset {self._offset + idx} of size {count}")
                    idx += count
                    self._state = self.State.EXTRA_0

                elif self._state == self.State.EXTRA_0:
                    # Consume extra zero bit after leading ones
                    if idx + X2D.EXTRA_0_LENGTH > len(in_data):
                        # Not enough data available for extra zero
                        break

                    # Process extra zero bit and transition to DATA state
                    self.info(f"Extra 0 at offset {self._offset + idx}")
                    idx += X2D.EXTRA_0_LENGTH
                    self._state = self.State.DATA

                elif self._state == self.State.DATA:
                    # Extract and decode frame data
                    end = self.find_end_frame(in_data[idx:], X2D.END_OF_FRAME)
                    if end is None:
                        # End of frame not found, check if we have enough data to determine error
                        if idx + X2D.MAX_BIT_LENGTH <= len(in_data):
                            self.info(f"Missing end of frame data after offset {self._offset + idx}")
                            s = Processor.Status.RESET
                            self._state = self.State.INIT
                        break

                    # Extract frame data up to end of frame marker
                    part = in_data[idx:idx + end]
                    frame_bitstream = self.strip_0_after_successive_1(part)
                    length, data, f = Bitstream.Decoder(False).process(frame_bitstream)
                    if length != len(frame_bitstream) or f != Processor.Status.CONTINUE:
                        # Invalid frame data, reset and return to INIT state
                        self.info(f"Invalid data at offset {self._offset + idx}")
                        s = Processor.Status.RESET
                        self._state = self.State.INIT
                        break
                    out_data = _set_or_extend(out_data, [data])

                    # Process end of frame marker and transition to TRAILING state
                    self.info(f"Data offset {self._offset + idx} of size {end}")
                    idx += end
                    self.info(f"End of frame offset {self._offset + idx} of size {len(X2D.END_OF_FRAME)}")
                    idx += len(X2D.END_OF_FRAME)
                    self._state = self.State.TRAILING

                elif self._state == self.State.TRAILING:
                    # Handle trailing separator bits
                    if idx + X2D.TRAILING_LENGTH > len(in_data):
                        # Not enough data available for trailing bits
                        break

                    # Process trailing bits and return to DATA state for next frame
                    self.info(f"Trailing at offset {self._offset + idx} of size {X2D.TRAILING_LENGTH}")
                    idx += X2D.TRAILING_LENGTH
                    self._state = self.State.DATA

            return idx, out_data, s


class X2DMessage(object):
    class Decoder(Processor):
        """
        Decoder class for parsing X2D message bytes into dictionaries.
        """

        def data(self, in_data: Sequence[bytes]) -> Tuple[int, Optional[Sequence[dict]], Processor.Status]:
            """
            Process incoming byte sequences and convert them to X2D message dictionaries.

            Args:
                in_data: Sequence of bytes representing X2D messages

            Returns:
                Tuple containing:
                - int: Number of processed items
                - Optional[Sequence[dict]]: List of parsed message dictionaries, or None if error
                - Processor.Status: Processing status indicator
            """
            from X2D import parse_x2d_message

            # 1. Initialize output data structure and index counter
            out_data = []
            idx = 0

            # 2. Process each message in the input sequence
            while idx < len(in_data):
                m = in_data[idx]
                try:
                    # 3. Attempt to parse the current message byte sequence
                    out_data.append(parse_x2d_message(m))
                except BaseException as e:
                    # 4. Log error and continue processing remaining messages
                    self.error(f"Can't parse X2D message: {e}")
                idx += 1

            # 5. Return processed count, parsed data, and continue status
            return idx, out_data, Processor.Status.CONTINUE

    class Encoder(Processor):
        """
        Encoder class for converting X2D message dictionaries into byte sequences.
        """

        def data(self, in_data: Sequence[dict]) -> Tuple[int, Optional[Sequence[bytes]], Processor.Status]:
            """
            Process incoming message dictionaries and convert them to X2D message bytes.

            Args:
                in_data: Sequence of dictionaries representing X2D messages

            Returns:
                Tuple containing:
                - int: Number of processed items
                - Optional[Sequence[bytes]]: List of formatted message bytes, or None if error
                - Processor.Status: Processing status indicator
            """
            from X2D import format_x2d_message

            # 1. Initialize output data structure and index counter
            out_data = []
            idx = 0

            # 2. Process each message dictionary in the input sequence
            while idx < len(in_data):
                m = in_data[idx]
                # 3. Format the current message dictionary into bytes
                out_data.append(format_x2d_message(m))
                idx += 1

            # 4. Return processed count, formatted data, and continue status
            return idx, out_data, Processor.Status.CONTINUE


class X3DMessage(object):
    class Decoder(Processor):
        """
        Decoder class for parsing X3D message bytes into dictionaries.
        """

        def data(self, in_data: Sequence[bytes]) -> Tuple[int, Optional[Sequence[dict]], Processor.Status]:
            """
            Process incoming byte sequences and convert them to X3D message dictionaries.

            Args:
                in_data: Sequence of bytes representing X3D messages

            Returns:
                Tuple containing:
                - int: Number of processed items
                - Optional[Sequence[dict]]: List of parsed message dictionaries, or None if error
                - Processor.Status: Processing status indicator
            """
            from X3D import parse_x3d_message

            # 1. Initialize output data structure and index counter
            out_data = []
            idx = 0

            # 2. Process each message in the input sequence
            while idx < len(in_data):
                m = in_data[idx]
                try:
                    # 3. Attempt to parse the current message byte sequence
                    out_data.append(parse_x3d_message(m))
                except BaseException as e:
                    # 4. Log error and continue processing remaining messages
                    self.error(f"Can't parse X3D message: {e}")
                idx += 1

            # 5. Return processed count, parsed data, and continue status
            return idx, out_data, Processor.Status.CONTINUE

    class Encoder(Processor):
        """
        Encoder class for converting X3D message dictionaries into byte sequences.
        """

        def data(self, in_data: Sequence[dict]) -> Tuple[int, Optional[Sequence[bytes]], Processor.Status]:
            """
            Process incoming message dictionaries and convert them to X3D message bytes.

            Args:
                in_data: Sequence of dictionaries representing X3D messages

            Returns:
                Tuple containing:
                - int: Number of processed items
                - Optional[Sequence[bytes]]: List of formatted message bytes, or None if error
                - Processor.Status: Processing status indicator
            """
            from X3D import format_x3d_message

            # 1. Initialize output data structure and index counter
            out_data = []
            idx = 0

            # 2. Process each message dictionary in the input sequence
            while idx < len(in_data):
                m = in_data[idx]
                # 3. Format the current message dictionary into bytes
                out_data.append(format_x3d_message(m))
                idx += 1

            # 4. Return processed count, formatted data, and continue status
            return idx, out_data, Processor.Status.CONTINUE


def process(processors: List[Processor], in_data: Sequence[T],
            count_fct: Optional[Callable[[Sequence[T]], int]] = None) -> Sequence[U]:
    """
    Processes input data through a sequence of processors, applying each processor in order.

    Args:
        processors: List of processor objects to process with.
        in_data: Input data to be processed.
        count_fct: Optional function to determine how much data to process at once.

    Returns:
        Processed output data after all processors have been applied.
    """
    # Early exit for edge cases
    if len(processors) == 0 or in_data is None:
        return in_data

    # Track position and accumulate results
    idx = 0
    out_data = None

    # Process input in batches
    while idx < len(in_data):
        # Determine batch size
        length = len(in_data) if count_fct is None else count_fct(in_data)
        data = in_data[idx:idx + length]

        # Apply first processor and advance index
        length, data, status = processors[0].process(data)
        idx += length

        # Chain remaining processors
        data = process(processors[1:], data)

        # Accumulate results
        out_data = _set_or_extend(out_data, data)

        # Handle reset signals
        if status == Processor.Status.RESET:
            for p in processors:
                p.reset()
    return out_data
