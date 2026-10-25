# CommTester

Портативный диагностический прибор линий связи и протоколов.

## Текущий этап

Сейчас разрабатывается компактный лабораторный тестер линий **Modbus RTU / RS-232** на **Arduino Nano / ATmega328P**. Он используется как испытательная платформа для последующего основного диагностического прибора CommTester.

Текущий стенд:

- Arduino Nano / ATmega328P
- RS-232 через MAX238
- 9600 8N1
- один физический SoftwareSerial для RS-232
- режимы SERVICE / SLAVE / MASTER
- RX / TX / ERROR / MODE LED
- пьезоизлучатель
- кнопка переключения режима

## Modbus Slave

Реализованы и протестированы:

- FC01 — Read Coils
- FC02 — Read Discrete Inputs
- FC03 — Read Holding Registers
- FC04 — Read Input Registers
- FC05 — Write Single Coil
- FC06 — Write Single Register
- FC15 — Write Multiple Coils
- FC16 — Write Multiple Registers

Память Coils и Discrete Inputs хранится в упакованном виде по битам, что позволяет использовать до 125 логических элементов без расхода SRAM как у `bool[125]`.

Holding/Input Registers используют `uint16_t`, по 2 байта на регистр.

## Тесты

В каталоге `tests/` находятся Python-тесты для:

- полного набора Modbus-функций;
- CRC;
- межбайтовых интервалов;
- границы t1.5.

Тесты рассчитаны на подключение стенда к ПК через COM-порт и Python `pyserial`.

## Архитектура

```text
CommTester_Bench.ino
        |
        +-- serial_console
        |
        +-- indicators
        |
        +-- modbus_slave
        |       |
        |       +-- modbus_slave_logic
        |
        +-- modbus_master
        |
        +-- modbus_rtu
                |
                +-- ModbusADU (внешняя библиотека)
```

Проектный принцип: внешние библиотеки не изменяются; собственная логика проекта находится в исходниках CommTester.

## Документация

- `PROJECT.md` — архитектура и долгосрочное назначение проекта.
- `CommTester_PROJECT_LOG.md` — рабочий журнал и контрольные точки.
