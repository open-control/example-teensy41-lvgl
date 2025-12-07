#pragma once

/**
 * @file Buffer.hpp
 * @brief DMAMEM buffers for display and LVGL
 */

#include <Arduino.h>
#include <lvgl.h>
#include "Config.hpp"

namespace Buffer {

inline DMAMEM uint16_t  framebuffer[Config::Display::BUFFER_SIZE];
inline DMAMEM uint8_t   diff1[Config::Display::DIFF_SIZE];
inline DMAMEM uint8_t   diff2[Config::Display::DIFF_SIZE];
inline DMAMEM lv_color_t lvgl[Config::Display::BUFFER_SIZE];

}  // namespace Buffer
