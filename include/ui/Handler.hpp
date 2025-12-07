#pragma once

/**
 * @file Handler.hpp
 * @brief Input handler - binds hardware controls to UI views
 *
 * Architecture:
 *   Config.hpp  -->  Handler  -->  View
 *   (hardware)      (binding)     (display)
 *
 * Responsibilities:
 *   - Bind encoders to MIDI CC + UI update
 *   - Bind buttons to actions (reset, etc.)
 *   - Sync encoder positions to prevent value jumps
 */

#include "Config.hpp"

#include <oc/api/ButtonAPI.hpp>
#include <oc/api/EncoderAPI.hpp>
#include <oc/api/MidiAPI.hpp>

namespace ui {

/**
 * @brief Binds hardware controls to a view
 *
 * @tparam View Must implement:
 *   - static constexpr float DEFAULT_VALUE
 *   - void setEncoder(size_t index, float value)
 *   - void resetEncoderPositions()
 */
template <typename View>
class Handler {
public:
    static constexpr size_t ENC_COUNT = Config::Encoder::ALL.size();

    Handler(oc::api::ButtonAPI& buttons, oc::api::EncoderAPI& encoders,
            oc::api::MidiAPI& midi, View& view)
        : buttons_(buttons), encoders_(encoders), midi_(midi), view_(view) {}

    /// Bind all configured inputs
    void bind() {
        bindEncoders();
        bindButtons();
    }

private:
    oc::api::ButtonAPI& buttons_;
    oc::api::EncoderAPI& encoders_;
    oc::api::MidiAPI& midi_;
    View& view_;

    // ───────────────────────────────────────────────────────────────────
    // Encoders: turn -> MIDI CC + UI
    // ───────────────────────────────────────────────────────────────────

    void bindEncoders() {
        for (size_t i = 0; i < ENC_COUNT; ++i) {
            encoders_.encoder(Config::Encoder::ALL[i].id)
                .turn()
                .then([this, i](float value) {
                    sendEncoderCC(i, value);
                    view_.setEncoder(i, value);
                });
        }
    }

    void sendEncoderCC(size_t index, float value) {
        midi_.sendCC(
            Config::Binding::Midi::ENC_LEFT_CH,
            Config::Binding::Midi::ENC_LEFT_CC + index,
            uint8_t(value * Config::Binding::Midi::VALUE_MAX)
        );
    }

    // ───────────────────────────────────────────────────────────────────
    // Buttons: press/release -> MIDI CC + actions
    // ───────────────────────────────────────────────────────────────────

    void bindButtons() {
        // Main button: reset all encoders on press
        buttons_.button(Config::Binding::BTN_MAIN)
            .press()
            .then([this] {
                sendButtonCC(Config::Binding::Midi::VALUE_ON);
                resetAllEncoders();
            });

        buttons_.button(Config::Binding::BTN_MAIN)
            .release()
            .then([this] {
                sendButtonCC(Config::Binding::Midi::VALUE_OFF);
            });
    }

    void sendButtonCC(uint8_t value) {
        midi_.sendCC(Config::Binding::Midi::BTN_MAIN_CH, Config::Binding::Midi::BTN_MAIN_CC, value);
    }

    void resetAllEncoders() {
        // Sync hardware positions (prevents jump on next turn)
        for (size_t i = 0; i < ENC_COUNT; ++i) {
            encoders_.setPosition(Config::Encoder::ALL[i].id, View::DEFAULT_VALUE);
        }
        // Update UI
        view_.resetEncoderPositions();
    }
};

}  // namespace ui
