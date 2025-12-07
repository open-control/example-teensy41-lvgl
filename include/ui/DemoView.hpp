#pragma once

/**
 * @file DemoView.hpp
 * @brief LVGL demo view - pure UI component
 *
 * Displays encoder sliders and responds to input events.
 * Input binding and MIDI output are handled by Handler.hpp.
 *
 * Adapts automatically to the number of encoders in Config::Enc::ALL.
 */

#include <Arduino.h>
#include <lvgl.h>
#include <string>

#include <oc/api/ControlAPI.hpp>
#include <oc/context/IContext.hpp>

#include "Config.hpp"
#include "Handler.hpp"

namespace ui {

namespace Enc = Config::Enc;

/**
 * @class DemoView
 * @brief Interactive encoder/button demo with LVGL UI
 *
 * Public interface for Handler:
 *   - setEncoder(index, value): update slider + label
 *   - onButtonPress(): reset all encoders to default
 *   - onButtonRelease(): (no-op, but called by handler)
 */
class DemoView : public oc::context::IContext {

    struct Colors {
        static constexpr uint32_t LABEL_DIM   = 0x888888;
        static constexpr uint32_t LABEL_MUTED = 0x555555;
        static constexpr uint32_t SLIDER_BG   = 0x333355;
        static constexpr uint32_t SLIDER_FILL = 0x6666ff;
        static constexpr uint32_t SLIDER_KNOB = 0x9999ff;
    };

public:
    static constexpr size_t ENC_COUNT = Enc::ALL.size();
    static constexpr int32_t SLIDER_MAX = 100;
    static constexpr float DEFAULT_VALUE = 0.5f;

    // ─────────────────────────────────────────────────────────────────
    // IContext
    // ─────────────────────────────────────────────────────────────────

    bool initialize(oc::api::ControlAPI& api) override {
        createUI();
        handler_.emplace(api, *this);
        handler_->bind();
        return true;
    }

    void update() override {}
    void cleanup() override {}
    const char* getName() const override { return "Demo"; }

    // ─────────────────────────────────────────────────────────────────
    // Public interface (called by Handler)
    // ─────────────────────────────────────────────────────────────────

    void setEncoder(size_t index, float value) {
        if (index >= ENC_COUNT) return;
        lv_slider_set_value(sliders_[index], int32_t(value * SLIDER_MAX), LV_ANIM_ON);
        lv_label_set_text(labels_[index], (std::to_string(int(value * SLIDER_MAX)) + "%").c_str());
    }

    void resetEncoderPositions() {
        for (size_t i = 0; i < ENC_COUNT; ++i) {
            setEncoder(i, DEFAULT_VALUE);
        }
    }

    void onButtonRelease() {
        // No visual feedback on release
    }

private:
    // ─────────────────────────────────────────────────────────────────
    // UI Creation
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
            lv_label_set_text(name, ("Enc " + std::to_string(i + 1)).c_str());
            lv_obj_set_style_text_color(name, lv_color_hex(Colors::LABEL_DIM), 0);
            lv_obj_set_width(name, 50);

            // Slider
            sliders_[i] = lv_slider_create(row);
            lv_obj_set_flex_grow(sliders_[i], 1);
            lv_obj_set_height(sliders_[i], 12);
            lv_slider_set_range(sliders_[i], 0, SLIDER_MAX);
            lv_slider_set_value(sliders_[i], int32_t(DEFAULT_VALUE * SLIDER_MAX), LV_ANIM_OFF);
            lv_obj_set_style_bg_color(sliders_[i], lv_color_hex(Colors::SLIDER_BG), LV_PART_MAIN);
            lv_obj_set_style_bg_color(sliders_[i], lv_color_hex(Colors::SLIDER_FILL), LV_PART_INDICATOR);
            lv_obj_set_style_bg_color(sliders_[i], lv_color_hex(Colors::SLIDER_KNOB), LV_PART_KNOB);
            lv_obj_set_style_pad_all(sliders_[i], 0, LV_PART_KNOB);

            // Value
            labels_[i] = lv_label_create(row);
            lv_label_set_text(labels_[i], (std::to_string(int(DEFAULT_VALUE * SLIDER_MAX)) + "%").c_str());
            lv_obj_set_style_text_color(labels_[i], lv_color_white(), 0);
            lv_obj_set_width(labels_[i], 40);
            lv_obj_set_style_text_align(labels_[i], LV_TEXT_ALIGN_RIGHT, 0);
        }
    }

    void createFooter(lv_obj_t* parent) {
        auto* lbl = lv_label_create(parent);
        lv_label_set_text(lbl, "Button: reset");
        lv_obj_set_style_text_color(lbl, lv_color_hex(Colors::LABEL_MUTED), 0);
        lv_obj_set_style_text_font(lbl, &lv_font_montserrat_12, 0);
    }

    // ─────────────────────────────────────────────────────────────────
    // Members
    // ─────────────────────────────────────────────────────────────────

    std::optional<Handler<DemoView>> handler_;
    lv_obj_t* sliders_[ENC_COUNT] = {};
    lv_obj_t* labels_[ENC_COUNT] = {};
};

}  // namespace ui
