#pragma once

/**
 * @file DemoView.hpp
 * @brief LVGL demo context - showcases Open Control framework capabilities
 *
 * Demonstrates:
 * - IContext lifecycle (initialize, update, cleanup)
 * - Encoder bindings with onTurned callbacks
 * - Button bindings with onPressed/onReleased
 * - MIDI CC output
 * - Dynamic LVGL flex layout
 */

#include <Arduino.h>
#include <lvgl.h>
#include <oc/api/ControlAPI.hpp>
#include <oc/context/IContext.hpp>

#include "Config.hpp"

namespace ui {

/**
 * @class DemoView
 * @brief Interactive encoder/button demo with LVGL UI and MIDI output
 */
class DemoView : public oc::context::IContext {
public:
    static constexpr size_t ENC_COUNT = Config::Enc::ALL.size();

    // ─────────────────────────────────────────────────────────────────
    // IContext
    // ─────────────────────────────────────────────────────────────────

    bool initialize(oc::api::ControlAPI& api) override {
        api_ = &api;
        createUI();
        bindInputs();
        return true;
    }

    void update() override {}
    void cleanup() override {}
    const char* getName() const override { return "Demo"; }
    const char* getId() const override { return "demo"; }

private:
    // ─────────────────────────────────────────────────────────────────
    // UI
    // ─────────────────────────────────────────────────────────────────

    void createUI() {
        auto* scr = lv_screen_active();
        lv_obj_set_style_bg_color(scr, lv_color_black(), 0);
        lv_obj_set_style_pad_all(scr, 10, 0);

        auto* main = flexCol(scr, 8);
        lv_obj_set_size(main, LV_PCT(100), LV_PCT(100));

        createTitle(main);
        createEncoderRows(main);
        createFooter(main);
    }

    lv_obj_t* flexCol(lv_obj_t* parent, int gap) {
        auto* c = lv_obj_create(parent);
        lv_obj_set_size(c, LV_PCT(100), LV_SIZE_CONTENT);
        lv_obj_set_style_bg_opa(c, LV_OPA_TRANSP, 0);
        lv_obj_set_style_border_width(c, 0, 0);
        lv_obj_set_style_pad_all(c, 0, 0);
        lv_obj_set_flex_flow(c, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_flex_align(c, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_style_pad_row(c, gap, 0);
        return c;
    }

    lv_obj_t* flexRow(lv_obj_t* parent, int gap) {
        auto* r = lv_obj_create(parent);
        lv_obj_set_size(r, LV_PCT(100), LV_SIZE_CONTENT);
        lv_obj_set_style_bg_opa(r, LV_OPA_TRANSP, 0);
        lv_obj_set_style_border_width(r, 0, 0);
        lv_obj_set_style_pad_all(r, 4, 0);
        lv_obj_set_flex_flow(r, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(r, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_style_pad_column(r, gap, 0);
        return r;
    }

    void createTitle(lv_obj_t* parent) {
        auto* lbl = lv_label_create(parent);
        lv_label_set_text(lbl, "Open Control");
        lv_obj_set_style_text_color(lbl, lv_color_white(), 0);
        lv_obj_set_style_text_font(lbl, &lv_font_montserrat_16, 0);
    }

    void createEncoderRows(lv_obj_t* parent) {
        auto* container = flexCol(parent, 12);
        for (size_t i = 0; i < ENC_COUNT; ++i) {
            auto* row = flexRow(container, 8);

            // Label
            auto* name = lv_label_create(row);
            char buf[12];
            snprintf(buf, sizeof(buf), "Enc %zu", i + 1);
            lv_label_set_text(name, buf);
            lv_obj_set_style_text_color(name, lv_color_hex(0x888888), 0);
            lv_obj_set_width(name, 50);

            // Slider
            sliders_[i] = lv_slider_create(row);
            lv_obj_set_flex_grow(sliders_[i], 1);
            lv_obj_set_height(sliders_[i], 12);
            lv_slider_set_range(sliders_[i], 0, 100);
            lv_slider_set_value(sliders_[i], 50, LV_ANIM_OFF);
            lv_obj_set_style_bg_color(sliders_[i], lv_color_hex(0x333355), LV_PART_MAIN);
            lv_obj_set_style_bg_color(sliders_[i], lv_color_hex(0x6666ff), LV_PART_INDICATOR);
            lv_obj_set_style_bg_color(sliders_[i], lv_color_hex(0x9999ff), LV_PART_KNOB);
            lv_obj_set_style_pad_all(sliders_[i], 0, LV_PART_KNOB);

            // Value
            labels_[i] = lv_label_create(row);
            lv_label_set_text(labels_[i], "50%");
            lv_obj_set_style_text_color(labels_[i], lv_color_white(), 0);
            lv_obj_set_width(labels_[i], 40);
            lv_obj_set_style_text_align(labels_[i], LV_TEXT_ALIGN_RIGHT, 0);
        }
    }

    void createFooter(lv_obj_t* parent) {
        auto* lbl = lv_label_create(parent);
        lv_label_set_text(lbl, "Button: reset");
        lv_obj_set_style_text_color(lbl, lv_color_hex(0x555555), 0);
        lv_obj_set_style_text_font(lbl, &lv_font_montserrat_12, 0);
    }

    // ─────────────────────────────────────────────────────────────────
    // Bindings
    // ─────────────────────────────────────────────────────────────────

    void bindInputs() {
        using namespace Config;

        // Encoders -> UI + MIDI
        for (size_t i = 0; i < ENC_COUNT; ++i) {
            api_->onTurned(Enc::ALL[i].id, [this, i](float v) {
                int pct = int(v * 100);
                lv_slider_set_value(sliders_[i], pct, LV_ANIM_ON);
                char buf[8];
                snprintf(buf, sizeof(buf), "%d%%", pct);
                lv_label_set_text(labels_[i], buf);
                api_->sendCC(Midi::CHANNEL, Midi::ENC_CC + i, uint8_t(v * 127));
            });
        }

        // Button -> reset + MIDI
        api_->onPressed(Btn::MAIN.id, [this]() {
            for (size_t i = 0; i < ENC_COUNT; ++i) {
                lv_slider_set_value(sliders_[i], 50, LV_ANIM_ON);
                lv_label_set_text(labels_[i], "50%");
            }
            api_->sendCC(Midi::CHANNEL, Midi::BTN_CC, 127);
        });

        api_->onReleased(Btn::MAIN.id, [this]() {
            api_->sendCC(Midi::CHANNEL, Midi::BTN_CC, 0);
        });
    }

    // ─────────────────────────────────────────────────────────────────
    // Members
    // ─────────────────────────────────────────────────────────────────

    oc::api::ControlAPI* api_ = nullptr;
    lv_obj_t* sliders_[ENC_COUNT] = {};
    lv_obj_t* labels_[ENC_COUNT] = {};
};

}  // namespace ui
