import io

import construct.core
from construct import Subconstruct, Struct, GreedyBytes, Bytes, stream_tell, stream_seek, stream_read, GreedyString
from construct.lib import Container


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
        if length <= 0:
            return None
        data = stream_read(stream, length, path)
        if self.subcon is GreedyBytes:
            return data
        if type(self.subcon) is GreedyString:
            return data.decode(self.subcon.encoding)
        return self.subcon._parsereport(io.BytesIO(data), context, path)

    def _build(self, obj, stream, context, path):
        return self.subcon._build(obj, stream, context, path)
