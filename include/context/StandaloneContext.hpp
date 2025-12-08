#pragma once

/**
 * @file StandaloneContext.hpp
 * @brief Standalone application context implementing IContext
 *
 * Demonstrates the IContext interface from the framework:
 *   - IContext: Application mode with lifecycle management
 *   - Receives initialize/update/cleanup callbacks
 *   - Access APIs via protected accessors (onButton, onEncoder, midi, etc.)
 *
 * IContext is NOT a UI concept - it's an application orchestrator.
 * A context can manage multiple views, handle state machines,
 * coordinate with external systems (DAW, network, etc.).
 *
 * Use cases:
 *   - Standalone mode (this example)
 *   - DAW integration mode
 *   - Configuration/calibration mode
 *   - Demo/attract mode
 */

#include "Config.hpp"
#include "handler/Handler.hpp"
#include "ui/view/DemoView.hpp"

#include <oc/context/IContext.hpp>
#include <oc/context/Requirements.hpp>

namespace context {

/**
 * @brief Standalone mode context
 *
 * Orchestrates:
 *   - View creation and lifecycle (DemoView)
 *   - Input bindings via Handler
 *   - MIDI output
 *
 * Uses direct members with two-phase initialization:
 *   - View and Handler are default-constructed as members
 *   - Actual setup happens in initialize() when APIs are available
 */
class StandaloneContext : public oc::context::IContext {
public:
    /// Declare required APIs (validated at registration time)
    static constexpr oc::context::Requirements REQUIRES{
        .button = true,
        .encoder = true,
        .midi = true
    };

    // ═══════════════════════════════════════════════════════════════════
    // IContext Lifecycle
    // ═══════════════════════════════════════════════════════════════════

    bool initialize() override {
        view_.onActivate();
        handler_.setup(buttons(), encoders(), midi(), view_);
        return true;
    }

    void update() override {
        // View updates handled by LVGL refresh
    }

    void cleanup() override {
        view_.onDeactivate();
    }

    const char* getName() const override { return "Standalone"; }

private:
    ui::DemoView view_;
    handler::Handler<ui::DemoView> handler_;
};

}  // namespace context
