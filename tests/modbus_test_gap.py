import serial
import time

PORT = "COM7"
BAUD = 9600
FRAME = bytes.fromhex("01 03 00 00 00 03 05 CB")
DELAYS_MS = [0.0, 3.0, 5.0, 10.0]


def send_frame_with_delay(ser, frame, delay_ms):
    start = time.perf_counter()
    for i, byte in enumerate(frame):
        ser.write(bytes([byte]))
        if i < len(frame) - 1:
            time.sleep(delay_ms / 1000.0)
    ser.flush()
    return (time.perf_counter() - start) * 1000.0


def read_response(ser, timeout=0.20):
    deadline = time.perf_counter() + timeout
    data = bytearray()
    while time.perf_counter() < deadline:
        if ser.in_waiting:
            data.extend(ser.read(ser.in_waiting))
            deadline = time.perf_counter() + 0.03
        else:
            time.sleep(0.001)
    return bytes(data)


def print_frame(data):
    if not data:
        print("(no data)")
        return
    print(" ".join(f"{b:02X}" for b in data))


print("Opening", PORT)

ser = serial.Serial(
    port=PORT,
    baudrate=BAUD,
    bytesize=serial.EIGHTBITS,
    parity=serial.PARITY_NONE,
    stopbits=serial.STOPBITS_ONE,
    timeout=0.01,
)

time.sleep(0.5)
ser.reset_input_buffer()
ser.reset_output_buffer()

print()
print("=" * 60)
print("MODBUS RTU INTER-BYTE GAP TEST")
print("=" * 60)
print(f"Port       : {PORT}")
print(f"Baud       : {BAUD}")
print("Format     : 8N1")
print("Tchar      : 1.042 ms")
print("t1.5       : 1.563 ms")
print("Frame      :", " ".join(f"{b:02X}" for b in FRAME))

results = []

for delay in DELAYS_MS:
    print()
    print("-" * 60)
    print("TEST: NORMAL FRAME" if delay == 0 else f"TEST: {delay:.1f} ms BETWEEN BYTES")

    ser.reset_input_buffer()
    tx_time = send_frame_with_delay(ser, FRAME, delay)
    response = read_response(ser)

    print("TX:", " ".join(f"{b:02X}" for b in FRAME))
    print(f"TX duration: {tx_time:.2f} ms")
    print("RX:", end=" ")
    print_frame(response)

    expected = "PASS" if delay == 0 else "FAIL"
    actual = "PASS" if response else "FAIL"

    if actual == expected:
        result = "PASS"
        reason = "EXPECTED BEHAVIOR"
    else:
        result = "FAIL"
        reason = "UNEXPECTED BEHAVIOR"

    results.append((delay, actual, len(response), reason))
    print(f"RESULT: {result}")
    print(f"REASON: {reason}")

print()
print("=" * 60)
print("TEST SUMMARY")
print("=" * 60)
print()
print(f"{'Delay':>10} | {'RX':>8} | {'Length':>8} | Result")
print("-" * 60)

for delay, actual, length, reason in results:
    print(f"{delay:8.1f} ms | {actual:>8} | {length:8} | {reason}")

print()
print("=" * 60)
print("TEST FINISHED")
print("=" * 60)

ser.close()
