/**
 * @file main.cpp
 * @brief Open Control Framework - Teensy 4.1 LVGL Example
 *
 * This example demonstrates the Open Control framework with:
 * - ILI9341 TFT display driven by DMA for flicker-free rendering
 * - LVGL graphics library for the user interface
 * - Rotary encoders sending MIDI CC messages
 * - Button triggering encoder reset
 *
 * Architecture:
 * - Display and LVGL are initialized first (static lifetime)
 * - OpenControlApp manages hardware polling and context lifecycle
 * - StandaloneContext creates the UI and binds inputs to MIDI
 *
 * The main loop runs at APP_HZ for responsive encoder tracking,
 * while LVGL refreshes at the lower LVGL_HZ to save CPU cycles.
 */

#include "Buffer.hpp"
#include "Config.hpp"
#include "context/StandaloneContext.hpp"

#include <optional>

#include <Arduino.h>
#include <oc/app/AppBuilder.hpp>
#include <oc/app/OpenControlApp.hpp>
#include <oc/teensy/Teensy.hpp>
#include <oc/teensy/UsbMidi.hpp>

// ═══════════════════════════════════════════════════════════════════════════
// Static Objects
// ═══════════════════════════════════════════════════════════════════════════

/**
 * Static objects using std::optional for deferred initialization.
 * This pattern allows complex constructors while avoiding global init order issues.
 */
static std::optional<oc::teensy::Ili9341> display;
static std::optional<oc::ui::LVGLBridge> lvgl;
static std::optional<oc::app::OpenControlApp> app;

// ═══════════════════════════════════════════════════════════════════════════
// Initialization
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief Initialize ILI9341 display with DMA acceleration
 *
 * Uses triple buffering (framebuffer + 2 diff buffers) for tear-free updates.
 * The driver handles SPI DMA transfers in the background.
 *
 * @return true if display initialized successfully
 */
static bool initDisplay() {
    display = oc::teensy::Ili9341(
        Config::Display::CONFIG,
        {.framebuffer = Buffer::framebuffer, .diff1 = Buffer::diff1, .diff2 = Buffer::diff2}
    );
    return display->init();
}

/**
 * @brief Initialize LVGL graphics library
 *
 * LVGLBridge handles lv_init(), tick callback, and display driver setup.
 * Uses the LVGL buffer allocated in DMAMEM for optimal performance.
 *
 * @return true if LVGL initialized successfully
 */
static bool initLVGL() {
    lvgl = oc::ui::LVGLBridge(*display, Buffer::lvgl, millis, Config::LVGL::CONFIG);
    return lvgl->init();
}

/**
 * @brief Build and initialize the Open Control application
 *
 * Creates the application with all hardware drivers, registers the
 * standalone context, and starts the context lifecycle.
 *
 * @return true if application started successfully
 */
static bool initApp() {
    using namespace oc::teensy;

    app = oc::app::AppBuilder()
        .timeProvider(defaultTimeProvider)
        .midi(std::make_unique<UsbMidi>())
        .encoders(makeEncoderController(Config::Encoder::ALL))
        .buttons(makeButtonController(Config::Button::ALL, nullptr, Config::Timing::DEBOUNCE_MS))
        .inputConfig(Config::Input::CONFIG)
        .build();

    app->registerContext<context::StandaloneContext>(Config::ContextID::STANDALONE);
    return app->begin();
}

// ═══════════════════════════════════════════════════════════════════════════
// Arduino Entry Points
// ═══════════════════════════════════════════════════════════════════════════

void setup() {
    // Wait for serial with timeout for standalone operation
    while (!Serial && millis() < 3000) {}

    Serial.printf("\n[Open Control] App %luHz, LVGL %luHz\n\n",
                  Config::Timing::APP_HZ, Config::Timing::LVGL_HZ);

    // Initialize in dependency order, halt on failure
    if (!initDisplay()) { Serial.println("[ERROR] Display init failed"); while (true); }
    if (!initLVGL())    { Serial.println("[ERROR] LVGL init failed");    while (true); }
    if (!initApp())     { Serial.println("[ERROR] App init failed");     while (true); }

    Serial.println("[OK] Ready\n");
}

void loop() {
    // Timing: run app at APP_HZ, LVGL at LVGL_HZ
    static uint32_t lastMicros = 0;
    static uint32_t lvglAccumulator = 0;

    constexpr uint32_t APP_PERIOD_US = 1'000'000 / Config::Timing::APP_HZ;
    constexpr uint32_t LVGL_PERIOD_US = 1'000'000 / Config::Timing::LVGL_HZ;

    const uint32_t now = micros();
    if (now - lastMicros < APP_PERIOD_US) return;
    lastMicros = now;

    // Poll hardware and update active context
    app->update();

    // Refresh LVGL at lower frequency to reduce CPU load
    lvglAccumulator += APP_PERIOD_US;
    if (lvglAccumulator >= LVGL_PERIOD_US) {
        lvglAccumulator = 0;
        lvgl->refresh();
    }
}
