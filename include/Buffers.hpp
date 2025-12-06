#pragma once

/**
 * @file Buffers.hpp
 * @brief Memory buffer declarations for display and LVGL
 *
 * All large buffers are allocated in DMAMEM (Teensy 4.x RAM2)
 * to keep RAM1 free for stack and fast variables.
 */

#include <Arduino.h>
#include <lvgl.h>

#include "Config.hpp"

namespace Buffers {

// Display framebuffer (ILI9341_T4 internal buffer)
inline DMAMEM uint16_t displayFramebuffer[Config::DISPLAY_WIDTH * Config::DISPLAY_HEIGHT];

// Differential update buffers (ILI9341_T4)
inline DMAMEM uint8_t diffBuffer1[Config::DIFFBUFFER_SIZE];
inline DMAMEM uint8_t diffBuffer2[Config::DIFFBUFFER_SIZE];

// LVGL draw buffer
inline DMAMEM lv_color_t lvglDrawBuffer[Config::DISPLAY_WIDTH * Config::DISPLAY_HEIGHT];

}  // namespace Buffers
