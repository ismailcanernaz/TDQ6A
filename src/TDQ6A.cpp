/*
 * TDQ6A - Arduino Library for TDQ-6A Terrestrial TV Tuner
 *
 * Description:
 * An Arduino library for controlling the TDQ-6A terrestrial television
 * tuner module via I2C. Supports automatic TV standard configuration,
 * RF band selection, PLL frequency tuning, and PLL lock status reading.
 *
 * Tested with terrestrial television broadcasts in Turkey.
 *
 * Author:      İsmail Caner Naz
 * Email:       ismailcanernaz@aol.com
 * Copyright:   (c) 2026 İsmail Caner Naz
 *
 * License: GNU General Public License v3.0 (GPL-3.0)
 * You may redistribute and modify this software under the terms of
 * the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the LICENSE file in the project repository for the full license.
 */

#include <TDQ6A.h>
#include <math.h>

// Constructor: fixed 0x43 internal address for TDA9885 IF, flexible PLL address via INO
TDQ6A::TDQ6A(
    TwoWire &wire,
    uint8_t pllAddress
) : _wire(&wire),
    _pllAddress(pllAddress),
    _ifAddress(0x43) { 
}

void TDQ6A::begin() {
    _wire->begin();
}

bool TDQ6A::writeIfBytes(uint8_t subAddress, const uint8_t *data, uint8_t count) {
    _wire->beginTransmission(_ifAddress);
    _wire->write(subAddress);
    for (uint8_t i = 0; i < count; ++i) {
        _wire->write(data[i]);
    }
    return _wire->endTransmission() == 0;
}

bool TDQ6A::writePllBytes(const uint8_t *data, uint8_t count) {
    _wire->beginTransmission(_pllAddress);
    for (uint8_t i = 0; i < count; ++i) {
        _wire->write(data[i]);
    }
    return _wire->endTransmission() == 0;
}

bool TDQ6A::validStep(Step step) {
    return step == STEP_50_KHZ || step == STEP_31_25_KHZ || step == STEP_62_5_KHZ;
}

// Core tuning algorithm with AUTOMATIC IF CONFIG, VISION IF, and BAND SELECTION
bool TDQ6A::tune(
    float frequencyMHz,
    uint8_t mode,
    Step step
) {
    if (!validStep(step) || frequencyMHz <= 0.0f) {
        return false;
    }

    // --- STEP 1: AUTOMATIC TV STANDARD & IF CONFIGURATION (Datasheet Page 2 & 15) ---
    float visionIfMHz = 38.90f; // Baseline Vision IF for all system modes in this module
    uint8_t ifSwitching = 0x16; // Fixed baseline switching register
    uint8_t ifAdjust = 0x70;    // Baseline adjust register
    uint8_t ifDataMode = 0x09;  // Default B/G Data mode

    switch (mode) {
        case MODE_PAL_BG: // Mode 1
            ifDataMode = 0x09; // 0x09 Hex
            ifAdjust   = 0x70;
            break;
            
        case MODE_PAL_DK: // Mode 2
            ifDataMode = 0x0B; // 0x0B Hex
            ifAdjust   = 0x70;
            break;
            
        case MODE_PAL_I:  // Mode 3
            ifDataMode = 0x0A; // 0x0A Hex
            ifAdjust   = 0x70;
            break;
            
        case MODE_NTSC_MN: // Mode 4
            ifDataMode = 0x08; // 0x08 Hex
            ifAdjust   = 0x30; // Adjusts to 0x30 for NTSC M/N
            break;
            
        default:
            return false; // Invalid mode selection
    }

    // Write computed register block directly to TDA9885 IF chip (0x43)
    uint8_t ifBlock[] = { ifSwitching, ifAdjust, ifDataMode };
    if (!writeIfBytes(0x00, ifBlock, sizeof(ifBlock))) {
        return false; // IF block setup failed
    }

    // --- STEP 2: AUTOMATIC BAND (MODE) SELECTION (Datasheet Page 2 limits) ---
    Band band;
    if (frequencyMHz >= 44.25f && frequencyMHz <= 140.25f) {
        band = BAND_FM_LOW;
    } 
    else if (frequencyMHz > 140.25f && frequencyMHz <= 423.25f) {
        band = BAND_VHF_MID;
    } 
    else if (frequencyMHz >= 431.25f && frequencyMHz <= 863.25f) {
        band = BAND_VHF_UHF_HIGH;
    } 
    else {
        return false; // Out of RF tuner physical specifications
    }

    // --- STEP 3: PLL REFERENCE DIVIDER CALCULATIONS ---
    float stepMHz;
    uint8_t rsa;
    uint8_t rsb;

    switch (step) {
        case STEP_50_KHZ:    stepMHz = 0.050f;   rsa = 0; rsb = 0; break;
        case STEP_31_25_KHZ: stepMHz = 0.03125f; rsa = 0; rsb = 1; break;
        case STEP_62_5_KHZ:  stepMHz = 0.0625f;  rsa = 1; rsb = 1; break;
        default: return false;
    }

    // High-side injection formula: fOSC = fRF + fIF
    const float oscillatorMHz = frequencyMHz + visionIfMHz;
    const uint32_t divider = (uint32_t)(oscillatorMHz / stepMHz + 0.5f);

    if (divider > 0x7FFFUL) {
        return false;
    }

    // Binary byte payload configuration
    const uint8_t db1 = (uint8_t)((divider >> 8) & 0x7F);
    const uint8_t db2 = (uint8_t)(divider & 0xFF);
    const uint8_t control = (uint8_t)(0xC8 | (rsa << 2) | (rsb << 1));
    const uint8_t ports = (uint8_t)(0x10 | (uint8_t)band);

    const uint8_t pllBlock[] = { db1, db2, control, ports };
    return writePllBytes(pllBlock, sizeof(pllBlock));
}

bool TDQ6A::readPllStatus(uint8_t &status) {
    if (_wire->requestFrom((int)_pllAddress, 1) != 1) return false;
    status = (uint8_t)_wire->read();
    return true;
}

bool TDQ6A::isLocked(bool &locked) {
    uint8_t status;
    if (!readPllStatus(status)) return false;
    locked = (status & 0x40) != 0;
    return true;
}

uint8_t TDQ6A::lastPllAddress() const {
    return _pllAddress;
}
