//
// Pervasive_Wide_Small.cpp
// Class library C++ code
// ----------------------------------
//
// Project Pervasive Displays Library Suite
// Based on highView technology
//
// Copyright (c) Pervasive Displays Inc., 2021-2026
// Licence All rights reserved

//
// See Pervasive_Wide_Small.h for references
//
// Release 611: Added proxy functions to normal update
// Release 611: Added support for red and yellow colour screens
// Release 614: Improved BWRY examples
// Release 702: Added support for 417-QS-0A
// Release 700: Refactored screen and board functions
// Release 800: Read OTP memory
// Release 801: Improved OTP implementation
// Release 802: Added references to application notes
// Release 802: Refactored CoG functions
// Release 900: Added new driver library
// Release 901: Added support for screen 340-KS-0G
// Release 903: Removed unlisted screens
// Release 904: Fixed OTP array size
// Release 905: Minor fixes
// Release 906: Added support for screen 290-KS-06
// Release 1001: Added support for screen 097-KS-0E
// Release 1011: Improved trace granualarity

// Header
#include "Pervasive_Wide_Small.h"

//
// === COG section
//
//
// --- Small screens with K film
//
void Pervasive_Wide_Small::COG_reset()
{
    // Application note § 2. Power on COG driver
    b_reset(1, 5, 5, 10, 20);

    // Check after reset
    switch (u_eScreen_EPD)
    {
        case eScreen_EPD_150_KS_0J:
        case eScreen_EPD_152_KS_0J:
        case eScreen_EPD_290_KS_06:

            if (hV_HAL_GPIO_get(b_pin.panelBusy) == HIGH)
            {
                hV_HAL_log_crlf();
                hV_HAL_log(LEVEL_CRITICAL, "Incorrect type for 1.52-Wide");
                hV_HAL_exit(0x01);
            }
            break;

        default:

            break;
    }
}

