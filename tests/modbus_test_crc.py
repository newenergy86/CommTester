import serial
import time


PORT = "COM7"
BAUD = 9600
SLAVE_ID = 1

GOOD_FRAME = bytes.fromhex(
    "01 03 00 00 00 03 05 CB"
)

BAD_CRC_FRAME = bytes.fromhex(
    "01 03 00 00 00 03 05 CC"
)

EXPECTED_REGISTERS = [100, 200, 300]


def crc16(data):
    crc = 0xFFFF
    for value in data:
        crc ^= value
        for _ in range(8):
            if crc & 1:
                crc = (crc >> 1) ^ 0xA001
            else:
                crc >>= 1
    return crc


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


def validate_fc03_response(response):
    if len(response) != 11:
        return False, f"WRONG RESPONSE LENGTH: {len(response)}"

    if response[0] != SLAVE_ID:
        return False, f"WRONG SLAVE ID: {response[0]}"

    if response[1] != 0x03:
        return False, f"WRONG FUNCTION CODE: {response[1]:02X}"

    if response[2] != 6:
        return False, f"WRONG BYTE COUNT: {response[2]}"

    actual_registers = [
        (response[3 + i * 2] << 8) | response[4 + i * 2]
        for i in range(3)
    ]

    if actual_registers != EXPECTED_REGISTERS:
        return False, f"REGISTER DATA MISMATCH: {actual_registers}"

    received_crc = response[-2] | (response[-1] << 8)
    calculated_crc = crc16(response[:-2])

    if received_crc != calculated_crc:
        return False, (
            f"RESPONSE CRC ERROR: received {received_crc:04X}, "
            f"calculated {calculated_crc:04X}"
        )

    return True, f"VALID FC03 RESPONSE, REGISTERS: {actual_registers}"


def run_valid_request(ser, title):
    print()
    print("=" * 60)
    print(title)
    print("=" * 60)

    ser.reset_input_buffer()
    print("TX:", frame_to_text(GOOD_FRAME))
    send_frame(ser, GOOD_FRAME)
    response = read_response(ser)

    print("RX:", frame_to_text(response) if response else "(no data)")

    if not response:
        print("RESULT: FAIL")
        print("REASON: NO RESPONSE TO VALID FRAME")
        return False

    ok, reason = validate_fc03_response(response)
    print("CHECK:", reason)
    print("RESULT:", "PASS" if ok else "FAIL")
    return ok


ser = None

try:
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
    print("MODBUS RTU CRC REJECTION AND RECOVERY TEST")
    print("=" * 60)
    print(f"Port       : {PORT}")
    print(f"Baud       : {BAUD}")
    print("Format     : 8N1")
    print(f"Slave ID   : {SLAVE_ID}")

    print()
    print("GOOD FRAME:")
    print(frame_to_text(GOOD_FRAME))
    print("BAD CRC FRAME:")
    print(frame_to_text(BAD_CRC_FRAME))

    # TEST 1: establish that the Slave is responding before the error.
    good_before = run_valid_request(
        ser, "TEST 1: VALID REQUEST BEFORE INVALID CRC"
    )

    # TEST 2: the same request with one CRC byte deliberately corrupted.
    print()
    print("=" * 60)
    print("TEST 2: INVALID CRC MUST BE REJECTED")
    print("=" * 60)

    ser.reset_input_buffer()
    print("TX:", frame_to_text(BAD_CRC_FRAME))
    send_frame(ser, BAD_CRC_FRAME)
    response = read_response(ser)

    print("RX:", frame_to_text(response) if response else "(no data)")

    bad_crc_rejected = not response
    print("RESULT:", "PASS" if bad_crc_rejected else "FAIL")
    if response:
        print("REASON: SLAVE RESPONDED TO INVALID CRC")
    else:
        print("REASON: NO RESPONSE TO INVALID CRC FRAME")

    # TEST 3: prove that a valid request still works after the rejected frame.
    recovery = run_valid_request(
        ser, "TEST 3: VALID REQUEST AFTER INVALID CRC"
    )

    print()
    print("=" * 60)
    print("CRC AND RECOVERY TEST SUMMARY")
    print("=" * 60)
    print(f"Valid request before error : {'PASS' if good_before else 'FAIL'}")
    print(f"Invalid CRC rejected      : {'PASS' if bad_crc_rejected else 'FAIL'}")
    print(f"Valid request after error  : {'PASS' if recovery else 'FAIL'}")

    overall = good_before and bad_crc_rejected and recovery

    print()
    print("OVERALL RESULT:", "PASS" if overall else "FAIL")

    if overall:
        print("Slave accepts valid frames, rejects invalid CRC, and recovers.")
    else:
        print("CRC rejection or recovery requires investigation.")

finally:
    if ser is not None:
        ser.close()

print()
print("=" * 60)
print("TEST FINISHED")
print("=" * 60)
