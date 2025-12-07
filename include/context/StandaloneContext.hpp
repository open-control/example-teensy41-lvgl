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
#include "ui/Handler.hpp"
#include "ui/view/DemoView.hpp"

#include <memory>

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
 * Registered with OpenControlApp in main.cpp:
 *   app.registerContext<StandaloneContext>(ContextID::STANDALONE);
 *
 * @code
 * // Framework calls these automatically:
 * context.setAPIs(apis);   // Inject API references
 * context.initialize();    // Create views, bind inputs
 * context.update();        // Called every frame
 * context.cleanup();       // Destroy views, release resources
 * @endcode
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

    /**
     * @brief Initialize context
     *
     * Called by framework when context becomes active.
     * APIs are already available via protected accessors.
     * Create views, set up input bindings, initialize state.
     */
    bool initialize() override {
        // Create view
        view_ = std::make_unique<ui::DemoView>();
        view_->onActivate();

        // Bind inputs to view
        handler_ = std::make_unique<ui::Handler<ui::DemoView>>(
            buttons(), encoders(), midi(), *view_);
        handler_->bind();

        return true;
    }

    void update() override {
        // View updates handled by LVGL refresh
    }

    void cleanup() override {
        handler_.reset();
        if (view_) {
            view_->onDeactivate();
        }
        view_.reset();
    }

    // ── IContext Identity ──

    const char* getName() const override { return "Standalone"; }

private:
    std::unique_ptr<ui::DemoView> view_;
    std::unique_ptr<ui::Handler<ui::DemoView>> handler_;
};

}  // namespace context
