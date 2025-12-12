import random
from typing import List, Sequence, Tuple

from X3D import MessageType
from encoding import Packetizer, X3DMessage, process, CcittWhitening, Duplicator, Processor


def print_message(name: str, msgs: List[X3DMessage]):
    print("#" * 80 + "\n" + name + "\n" + "#" * 80)
    assert len(msgs) > 0
    for msg in msgs:
        print(msg)
    print("")


def to_hex(data: bytes) -> str:
    return ', '.join('0x{:02x}'.format(x) for x in data)


class Dumper(Processor):
    def __init__(self):
        super().__init__()
        self.last = None

    def data(self, in_data: Sequence[bytes]) -> Tuple[int, Sequence[bytes], Processor.Status]:
        out_data = []
        idx = 0
        while idx < len(in_data):
            data = in_data[idx]
            print(to_hex(data))
            self.last = data
            out_data.append(data)
            idx += 1
        return idx, out_data, Processor.Status.CONTINUE


filename = "raw_x3d.bin"
dumper = None

if filename is not None:
    with open(filename, 'rb') as file:
        dumper = Dumper()
        data = bytes(file.read())
        msgs = process([
            Packetizer.Decoder(40_000 * 10, 40_000, preamble=bytes([0xAA, 0xAA, 0xAA, 0xAA]),
                               syncword=bytes([0x81, 0x69, 0x96, 0x7e]), verbose=False,
                               throw=False),
            CcittWhitening.Decoder(),
            Duplicator.Decoder(),
            dumper,
            X3DMessage.Decoder(verbose=True)
        ], data, lambda x: random.randint(1, min(len(x), 1024)))  # Test the behaviour with irregular packet size
        for m in msgs:
            print(
                f"Seq {m.seq}, type {m.type}, " +
                f"Device ID {m.header.value.device_id}, Network ID {m.header.value.network_id}, Msg ID {m.header.value.msg_id}, " +
                f"Header Data:[{to_hex(m.header.value.payload.data)}], " +
                f"Payload Data:[{to_hex(m.payload.data)}]"
            )
            # print(x3d_dec_msg_id(m.data.value.id, m.data.value.device_id))

d = {
    "seq": 217,
    "type": MessageType.Sensor,
    "header": {"value": {
        "device_id": 1597585,
        "network_id": 1,
        "msg_id": 3034,
        "payload": {"data": bytes([0x05, 0x98, 0x22, 0x81, 0x00])}
    }}
}

msgs = process([X3DMessage.Encoder(verbose=False)], [d])
for msg in msgs:
    print(to_hex(msg))

if filename == "raw_x3d.bin" and dumper is not None:
    assert msgs[0] == dumper.last

# Encode, decode and check that we have the same bytes
out_data_1 = process([CcittWhitening.Encoder()], [msgs[0]])
out_data_2 = process([Packetizer.Encoder(40_000 * 10, 40_000, preamble=bytearray([0xAA, 0xAA, 0xAA, 0xAA]),
                                         syncword=bytearray([0x81, 0x69, 0x96, 0x7e]), verbose=False, throw=False)],
                     out_data_1)
in_data_1 = process([Packetizer.Decoder(40_000 * 10, 40_000, preamble=bytearray([0xAA, 0xAA, 0xAA, 0xAA]),
                                        syncword=bytearray([0x81, 0x69, 0x96, 0x7e]), verbose=False, throw=False)],
                    out_data_2)
in_data_2 = process([CcittWhitening.Decoder()], in_data_1)
assert msgs[0] == in_data_2[0]

# Data from https://github.com/mr-sven/x3d-rfm-esp32/blob/main/X3D-Message-Log.md
d1 = bytes([0x26, 0xFF, 0x40, 0x02, 0x0C, 0x10, 0x02, 0x03, 0x84, 0x85, 0x98, 0x00, 0xAA, 0xBB, 0xFC, 0xE5, 0x04, 0x00,
            0x00, 0x00, 0x00, 0x1F, 0xFF, 0x00, 0x00, 0x00, 0x00, 0xE0, 0x00, 0x00, 0x01, 0xFF, 0xFF, 0xFF, 0xF6, 0x3C,
            0x30,
            0x7D])
d2 = bytes(
    [0x1F, 0xFF, 0x32, 0x02, 0x0C, 0xAA, 0xBB, 0xCC, 0x00, 0x85, 0x98, 0x00, 0xAA, 0xBB, 0xFB, 0x4D, 0x40, 0x00, 0x00,
     0x01, 0x00, 0x1F, 0xFF, 0x00, 0x00, 0x12, 0x34, 0xE0, 0x00, 0x82, 0x0D])
msgs = process([X3DMessage.Decoder(verbose=True)], [d1, d2])
for i, m in enumerate(msgs):
    print(i)
    print(m)
