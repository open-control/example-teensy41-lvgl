#pragma once

/**
 * @file Config.hpp
 * @brief Hardware configuration
 */

#include <array>
#include <oc/common/ButtonDef.hpp>
#include <oc/common/EncoderDef.hpp>
#include <oc/teensy/Ili9341.hpp>

namespace Config {

// ─────────────────────────────────────────────────────────────────────
// Timing (milliseconds)
// ─────────────────────────────────────────────────────────────────────

namespace Timing {
    constexpr uint32_t APP_HZ  = 2000;   ///< Main loop rate
    constexpr uint32_t LVGL_HZ = 100;    ///< UI refresh rate

    constexpr uint32_t LONG_PRESS_MS = 500;
    constexpr uint32_t DOUBLE_TAP_MS = 300;
    constexpr uint8_t  DEBOUNCE_MS   = 5;
}

// ─────────────────────────────────────────────────────────────────────
// Display
// 
// Uses hal-teensy defaults. Override only what differs from your setup.
// See oc::teensy::Ili9341Config for all options.
// ─────────────────────────────────────────────────────────────────────

namespace Display {
    constexpr uint16_t WIDTH  = 320;
    constexpr uint16_t HEIGHT = 240;
    constexpr size_t   BUFFER_SIZE = WIDTH * HEIGHT;
    constexpr size_t   DIFF_SIZE   = 8192;
}

// ─────────────────────────────────────────────────────────────────────
// Encoders
//
// EncoderDef fields:
//   id            - Unique identifier for input binding
//   pinA, pinB    - Quadrature encoder pins
//   ppr           - Pulses per revolution (detents × 4 for most encoders)
//   rangeAngle    - Physical rotation range in degrees
//   ticksPerEvent - Ticks required to trigger onTurned (1 = every tick)
//   invertDirection - Reverse rotation direction
// ─────────────────────────────────────────────────────────────────────

namespace Enc {
    using Def = oc::common::EncoderDef;

    constexpr uint16_t PPR   = 24;    ///< 24 pulses/rev (6 detents × 4)
    constexpr uint16_t RANGE = 270;   ///< 270° rotation range
    constexpr uint8_t  TICKS = 1;     ///< Event on every tick
    constexpr bool     INV   = true;  ///< Invert direction

    constexpr Def LEFT  { 10, 22, 23, PPR, RANGE, TICKS, INV };
    constexpr Def RIGHT { 11, 18, 19, PPR, RANGE, TICKS, INV };

    constexpr std::array ALL = { LEFT, RIGHT };
}

// ─────────────────────────────────────────────────────────────────────
// Buttons
//
// ButtonDef fields:
//   id        - Unique identifier for input binding
//   pin       - GPIO pin and source (MCU or MUX)
//   activeLow - true if pressed = LOW
// ─────────────────────────────────────────────────────────────────────

namespace Btn {
    using Def = oc::common::ButtonDef;
    using Src = oc::common::GpioPin::Source;

    constexpr Def MAIN { 100, {32, Src::MCU}, true };

    constexpr std::array ALL = { MAIN };
}

// ─────────────────────────────────────────────────────────────────────
// MIDI
// ─────────────────────────────────────────────────────────────────────

namespace Midi {
    constexpr uint8_t CHANNEL = 0;
    constexpr uint8_t ENC_CC  = 16;   ///< CC 16+ for encoders
    constexpr uint8_t BTN_CC  = 20;
}

}  // namespace Config
