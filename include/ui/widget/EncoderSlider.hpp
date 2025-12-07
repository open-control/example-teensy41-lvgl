#pragma once

/**
 * @file EncoderSlider.hpp
 * @brief Encoder slider widget with embedded label
 *
 * Horizontal slider with label overlay in the background.
 * Auto-generated from Config::Encoder::ENCODERS array.
 */

#include <lvgl.h>
#include <oc/ui/interface/IWidget.hpp>

namespace ui {

/// Visual style for EncoderSlider
struct EncoderSliderStyle {
    uint32_t bgColor = 0x333355;
    uint32_t fillColor = 0x6666ff;
    uint32_t labelColor = 0xAAAAAA;
    int32_t height = 32;
    int32_t radius = 4;
};

/**
 * @brief Encoder value slider with embedded label
 *
 * Slider fills from left based on normalized value (0.0-1.0).
 * Label is centered inside the slider background.
 */
class EncoderSlider : public oc::ui::IWidget {
public:
    using Style = EncoderSliderStyle;

    EncoderSlider(lv_obj_t* parent, const char* name)
        : EncoderSlider(parent, name, Style{}) {}

    EncoderSlider(lv_obj_t* parent, const char* name, const Style& style)
        : style_(style)
    {
        // Slider (full width, acts as container)
        slider_ = lv_slider_create(parent);
        lv_obj_set_size(slider_, LV_PCT(100), style_.height);
        lv_slider_set_range(slider_, 0, 100);
        lv_obj_set_style_radius(slider_, style_.radius, LV_PART_MAIN);
        lv_obj_set_style_radius(slider_, style_.radius, LV_PART_INDICATOR);
        lv_obj_set_style_bg_color(slider_, lv_color_hex(style_.bgColor), LV_PART_MAIN);
        lv_obj_set_style_bg_color(slider_, lv_color_hex(style_.fillColor), LV_PART_INDICATOR);

        // Hide knob
        lv_obj_set_style_bg_opa(slider_, LV_OPA_TRANSP, LV_PART_KNOB);
        lv_obj_set_style_pad_all(slider_, 0, LV_PART_KNOB);

        // Label overlay (centered in slider)
        label_ = lv_label_create(slider_);
        lv_label_set_text(label_, name);
        lv_obj_set_style_text_color(label_, lv_color_hex(style_.labelColor), 0);
        lv_obj_center(label_);

        setValue(0.5f);
    }

    // ═══════════════════════════════════════════════════════════════════
    // IWidget interface
    // ═══════════════════════════════════════════════════════════════════

    lv_obj_t* getElement() const override { return slider_; }

    // ═══════════════════════════════════════════════════════════════════
    // Public API
    // ═══════════════════════════════════════════════════════════════════

    /// Set slider value (0.0-1.0 normalized)
    void setValue(float normalized) {
        lv_slider_set_value(slider_, int32_t(normalized * 100), LV_ANIM_ON);
    }

    /// Get current value (0.0-1.0 normalized)
    float getValue() const {
        return lv_slider_get_value(slider_) / 100.0f;
    }

private:
    Style style_;
    lv_obj_t* slider_ = nullptr;
    lv_obj_t* label_ = nullptr;
};

}  // namespace ui
