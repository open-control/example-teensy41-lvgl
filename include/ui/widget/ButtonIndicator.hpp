#pragma once

/**
 * @file ButtonIndicator.hpp
 * @brief Visual button state indicator implementing IWidget
 *
 * Simple rectangle that changes color when pressed/released.
 * Auto-generated from Config::Button::BUTTONS array.
 */

#include <oc/ui/lvgl/IWidget.hpp>

namespace ui {

/// Visual style for ButtonIndicator
struct ButtonIndicatorStyle {
    uint32_t bgColor = 0x333355;       // Released state
    uint32_t activeColor = 0x6666ff;   // Pressed state
    int32_t width = 60;
    int32_t height = 40;
    int32_t radius = 6;
};

/**
 * @brief Button state indicator widget
 *
 * Displays a colored rectangle that reflects button state.
 * Call setPressed(true/false) to update visual feedback.
 */
class ButtonIndicator : public oc::ui::lvgl::IWidget {
public:
    using Style = ButtonIndicatorStyle;

    ButtonIndicator(lv_obj_t* parent, const char* label)
        : ButtonIndicator(parent, label, Style{}) {}

    ButtonIndicator(lv_obj_t* parent, const char* label, const Style& style)
        : style_(style)
    {
        // Button container
        container_ = lv_obj_create(parent);
        lv_obj_set_size(container_, style_.width, style_.height);
        lv_obj_set_style_bg_color(container_, lv_color_hex(style_.bgColor), 0);
        lv_obj_set_style_radius(container_, style_.radius, 0);
        lv_obj_set_style_border_width(container_, 0, 0);

        // Centered label
        label_ = lv_label_create(container_);
        lv_label_set_text(label_, label);
        lv_obj_set_style_text_color(label_, lv_color_white(), 0);
        lv_obj_center(label_);
    }

    // ═══════════════════════════════════════════════════════════════════
    // IWidget interface
    // ═══════════════════════════════════════════════════════════════════

    lv_obj_t* getElement() const override { return container_; }

    // ═══════════════════════════════════════════════════════════════════
    // Public API
    // ═══════════════════════════════════════════════════════════════════

    /// Update visual state
    void setPressed(bool pressed) {
        uint32_t color = pressed ? style_.activeColor : style_.bgColor;
        lv_obj_set_style_bg_color(container_, lv_color_hex(color), 0);
    }

private:
    Style style_;
    lv_obj_t* container_ = nullptr;
    lv_obj_t* label_ = nullptr;
};

}  // namespace ui
