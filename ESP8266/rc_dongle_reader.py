import argparse
import sys
import time

import serial
from serial.tools import list_ports


IBUS_FRAME_LEN = 32
IBUS_HEADER_0 = 0x20
IBUS_HEADER_1 = 0x40
IBUS_CHANNEL_COUNT = 14


def parse_channel_selection(channel_text):
    channels = []
    for item in channel_text.split(","):
        item = item.strip()
        if not item:
            continue
        index = int(item)
        if index < 1 or index > IBUS_CHANNEL_COUNT:
            raise ValueError(f"Channel index out of range: {index} (valid 1..{IBUS_CHANNEL_COUNT})")
        channels.append(index)
    if not channels:
        raise ValueError("No channels selected")
    return channels


def list_serial_ports():
    ports = list(list_ports.comports())
    if not ports:
        print("No serial ports found.")
        return
    for port in ports:
        desc = port.description or ""
        hwid = port.hwid or ""
        print(f"{port.device:>8}  {desc}  {hwid}")


def ibus_checksum_ok(frame):
    if len(frame) != IBUS_FRAME_LEN:
        return False
    expected = (0xFFFF - (sum(frame[0:30]) & 0xFFFF)) & 0xFFFF
    got = frame[30] | (frame[31] << 8)
    return expected == got


def decode_ibus_channels(frame):
    channels = []
    for idx in range(IBUS_CHANNEL_COUNT):
        offset = 2 + idx * 2
        value = frame[offset] | (frame[offset + 1] << 8)
        channels.append(value)
    return channels


def extract_ibus_frame(rx_buffer):
    while len(rx_buffer) >= IBUS_FRAME_LEN:
        if rx_buffer[0] != IBUS_HEADER_0 or rx_buffer[1] != IBUS_HEADER_1:
            del rx_buffer[0]
            continue

        frame = bytes(rx_buffer[:IBUS_FRAME_LEN])
        del rx_buffer[:IBUS_FRAME_LEN]
        if ibus_checksum_ok(frame):
            return frame
    return None


def format_channels(channels, selected):
    parts = []
    for ch in selected:
        value = channels[ch - 1]
        parts.append(f"CH{ch}:{value:4}")
    return " | ".join(parts)


def main():
    parser = argparse.ArgumentParser(description="Read RC iBus channels from serial port")
    parser.add_argument("--port", default="COM8", help="Serial port, e.g. COM8")
    parser.add_argument("--baud", type=int, default=115200, help="iBus baud rate (default 115200)")
    parser.add_argument("--timeout", type=float, default=0.01, help="Serial timeout in seconds")
    parser.add_argument("--channels", default="1,2,3,4,5,6", help="Channels to print, e.g. 1,2,3,4,5,6")
    parser.add_argument("--hz", type=float, default=50.0, help="Print rate limit in Hz")
    parser.add_argument("--list-ports", action="store_true", help="List serial ports and exit")
    args = parser.parse_args()

    if args.list_ports:
        list_serial_ports()
        return 0

    try:
        selected_channels = parse_channel_selection(args.channels)
    except ValueError as exc:
        print(f"Invalid --channels: {exc}")
        return 1

    if args.hz <= 0:
        print("--hz must be > 0")
        return 1

    try:
        ser = serial.Serial(args.port, args.baud, timeout=args.timeout)
    except Exception as exc:
        print(f"Failed to open serial port {args.port}: {exc}")
        return 1

    print(f"Connected to {args.port} @ {args.baud} bps, waiting for iBus frames...")
    print(f"Displaying channels: {','.join(str(ch) for ch in selected_channels)}")
    print("Press Ctrl+C to stop")

    rx_buffer = bytearray()
    min_interval = 1.0 / args.hz
    last_print = 0.0

    try:
        while True:
            chunk = ser.read(ser.in_waiting or 1)
            if chunk:
                rx_buffer.extend(chunk)

                frame = extract_ibus_frame(rx_buffer)
                while frame is not None:
                    channels = decode_ibus_channels(frame)
                    now = time.time()
                    if now - last_print >= min_interval:
                        print(format_channels(channels, selected_channels), end="\r", flush=True)
                        last_print = now
                    frame = extract_ibus_frame(rx_buffer)

            if not chunk:
                time.sleep(0.001)

    except KeyboardInterrupt:
        print("\nStopped by user")
    finally:
        ser.close()

    return 0


if __name__ == "__main__":
    sys.exit(main())