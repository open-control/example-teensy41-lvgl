#pragma once

/**
 * @file Config.hpp
 * @brief Hardware and timing configuration
 */

#include <array>
#include <cstddef>
#include <cstdint>

#include <oc/common/ButtonDef.hpp>
#include <oc/common/EncoderDef.hpp>

namespace Config {

// ─────────────────────────────────────────────────────────────────────
// Display
// ─────────────────────────────────────────────────────────────────────

constexpr uint16_t DISPLAY_WIDTH = 320;
constexpr uint16_t DISPLAY_HEIGHT = 240;
constexpr size_t FRAMEBUFFER_SIZE = DISPLAY_WIDTH * DISPLAY_HEIGHT;
constexpr size_t DIFFBUFFER_SIZE = 8192;

namespace DisplayPins {
    constexpr uint8_t CS   = 28;
    constexpr uint8_t DC   = 0;
    constexpr uint8_t RST  = 29;
    constexpr uint8_t MOSI = 26;
    constexpr uint8_t SCK  = 27;
    constexpr uint8_t MISO = 1;
    constexpr uint32_t SPI_SPEED = 40'000'000;
}

namespace DisplaySettings {
    constexpr uint8_t ROTATION = 3;
    constexpr bool INVERT = true;
    constexpr uint8_t VSYNC_SPACING = 1;
    constexpr uint8_t DIFF_GAP = 6;
    constexpr uint8_t IRQ_PRIORITY = 128;
    constexpr float LATE_START_RATIO = 0.3f;
}

// ─────────────────────────────────────────────────────────────────────
// Timing
// ─────────────────────────────────────────────────────────────────────

namespace Timing {
    constexpr uint32_t APP_REFRESH_HZ  = 2000;
    constexpr uint32_t LVGL_REFRESH_HZ = 100;
    constexpr uint32_t APP_PERIOD_US   = 1'000'000 / APP_REFRESH_HZ;
    constexpr uint32_t LVGL_PERIOD_US  = 1'000'000 / LVGL_REFRESH_HZ;

    constexpr uint32_t LONG_PRESS_MS = 500;
    constexpr uint32_t DOUBLE_TAP_MS = 300;
    constexpr uint8_t  DEBOUNCE_MS   = 5;
}

// ─────────────────────────────────────────────────────────────────────
// Encoders
// ─────────────────────────────────────────────────────────────────────

namespace Enc {
    using Def = oc::common::EncoderDef;

    // Project defaults
    constexpr uint16_t PPR = 24;
    constexpr uint16_t RANGE = 270;
    constexpr uint8_t  TICKS = 1;
    constexpr bool     INVERT = true;

    // Named encoder definitions
    //                    ID   pinA  pinB
    constexpr Def LEFT  { 10,  22,   23,   PPR, RANGE, TICKS, INVERT };
    constexpr Def RIGHT { 11,  18,   19,   PPR, RANGE, TICKS, INVERT };

    // All encoders for iteration
    constexpr std::array ALL = { LEFT, RIGHT };
}

// ─────────────────────────────────────────────────────────────────────
// Buttons
// ─────────────────────────────────────────────────────────────────────

namespace Btn {
    using Def = oc::common::ButtonDef;
    using Src = oc::common::GpioPin::Source;

    // Named button definitions
    //                     ID   pin          activeLow
    constexpr Def MAIN  { 100, {32, Src::MCU}, true };

    // All buttons for iteration
    constexpr std::array ALL = { MAIN };
}

// ─────────────────────────────────────────────────────────────────────
// MIDI
// ─────────────────────────────────────────────────────────────────────

namespace Midi {
    constexpr uint8_t CHANNEL    = 0;
    constexpr uint8_t ENC_CC     = 16;  // CC 16-17 for encoders
    constexpr uint8_t BTN_CC     = 20;
}

}  // namespace Config
