import serial
import time

PORT = "COM7"
BAUD = 9600
FRAME = bytes.fromhex("01 03 00 00 00 03 05 CB")
DELAYS_MS = [1.40, 1.45, 1.50, 1.55, 1.60, 1.65, 1.70]


def send_frame_with_delay(ser, frame, delay_ms):
    start = time.perf_counter()
    for i, byte in enumerate(frame):
        ser.write(bytes([byte]))
        if i < len(frame) - 1:
            time.sleep(delay_ms / 1000.0)
    ser.flush()
    return (time.perf_counter() - start) * 1000.0


def read_response(ser, timeout=0.15):
    deadline = time.perf_counter() + timeout
    data = bytearray()
    while time.perf_counter() < deadline:
        waiting = ser.in_waiting
        if waiting:
            data.extend(ser.read(waiting))
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
print("MODBUS RTU t1.5 BOUNDARY TEST")
print("=" * 60)
print(f"Port       : {PORT}")
print(f"Baud       : {BAUD}")
print("Format     : 8N1")
print("Calculated : Tchar = 1.042 ms")
print("Calculated : t1.5  = 1.563 ms")
print("Frame      :", " ".join(f"{b:02X}" for b in FRAME))

print()
print("=" * 60)
print("CONTROL TEST")
print("=" * 60)

ser.reset_input_buffer()
tx_time = send_frame_with_delay(ser, FRAME, 0)
response = read_response(ser)

print("TX:", " ".join(f"{b:02X}" for b in FRAME))
print(f"TX duration: {tx_time:.2f} ms")
print("RX:", end=" ")
print_frame(response)
print("CONTROL RESULT:", "PASS" if response else "FAIL")

print()
print("=" * 60)
print("BOUNDARY TESTS")
print("=" * 60)

results = []

for delay in DELAYS_MS:
    print()
    print("-" * 60)
    print(f"TEST: {delay:.2f} ms BETWEEN BYTES")

    ser.reset_input_buffer()
    tx_time = send_frame_with_delay(ser, FRAME, delay)
    response = read_response(ser)

    print("TX:", " ".join(f"{b:02X}" for b in FRAME))
    print(f"TX duration: {tx_time:.2f} ms")
    print("RX:", end=" ")
    print_frame(response)

    if response:
        result = "PASS"
        reason = "RESPONSE RECEIVED"
    else:
        result = "FAIL"
        reason = "NO RESPONSE"

    results.append((delay, result, len(response), reason))
    print(f"RESULT: {result}")
    print(f"REASON: {reason}")

print()
print("=" * 60)
print("t1.5 BOUNDARY TEST SUMMARY")
print("=" * 60)
print()
print(f"{'Delay':>8} | {'Result':>8} | {'Length':>8} | Reason")
print("-" * 60)

for delay, result, length, reason in results:
    print(f"{delay:7.2f} ms | {result:>8} | {length:8} | {reason}")

passed = [r[0] for r in results if r[1] == "PASS"]
failed = [r[0] for r in results if r[1] == "FAIL"]

print()
if passed:
    print(f"Maximum tested delay with PASS: {max(passed):.2f} ms")
if failed:
    print(f"First tested delay with FAIL: {min(failed):.2f} ms")

print()
print("Expected theoretical t1.5: approximately 1.563 ms")
print()
print("=" * 60)
print("TEST FINISHED")
print("=" * 60)

ser.close()
