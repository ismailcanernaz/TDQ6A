/*
 * TDQ6A - Tuner Test Sketch
 *
 * Description:
 * Test sketch for the TDQ-6A terrestrial TV tuner library
 * Allows frequency and TV system selection
 * Reports tuning results and PLL lock status via Serial Monitor
 *
 * Author:    İsmail Caner Naz
 * Email:     ismailcanernaz@aol.com
 * Copyright: Copyright (C) 2026 İsmail Caner Naz
 *
 * License: GNU General Public License v3.0 (GPL-3.0)
 * See the LICENSE file in the project repository for details
 */

#include <Wire.h>
#include <TDQ6A.h>

// Passing only the dynamic 0x63 address from the sketch
TDQ6A tuner(Wire, 0x63);

void setup() {
    Serial.begin(9600);
    while (!Serial) { ; } 
    Serial.println(F("--- TDQ-6A Tuner Smart Multi-System Test ---"));

    tuner.begin();

    // Define target channel frequency (MHz)
    float targetFreqMHz = 572.75f;

    // Select TV Mode simply using numbers or enums:
    // 1 (or TDQ6A::MODE_PAL_BG)  -> PAL B/G System
    // 2 (or TDQ6A::MODE_PAL_DK)  -> PAL D/K System
    // 3 (or TDQ6A::MODE_PAL_I)   -> PAL I System
    // 4 (or TDQ6A::MODE_NTSC_MN) -> NTSC M/N System
    uint8_t selectedMode = 1; 

    Serial.print(F("Executing Auto-Tune to: "));
    Serial.print(targetFreqMHz);
    Serial.print(F(" MHz with TV Mode ["));
    Serial.print((uint8_t)selectedMode);
    Serial.println(F("]..."));

    // Single-line execution! Configures IF registers, calculates IF offsets and sets the correct band
    if (tuner.tune(targetFreqMHz, selectedMode, TDQ6A::STEP_62_5_KHZ)) {
        Serial.println(F("Tuner setup and configuration commands sent successfully!"));
    } else {
        Serial.println(F("Error: Hardware timeout or invalid frequency range."));
    }
    
    Serial.println(F("---------------------------------------"));
}


void loop() {
    uint8_t rawStatusByte = 0x00;

    // Read the PLL status register only once.
    if (tuner.readPllStatus(rawStatusByte)) {
        bool lockStatus = (rawStatusByte & 0x40) != 0;

        Serial.print(F("[TUNER TELEMETRY] "));
        if (lockStatus) {
            Serial.print(F("TUNED & LOCKED | "));
        } else {
            Serial.print(F("UNLOCKED/SEARCHING | "));
        }

        Serial.print(F("Status Register: 0x"));
        if (rawStatusByte < 0x10) {
            Serial.print(F("0"));
        }
        Serial.println(rawStatusByte, HEX);
    } else {
        Serial.println(F("[ERROR] I2C communication failure with RF PLL"));
    }

    delay(1000);
}
