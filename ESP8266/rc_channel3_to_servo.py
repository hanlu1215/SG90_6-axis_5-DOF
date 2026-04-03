import argparse
import socket
import sys
import time
import urllib.parse
import urllib.request

import serial
from serial import SerialException
from serial.tools import list_ports


DEFAULT_ESP_IP = "192.168.199.236"
IBUS_FRAME_LEN = 32
IBUS_HEADER_0 = 0x20
IBUS_HEADER_1 = 0x40
IBUS_CHANNEL_COUNT = 14


def clamp(value, low, high):
    return max(low, min(high, value))


def map_raw_to_axis(raw_value, raw_min, raw_max):
    if raw_max == raw_min:
        return 0.0
    value = (float(raw_value) - float(raw_min)) / float(raw_max - raw_min)
    return clamp(value * 2.0 - 1.0, -1.0, 1.0)


def map_axis_to_angle(axis_value, invert=False):
    if invert:
        axis_value = -axis_value
    angle = int(round((axis_value + 1.0) * 90.0))
    return clamp(angle, 0, 180)


def send_angle(esp_ip, angle, timeout):
    query = urllib.parse.urlencode({"angle": angle})
    url = f"http://{esp_ip}/set?{query}"
    req = urllib.request.Request(url, method="GET")
    with urllib.request.urlopen(req, timeout=timeout) as resp:
        data = resp.read()
    return data.decode("utf-8", errors="ignore")


def send_angle_udp(sock, esp_ip, udp_port, angle):
    payload = f"{angle}\n".encode("ascii")
    sock.sendto(payload, (esp_ip, udp_port))
    return "ok"


def list_serial_ports():
    ports = list(list_ports.comports())
    if not ports:
        print("No serial ports found.")
        return
    for port in ports:
        desc = port.description or ""
        hwid = port.hwid or ""
        print(f"{port.device:>8}  {desc}  {hwid}")


def open_serial_with_retry(port, baud, timeout):
    ser = serial.Serial(port, baud, timeout=timeout)
    ser.reset_input_buffer()
    return ser


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


