/**
 * @file main.cpp
 * @brief Open Control - Teensy 4.1 LVGL Example
 *
 * Demonstrates:
 * - ILI9341 display with async DMA (ILI9341_T4)
 * - LVGL 9.x integration
 * - Encoder/button input handling
 * - MIDI CC output
 */

#include <Arduino.h>
#include <lvgl.h>
#include <optional>

#include <oc/app/AppBuilder.hpp>
#include <oc/app/OpenControlApp.hpp>
#include <oc/teensy/ButtonController.hpp>
#include <oc/teensy/EncoderController.hpp>
#include <oc/teensy/Ili9341.hpp>
#include <oc/teensy/UsbMidi.hpp>
#include <oc/ui/bridge/LVGLBridge.hpp>

#include "Buffers.hpp"
#include "Config.hpp"
#include "ui/DemoView.hpp"

// ─────────────────────────────────────────────────────────────────────
// Static Objects
// ─────────────────────────────────────────────────────────────────────

static std::optional<oc::teensy::Ili9341> display;
static std::optional<oc::ui::LVGLBridge> lvglBridge;
static std::unique_ptr<oc::app::OpenControlApp> app;

// ─────────────────────────────────────────────────────────────────────
// Initialization
// ─────────────────────────────────────────────────────────────────────

static bool initDisplay() {
    using namespace Config;

    display.emplace(oc::teensy::Ili9341Config{
        .width = DISPLAY_WIDTH,
        .height = DISPLAY_HEIGHT,
        .csPin = DisplayPins::CS,
        .dcPin = DisplayPins::DC,
        .rstPin = DisplayPins::RST,
        .mosiPin = DisplayPins::MOSI,
        .sckPin = DisplayPins::SCK,
        .misoPin = DisplayPins::MISO,
        .spiSpeed = DisplayPins::SPI_SPEED,
        .rotation = DisplaySettings::ROTATION,
        .invertDisplay = DisplaySettings::INVERT,
        .vsyncSpacing = DisplaySettings::VSYNC_SPACING,
        .diffGap = DisplaySettings::DIFF_GAP,
        .irqPriority = DisplaySettings::IRQ_PRIORITY,
        .lateStartRatio = DisplaySettings::LATE_START_RATIO,
        .refreshRate = Timing::LVGL_REFRESH_HZ * DisplaySettings::VSYNC_SPACING,
        .framebuffer = Buffers::displayFramebuffer,
        .diffBuffer1 = Buffers::diffBuffer1,
        .diffBuffer1Size = sizeof(Buffers::diffBuffer1),
        .diffBuffer2 = Buffers::diffBuffer2,
        .diffBuffer2Size = sizeof(Buffers::diffBuffer2)
    });

    return display->init();
}

static bool initLVGL() {
    lv_init();
    lv_tick_set_cb([]() -> uint32_t { return millis(); });

    lvglBridge.emplace(oc::ui::LVGLBridgeConfig{
        .width = Config::DISPLAY_WIDTH,
        .height = Config::DISPLAY_HEIGHT,
        .buffer1 = Buffers::lvglDrawBuffer,
        .buffer2 = nullptr,
        .bufferSizeBytes = sizeof(Buffers::lvglDrawBuffer),
        .driver = &*display,
        .renderMode = LV_DISPLAY_RENDER_MODE_FULL
    });

    if (!lvglBridge->init()) return false;

    lv_timer_set_period(lv_display_get_refr_timer(lvglBridge->getDisplay()),
                        Config::Timing::LVGL_PERIOD_US / 1000);
    return true;
}

static bool initApp() {
    using namespace Config;

    app.reset(new oc::app::OpenControlApp(
        oc::app::AppBuilder()
            .timeProvider(millis)
            .midi(std::make_unique<oc::teensy::UsbMidi>())
            .encoders(std::make_unique<oc::teensy::EncoderController<Enc::ALL.size()>>(Enc::ALL))
            .buttons(std::make_unique<oc::teensy::ButtonController<Btn::ALL.size()>>(Btn::ALL, nullptr, Timing::DEBOUNCE_MS))
            .inputConfig({.longPressMs = Timing::LONG_PRESS_MS, .doubleTapWindowMs = Timing::DOUBLE_TAP_MS})
            .build()
    ));

    app->registerContext<ui::DemoView>("demo");
    return app->begin();
}

// ─────────────────────────────────────────────────────────────────────
// Arduino Entry Points
// ─────────────────────────────────────────────────────────────────────

void setup() {
    while (!Serial && millis() < 3000) {}

    Serial.println("\n[Open Control] LVGL Example");
    Serial.printf("App: %luHz, LVGL: %luHz\n\n",
                  Config::Timing::APP_REFRESH_HZ, Config::Timing::LVGL_REFRESH_HZ);

    if (!initDisplay()) { Serial.println("[ERROR] Display"); while (true) delay(1000); }
    Serial.println("[OK] Display");

    if (!initLVGL()) { Serial.println("[ERROR] LVGL"); while (true) delay(1000); }
    Serial.println("[OK] LVGL");

    if (!initApp()) { Serial.println("[ERROR] App"); while (true) delay(1000); }
    Serial.println("[OK] App\n");
}

void loop() {
    static uint32_t lastUs = 0;
    static uint32_t lvglAccum = 0;

    uint32_t nowUs = micros();
    uint32_t deltaUs = nowUs - lastUs;

    if (deltaUs < Config::Timing::APP_PERIOD_US) return;
    lastUs = nowUs;

    app->update();

    lvglAccum += deltaUs;
    if (lvglAccum >= Config::Timing::LVGL_PERIOD_US) {
        lvglAccum = 0;
        lvglBridge->refresh();
    }
}
