/**
 * @file main.cpp
 * @brief Open Control - Teensy 4.1 LVGL Example
 */

#include "Buffer.hpp"
#include "Config.hpp"

#include <optional>

#include <Arduino.h>
#include <oc/app/AppBuilder.hpp>
#include <oc/app/OpenControlApp.hpp>
#include <oc/teensy/Teensy.hpp>
#include <oc/teensy/UsbMidi.hpp>

#include "ui/DemoView.hpp"

using namespace Config;

// ─────────────────────────────────────────────────────────────────────
// Objects
// ─────────────────────────────────────────────────────────────────────

static std::optional<oc::teensy::Ili9341> display;
static std::optional<oc::ui::LVGLBridge> lvgl;
static std::optional<oc::app::OpenControlApp> app;

// ─────────────────────────────────────────────────────────────────────
// Init
// ─────────────────────────────────────────────────────────────────────

static bool initDisplay() {
    display = oc::teensy::Ili9341(
        Display::CONFIG,
        {.framebuffer = Buffer::framebuffer, .diff1 = Buffer::diff1, .diff2 = Buffer::diff2});
    return display->init();
}

static bool initLVGL() {
    lvgl = oc::ui::LVGLBridge(*display, Buffer::lvgl, millis, LVGL::CONFIG);
    return lvgl->init();
}

static bool initApp() {
    using namespace oc::teensy;

    app = oc::app::AppBuilder()
              .timeProvider(defaultTimeProvider)
              .midi(std::make_unique<UsbMidi>())
              .encoders(makeEncoderController(Enc::ALL))
              .buttons(makeButtonController(Btn::ALL, nullptr, Timing::DEBOUNCE_MS))
              .inputConfig(Input::CONFIG)
              .build();

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