def main():
    parser = argparse.ArgumentParser(
        description="Read RC iBus CH3 from serial and send servo angle to ESP8266 over WiFi"
    )
    parser.add_argument("--esp-ip", default=DEFAULT_ESP_IP, help="ESP8266 LAN IP")
    parser.add_argument("--port", default="COM8", help="Serial port, e.g. COM8")
    parser.add_argument("--baud", type=int, default=115200, help="iBus baud rate")
    parser.add_argument("--serial-timeout", type=float, default=0.01, help="Serial timeout in seconds")
    parser.add_argument("--http-timeout", type=float, default=0.08, help="HTTP timeout in seconds")
    parser.add_argument("--transport", choices=("udp", "http"), default="udp", help="Angle transport to ESP8266")
    parser.add_argument("--udp-port", type=int, default=4210, help="UDP port on ESP8266")
    parser.add_argument("--hz", type=float, default=20.0, help="Control loop rate, default 20 Hz")
    parser.add_argument("--ch3-index", type=int, default=3, help="iBus channel index used as throttle (1..14)")
    parser.add_argument("--raw-min", type=int, default=1000, help="RC minimum raw value")
    parser.add_argument("--raw-max", type=int, default=2000, help="RC maximum raw value")
    parser.add_argument("--min-delta", type=int, default=1, help="Send only if angle change >= min-delta")
    parser.add_argument("--invert", action="store_true", help="Invert channel direction")
    parser.add_argument("--log-hz", type=float, default=10.0, help="Status print rate limit, set 0 to disable")
    parser.add_argument("--no-frame-timeout", type=float, default=1.0, help="Warn/recover when no valid iBus frame for this long")
    parser.add_argument("--reconnect-delay", type=float, default=0.5, help="Seconds between serial reconnect attempts")
    parser.add_argument("--list-ports", action="store_true", help="List serial ports and exit")
    args = parser.parse_args()

    if args.list_ports:
        list_serial_ports()
        return 0

    if args.ch3_index < 1 or args.ch3_index > IBUS_CHANNEL_COUNT:
        print(f"Invalid --ch3-index: {args.ch3_index}. Valid range is 1..{IBUS_CHANNEL_COUNT}")
        return 1

    if args.raw_max == args.raw_min:
        print("Invalid raw range. raw-max must differ from raw-min")
        return 1

    if args.hz <= 0:
        print("--hz must be > 0")
        return 1

    try:
        ser = open_serial_with_retry(args.port, args.baud, args.serial_timeout)
    except Exception as exc:
        print(f"Failed to open serial port {args.port}: {exc}")
        return 1

    udp_sock = None
    if args.transport == "udp":
        udp_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        udp_sock.setblocking(False)

    print(f"Connected to {args.port} @ {args.baud} bps")
    if args.transport == "udp":
        print(f"Reading iBus CH{args.ch3_index} and sending to udp://{args.esp_ip}:{args.udp_port}")
    else:
        print(f"Reading iBus CH{args.ch3_index} and sending to http://{args.esp_ip}/set?angle=<0..180>")
    print("Press Ctrl+C to stop")

    rx_buffer = bytearray()
    period = 1.0 / max(args.hz, 1.0)
    last_angle = None
    last_send_time = 0.0
    log_period = (1.0 / args.log_hz) if args.log_hz > 0 else 0.0
    last_log_time = 0.0
    last_frame_time = time.time()
    warned_no_frame = False

    try:
        while True:
            t0 = time.time()

            try:
                read_len = max(1, min(ser.in_waiting, 512))
                chunk = ser.read(read_len)
            except (SerialException, OSError) as exc:
                print(f"Serial read failed: {exc}")
                print(f"Reconnecting {args.port} ...")
                while True:
                    try:
                        ser.close()
                    except Exception:
                        pass
                    try:
                        ser = open_serial_with_retry(args.port, args.baud, args.serial_timeout)
                        rx_buffer.clear()
                        warned_no_frame = False
                        last_frame_time = time.time()
                        print(f"Reconnected to {args.port}")
                        break
                    except Exception as rec_exc:
                        print(f"Reconnect failed: {rec_exc}")
                        time.sleep(max(args.reconnect_delay, 0.1))
                continue

            if chunk:
                rx_buffer.extend(chunk)

                latest_channels = None
                frame = extract_ibus_frame(rx_buffer)
                while frame is not None:
                    # Keep only the newest valid frame to avoid latency from stale backlog.
                    latest_channels = decode_ibus_channels(frame)
                    last_frame_time = time.time()
                    warned_no_frame = False
                    frame = extract_ibus_frame(rx_buffer)

                if latest_channels is not None:
                    raw_ch3 = latest_channels[args.ch3_index - 1]
                    axis_value = map_raw_to_axis(raw_ch3, args.raw_min, args.raw_max)
                    angle = map_axis_to_angle(axis_value, invert=args.invert)

                    now = time.time()
                    interval_ok = (now - last_send_time) >= period
                    delta_ok = last_angle is None or abs(angle - last_angle) >= args.min_delta
                    should_send = interval_ok and delta_ok
                    if should_send:
                        try:
                            if args.transport == "udp":
                                text = send_angle_udp(udp_sock, args.esp_ip, args.udp_port, angle)
                            else:
                                text = send_angle(args.esp_ip, angle, args.http_timeout)

                            now2 = time.time()
                            if log_period > 0 and (now2 - last_log_time) >= log_period:
                                print(f"CH{args.ch3_index}={raw_ch3} axis={axis_value:+.3f} angle={angle} -> {text}")
                                last_log_time = now2

                            last_angle = angle
                            last_send_time = now
                        except Exception as exc:
                            print(f"{args.transport.upper()} send failed: {exc}")
            else:
                time.sleep(0.001)

            if args.no_frame_timeout > 0:
                now3 = time.time()
                if (now3 - last_frame_time) >= args.no_frame_timeout and not warned_no_frame:
                    print(
                        f"Warning: no valid iBus frame for {now3 - last_frame_time:.2f}s on {args.port}. "
                        "Check receiver power/wiring, or if COM port changed."
                    )
                    warned_no_frame = True
                    try:
                        ser.reset_input_buffer()
                        rx_buffer.clear()
                    except Exception:
                        pass

            # Keep loop responsive; sending rate is already limited by interval_ok.
            elapsed = time.time() - t0
            if elapsed < 0.001:
                time.sleep(0.001)

    except KeyboardInterrupt:
        print("Stopped by user")
    finally:
        ser.close()
        if udp_sock is not None:
            udp_sock.close()

    return 0


if __name__ == "__main__":
    sys.exit(main())
