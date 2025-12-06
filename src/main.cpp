/**
 * @file main.cpp
 * @brief Open Control - Teensy 4.1 LVGL Example
 */

#include <Arduino.h>
#include <lvgl.h>
#include <optional>

#include <oc/app/AppBuilder.hpp>
#include <oc/app/OpenControlApp.hpp>
#include <oc/teensy/Teensy.hpp>
#include <oc/teensy/Ili9341.hpp>
#include <oc/teensy/UsbMidi.hpp>
#include <oc/ui/bridge/LVGLBridge.hpp>

#include "Buffers.hpp"
#include "Config.hpp"
#include "ui/DemoView.hpp"

using namespace Config;

// ─────────────────────────────────────────────────────────────────────
// Objects
// ─────────────────────────────────────────────────────────────────────

static std::optional<oc::teensy::Ili9341> display;
static std::optional<oc::ui::LVGLBridge> lvgl;
static std::unique_ptr<oc::app::OpenControlApp> app;

// ─────────────────────────────────────────────────────────────────────
// Init
// ─────────────────────────────────────────────────────────────────────

static bool initDisplay() {
    display.emplace(oc::teensy::Ili9341Config{
        .width = Display::WIDTH,
        .height = Display::HEIGHT,
        .refreshRate = Timing::LVGL_HZ,
        .framebuffer = Buffers::framebuffer,
        .diffBuffer1 = Buffers::diff1,
        .diffBuffer1Size = sizeof(Buffers::diff1),
        .diffBuffer2 = Buffers::diff2,
        .diffBuffer2Size = sizeof(Buffers::diff2)
    });
    return display->init();
}

static bool initLVGL() {
    lv_init();
    lv_tick_set_cb([]() -> uint32_t { return millis(); });

    lvgl.emplace(oc::ui::LVGLBridgeConfig{
        .width = Display::WIDTH,
        .height = Display::HEIGHT,
        .buffer1 = Buffers::lvgl,
        .buffer2 = nullptr,
        .bufferSizeBytes = sizeof(Buffers::lvgl),
        .driver = &*display,
        .renderMode = LV_DISPLAY_RENDER_MODE_FULL
    });

    if (!lvgl->init()) return false;

    lv_timer_set_period(lv_display_get_refr_timer(lvgl->getDisplay()),
                        1000 / Timing::LVGL_HZ);
    return true;
}

static bool initApp() {
    using namespace oc::teensy;

    app.reset(new oc::app::OpenControlApp(
        oc::app::AppBuilder()
            .timeProvider(defaultTimeProvider)
            .midi(std::make_unique<UsbMidi>())
            .encoders(makeEncoderController(Enc::ALL))
            .buttons(makeButtonController(Btn::ALL, nullptr, Timing::DEBOUNCE_MS))
            .inputConfig({.longPressMs = Timing::LONG_PRESS_MS, .doubleTapWindowMs = Timing::DOUBLE_TAP_MS})
            .build()
    ));

    app->registerContext<ui::DemoView>("demo");
    return app->begin();
}

// ─────────────────────────────────────────────────────────────────────
// Arduino
// ─────────────────────────────────────────────────────────────────────

void setup() {
    while (!Serial && millis() < 3000) {}
    Serial.printf("\n[Open Control] App %luHz, LVGL %luHz\n\n", Timing::APP_HZ, Timing::LVGL_HZ);

    if (!initDisplay()) { Serial.println("[ERROR] Display"); while(1); }
    if (!initLVGL())    { Serial.println("[ERROR] LVGL");    while(1); }
    if (!initApp())     { Serial.println("[ERROR] App");     while(1); }
    
    Serial.println("[OK] Ready\n");
}

void loop() {
    static uint32_t lastUs = 0, lvglUs = 0;
    constexpr uint32_t appPeriod  = 1'000'000 / Timing::APP_HZ;
    constexpr uint32_t lvglPeriod = 1'000'000 / Timing::LVGL_HZ;

    uint32_t now = micros();
    if (now - lastUs < appPeriod) return;
    lastUs = now;

    app->update();

    lvglUs += appPeriod;
    if (lvglUs >= lvglPeriod) {
        lvglUs = 0;
        lvgl->refresh();
    }
}
