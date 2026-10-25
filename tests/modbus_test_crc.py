import serial
import time


PORT = "COM7"
BAUD = 9600

GOOD_FRAME = bytes.fromhex(
    "01 03 00 00 00 03 05 CB"
)

BAD_CRC_FRAME = bytes.fromhex(
    "01 03 00 00 00 03 05 CC"
)


def read_response(ser, timeout=0.25):
    deadline = time.perf_counter() + timeout
    data = bytearray()

    while time.perf_counter() < deadline:
        if ser.in_waiting:
            data.extend(ser.read(ser.in_waiting))
            deadline = time.perf_counter() + 0.03
        else:
            time.sleep(0.001)

    return bytes(data)


def send_frame(ser, frame):
    ser.write(frame)
    ser.flush()
    time.sleep(0.005)


def frame_to_text(frame):
    return " ".join(f"{b:02X}" for b in frame)


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
print("MODBUS RTU CRC ERROR TEST")
print("=" * 60)
print(f"Port       : {PORT}")
print(f"Baud       : {BAUD}")
print("Format     : 8N1")

print()
print("GOOD FRAME:")
print(frame_to_text(GOOD_FRAME))

print()
print("BAD CRC FRAME:")
print(frame_to_text(BAD_CRC_FRAME))

print()
print("=" * 60)
print("TEST 1: CORRECT CRC")
print("=" * 60)

ser.reset_input_buffer()
print("TX:", frame_to_text(GOOD_FRAME))
send_frame(ser, GOOD_FRAME)
response = read_response(ser)

print("RX:", end=" ")
print(frame_to_text(response) if response else "(no data)")

if response:
    good_result = "PASS"
    print("RESULT: PASS")
    print("REASON: VALID FRAME RECEIVED AND ANSWERED")
else:
    good_result = "FAIL"
    print("RESULT: FAIL")
    print("REASON: NO RESPONSE TO VALID FRAME")

print()
print("=" * 60)
print("TEST 2: INVALID CRC")
print("=" * 60)

ser.reset_input_buffer()
print("TX:", frame_to_text(BAD_CRC_FRAME))
send_frame(ser, BAD_CRC_FRAME)
response = read_response(ser)

print("RX:", end=" ")
print(frame_to_text(response) if response else "(no data)")

if not response:
    bad_crc_result = "PASS"
    print("RESULT: PASS")
    print("REASON: INVALID CRC FRAME REJECTED")
else:
    bad_crc_result = "FAIL"
    print("RESULT: FAIL")
    print("REASON: SLAVE RESPONDED TO INVALID CRC")

print()
print("=" * 60)
print("CRC TEST SUMMARY")
print("=" * 60)
print()
print(f"Correct CRC : {good_result}")
print(f"Invalid CRC : {bad_crc_result}")
print()

if good_result == "PASS" and bad_crc_result == "PASS":
    print("OVERALL RESULT: PASS")
    print()
    print("Slave correctly accepts valid frames and rejects frames with invalid CRC.")
else:
    print("OVERALL RESULT: FAIL")
    print()
    print("CRC handling requires further investigation.")

print()
print("=" * 60)
print("TEST FINISHED")
print("=" * 60)

ser.close()
