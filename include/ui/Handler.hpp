#pragma once

/**
 * @file Handler.hpp
 * @brief Input handler - connects Config bindings to UI actions
 *
 * Automatically binds all controls declared in Config.hpp to their
 * corresponding UI and MIDI actions. Adapts to the number of encoders
 * and buttons defined in the configuration.
 */

#include <oc/api/ControlAPI.hpp>

#include "Config.hpp"

namespace ui {

namespace Enc  = Config::Enc;
namespace Btn  = Config::Btn;
namespace Bind = Config::Bind;

/**
 * @brief Input handler template - binds controls to a view
 *
 * @tparam View Type with setEncoder(index, value), onButtonPress(), onButtonRelease()
 *
 * Usage:
 *   Handler<DemoView> handler(api, view);
 *   handler.bind();
 */
template <typename View>
class Handler {
public:
    static constexpr size_t ENC_COUNT = Enc::ALL.size();
    static constexpr size_t BTN_COUNT = Btn::ALL.size();

    Handler(oc::api::ControlAPI& api, View& view)
        : api_(api), view_(view) {}

    /**
     * @brief Bind all inputs from Config to view actions
     */
    void bind() {
        bindEncoders();
        bindButtons();
    }

private:
    // ─────────────────────────────────────────────────────────────────
    // Encoder bindings
    // ─────────────────────────────────────────────────────────────────

    void bindEncoders() {
        // Bind each encoder declared in Config::Enc::ALL
        for (size_t i = 0; i < ENC_COUNT; ++i) {
            api_.onTurned(Enc::ALL[i].id, [this, i](float value) {
                onEncoderTurn(i, value);
            });
        }
    }

    void onEncoderTurn(size_t index, float value) {
        // Send MIDI CC (base CC + index)
        api_.sendCC(
            Bind::Midi::ENC_LEFT_CH,
            Bind::Midi::ENC_LEFT_CC + index,
            uint8_t(value * Bind::Midi::VALUE_MAX)
        );

        // Update view
        view_.setEncoder(index, value);
    }

    // ─────────────────────────────────────────────────────────────────
    // Button bindings
    // ─────────────────────────────────────────────────────────────────

    void bindButtons() {
        // Main button - press/release
        api_.onPressed(Bind::BTN_MAIN, [this] {
            api_.sendCC(Bind::Midi::BTN_MAIN_CH, Bind::Midi::BTN_MAIN_CC, Bind::Midi::VALUE_ON);
            view_.resetEncoderPositions();
        });

        api_.onReleased(Bind::BTN_MAIN, [this] {
            api_.sendCC(Bind::Midi::BTN_MAIN_CH, Bind::Midi::BTN_MAIN_CC, Bind::Midi::VALUE_OFF);
        });
    }

    // ─────────────────────────────────────────────────────────────────
    // Members
    // ─────────────────────────────────────────────────────────────────

    oc::api::ControlAPI& api_;
    View& view_;
};

}  // namespace ui
