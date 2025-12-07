#pragma once

/**
 * @file Handler.hpp
 * @brief Input handler with auto-generated MIDI bindings
 *
 * Auto-generates MIDI CC mappings from array indices:
 *   - Encoder[i] -> Config::Midi::ENC_CC_RANGE_START + i
 *   - Button[i]  -> Config::Midi::BTN_CC_RANGE_START + i
 *
 * Architecture:
 *   Config::Encoder::ALL  -->  Handler  -->  MIDI + View
 *   Config::Button::ALL   -->  (auto)   -->  (callbacks)
 */

#include "Config.hpp"

#include <oc/api/ButtonAPI.hpp>
#include <oc/api/EncoderAPI.hpp>
#include <oc/api/MidiAPI.hpp>

namespace handler {

/**
 * @brief Auto-binding input handler
 *
 * Iterates over Config::Encoder::ENCODERS and Config::Button::BUTTONS,
 * automatically generating MIDI CC bindings and view updates.
 *
 * @tparam View Must implement:
 *   - static constexpr float DEFAULT_VALUE
 *   - void setButton(size_t index, bool pressed)
 *   - void setEncoder(size_t index, float value)
 *   - void resetEncoderPositions()
 */
template <typename View>
class Handler {
public:
    static constexpr size_t ENCODER_COUNT = Config::Encoder::ENCODERS.size();
    static constexpr size_t BUTTON_COUNT = Config::Button::BUTTONS.size();

    Handler(oc::api::ButtonAPI& buttons, oc::api::EncoderAPI& encoders,
            oc::api::MidiAPI& midi, View& view)
        : buttons_(buttons), encoders_(encoders), midi_(midi), view_(view) {}

    /// Auto-bind all encoders and buttons from Config arrays
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
    // Encoders: auto-bind ENCODERS[] -> MIDI CC + view
    // ───────────────────────────────────────────────────────────────────

    void bindEncoders() {
        for (size_t i = 0; i < ENCODER_COUNT; ++i) {
            encoders_.encoder(Config::Encoder::ENCODERS[i].id)
                .turn()
                .then([this, i](float value) {
                    sendEncoderCC(i, value);
                    view_.setEncoder(i, value);
                });
        }
    }

    /// Auto-generated: Config::Midi::ENC_CC_RANGE_START + index
    void sendEncoderCC(size_t index, float value) {
        midi_.sendCC(
            Config::Midi::CHANNEL,
            Config::Midi::ENC_CC_RANGE_START + index,
            uint8_t(value * 127)
        );
    }

    // ───────────────────────────────────────────────────────────────────
    // Buttons: auto-bind BUTTONS[] -> MIDI CC + actions
    // ───────────────────────────────────────────────────────────────────

    void bindButtons() {
        for (size_t i = 0; i < BUTTON_COUNT; ++i) {
            auto id = Config::Button::BUTTONS[i].id;

            buttons_.button(id)
                .press()
                .then([this, i] {
                    sendButtonCC(i, 127);
                    view_.setButton(i, true);
                    onButtonPress(i);
                });

            buttons_.button(id)
                .release()
                .then([this, i] {
                    sendButtonCC(i, 0);
                    view_.setButton(i, false);
                });
        }
    }

    /// Auto-generated: Config::Midi::BTN_CC_RANGE_START + index
    void sendButtonCC(size_t index, uint8_t value) {
        midi_.sendCC(
            Config::Midi::CHANNEL,
            Config::Midi::BTN_CC_RANGE_START + index,
            value
        );
    }

    /// Button actions - customize per button index
    void onButtonPress(size_t index) {
        if (index == 0) {
            resetAllEncoders();
        }
    }

    void resetAllEncoders() {
        for (size_t i = 0; i < ENCODER_COUNT; ++i) {
            encoders_.setPosition(Config::Encoder::ENCODERS[i].id, View::DEFAULT_VALUE);
        }
        view_.resetEncoderPositions();
    }
};

}  // namespace handler
