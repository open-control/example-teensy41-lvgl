# Open Control - Teensy 4.1 LVGL Example

A complete [Open Control Framework](https://github.com/open-control/framework) example demonstrating:
- ILI9341 TFT display with DMA acceleration (flicker-free)
- LVGL 9.x graphics library for responsive UI
- Rotary encoders with visual feedback and MIDI CC output
- Buttons with press/release handling
- Clean MVC architecture with Context, Handler, and View separation

## Hardware Requirements

- Teensy 4.1
- ILI9341 320x240 TFT display (SPI)
- 2x Rotary encoders (quadrature, 24 PPR recommended)
- 1x Momentary push button
- USB cable for MIDI and power

## Default Wiring

### Display (SPI1)

| ILI9341 Pin | Teensy Pin | Notes |
|-------------|------------|-------|
| VCC | 3.3V | Power |
| GND | GND | Ground |
| CS | 28 | Chip Select |
| DC | 0 | Data/Command |
| RST | 29 | Reset |
| MOSI | 26 | SPI1 MOSI |
| SCK | 27 | SPI1 SCK |
| MISO | 1 | SPI1 MISO |

### Encoders & Buttons

| Component | Pin A | Pin B | Notes |
|-----------|-------|-------|-------|
| Encoder 1 | 22 | 23 | → CC 60 |
| Encoder 2 | 18 | 19 | → CC 61 |
| Button 1 | 32 | GND | → CC 10, resets encoders |

> Buttons use internal pull-up resistors. Connect one leg to the pin, the other to GND.

## MIDI Mapping

| Control | MIDI Message | Channel |
|---------|--------------|---------|
| Encoder 1 | CC 60 (0-127) | 1 |
| Encoder 2 | CC 61 (0-127) | 1 |
| Button 1 Press | CC 10 = 127 | 1 |
| Button 1 Release | CC 10 = 0 | 1 |

## Quick Start

### 1. Install PlatformIO

```bash
# VS Code extension (recommended)
# Or CLI: pip install platformio
```

### 2. Clone and Build

```bash
git clone https://github.com/open-control/example-teensy41-lvgl.git
cd example-teensy41-lvgl
pio run -e release
```

### 3. Upload

```bash
pio run -e release -t upload
```

### 4. Monitor Serial Output

```bash
pio device monitor -b 115200
```

## Project Structure

```
example-teensy41-lvgl/
├── include/
│   ├── Config.hpp              # All hardware configuration
│   ├── Buffer.hpp              # DMAMEM display buffers
│   ├── lv_conf.h               # LVGL configuration
│   ├── context/
│   │   └── StandaloneContext.hpp   # Application context
│   ├── handler/
│   │   └── Handler.hpp         # Input→MIDI+View bindings
│   └── ui/
│       ├── view/
│       │   └── DemoView.hpp    # Main UI view
│       └── widget/
│           ├── ButtonIndicator.hpp
│           └── EncoderSlider.hpp
├── src/
│   └── main.cpp                # Application entry point
├── platformio.ini              # Build configuration
└── README.md
```

## Architecture

### Separation of Concerns

```
┌─────────────┐    ┌──────────────┐    ┌──────────────┐
│   Context   │───▶│   Handler    │───▶│    View      │
│ (IContext)  │    │ (Bindings)   │    │  (LVGL UI)   │
└─────────────┘    └──────────────┘    └──────────────┘
       │                  │                    │
       │                  ▼                    │
       │           ┌──────────────┐            │
       └──────────▶│   MIDI API   │◀───────────┘
                   └──────────────┘
```

- **Context**: Application lifecycle (initialize, update, cleanup)
- **Handler**: Maps inputs to MIDI and view updates
- **View**: LVGL widgets, purely presentational

### Context Implementation

```cpp
class StandaloneContext : public oc::context::IContext {
public:
    // Declare required APIs (validated at registration)
    static constexpr oc::context::Requirements REQUIRES{
        .button = true,
        .encoder = true,
        .midi = true
    };

    bool initialize() override {
        view_ = std::make_unique<DemoView>();
        handler_ = std::make_unique<Handler>(
            buttons(), encoders(), midi(), *view_);
        handler_->bind();
        return true;
    }

    void cleanup() override {
        handler_.reset();
        view_.reset();
    }
};
```

### Auto-Generated Bindings

The Handler auto-generates MIDI mappings from Config arrays:

```cpp
// Config.hpp
namespace Encoder {
    constexpr std::array ENCODERS = {
        EncoderDef{10, 22, 23, ...},  // → CC 60
        EncoderDef{11, 18, 19, ...},  // → CC 61
    };
}

// Handler.hpp - auto-binds all encoders
for (size_t i = 0; i < ENCODER_COUNT; ++i) {
    encoders_.encoder(Config::Encoder::ENCODERS[i].id)
        .turn()
        .then([this, i](float value) {
            sendEncoderCC(i, value);  // CC = ENC_CC_RANGE_START + i
            view_.setEncoder(i, value);
        });
}
```

## Customization

### Change Pin Assignments

Edit `include/Config.hpp`:

```cpp
namespace Display {
constexpr Ili9341Config CONFIG = {
    .csPin = 28,    // Change these
    .dcPin = 0,
    .rstPin = 29,
    // ...
};
}

namespace Encoder {
constexpr std::array ENCODERS = {
    EncoderDef{10, 22, 23, PPR, RANGE, TICKS, INVERT},  // Change pins
};
}
```

### Change MIDI Mapping

Edit `include/Config.hpp`:

```cpp
namespace Midi {
constexpr uint8_t CHANNEL = 0;              // 0-15 (displayed as 1-16)
constexpr uint8_t BTN_CC_RANGE_START = 10;  // Buttons: CC 10, 11, 12...
constexpr uint8_t ENC_CC_RANGE_START = 60;  // Encoders: CC 60, 61, 62...
}
```

### Add More Encoders/Buttons

Simply add entries to the arrays in `Config.hpp`:

```cpp
constexpr std::array ENCODERS = {
    EncoderDef{10, 22, 23, PPR, RANGE, TICKS, INVERT},
    EncoderDef{11, 18, 19, PPR, RANGE, TICKS, INVERT},
    EncoderDef{12, 40, 41, PPR, RANGE, TICKS, INVERT},  // Add more
};
```

The Handler and View auto-generate UI and bindings from these arrays.

### Adjust Display Settings

```cpp
namespace Display {
constexpr Ili9341Config CONFIG = {
    .spiSpeed = 40'000'000,  // Reduce if artifacts appear
    .rotation = 3,           // 0-3: 90° increments
    .invertDisplay = true,   // Toggle if colors wrong
    .vsyncSpacing = 1,       // Increase if tearing
};
}
```

## Input Binding API Reference

### Button Bindings

```cpp
// Simple press
buttons_.button(buttonId).press().then([]() { /* action */ });

// Release
buttons_.button(buttonId).release().then([]() { /* action */ });

// Long press (0 = use config default)
buttons_.button(buttonId).longPress(500).then([]() { /* 500ms */ });

// Double tap
buttons_.button(buttonId).doubleTap(300).then([]() { /* 300ms window */ });
```

### Encoder Bindings

```cpp
// Turn - value is always 0.0-1.0 (normalized)
encoders_.encoder(encoderId).turn().then([](float value) {
    uint8_t midiValue = static_cast<uint8_t>(value * 127.0f);
});

// Set position programmatically
encoders_.setPosition(encoderId, 0.5f);  // Reset to center
```

### MIDI Output

```cpp
midi_.sendCC(channel, cc, value);              // Control Change
midi_.sendNoteOn(channel, note, velocity);     // Note On
midi_.sendNoteOff(channel, note, velocity);    // Note Off
midi_.sendProgramChange(channel, program);     // Program Change
midi_.sendPitchBend(channel, value);           // Pitch Bend
midi_.sendChannelPressure(channel, pressure);  // Aftertouch
midi_.allNotesOff();                           // Panic
```

## Troubleshooting

### Display Issues

| Problem | Solution |
|---------|----------|
| White/blank screen | Check SPI wiring, reduce `spiSpeed` to 20 MHz |
| Wrong colors | Toggle `invertDisplay` in Config |
| Flickering | Reduce `spiSpeed` or increase `vsyncSpacing` |
| Tearing | Increase `vsyncSpacing` to 2 |

### Encoder Issues

| Problem | Solution |
|---------|----------|
| Erratic values | Check PPR matches datasheet |
| Wrong direction | Set `invertDirection = true` |
| Missing steps | Increase `APP_HZ` or reduce `ticksPerEvent` |

### Button Issues

| Problem | Solution |
|---------|----------|
| Not responding | Check wiring (pin to GND when pressed) |
| Multiple triggers | Increase `DEBOUNCE_MS` to 10-20 |

## Performance

- **APP_HZ = 2000**: Encoder polling rate (below 1000 may miss fast rotation)
- **LVGL_HZ = 100**: UI refresh rate (saves CPU while maintaining smooth display)
- **DMA rendering**: Display updates happen in background, no CPU blocking

Memory usage (320x240 RGB565):
- Framebuffer: ~150 KB (DMAMEM)
- LVGL buffer: ~150 KB (DMAMEM)
- Diff buffers: ~40 KB (DMAMEM)

## Development Mode

To use local development versions of the framework:

```bash
# Clone all repos in same parent directory
git clone https://github.com/open-control/framework.git
git clone https://github.com/open-control/hal-common.git
git clone https://github.com/open-control/hal-teensy.git
git clone https://github.com/open-control/ui-lvgl.git
git clone https://github.com/open-control/example-teensy41-lvgl.git

# Build with local deps
cd example-teensy41-lvgl
pio run -e dev
```

## License

Apache 2.0 - See [LICENSE](LICENSE)
