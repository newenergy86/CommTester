import serial
import time

PORT = "COM7"
BAUD = 9600
SLAVE_ID = 1
TIMEOUT = 0.5


def crc16(data):
    crc = 0xFFFF
    for b in data:
        crc ^= b
        for _ in range(8):
            if crc & 1:
                crc = (crc >> 1) ^ 0xA001
            else:
                crc >>= 1
    return crc


def frame(payload):
    crc = crc16(payload)
    return payload + bytes((crc & 0xFF, (crc >> 8) & 0xFF))


def hx(data):
    return " ".join(f"{b:02X}" for b in data)


ser = serial.Serial(
    PORT, BAUD,
    bytesize=serial.EIGHTBITS,
    parity=serial.PARITY_NONE,
    stopbits=serial.STOPBITS_ONE,
    timeout=TIMEOUT
)

results = []


def exchange(tx):
    ser.reset_input_buffer()
    print("TX:", hx(tx))
    ser.write(tx)
    ser.flush()
    time.sleep(0.05)
    rx = ser.read(256)
    print("RX:", hx(rx) if rx else "(no data)")
    return rx


def check(rx, fc):
    if len(rx) < 5:
        return False, "RESPONSE TOO SHORT"
    if rx[0] != SLAVE_ID:
        return False, f"SLAVE ID MISMATCH: {rx[0]}"
    if rx[1] == (fc | 0x80):
        return False, f"MODBUS EXCEPTION: {rx[2]:02X}"
    if rx[1] != fc:
        return False, f"FUNCTION MISMATCH: {rx[1]:02X}"
    if len(rx) < 4 or crc16(rx[:-2]) != (rx[-2] | (rx[-1] << 8)):
        return False, "CRC ERROR"
    return True, "OK"


