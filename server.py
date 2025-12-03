import argparse
import asyncio
import logging
import sys

logger = logging.getLogger(__name__)


def to_hex(data):
    return ', '.join('0x{:02x}'.format(x) for x in data)


def remove_consecutive_duplicates(lst):
    if not lst:
        return []
    result = [lst[0]]  # start with the first element
    for item in lst[1:]:
        if item != result[-1]:
            result.append(item)
    return result


class X3DHandler:
    def __init__(self, bitrate, symrate):
        from encoding import Packetizer
        from encoding import CcittWhitening
        from encoding import X3DMessage
        from encoding import Duplicator
        self._processors = [
            Packetizer.Decoder(
                bitrate,
                symrate,
                preamble=bytearray([0xAA, 0xAA, 0xAA, 0xAA]),
                syncword=bytearray([0x81, 0x69, 0x96, 0x7e]),
                verbose=False
            ),
            Duplicator.Decoder(),
            CcittWhitening.Decoder(),
            X3DMessage.Decoder(throw=False)]

    def __call__(self, *args, **kwargs):
        from encoding import process
        data = bytearray(args[0])

        msgs = process(self._processors, data)
        if msgs is not None:
            for m in msgs:
                print(
                    f"Seq {m.seq}, type {m.type}, " +
                    f"Device ID {m.header.value.device_id}, Network ID {m.header.value.network_id}, Msg ID {m.header.value.msg_id}, " +
                    f"Header Data:[{to_hex(m.header.value.payload.data)}], " +
                    f"Payload Data:[{to_hex(m.payload.data)}]"
                )


def x2d_handler(data):
    pass


async def run_server(host: str, port: int, handler):
    async def handle_client(reader: asyncio.StreamReader, writer: asyncio.StreamWriter):
        addr = writer.get_extra_info('peername')
        logger.info(f"Connection from {addr}")

        try:
            while True:
                data = await reader.read(1024)  # read stream chunk
                if not data:
                    logger.info(f"Client {addr} disconnected")
                    break
                logger.debug(f"Received data from {addr}: size {len(data)}")
                handler(data)

        except asyncio.CancelledError:
            pass
        finally:
            writer.close()
            await writer.wait_closed()
            logger.info(f"Connection closed: {addr}")

    server = await asyncio.start_server(handle_client, host, port)
    logger.info(f"TCP server listening on {host}:{port}")

    async with server:
        await server.serve_forever()


def server(args):
    handler = x2d_handler if args.x.lower() == 'x2d' else X3DHandler(args.bitrate, args.symrate)
    asyncio.run(run_server(args.address, args.port, handler))


def _main(argv=sys.argv):
    def auto_int(x):
        return int(x, 0)

    logging.basicConfig(stream=sys.stderr, level=logging.DEBUG,
                        format='%(asctime)s - %(threadName)s - %(name)s - %(levelname)s - %(message)s')
    parser = argparse.ArgumentParser(prog=argv[0], description='XxD Server',
                                     formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    parser.add_argument('-v', '--verbose', dest='verbose_count', action='count', default=0,
                        help="increases log verbosity for each occurrence.")

    parser.add_argument('-b', '--bitrate', default=400000, type=int,
                        help="bitrate")

    parser.add_argument('-s', '--symrate', default=40000, type=int,
                        help="symbol rate")

    parser.add_argument('-p', '--port', default=5165, type=int,
                        help="server port")

    parser.add_argument('-a', '--address', default='0.0.0.0', type=str,
                        help="server address")

    parser.add_argument('-x', default='X3D', type=str,
                        help="XxD protocol")

    # Parse
    args, unknown_args = parser.parse_known_args(argv[1:])

    # Set logging level
    logging.getLogger().setLevel(max(3 - args.verbose_count, 0) * 10)

    server(args)

    return 0


# ------------------------------------------------------------------------------

def main():
    try:
        sys.exit(_main(sys.argv))
    except Exception as e:
        logger.exception(e)
        sys.exit(-1)
    finally:
        logging.shutdown()


if __name__ == "__main__":
    main()
