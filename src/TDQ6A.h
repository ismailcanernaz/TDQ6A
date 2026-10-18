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

#ifndef TDQ6A_H
#define TDQ6A_H

#include <Arduino.h>
#include <Wire.h>

class TDQ6A {
public:
    // Enumeration for PLL step sizes (Datasheet Page 6)
    enum Step {
        STEP_50_KHZ,    
        STEP_31_25_KHZ, 
        STEP_62_5_KHZ   
    };

    // TV Standard Selection (1=B/G, 2=D/K, 3=I, 4=M/N NTSC) (Datasheet Page 2 & 15)
    enum TvMode {
        MODE_PAL_BG     = 1,
        MODE_PAL_DK     = 2,
        MODE_PAL_I      = 3,
        MODE_NTSC_MN    = 4
    };

    // Constructor: Accepts Wire instance and dynamic 7-bit RF PLL address (0x63)
    TDQ6A(TwoWire &wire = Wire, uint8_t pllAddress = 0x63);
    void begin();

    // Low-level hardware register write operations
    bool writeIfBytes(uint8_t subAddress, const uint8_t *data, uint8_t count);
    bool writePllBytes(const uint8_t *data, uint8_t count);
    bool validStep(Step step);

    // Main tuning function - Automatically handles IF configuration, Vision IF, and Band Selection!
    bool tune(float frequencyMHz, uint8_t mode = MODE_PAL_BG, Step step = STEP_62_5_KHZ);

    // Status monitoring functions
    bool readPllStatus(uint8_t &status);
    bool isLocked(bool &locked);
    uint8_t lastPllAddress() const;

private:
    // Private enumeration for internal RF Band Selection (Datasheet Page 6)
    enum Band {
        BAND_FM_LOW        = 0x01, 
        BAND_VHF_MID       = 0x02, 
        BAND_VHF_UHF_HIGH  = 0x08  
    };

    TwoWire *_wire;        
    uint8_t _pllAddress;   
    uint8_t _ifAddress;    
};

#endif // TDQ6A_H
