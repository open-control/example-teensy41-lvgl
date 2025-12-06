#pragma once

/**
 * @file Config.hpp
 * @brief Hardware configuration for Open Control LVGL example
 */

#include <array>
#include <oc/common/ButtonDef.hpp>
#include <oc/common/EncoderDef.hpp>

namespace Config {

// ═══════════════════════════════════════════════════════════════════════════
// TIMING
// ═══════════════════════════════════════════════════════════════════════════

namespace Timing {
    constexpr uint32_t APP_HZ  = 2000;   ///< Main loop rate (encoder polling)
    constexpr uint32_t LVGL_HZ = 100;    ///< UI refresh rate

    constexpr uint32_t LONG_PRESS_MS = 500;
    constexpr uint32_t DOUBLE_TAP_MS = 300;
    constexpr uint8_t  DEBOUNCE_MS   = 5;
}

// ═══════════════════════════════════════════════════════════════════════════
// DISPLAY - ILI9341 via ILI9341_T4 library
//
// All fields have defaults in oc::teensy::Ili9341Config.
// Override only what differs from your hardware.
//
// TROUBLESHOOTING:
// - White screen: check SPI pins wiring, try lower spiSpeed (20MHz)
// - Flickering: increase vsyncSpacing (2-3), reduce refreshRate
// - Tearing: adjust lateStartRatio (0.2-0.5)
// - Glitches: ensure diffBuffers are in DMAMEM, increase diffGap
// - Slow refresh: check async flush, increase spiSpeed
// ═══════════════════════════════════════════════════════════════════════════

namespace Display {
    // ── Dimensions ──
    constexpr uint16_t WIDTH  = 320;
    constexpr uint16_t HEIGHT = 240;

    // ── Buffer sizes ──
    constexpr size_t BUFFER_SIZE = WIDTH * HEIGHT;
    constexpr size_t DIFF_SIZE   = 8192;  ///< 8KB recommended for 320x240

    // ── SPI Pins (Teensy 4.1) ──
    // Default: CS=28, DC=0, RST=29, MOSI=26, SCK=27, MISO=1
    // Uncomment to override:
    // constexpr uint8_t CS   = 28;
    // constexpr uint8_t DC   = 0;
    // constexpr uint8_t RST  = 29;
    // constexpr uint8_t MOSI = 26;
    // constexpr uint8_t SCK  = 27;
    // constexpr uint8_t MISO = 1;

    // ── SPI Speed ──
    // Default: 40 MHz. Reduce if unstable (20-30 MHz safer)
    // constexpr uint32_t SPI_SPEED = 40'000'000;

    // ── Display orientation ──
    // Default: rotation=3 (landscape, USB right), invertDisplay=true
    // constexpr uint8_t ROTATION = 3;      // 0-3
    // constexpr bool INVERT = true;

    // ── DMA/Refresh tuning ──
    // vsyncSpacing: frames between refreshes (1=60Hz, 2=30Hz)
    // diffGap: diff algorithm precision (4-8, higher=more precise)
    // irqPriority: DMA interrupt priority (lower=higher priority)
    // lateStartRatio: late start optimization (0.2-0.5)
    // refreshRate: target Hz for display sync
    //
    // constexpr uint8_t VSYNC_SPACING = 1;
    // constexpr uint8_t DIFF_GAP = 6;
    // constexpr uint8_t IRQ_PRIORITY = 128;
    // constexpr float LATE_START = 0.3f;
    // constexpr uint8_t REFRESH_HZ = 60;
}

// ═══════════════════════════════════════════════════════════════════════════
// ENCODERS
//
// EncoderDef fields:
//   id              Unique ID for api.onTurned(id, callback)
//   pinA, pinB      Quadrature encoder pins (use interrupt-capable pins)
//   ppr             Pulses per revolution (typically detents × 4)
//   rangeAngle      Physical rotation range in degrees (270° typical)
//   ticksPerEvent   Ticks to trigger callback (1=responsive, 4=smoother)
//   invertDirection Reverse direction if encoder wired backwards
//
// TROUBLESHOOTING:
// - Erratic values: swap pinA/pinB, check pullups, try ticksPerEvent=2
// - Wrong direction: set invertDirection=true
// - Too sensitive: increase ticksPerEvent
// - Missed steps: ensure pins support interrupts, check wiring
// ═══════════════════════════════════════════════════════════════════════════

namespace Enc {
    using Def = oc::common::EncoderDef;

    constexpr uint16_t PPR   = 24;    ///< 6 detents × 4 = 24 PPR
    constexpr uint16_t RANGE = 270;   ///< 270° physical range
    constexpr uint8_t  TICKS = 1;     ///< Callback every tick
    constexpr bool     INV   = true;  ///< Invert direction

    //              ID   pinA  pinB   PPR    RANGE  TICKS  INV
    constexpr Def LEFT  { 10,  22,   23,    PPR,   RANGE, TICKS, INV };
    constexpr Def RIGHT { 11,  18,   19,    PPR,   RANGE, TICKS, INV };

    constexpr std::array ALL = { LEFT, RIGHT };
}

// ═══════════════════════════════════════════════════════════════════════════
// BUTTONS
//
// ButtonDef fields:
//   id        Unique ID for api.onPressed(id, callback)
//   pin       {gpio, source} - source is MCU or MUX
//   activeLow true if pressed state = LOW (pullup config)
//
// TROUBLESHOOTING:
// - Not responding: check activeLow matches your wiring
// - Bouncing: increase Timing::DEBOUNCE_MS
// - Via multiplexer: use GpioPin::Source::MUX
// ═══════════════════════════════════════════════════════════════════════════

namespace Btn {
    using Def = oc::common::ButtonDef;
    using Src = oc::common::GpioPin::Source;

    //               ID    pin           activeLow
    constexpr Def MAIN { 100, {32, Src::MCU}, true };

    constexpr std::array ALL = { MAIN };
}

// ═══════════════════════════════════════════════════════════════════════════
// MIDI
// ═══════════════════════════════════════════════════════════════════════════

namespace Midi {
    constexpr uint8_t CHANNEL = 0;    ///< MIDI channel (0-15)
    constexpr uint8_t ENC_CC  = 16;   ///< CC number for first encoder
    constexpr uint8_t BTN_CC  = 20;   ///< CC number for button
}

}  // namespace Config
