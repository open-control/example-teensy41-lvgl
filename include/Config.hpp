#pragma once

/**
 * @file Config.hpp
 * @brief Hardware configuration for Open Control Teensy 4.1 LVGL Example
 *
 * Pure compile-time configuration. No object creation, no runtime pointers.
 * Buffer sizes are auto-calculated from display dimensions.
 *
 * Modify these values to match your hardware setup.
 */

#include <array>
#include <lvgl.h>

#include <oc/common/ButtonDef.hpp>
#include <oc/common/EncoderDef.hpp>
#include <oc/core/input/InputConfig.hpp>
#include <oc/teensy/Ili9341.hpp>
#include <oc/ui/lvgl/Bridge.hpp>

namespace Config {

// ═══════════════════════════════════════════════════════════════════════════
// TIMING
// ═══════════════════════════════════════════════════════════════════════════

/**
 * System timing constants controlling responsiveness vs CPU load.
 *
 * APP_HZ controls encoder/button polling. Too low = missed encoder steps.
 * LVGL_HZ controls UI refresh. Must be <= APP_HZ.
 */
namespace Timing {
constexpr uint32_t APP_HZ = 2000;  // WARNING: Below 1000 Hz may miss encoder steps at fast rotation
constexpr uint32_t LVGL_HZ = 100;

constexpr uint16_t LONG_PRESS_MS = 500;
constexpr uint16_t DOUBLE_TAP_MS = 300;
constexpr uint8_t DEBOUNCE_MS = 5;  // Increase to 10-20 if buttons trigger multiple times
}

// ═══════════════════════════════════════════════════════════════════════════
// CONTEXT IDS
// ═══════════════════════════════════════════════════════════════════════════

/**
 * User-defined context identifiers.
 *
 * Used for type-safe context registration and switching.
 * Values must be < 16 (MAX_CONTEXTS).
 *
 * _COUNT is optional but enables compile-time array sizing.
 */
enum class ContextID : uint8_t {
    STANDALONE = 0,
    // Add more contexts here:
    // BITWIG = 1,
    // ABLETON = 2,
    _COUNT
};

// ═══════════════════════════════════════════════════════════════════════════
// DISPLAY
// ═══════════════════════════════════════════════════════════════════════════

/**
 * ILI9341 320x240 TFT display with DMA acceleration.
 *
 * Uses SPI1 on Teensy 4.1 for optimal DMA performance.
 * Requires DMAMEM buffers defined in Buffer.hpp.
 *
 * Troubleshooting:
 *   - White screen: Check SPI wiring and spiSpeed (try 20 MHz)
 *   - Wrong colors: Toggle invertDisplay
 *   - Flickering: Reduce spiSpeed or increase vsyncSpacing
 *   - Tearing: Adjust refreshRate to match actual display capability and/or increase vsync to 2
 */
namespace Display {
constexpr oc::teensy::Ili9341Config CONFIG = {
    .width = 320,
    .height = 240,

    .csPin = 28,
    .dcPin = 0,
    .rstPin = 29,
    .mosiPin = 26,  // SPI1 MOSI - change to 11 for SPI0
    .sckPin = 27,   // SPI1 SCK  - change to 13 for SPI0
    .misoPin = 1,   // SPI1 MISO - change to 12 for SPI0

    .spiSpeed = 40'000'000,  // WARNING: Above 40 MHz may cause artifacts with long wires

    .rotation = 3,          // 0-3: 90° increments. 3 = landscape
    .invertDisplay = true,  // Toggle if colors are inverted

    .vsyncSpacing = 1,
    .refreshRate = Timing::LVGL_HZ * CONFIG.vsyncSpacing};

constexpr size_t BUFFER_SIZE = CONFIG.framebufferSize();
constexpr size_t DIFF_SIZE = CONFIG.recommendedDiffSize();
}

// ═══════════════════════════════════════════════════════════════════════════
// LVGL BRIDGE
// ═══════════════════════════════════════════════════════════════════════════

/**
 * LVGL display bridge configuration.
 *
 * Handles lv_init(), tick callback, and display setup internally.
 * Call init() once after display is initialized.
 *
 * Memory usage (320x240 RGB565):
 *   - FULL mode:    ~150 KB (best quality and performances, no flicker)
 *   - PARTIAL mode: ~20-40 KB (may flicker on fast animations)
 */
namespace LVGL {
constexpr oc::ui::lvgl::BridgeConfig CONFIG = {
    .renderMode = LV_DISPLAY_RENDER_MODE_FULL,  // Change to PARTIAL if RAM is tight but you may
                                                // have with less deterministic timing and tearing
    .buffer2 = nullptr,                         // Buffering is optimized at driver level with ILI9341_T4 dep in the
                                                // framework driver (only compatible w/ Teensy 4.x)
    .refreshHz = Timing::LVGL_HZ};
}

// ═══════════════════════════════════════════════════════════════════════════
// ENCODERS
// ═══════════════════════════════════════════════════════════════════════════

/**
 * Quadrature rotary encoders with interrupt-driven decoding.
 *
 * SINGLE SOURCE OF TRUTH: Duplicate a line to add an encoder.
 * Auto-generates: MIDI CC (Midi::ENC_CC_RANGE_START + index), UI slider.
 *
 * Definition: { id, pinA, pinB, ppr, rangeAngle, ticksPerEvent, invertDirection }
 *
 * Common issues:
 *   - Erratic values: Check PPR matches datasheet, increase APP_HZ
 *   - Wrong direction: Set invertDirection = true
 *   - Skipping steps: Reduce ticksPerEvent or increase APP_HZ
 */
namespace Encoder {
using namespace oc::common;

// Shared parameters
constexpr uint16_t PPR = 24;  // CRITICAL: Must match encoder datasheet
constexpr uint16_t RANGE = 270;
constexpr uint8_t TICKS = 1;  // 1 = every pulse, 4 = every detent
constexpr bool INVERT = true;

constexpr std::array ENCODERS = {
    //         id  pinA pinB ppr  range  ticks  invert
    EncoderDef{10, 22, 23, PPR, RANGE, TICKS, INVERT},  // -> CC 10, ENC 1
    EncoderDef{11, 18, 19, PPR, RANGE, TICKS, INVERT},  // -> CC 11, ENC 2
    // Adjust to your needs, add more encoders here...
};
}

// ═══════════════════════════════════════════════════════════════════════════
// BUTTONS
// ═══════════════════════════════════════════════════════════════════════════

/**
 * Push buttons with debouncing and gesture detection.
 *
 * SINGLE SOURCE OF TRUTH: Duplicate a line to add a button.
 * Auto-generates: MIDI CC (Midi::BTN_CC_RANGE_START + index).
 *
 * Definition: { id, {pin, source}, activeLow }
 * Source: MCU (direct GPIO) or MUX (via multiplexer)
 */
namespace Button {
using namespace oc::common;
using Source = oc::hal::GpioPin::Source;

constexpr std::array BUTTONS = {
    //        id    pin source        activeLow
    ButtonDef{100, {32, Source::MCU}, true},  // -> CC 60, BTN 1
    // Adjust to your needs, add more buttons here...
};
}

// ═══════════════════════════════════════════════════════════════════════════
// MIDI
// ═══════════════════════════════════════════════════════════════════════════

/**
 * USB MIDI output configuration.
 *
 * Requires -D USB_MIDI_SERIAL in platformio.ini build_flags.
 * CC numbers 0-13 are reserved (bank select, mod wheel, etc).
 */
namespace Midi {
constexpr uint8_t CHANNEL = 0;              // 0-15, DAWs display as 1-16
constexpr uint8_t BTN_CC_RANGE_START = 10;  // Buttons: CC 10, 11, 12...
constexpr uint8_t ENC_CC_RANGE_START = 60;  // Encoders: CC 60, 61, 62...
}

// ═══════════════════════════════════════════════════════════════════════════
// INPUT
// ═══════════════════════════════════════════════════════════════════════════

/**
 * Input gesture detection configuration.
 *
 * longPressMs: Duration to trigger long press (300-800 ms typical)
 * doubleTapWindowMs: Max gap between taps (too long delays single-tap response)
 */
namespace Input {
constexpr oc::core::InputConfig CONFIG = {.longPressMs = Timing::LONG_PRESS_MS,
                                          .doubleTapWindowMs = Timing::DOUBLE_TAP_MS};
}

}  // namespace Config
