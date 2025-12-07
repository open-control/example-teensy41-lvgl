#pragma once

/**
 * @file DemoView.hpp
 * @brief Demo view with auto-generated buttons and encoder sliders
 *
 * Layout:
 * - Title at top
 * - Buttons in horizontal flex row
 * - Encoders in vertical flex column
 *
 * Widgets are auto-generated from Config arrays.
 */

#include "Config.hpp"
#include "ui/widget/ButtonIndicator.hpp"
#include "ui/widget/EncoderSlider.hpp"

#include <memory>
#include <string>
#include <vector>

#include <lvgl.h>
#include <oc/ui/interface/IView.hpp>

namespace ui {

/**
 * @brief Full-screen view with buttons and encoder sliders
 *
 * Auto-generates UI from Config::Button::BUTTONS and Config::Encoder::ENCODERS.
 */
class DemoView : public oc::ui::IView {
public:
    static constexpr size_t BUTTON_COUNT = Config::Button::BUTTONS.size();
    static constexpr size_t ENCODER_COUNT = Config::Encoder::ENCODERS.size();
    static constexpr float DEFAULT_VALUE = 0.5f;

    DemoView() { create(); }
    ~DemoView() override { destroy(); }

    // ═══════════════════════════════════════════════════════════════════
    // IElement Interface
    // ═══════════════════════════════════════════════════════════════════

    lv_obj_t* getElement() const override { return container_; }

    // ═══════════════════════════════════════════════════════════════════
    // IView Interface
    // ═══════════════════════════════════════════════════════════════════

    void onActivate() override {
        if (container_) {
            lv_obj_clear_flag(container_, LV_OBJ_FLAG_HIDDEN);
        }
    }

    void onDeactivate() override {
        if (container_) {
            lv_obj_add_flag(container_, LV_OBJ_FLAG_HIDDEN);
        }
    }

    const char* getViewId() const override { return "demo"; }

    // ═══════════════════════════════════════════════════════════════════
    // Public API (called by Handler)
    // ═══════════════════════════════════════════════════════════════════

    void setButton(size_t index, bool pressed) {
        if (index < buttons_.size()) {
            buttons_[index]->setPressed(pressed);
        }
    }

    void setEncoder(size_t index, float value) {
        if (index < sliders_.size()) {
            sliders_[index]->setValue(value);
        }
    }

    void resetEncoderPositions() {
        for (auto& slider : sliders_) {
            slider->setValue(DEFAULT_VALUE);
        }
    }

private:
    void create() {
        auto* screen = lv_screen_active();

        // Root container
        container_ = lv_obj_create(screen);
        lv_obj_set_size(container_, LV_PCT(100), LV_PCT(100));
        lv_obj_set_style_bg_color(container_, lv_color_black(), 0);
        lv_obj_set_style_border_width(container_, 0, 0);
        lv_obj_set_style_pad_all(container_, 12, 0);
        lv_obj_set_flex_flow(container_, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_style_pad_row(container_, 12, 0);

        createTitle();
        createButtons();
        createEncoders();
    }

    void destroy() {
        buttons_.clear();
        sliders_.clear();
        if (container_) {
            lv_obj_delete(container_);
            container_ = nullptr;
        }
    }

    void createTitle() {
        auto* label = lv_label_create(container_);
        lv_label_set_text(label, "Open Control");
        lv_obj_set_style_text_color(label, lv_color_white(), 0);
        lv_obj_set_style_text_font(label, &lv_font_montserrat_16, 0);
    }

    void createButtons() {
        // Horizontal flex container
        auto* row = lv_obj_create(container_);
        lv_obj_set_size(row, LV_PCT(100), LV_SIZE_CONTENT);
        lv_obj_set_style_bg_opa(row, LV_OPA_TRANSP, 0);
        lv_obj_set_style_border_width(row, 0, 0);
        lv_obj_set_style_pad_all(row, 0, 0);
        lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
        lv_obj_set_style_pad_column(row, 8, 0);

        for (size_t i = 0; i < BUTTON_COUNT; ++i) {
            std::string name = "BTN " + std::to_string(i + 1);
            buttons_.push_back(
                std::make_unique<ButtonIndicator>(row, name.c_str())
            );
        }
    }

    void createEncoders() {
        // Vertical flex container
        auto* column = lv_obj_create(container_);
        lv_obj_set_size(column, LV_PCT(100), LV_SIZE_CONTENT);
        lv_obj_set_style_bg_opa(column, LV_OPA_TRANSP, 0);
        lv_obj_set_style_border_width(column, 0, 0);
        lv_obj_set_style_pad_all(column, 0, 0);
        lv_obj_set_flex_flow(column, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_style_pad_row(column, 8, 0);

        for (size_t i = 0; i < ENCODER_COUNT; ++i) {
            std::string name = "ENC " + std::to_string(i + 1);
            sliders_.push_back(
                std::make_unique<EncoderSlider>(column, name.c_str())
            );
        }
    }

    lv_obj_t* container_ = nullptr;
    std::vector<std::unique_ptr<ButtonIndicator>> buttons_;
    std::vector<std::unique_ptr<EncoderSlider>> sliders_;
};

}  // namespace ui
