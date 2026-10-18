# TDQ6A

**Arduino library for the TDQ-6A terrestrial TV tuner**

TDQ6A is an Arduino library for controlling the TDQ-6A terrestrial television tuner through the I²C bus. It provides TV system selection, automatic RF band selection, PLL frequency tuning, and PLL lock status monitoring.

The library uses the Arduino `Wire` library and supports configurable I²C interfaces and PLL addresses.

## Features

- I²C communication using Arduino `Wire`
- PAL B/G, PAL D/K, PAL I, and NTSC M/N mode selection
- Automatic RF band selection
- PLL frequency calculation and tuning
- Configurable PLL reference step
- PLL lock status monitoring
- Configurable 7-bit PLL I²C address

## Hardware

- **Tuner:** TDQ-6A terrestrial TV tuner
- **Supply voltage:** 5 V
- **Communication:** I²C
- **Default PLL address:** `0x63` (7-bit address)
- **IF configuration address:** `0x43` (7-bit address)

The library configures the IF section before sending the PLL tuning data, following the tested hardware configuration.

### Wiring

Connect the tuner to the Arduino using the I²C interface:

| Tuner signal | Arduino connection |
|---|---|
| VCC | 5 V |
| GND | GND |
| SDA | Arduino SDA |
| SCL | Arduino SCL |

Use the SDA and SCL pins designated for your Arduino board. The exact physical pin numbers of the 11-pin tuner connector must be confirmed against the pinout for the specific TDQ-6A module revision before connecting power.

**Important:** Confirm the module's supply requirements and I²C logic-level compatibility before connecting it to your board.

## Installation

### Arduino Library Manager

Search for `TDQ6A` in the Arduino IDE Library Manager and install the library once it is available there.

### Manual installation

1. Download or clone this repository.
2. Place the library folder in your Arduino `libraries` directory.
3. Restart the Arduino IDE if necessary.
4. Include `TDQ6A.h` in your sketch.

## Quick start

```cpp
#include <Wire.h>
#include <TDQ6A.h>

TDQ6A tuner(Wire, 0x63);

void setup() {
    Serial.begin(9600);

    tuner.begin();

    const float frequencyMHz = 572.75f;
    const uint8_t mode = TDQ6A::MODE_PAL_BG;

    if (tuner.tune(
            frequencyMHz,
            mode,
            TDQ6A::STEP_62_5_KHZ)) {
        Serial.println(F("Tuning commands sent successfully."));
    } else {
        Serial.println(F("Tuning failed."));
    }
}

void loop() {
    uint8_t status = 0;

    if (tuner.readPllStatus(status)) {
        const bool locked = (status & 0x40) != 0;

        Serial.print(F("PLL status: 0x"));
        Serial.println(status, HEX);

        Serial.println(
            locked ? F("PLL locked") : F("PLL not locked"));
    } else {
        Serial.println(F("PLL status read failed."));
    }

    delay(1000);
}
```

A successful `tune()` call indicates that the library successfully sent the configuration data. It does not, by itself, guarantee that a usable broadcast signal has been received or that the PLL has locked.

## TV system selection

Use one of the following constants as the `mode` argument to `tune()`.

| Constant | TV system |
|---|---|
| `TDQ6A::MODE_PAL_BG` | PAL B/G |
| `TDQ6A::MODE_PAL_DK` | PAL D/K |
| `TDQ6A::MODE_PAL_I` | PAL I |
| `TDQ6A::MODE_NTSC_MN` | NTSC M/N |

Example:

```cpp
tuner.tune(572.75f, TDQ6A::MODE_PAL_BG);
```

## PLL reference steps

The library supports the following PLL reference steps:

| Constant | Step size |
|---|---:|
| `TDQ6A::STEP_50_KHZ` | 50 kHz |
| `TDQ6A::STEP_31_25_KHZ` | 31.25 kHz |
| `TDQ6A::STEP_62_5_KHZ` | 62.5 kHz |

Example:

```cpp
tuner.tune(
    572.75f,
    TDQ6A::MODE_PAL_BG,
    TDQ6A::STEP_62_5_KHZ);
```

## RF frequency ranges

The current implementation supports these RF tuning ranges:

- 44.25–140.25 MHz
- Above 140.25 MHz through 423.25 MHz
- 431.25–863.25 MHz

Frequencies outside these ranges are rejected by the library.

## API reference

### Constructor

```cpp
TDQ6A(TwoWire &wire = Wire, uint8_t pllAddress = 0x63);
```

Creates a tuner instance using the specified I²C interface and PLL address.

### Initialization

```cpp
void begin();
```

Initializes the selected I²C interface.

### Tuning

```cpp
bool tune(
    float frequencyMHz,
    uint8_t mode = TDQ6A::MODE_PAL_BG,
    Step step = TDQ6A::STEP_62_5_KHZ);
```

Configures the IF section and sends the PLL tuning data. Returns `true` when the I²C write operations succeed.

### PLL status

```cpp
bool readPllStatus(uint8_t &status);
bool isLocked(bool &locked);
uint8_t lastPllAddress() const;
```

Reads the PLL status, checks its lock state, and returns the configured PLL address.

### Low-level operations

```cpp
bool writeIfBytes(
    uint8_t subAddress,
    const uint8_t *data,
    uint8_t count);

bool writePllBytes(
    const uint8_t *data,
    uint8_t count);

bool validStep(Step step);
```

These methods provide low-level access to IF and PLL writes and PLL step validation.

## Tested hardware

The library has been tested with a TDQ-6A tuner receiving terrestrial television broadcasts in Turkey. A working hardware test successfully tuned the tuner and produced audio.

Results may depend on the tuner module revision, antenna, signal availability, wiring, and TV system.

## License

This project is licensed under the GNU General Public License v3.0 (GPL-3.0). See the `LICENSE` file for the full license text.

## Author

**İsmail Caner Naz**

Email: ismailcanernaz@aol.com
