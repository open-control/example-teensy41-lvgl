#pragma once

/**
 * @file DemoView.hpp
 * @brief Demo view implementing IView interface
 *
 * Demonstrates the IView interface from ui-lvgl:
 *   - IView: Full-screen view with system-managed lifecycle
 *   - Receives onActivate/onDeactivate from view manager
 *   - Provides getElement() for scoped input bindings
 *
 * Hierarchy: IElement → IView
 *
 * Lifecycle model:
 *   - IComponent: Imperative (code calls show/hide)
 *   - IView: Declarative (system notifies activate/deactivate)
 *
 * Use cases:
 *   - Main parameter screen
 *   - Settings page
 *   - Splash screen
 */

#include "Config.hpp"
#include "ui/widget/EncoderSlider.hpp"

#include <memory>
#include <vector>

#include <lvgl.h>
#include <oc/ui/interface/IView.hpp>

namespace ui {

namespace Enc = Config::Enc;

/**
 * @brief Demo view displaying encoder sliders
 *
 * Shows encoder values using EncoderSlider widgets.
 * Handler calls setEncoder() and resetEncoderPositions().
 *
 * @code
 * DemoView view;
 * view.onActivate();   // Show view
 * view.setEncoder(0, 0.75f);
 * view.onDeactivate(); // Hide view
 * @endcode
 */
class DemoView : public oc::ui::IView {
public:
    static constexpr size_t ENC_COUNT = Enc::ALL.size();
    static constexpr float DEFAULT_VALUE = 0.5f;  ///< Used by Handler for reset

    DemoView() { create(); }
    ~DemoView() override { destroy(); }

    // ═══════════════════════════════════════════════════════════════════
    // IElement interface
    // ═══════════════════════════════════════════════════════════════════

    /**
     * @brief Get root LVGL container
     *
     * Used by LVGLAdapter for scoped bindings:
     *   api.onPressed(id, lvglIsActive(view.getElement()), callback);
     */
    lv_obj_t* getElement() const override { return container_; }

    // ═══════════════════════════════════════════════════════════════════
    // IView interface
    // ═══════════════════════════════════════════════════════════════════

    /**
     * @brief Called when view becomes visible
     *
     * Clears hidden flag, input bindings become active.
     */
    void onActivate() override {
        if (container_) {
            lv_obj_clear_flag(container_, LV_OBJ_FLAG_HIDDEN);
        }
    }

    /**
     * @brief Called when view becomes hidden
     *
     * Sets hidden flag, input bindings become inactive.
     */
    void onDeactivate() override {
        if (container_) {
            lv_obj_add_flag(container_, LV_OBJ_FLAG_HIDDEN);
        }
    }

    /// Unique view identifier for debugging/switching
    const char* getViewId() const override { return "demo"; }

    // ═══════════════════════════════════════════════════════════════════
    // Public API (called by Handler)
    // ═══════════════════════════════════════════════════════════════════

    /// Update single encoder display
    void setEncoder(size_t index, float value) {
        if (index < sliders_.size()) {
            sliders_[index]->setValue(value);
        }
    }

    /// Reset all encoder displays to DEFAULT_VALUE
    void resetEncoderPositions() {
        for (auto& slider : sliders_) {
            slider->setValue(DEFAULT_VALUE);
        }
    }

private:
    void create() {
        auto* scr = lv_screen_active();

        // Main container
        container_ = lv_obj_create(scr);
        lv_obj_set_size(container_, LV_PCT(100), LV_PCT(100));
        lv_obj_set_style_bg_color(container_, lv_color_black(), 0);
        lv_obj_set_style_border_width(container_, 0, 0);
        lv_obj_set_style_pad_all(container_, 10, 0);
        lv_obj_set_flex_flow(container_, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_flex_align(container_, LV_FLEX_ALIGN_START,
                              LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_style_pad_row(container_, 8, 0);

        createTitle();
        createEncoderSliders();
        createFooter();
    }

    void destroy() {
        sliders_.clear();
        if (container_) {
            lv_obj_delete(container_);
            container_ = nullptr;
        }
    }

    void createTitle() {
        auto* lbl = lv_label_create(container_);
        lv_label_set_text(lbl, "Open Control");
        lv_obj_set_style_text_color(lbl, lv_color_white(), 0);
        lv_obj_set_style_text_font(lbl, &lv_font_montserrat_16, 0);
    }

    void createEncoderSliders() {
        auto* sliderContainer = lv_obj_create(container_);
        lv_obj_set_size(sliderContainer, LV_PCT(100), LV_SIZE_CONTENT);
        lv_obj_set_style_bg_opa(sliderContainer, LV_OPA_TRANSP, 0);
        lv_obj_set_style_border_width(sliderContainer, 0, 0);
        lv_obj_set_style_pad_all(sliderContainer, 0, 0);
        lv_obj_set_flex_flow(sliderContainer, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_style_pad_row(sliderContainer, 12, 0);

        for (size_t i = 0; i < ENC_COUNT; ++i) {
            std::string name = "Enc " + std::to_string(i + 1);
            sliders_.push_back(
                std::make_unique<EncoderSlider>(sliderContainer, name.c_str())
            );
        }
    }

    void createFooter() {
        auto* lbl = lv_label_create(container_);
        lv_label_set_text(lbl, "Button: reset");
        lv_obj_set_style_text_color(lbl, lv_color_hex(0x555555), 0);
        lv_obj_set_style_text_font(lbl, &lv_font_montserrat_12, 0);
    }

    lv_obj_t* container_ = nullptr;
    std::vector<std::unique_ptr<EncoderSlider>> sliders_;
};

}  // namespace ui