def bits(rx, quantity):
    n = (quantity + 7) // 8
    if len(rx) != 5 + n or rx[2] != n:
        return None
    return [(rx[3 + i // 8] >> (i % 8)) & 1 for i in range(quantity)]


def regs(rx, quantity):
    if len(rx) != 5 + quantity * 2 or rx[2] != quantity * 2:
        return None
    return [
        (rx[3 + i * 2] << 8) | rx[4 + i * 2]
        for i in range(quantity)
    ]


def result(name, ok):
    results.append((name, ok))
    print("RESULT:", "PASS" if ok else "FAIL")


def read_bits_test(fc, name, expected):
    print("\n" + "=" * 60)
    print(name)
    print("=" * 60)

    q = len(expected)
    tx = frame(bytes([SLAVE_ID, fc, 0, 0, 0, q]))
    rx = exchange(tx)

    ok, reason = check(rx, fc)
    actual = bits(rx, q) if ok else None

    print("EXPECTED:", "".join(map(str, expected)))
    print("ACTUAL  :", "".join(map(str, actual)) if actual is not None else "INVALID")

    passed = ok and actual == expected
    if not passed:
        print("REASON:", reason if not ok else "DATA MISMATCH")
    result(name.split(" — ")[0], passed)


def read_regs_test(fc, name, start, quantity, expected):
    print("\n" + "=" * 60)
    print(name)
    print("=" * 60)

    tx = frame(bytes([
        SLAVE_ID, fc,
        start >> 8, start & 0xFF,
        quantity >> 8, quantity & 0xFF
    ]))
    rx = exchange(tx)

    ok, reason = check(rx, fc)
    actual = regs(rx, quantity) if ok else None

    print("EXPECTED:", expected)
    print("ACTUAL  :", actual)

    passed = ok and actual == expected
    if not passed:
        print("REASON:", reason if not ok else "REGISTER DATA MISMATCH")
    result(name.split(" — ")[0], passed)


def fc05():
    print("\n" + "=" * 60)
    print("TEST FC05 — WRITE SINGLE COIL")
    print("=" * 60)

    address = 0
    tx = frame(bytes([SLAVE_ID, 5, 0, address, 0xFF, 0]))
    rx = exchange(tx)

    ok, reason = check(rx, 5)
    write_ok = ok and len(rx) == 8 and rx[2:6] == tx[2:6]
    print("WRITE RESPONSE:", "PASS" if write_ok else "FAIL")

    read_tx = frame(bytes([SLAVE_ID, 1, 0, address, 0, 1]))
    read_rx = exchange(read_tx)
    rok, rreason = check(read_rx, 1)
    actual = bits(read_rx, 1) if rok else None
    read_ok = actual == [1]

    print("EXPECTED: ON")
    print("ACTUAL  :", "ON" if actual == [1] else "OFF" if actual == [0] else "INVALID")
    if not write_ok:
        print("REASON:", reason)
    if not read_ok:
        print("REASON READ-BACK:", rreason if not rok else "COIL DATA MISMATCH")
    result("FC05 Write Single Coil", write_ok and read_ok)


def fc06():
    print("\n" + "=" * 60)
    print("TEST FC06 — WRITE SINGLE HOLDING REGISTER")
    print("=" * 60)

    address, value = 0, 12345
    tx = frame(bytes([SLAVE_ID, 6, 0, address, value >> 8, value & 0xFF]))
    rx = exchange(tx)

    ok, reason = check(rx, 6)
    write_ok = ok and len(rx) == 8 and rx[2:6] == tx[2:6]
    print("WRITE RESPONSE:", "PASS" if write_ok else "FAIL")

    read_tx = frame(bytes([SLAVE_ID, 3, 0, address, 0, 1]))
    read_rx = exchange(read_tx)
    rok, rreason = check(read_rx, 3)
    actual = regs(read_rx, 1) if rok else None
    read_ok = actual == [value]

    print("EXPECTED:", [value])
    print("ACTUAL  :", actual)
    if not write_ok:
        print("REASON:", reason)
    if not read_ok:
        print("REASON READ-BACK:", rreason if not rok else "REGISTER DATA MISMATCH")
    result("FC06 Write Single Register", write_ok and read_ok)


def fc15():
    print("\n" + "=" * 60)
    print("TEST FC15 — WRITE MULTIPLE COILS")
    print("=" * 60)

    start, quantity = 0, 16
    expected = [1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0]

    tx = frame(bytes([SLAVE_ID, 15, 0, start, 0, quantity, 2, 0x55, 0x55]))
    rx = exchange(tx)

    ok, reason = check(rx, 15)
    write_ok = ok and len(rx) == 8 and rx[2:6] == tx[2:6]
    print("WRITE RESPONSE:", "PASS" if write_ok else "FAIL")

    read_tx = frame(bytes([SLAVE_ID, 1, 0, start, 0, quantity]))
    read_rx = exchange(read_tx)
    rok, rreason = check(read_rx, 1)
    actual = bits(read_rx, quantity) if rok else None
    read_ok = actual == expected

    print("EXPECTED:", "".join(map(str, expected)))
    print("ACTUAL  :", "".join(map(str, actual)) if actual is not None else "INVALID")
    if not write_ok:
        print("REASON:", reason)
    if not read_ok:
        print("REASON READ-BACK:", rreason if not rok else "COIL DATA MISMATCH")
    result("FC15 Write Multiple Coils", write_ok and read_ok)


def fc16():
    print("\n" + "=" * 60)
    print("TEST FC16 — WRITE MULTIPLE HOLDING REGISTERS")
    print("=" * 60)

    start = 10
    values = [1111, 2222, 3333, 4444, 5555]
    q = len(values)

    payload = bytearray([SLAVE_ID, 16, 0, start, 0, q, q * 2])
    for v in values:
        payload += bytes([v >> 8, v & 0xFF])

    rx = exchange(frame(payload))
    ok, reason = check(rx, 16)
    write_ok = ok and len(rx) == 8 and rx[2:6] == bytes([0, start, 0, q])
    print("WRITE RESPONSE:", "PASS" if write_ok else "FAIL")

    read_tx = frame(bytes([SLAVE_ID, 3, 0, start, 0, q]))
    read_rx = exchange(read_tx)
    rok, rreason = check(read_rx, 3)
    actual = regs(read_rx, q) if rok else None
    read_ok = actual == values

    print("EXPECTED:", values)
    print("ACTUAL  :", actual)
    if not write_ok:
        print("REASON:", reason)
    if not read_ok:
        print("REASON READ-BACK:", rreason if not rok else "REGISTER DATA MISMATCH")
    result("FC16 Write Multiple Registers", write_ok and read_ok)


try:
    print("Opening", PORT)
    print("\n" + "=" * 60)
    print("MODBUS RTU COMPLETE FUNCTION TEST")
    print("=" * 60)
    print("Port       :", PORT)
    print("Baud       :", BAUD)
    print("Format     : 8N1")
    print("Slave ID   :", SLAVE_ID)

    time.sleep(0.2)

    read_bits_test(1, "TEST FC01 — READ COILS",
                   [0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1])

    read_bits_test(2, "TEST FC02 — READ DISCRETE INPUTS",
                   [1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0])

    read_regs_test(3, "TEST FC03 — READ HOLDING REGISTERS (ADDRESS 0)",
                    0, 5, [100,200,300,400,500])

    read_regs_test(3, "TEST FC03 — READ HOLDING REGISTERS (ADDRESS 120)",
                    120, 5, [12100,12200,12300,12400,12500])

    read_regs_test(4, "TEST FC04 — READ INPUT REGISTERS (ADDRESS 0)",
                    0, 5, [1000,1001,1002,1003,1004])

    read_regs_test(4, "TEST FC04 — READ INPUT REGISTERS (ADDRESS 120)",
                    120, 5, [1120,1121,1122,1123,1124])

    fc05()
    fc06()
    fc15()
    fc16()

    print("\n" + "=" * 60)
    print("COMPLETE FUNCTION TEST SUMMARY")
    print("=" * 60)

    for name, ok in results:
        print(f"{name:<40} {'PASS' if ok else 'FAIL'}")

    overall = all(ok for _, ok in results)

    print("\n" + "=" * 60)
    print("OVERALL RESULT:", "PASS" if overall else "FAIL")
    print("=" * 60)

    if overall:
        print("\nAll tested Modbus RTU functions passed.")
    else:
        print("\nOne or more Modbus RTU functions failed.")

finally:
    ser.close()

print("\nTEST FINISHED")