void Pervasive_Wide_Small::COG_getDataOTP()
{
    hV_HAL_SPI_end(); // With unicity check
    hV_HAL_SPI3_begin(); // Define 3-wire SPI pins

    // Read OTP
    u_flagOTP = false;

    // Application note § 3. Read OTP memory
    // Register 0x50 flag
    // Additional settings for fast update, 154 206 213 266 271A 370 and 437 screens (s_flag50)
    switch (u_eScreen_EPD)
    {
        case eScreen_EPD_097_KS_0E:
        case eScreen_EPD_154_KS_0C:
        case eScreen_EPD_206_KS_0E:
        case eScreen_EPD_213_KS_0E:
        case eScreen_EPD_266_KS_0C:
        case eScreen_EPD_271_KS_0C: // 2.71(A)
        case eScreen_EPD_370_KS_0C:
        case eScreen_EPD_437_KS_0C:

            s_flag50 = true;
            break;

        default:

            s_flag50 = false;
            break;
    }

    // Screens with no OTP
    switch (u_eScreen_EPD)
    {
        case eScreen_EPD_150_KS_0J:
        case eScreen_EPD_152_KS_0J:
        case eScreen_EPD_290_KS_0F:
        case eScreen_EPD_290_KS_06:

            u_flagOTP = true;
            hV_HAL_log(LEVEL_INFO, "OTP check passed - no embedded PSR");
            return; // No PSR
            break;

        default:

            break;
    }

    hV_HAL_GPIO_set(b_pin.panelDC);
    hV_HAL_GPIO_set(b_pin.panelReset);
    hV_HAL_GPIO_set(b_pin.panelCS);

    b_reset(0, 5, 5, 10, 20);
    uint8_t ui8 = 0;
    uint16_t offsetA5 = 0x0000;
    uint16_t offsetPSR = 0x0000;
    uint16_t u_readBytes = 2;

    hV_HAL_GPIO_clear(b_pin.panelDC); // Command
    hV_HAL_GPIO_clear(b_pin.panelCS); // Select
    hV_HAL_SPI3_write(0xa2);
    hV_HAL_GPIO_set(b_pin.panelCS); // Unselect
    hV_HAL_delayMilliseconds(10);

    hV_HAL_GPIO_set(b_pin.panelDC); // Data
    hV_HAL_GPIO_clear(b_pin.panelCS); // Select
    ui8 = hV_HAL_SPI3_read(); // Dummy
    hV_HAL_GPIO_set(b_pin.panelCS); // Unselect

    hV_HAL_GPIO_clear(b_pin.panelCS); // Select
    ui8 = hV_HAL_SPI3_read(); // First byte to be checked
    hV_HAL_GPIO_set(b_pin.panelCS); // Unselect

    // Check bank
    uint8_t bank = ((ui8 == 0xa5) ? 0 : 1);

    switch (u_eScreen_EPD)
    {
        case eScreen_EPD_271_KS_09:

            offsetPSR = 0x004b;
            offsetA5 = 0x0000;

            if (bank > 0)
            {
                COG_data[0] = 0xcf;
                COG_data[1] = 0x82;
                return;
            }
            break;

        case eScreen_EPD_154_KS_0C:
        case eScreen_EPD_266_KS_0C:
        case eScreen_EPD_271_KS_0C: // 2.71(A)
        case eScreen_EPD_370_KS_0C:
        case eScreen_EPD_437_KS_0C:

            offsetPSR = (bank == 0) ? 0x0fb4 : 0x1fb4;
            offsetA5 = (bank == 0) ? 0x0000 : 0x1000;
            break;

        case eScreen_EPD_097_KS_0E:
        case eScreen_EPD_206_KS_0E:
        case eScreen_EPD_213_KS_0E:

            offsetPSR = (bank == 0) ? 0x0b1b : 0x171b;
            offsetA5 = (bank == 0) ? 0x0000 : 0x0c00;
            break;

        case eScreen_EPD_417_KS_0D:

            offsetPSR = (bank == 0) ? 0x0b1f : 0x171f;
            offsetA5 = (bank == 0) ? 0x0000 : 0x0c00;
            break;

        default:
            hV_HAL_log_crlf();
            hV_HAL_log(LEVEL_CRITICAL, "OTP check failed - Screen %i-%cS-0%c not supported", SCREEN_SIZE(u_eScreen_EPD), SCREEN_FILM(u_eScreen_EPD), SCREEN_DRIVER(u_eScreen_EPD));
            hV_HAL_exit(0x01);
            break;
    }

    // Check second bank
    if (offsetA5 > 0x0000)
    {
        for (uint16_t index = 1; index < offsetA5; index += 1)
        {
            hV_HAL_GPIO_clear(b_pin.panelCS); // CS low = Select
            ui8 = hV_HAL_SPI3_read();
            hV_HAL_GPIO_set(b_pin.panelCS); // CS high = Unselect
        }

        hV_HAL_GPIO_clear(b_pin.panelCS); // CS low = Select
        ui8 = hV_HAL_SPI3_read(); // First byte to be checked
        hV_HAL_GPIO_set(b_pin.panelCS); // CS high = Unselect

        if (ui8 != 0xa5)
        {
            hV_HAL_log_crlf();
            hV_HAL_log(LEVEL_CRITICAL, "OTP check failed - Bank %i, first 0x%02x, expected 0x%02x", bank, ui8, 0xa5);
            hV_HAL_exit(0x01);
        }
    }

    switch (u_eScreen_EPD)
    {
        case eScreen_EPD_271_KS_09:

            hV_HAL_log(LEVEL_INFO, "OTP check passed - Bank %i, first 0x%02x %s", bank, ui8, (bank == 0) ? "as expected" : "not checked");
            break;

        default:

            hV_HAL_log(LEVEL_INFO, "OTP check passed - Bank %i, first 0x%02x as expected", bank, ui8);
            break;
    }

    for (uint16_t index = offsetA5 + 1; index < offsetPSR; index += 1)
    {
        hV_HAL_GPIO_clear(b_pin.panelCS); // Select
        ui8 = hV_HAL_SPI3_read();
        hV_HAL_GPIO_set(b_pin.panelCS); // Unselect
    }
    // Populate COG_initialData
    for (uint16_t index = 0; index < u_readBytes; index += 1)
    {
        hV_HAL_GPIO_clear(b_pin.panelCS); // Select
        COG_data[index] = hV_HAL_SPI3_read(); // Read OTP
        hV_HAL_GPIO_set(b_pin.panelCS); // Unselect
    }

    hV_HAL_SPI3_end();
    u_flagOTP = true;

#if (DEBUG_LOG > 0)
#if (DEBUG_OTP > 0) // Debug COG_data
    debugOTP(COG_data, u_readBytes, COG_WIDE_SMALL, SCREEN_DRIVER(u_eScreen_EPD));
#endif // DEBUG_OTP
#endif // DEBUG_LOG
}

