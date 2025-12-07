#pragma once

/**
 * @file DemoView.hpp
 * @brief Demo view displaying encoder sliders with LVGL
 *
 * Implements the IView interface from the ui-lvgl framework:
 * - IView provides lifecycle callbacks (onActivate/onDeactivate)
 * - getElement() returns the root container for scoped bindings
 *
 * This view displays one slider per encoder defined in Config::Encoder::ALL.
 * The Handler class calls setEncoder() when encoders rotate, and
 * resetEncoderPositions() when the button is pressed.
 *
 * Hierarchy: IElement -> IView -> DemoView
 */

#include "Config.hpp"
#include "ui/widget/EncoderSlider.hpp"

#include <memory>
#include <string>
#include <vector>

#include <lvgl.h>
#include <oc/ui/interface/IView.hpp>

namespace ui {

/**
 * @brief Full-screen view displaying encoder values as sliders
 *
 * Layout:
 * - Title "Open Control" at top
 * - One EncoderSlider widget per encoder
 * - Footer showing button action hint
 *
 * The view adapts automatically to the number of encoders in Config::Encoder::ALL.
 */
class DemoView : public oc::ui::IView {
public:
    /// Number of encoders, derived from hardware configuration
    static constexpr size_t ENCODER_COUNT = Config::Encoder::ALL.size();

    /// Default normalized value (0.5 = 50%), used for reset
    static constexpr float DEFAULT_VALUE = 0.5f;

    DemoView() { create(); }
    ~DemoView() override { destroy(); }

    // ═══════════════════════════════════════════════════════════════════════════
    // IElement Interface
    // ═══════════════════════════════════════════════════════════════════════════

    /**
     * @brief Get root LVGL container
     *
     * Used by the framework for scoped input bindings. Bindings attached to
     * this element are automatically disabled when the view is hidden.
     */
    lv_obj_t* getElement() const override { return container_; }

    // ═══════════════════════════════════════════════════════════════════════════
    // IView Interface
    // ═══════════════════════════════════════════════════════════════════════════

    /**
     * @brief Called when view becomes visible
     *
     * Clears the hidden flag so LVGL renders this view.
     * Input bindings using this view's scope become active.
     */
    void onActivate() override {
        if (container_) {
            lv_obj_clear_flag(container_, LV_OBJ_FLAG_HIDDEN);
        }
    }

    /**
     * @brief Called when view becomes hidden
     *
     * Sets the hidden flag to stop rendering this view.
     * Input bindings using this view's scope become inactive.
     */
    void onDeactivate() override {
        if (container_) {
            lv_obj_add_flag(container_, LV_OBJ_FLAG_HIDDEN);
        }
    }

    /// Unique identifier for debugging and view switching
    const char* getViewId() const override { return "demo"; }

    // ═══════════════════════════════════════════════════════════════════════════
    // Public API (called by Handler)
    // ═══════════════════════════════════════════════════════════════════════════

    /**
     * @brief Update a single encoder's display
     * @param index Encoder index (0-based)
     * @param value Normalized value (0.0 to 1.0)
     */
    void setEncoder(size_t index, float value) {
        if (index < sliders_.size()) {
            sliders_[index]->setValue(value);
        }
    }

    /**
     * @brief Reset all encoder displays to DEFAULT_VALUE
     *
     * Called by Handler when the reset button is pressed.
     * This only updates the UI - hardware positions are synced separately.
     */
    void resetEncoderPositions() {
        for (auto& slider : sliders_) {
            slider->setValue(DEFAULT_VALUE);
        }
    }

private:
    /// Create the complete UI hierarchy
    void create() {
        auto* screen = lv_screen_active();

        // Root container fills the screen
        container_ = lv_obj_create(screen);
        lv_obj_set_size(container_, LV_PCT(100), LV_PCT(100));
        lv_obj_set_style_bg_color(container_, lv_color_black(), 0);
        lv_obj_set_style_border_width(container_, 0, 0);
        lv_obj_set_style_pad_all(container_, 10, 0);

        // Vertical flex layout for stacking elements
        lv_obj_set_flex_flow(container_, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_flex_align(container_, LV_FLEX_ALIGN_START,
                              LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_style_pad_row(container_, 8, 0);

        createTitle();
        createEncoderSliders();
        createFooter();
    }

    /// Clean up LVGL objects
    void destroy() {
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

    void createEncoderSliders() {
        // Container for all sliders with vertical spacing
        auto* sliderContainer = lv_obj_create(container_);
        lv_obj_set_size(sliderContainer, LV_PCT(100), LV_SIZE_CONTENT);
        lv_obj_set_style_bg_opa(sliderContainer, LV_OPA_TRANSP, 0);
        lv_obj_set_style_border_width(sliderContainer, 0, 0);
        lv_obj_set_style_pad_all(sliderContainer, 0, 0);
        lv_obj_set_flex_flow(sliderContainer, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_style_pad_row(sliderContainer, 12, 0);

        // Create one slider per encoder
        for (size_t i = 0; i < ENCODER_COUNT; ++i) {
            std::string name = "Encoder " + std::to_string(i + 1);
            sliders_.push_back(
                std::make_unique<EncoderSlider>(sliderContainer, name.c_str())
            );
        }
    }

    void createFooter() {
        auto* label = lv_label_create(container_);
        lv_label_set_text(label, "Button: reset all");
        lv_obj_set_style_text_color(label, lv_color_hex(0x555555), 0);
        lv_obj_set_style_text_font(label, &lv_font_montserrat_12, 0);
    }

    lv_obj_t* container_ = nullptr;
    std::vector<std::unique_ptr<EncoderSlider>> sliders_;
};

}  // namespace ui
