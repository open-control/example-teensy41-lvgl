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
 *
 * NOTE: Enable -D OC_LOG in platformio.ini build_flags to see debug output.
 *       Remove it for production (zero overhead, instant boot).
 *
 * Hardware configuration is in Config.hpp - ADAPT pins to your wiring.
 */

#include "Buffer.hpp"
#include "Config.hpp"
#include "context/StandaloneContext.hpp"

#include <optional>

#include <oc/teensy/Teensy.hpp>
#include <oc/app/OpenControlApp.hpp>
#include <oc/core/Result.hpp>

// ═══════════════════════════════════════════════════════════════════════════
// Static Objects
// ═══════════════════════════════════════════════════════════════════════════

/**
 * Static objects using std::optional for deferred initialization.
 * This pattern allows complex constructors while avoiding global init order issues.
 */
static std::optional<oc::teensy::Ili9341> display;
static std::optional<oc::ui::lvgl::Bridge> lvgl;
static std::optional<oc::app::OpenControlApp> app;

// ═══════════════════════════════════════════════════════════════════════════
// Initialization helpers
// ═══════════════════════════════════════════════════════════════════════════

// Check result and halt on error (embedded systems have no recovery)
static void checkOrHalt(const oc::core::Result<void>& result, const char* component) {
    if (!result) {
        OC_LOG_ERROR("{} init failed: {}", component, oc::core::errorCodeToString(result.error().code));
        while (true) {}
    }
}

static void initDisplay() {
    display = oc::teensy::Ili9341(
        Config::Display::CONFIG,
        {.framebuffer = Buffer::framebuffer, .diff1 = Buffer::diff1, .diff2 = Buffer::diff2});
    checkOrHalt(display->init(), "Display");
}

static void initLVGL() {
    lvgl = oc::ui::lvgl::Bridge(*display, Buffer::lvgl, oc::teensy::defaultTimeProvider, Config::LVGL::CONFIG);
    checkOrHalt(lvgl->init(), "LVGL");
}

static void initApp() {
    app = oc::teensy::AppBuilder()
        .midi()
        .encoders(Config::Encoder::ENCODERS)
        .buttons(Config::Button::BUTTONS, Config::Timing::DEBOUNCE_MS)
        .inputConfig(Config::Input::CONFIG);

    app->registerContext<context::StandaloneContext>(Config::ContextID::STANDALONE, "Standalone");
    app->begin();
}

// ═══════════════════════════════════════════════════════════════════════════
// Arduino Entry Points
// ═══════════════════════════════════════════════════════════════════════════

void setup() {
    OC_LOG_INFO("LVGL Example");

    initDisplay();
    initLVGL();
    initApp();

    OC_LOG_INFO("Ready");
}

// Timing constants for main loop
constexpr uint32_t APP_PERIOD_US = 1'000'000 / Config::Timing::APP_HZ;
constexpr uint32_t LVGL_PERIOD_US = 1'000'000 / Config::Timing::LVGL_HZ;

void loop() {
    static uint32_t lastMicros = 0;
    static uint32_t lvglAccumulator = 0;

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