void Pervasive_Wide_Small::COG_initial(uint8_t updateMode)
{
    // Application note § 4. Input initial command
    switch (u_eScreen_EPD)
    {
        case eScreen_EPD_150_KS_0J:
        case eScreen_EPD_152_KS_0J:
        case eScreen_EPD_290_KS_06:

            // Soft reset
            b_sendCommand8(0x12);
            hV_HAL_GPIO_clear(b_pin.panelDC);
            b_waitBusy(LOW); // 150 and 152 specific

            // Work settings
            b_sendCommandData8(0x1a, u_temperature);

            if (updateMode == UPDATE_NORMAL)
            {
                b_sendCommandData8(0x22, 0xd7);
            }
            else if (updateMode == UPDATE_FAST)
            {
                b_sendCommandData8(0x3c, 0xc0);
                b_sendCommandData8(0x22, 0xdf);
            }
            break;

        default:

            // Work settings
            uint8_t indexTemperature; // Temperature
            uint8_t index00_work[2]; // PSR

            // FILM_K already checked
            if (updateMode != UPDATE_NORMAL) // Specific settings for fast update
            {
                indexTemperature = u_temperature | 0x40; // temperature | 0x40
                index00_work[0] = COG_data[0] | 0x10; // PSR0 | 0x10
                index00_work[1] = COG_data[1] | 0x02; // PSR1 | 0x02
            }
            else // Common settings
            {
                indexTemperature = u_temperature; // Temperature
                index00_work[0] = COG_data[0]; // PSR0
                index00_work[1] = COG_data[1]; // PSR1
            } // u_codeExtra updateMode

            // New algorithm
            b_sendCommandData8(0x00, 0x0e); // Soft-reset
            b_waitBusy();

            b_sendCommandData8(0xe5, indexTemperature); // Input Temperature
            b_sendCommandData8(0xe0, 0x02); // Activate Temperature
            switch (u_eScreen_EPD)
            {
                case eScreen_EPD_290_KS_0F: // No PSR

                    b_sendCommandData8(0x4d, 0x55);
                    b_sendCommandData8(0xe9, 0x02);
                    break;

                default:

                    b_sendIndexData(0x00, index00_work, 2); // PSR
                    break;
            }


            // Specific settings for fast update, all screens
            // FILM_K already checked
            if (updateMode != UPDATE_NORMAL)
            {
                b_sendCommandData8(0x50, 0x07); // Vcom and data interval setting
            }
            break;
    }
}

void Pervasive_Wide_Small::COG_sendImageDataNormal(FRAMEBUFFER_CONST_TYPE firstFrame, uint32_t sizeFrame) // First frame, blackBuffer
{
    // firstFrame: New Image for NORMAL
    // secondFrame: 0x00 data for NORMAL

    // Application note § 4. Input image to the EPD
    // b_sendIndexData(0x10, s_newImage, sizeFrame); // First frame, blackBuffer
    // b_sendIndexData(0x10, frame, sizeFrame); // First frame, blackBuffer

    // Send image data
    switch (u_eScreen_EPD)
    {
        case eScreen_EPD_150_KS_0J:
        case eScreen_EPD_152_KS_0J:
        case eScreen_EPD_290_KS_06:

            b_sendIndexData(0x24, firstFrame, sizeFrame); // Next frame, blackBuffer
            b_sendIndexFixed(0x26, 0x00, sizeFrame); // Previous frame, 0x00
            break;

        default:

            b_sendIndexData(0x10, firstFrame, sizeFrame); // First frame, blackBuffer
            b_sendIndexFixed(0x13, 0x00, sizeFrame); // Second frame, 0x00
            break;
    } // u_eScreen_EPD
}

void Pervasive_Wide_Small::COG_sendImageDataFast(FRAMEBUFFER_CONST_TYPE firstFrame, FRAMEBUFFER_CONST_TYPE secondFrame, uint32_t sizeFrame) // First frame, blackBuffer
{
    // firstFrame: New Image for FAST
    // secondFrame: Old Image for FAST

    // Application note § 4. Input image to the EPD
    // b_sendIndexData(0x10, s_newImage, sizeFrame); // First frame, blackBuffer
    // b_sendIndexData(0x10, frame, sizeFrame); // First frame, blackBuffer

    // Send image data
    switch (u_eScreen_EPD)
    {
        case eScreen_EPD_150_KS_0J:
        case eScreen_EPD_152_KS_0J:
        case eScreen_EPD_290_KS_06:

            b_sendIndexData(0x24, secondFrame, sizeFrame); // Previous frame
            b_sendIndexData(0x26, firstFrame, sizeFrame); // Next frame
            break;

        default:
            // Additional settings for fast update, 154 213 266 370 and 437 screens (s_flag50)
            if (s_flag50)
            {
                b_sendCommandData8(0x50, 0x27); // Vcom and data interval setting
            }

            b_sendIndexData(0x10, secondFrame, sizeFrame); // Previous frame
            b_sendIndexData(0x13, firstFrame, sizeFrame); // Next frame

            // Additional settings for fast update, 154 213 266 370 and 437 screens (s_flag50)
            if (s_flag50)
            {
                b_sendCommandData8(0x50, 0x07); // Vcom and data interval setting
            }
            break;
    } // u_eScreen_EPD
}

