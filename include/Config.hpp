#pragma once

/**
 * @file Config.hpp
 * @brief Hardware configuration for Open Control LVGL example
 *
 * Pure configuration data - all values are compile-time constants.
 * No object creation, no runtime pointers.
 */

#include <array>
#include <lvgl.h>

#include <oc/common/ButtonDef.hpp>
#include <oc/common/EncoderDef.hpp>
#include <oc/core/input/InputConfig.hpp>
#include <oc/teensy/Ili9341.hpp>
#include <oc/ui/bridge/LVGLBridge.hpp>

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
// DISPLAY
// ═══════════════════════════════════════════════════════════════════════════

namespace Display {
    constexpr oc::teensy::Ili9341Config CONFIG = {
        .width = 320,
        .height = 240,
        .csPin = 28,
        .dcPin = 0,
        .rstPin = 29,
        .mosiPin = 26,
        .sckPin = 27,
        .misoPin = 1,
        .spiSpeed = 40'000'000,
        .rotation = 3,
        .invertDisplay = true,
        .vsyncSpacing = 2,
        .refreshRate = Timing::LVGL_HZ * 2
    };

    // Buffer sizes derived from config
    constexpr size_t BUFFER_SIZE = CONFIG.framebufferSize();
    constexpr size_t DIFF_SIZE = CONFIG.recommendedDiffSize();
}

// ═══════════════════════════════════════════════════════════════════════════
// LVGL BRIDGE
// ═══════════════════════════════════════════════════════════════════════════

namespace LVGL {
    constexpr oc::ui::LVGLBridgeConfig CONFIG = {
        .renderMode = LV_DISPLAY_RENDER_MODE_FULL,
        .buffer2 = nullptr,
        .refreshHz = Timing::LVGL_HZ
    };
}

// ═══════════════════════════════════════════════════════════════════════════
// ENCODERS
// ═══════════════════════════════════════════════════════════════════════════

namespace Enc {
    using Def = oc::common::EncoderDef;

    constexpr uint16_t PPR   = 24;
    constexpr uint16_t RANGE = 270;
    constexpr uint8_t  TICKS = 1;
    constexpr bool     INV   = true;

    constexpr Def LEFT  { 10, 22, 23, PPR, RANGE, TICKS, INV };
    constexpr Def RIGHT { 11, 18, 19, PPR, RANGE, TICKS, INV };

    constexpr std::array ALL = { LEFT, RIGHT };
}

// ═══════════════════════════════════════════════════════════════════════════
// BUTTONS
// ═══════════════════════════════════════════════════════════════════════════

namespace Btn {
    using Def = oc::common::ButtonDef;
    using Src = oc::hal::GpioPin::Source;

    constexpr Def MAIN { 100, {32, Src::MCU}, true };

    constexpr std::array ALL = { MAIN };
}

// ═══════════════════════════════════════════════════════════════════════════
// MIDI
// ═══════════════════════════════════════════════════════════════════════════

namespace Midi {
    constexpr uint8_t CHANNEL = 0;
    constexpr uint8_t ENC_CC  = 16;
    constexpr uint8_t BTN_CC  = 20;
}

// ═══════════════════════════════════════════════════════════════════════════
// INPUT
// ═══════════════════════════════════════════════════════════════════════════

namespace Input {
    constexpr oc::core::InputConfig CONFIG = {
        .longPressMs = Timing::LONG_PRESS_MS,
        .doubleTapWindowMs = Timing::DOUBLE_TAP_MS
    };
}

}  // namespace Config
