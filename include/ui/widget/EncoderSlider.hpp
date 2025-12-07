#pragma once

/**
 * @file EncoderSlider.hpp
 * @brief Reusable encoder slider widget implementing IWidget
 *
 * Demonstrates the IWidget interface from ui-lvgl:
 *   - IWidget: Simple UI element, always visible once created
 *   - Visibility controlled by parent container
 *   - Provides getElement() for LVGL object access
 *
 * Hierarchy: IElement → IWidget
 *
 * Use cases:
 *   - Labels, progress bars, status indicators
 *   - Any element without explicit show/hide semantics
 */

#include <string>

#include <lvgl.h>
#include <oc/ui/interface/IWidget.hpp>

namespace ui {

/// Visual style configuration for EncoderSlider
struct EncoderSliderStyle {
    uint32_t labelColor   = 0x888888;
    uint32_t sliderBg     = 0x333355;
    uint32_t sliderFill   = 0x6666ff;
    uint32_t sliderKnob   = 0x9999ff;
    uint32_t valueColor   = 0xFFFFFF;
    int32_t  labelWidth   = 50;
    int32_t  valueWidth   = 40;
    int32_t  sliderHeight = 12;
};

/**
 * @brief Encoder value display widget (slider + percentage label)
 *
 * Composite widget showing:
 *   - Name label (left)
 *   - Slider bar (center, flex-grow)
 *   - Value percentage (right)
 *
 * @code
 * EncoderSlider slider(parent, "Volume");
 * slider.setValue(0.75f);  // 75%
 * float val = slider.getValue();
 * @endcode
 */
class EncoderSlider : public oc::ui::IWidget {
public:
    using Style = EncoderSliderStyle;

    EncoderSlider(lv_obj_t* parent, const char* name)
        : EncoderSlider(parent, name, Style{}) {}

    EncoderSlider(lv_obj_t* parent, const char* name, const Style& style)
        : style_(style)
    {
        // Container row
        container_ = lv_obj_create(parent);
        lv_obj_set_size(container_, LV_PCT(100), LV_SIZE_CONTENT);
        lv_obj_set_style_bg_opa(container_, LV_OPA_TRANSP, 0);
        lv_obj_set_style_border_width(container_, 0, 0);
        lv_obj_set_style_pad_all(container_, 4, 0);
        lv_obj_set_flex_flow(container_, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(container_, LV_FLEX_ALIGN_SPACE_BETWEEN,
                              LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_style_pad_column(container_, 8, 0);

        // Label
        label_ = lv_label_create(container_);
        lv_label_set_text(label_, name);
        lv_obj_set_style_text_color(label_, lv_color_hex(style_.labelColor), 0);
        lv_obj_set_width(label_, style_.labelWidth);

        // Slider
        slider_ = lv_slider_create(container_);
        lv_obj_set_flex_grow(slider_, 1);
        lv_obj_set_height(slider_, style_.sliderHeight);
        lv_slider_set_range(slider_, 0, 100);
        lv_obj_set_style_bg_color(slider_, lv_color_hex(style_.sliderBg), LV_PART_MAIN);
        lv_obj_set_style_bg_color(slider_, lv_color_hex(style_.sliderFill), LV_PART_INDICATOR);
        lv_obj_set_style_bg_color(slider_, lv_color_hex(style_.sliderKnob), LV_PART_KNOB);
        lv_obj_set_style_pad_all(slider_, 0, LV_PART_KNOB);

        // Value label
        value_ = lv_label_create(container_);
        lv_obj_set_style_text_color(value_, lv_color_hex(style_.valueColor), 0);
        lv_obj_set_width(value_, style_.valueWidth);
        lv_obj_set_style_text_align(value_, LV_TEXT_ALIGN_RIGHT, 0);

        setValue(0.5f);  // Default 50%
    }

    // ═══════════════════════════════════════════════════════════════════
    // IWidget interface
    // ═══════════════════════════════════════════════════════════════════

    /**
     * @brief Get underlying LVGL container
     *
     * Used by LVGLAdapter for scoped bindings:
     *   lvglIsActive(slider.getElement())
     */
    lv_obj_t* getElement() const override { return container_; }

    // ═══════════════════════════════════════════════════════════════════
    // Public API
    // ═══════════════════════════════════════════════════════════════════

    /// Set slider value (0.0-1.0 normalized)
    void setValue(float normalized) {
        int32_t percent = int32_t(normalized * 100);
        lv_slider_set_value(slider_, percent, LV_ANIM_ON);
        lv_label_set_text(value_, (std::to_string(percent) + "%").c_str());
    }

    /// Get current value (0.0-1.0 normalized)
    float getValue() const {
        return lv_slider_get_value(slider_) / 100.0f;
    }

private:
    Style style_;
    lv_obj_t* container_ = nullptr;
    lv_obj_t* label_ = nullptr;
    lv_obj_t* slider_ = nullptr;
    lv_obj_t* value_ = nullptr;
};

}  // namespace ui