void Pervasive_Wide_Small::COG_update(uint8_t updateMode)
{
    // Application note § 6. Send updating command
    switch (u_eScreen_EPD)
    {
        case eScreen_EPD_150_KS_0J:
        case eScreen_EPD_152_KS_0J:
        case eScreen_EPD_290_KS_06:

            b_waitBusy(LOW); // 152 specific
            b_sendCommand8(0x20); // Display Refresh
            hV_HAL_GPIO_set(b_pin.panelCS); // CS# = 1
            b_waitBusy(LOW); // 152 specific
            break;

        default:

            b_waitBusy();

            b_sendCommand8(0x04); // Power on
            b_waitBusy();

            b_sendCommand8(0x12); // Display Refresh
            b_waitBusy();
            break;
    }
}

void Pervasive_Wide_Small::COG_stopDCDC()
{
    // Application note § 7. Turn-off DC/DC
    switch (u_eScreen_EPD)
    {
        case eScreen_EPD_150_KS_0J:
        case eScreen_EPD_152_KS_0J:
        case eScreen_EPD_290_KS_06:

            break;

        default:

            b_sendCommand8(0x02); // Turn off DC/DC
            b_waitBusy();
            break;
    }

    // Additional steps for Power OFF
    // clear D/C, RESET, CS, SPI pins to LOW
    // keep BUSY floating
    // cut Vcc off
}
//
// --- End of Small screens with K film
//
//
// === End of COG section
//

Pervasive_Wide_Small::Pervasive_Wide_Small(eScreen_EPD_t eScreen_EPD, pins_t board)
{
    d_COG = COG_WIDE_SMALL;
    u_eScreen_EPD = eScreen_EPD;
    b_pin = board;
}

void Pervasive_Wide_Small::begin()
{
    b_begin(b_pin, FAMILY_SMALL, b_delayCS);
    b_resume(); // GPIO

    COG_reset(); // Reset
    COG_getDataOTP(); // 3-wire SPI read OTP memory
}

STRING_CONST_TYPE Pervasive_Wide_Small::reference()
{
    return formatString("%s v%i.%i.%i", DRIVER_EPD_VARIANT, DRIVER_EPD_RELEASE / 100, (DRIVER_EPD_RELEASE / 10) % 10, DRIVER_EPD_RELEASE % 10);
}

void Pervasive_Wide_Small::updateNormal(FRAMEBUFFER_CONST_TYPE frame, uint32_t sizeFrame)
{
    b_resume(); // GPIO
    COG_reset(); // Reset

    if (u_flagOTP == false)
    {
        COG_getDataOTP(); // 3-wire SPI read OTP memory
        COG_reset(); // Reset
    }

    // Start SPI
    hV_HAL_SPI_begin(16000000); // Fast 16 MHz, with unicity check

    COG_initial(UPDATE_NORMAL); // Initialise
    COG_sendImageDataNormal(frame, sizeFrame);

    COG_update(UPDATE_NORMAL); // Update
    COG_stopDCDC(); // Power off

    // hV_HAL_SPI_end(); // With unicity check
}

void Pervasive_Wide_Small::updateFast(FRAMEBUFFER_CONST_TYPE frame, FRAMEBUFFER_CONST_TYPE frame2, uint32_t sizeFrame)
{
    b_resume(); // GPIO
    COG_reset(); // Reset

    // Start SPI
    hV_HAL_SPI_begin(16000000); // Fast 16 MHz, with unicity check

    COG_initial(UPDATE_FAST); // Initialise
    COG_sendImageDataFast(frame, frame2, sizeFrame);

    COG_update(UPDATE_FAST); // Update
    COG_stopDCDC(); // Power DC/DC off
}

