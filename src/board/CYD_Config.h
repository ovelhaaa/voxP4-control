#ifndef CYD_CONFIG_H
#define CYD_CONFIG_H

#include <stdint.h>
#include <cstddef>

namespace VoxCydConfig {

// Display configuration
constexpr uint16_t ScreenWidth = 320;
constexpr uint16_t ScreenHeight = 240;
constexpr uint8_t ScreenRotation = 1;  // Landscape

// ST7789 display pins
constexpr int TftSclk = 14;
constexpr int TftMiso = 12;
constexpr int TftMosi = 13;
constexpr int TftCs   = 15;
constexpr int TftDc   = 2;
constexpr int TftRst  = -1;  // Use board reset
constexpr int TftBacklight = 21;

// XPT2046 touch pins
constexpr int TouchSclk = 25;
constexpr int TouchMiso = 39;
constexpr int TouchMosi = 32;
constexpr int TouchCs   = 33;
constexpr int TouchIrq  = -1;  // Not used

// Touch calibration (from BeatCYDa)
constexpr uint16_t TouchMinX = 240;
constexpr uint16_t TouchMaxX = 3800;
constexpr uint16_t TouchMinY = 3700;
constexpr uint16_t TouchMaxY = 200;

// VoxP4 UART - reuses SD bus pins (SD disabled)
constexpr int VoxUartTx = 18;
constexpr int VoxUartRx = 19;
constexpr uint32_t VoxUartBaud = 921600;
constexpr uint32_t VoxUartBaudFallback = 460800;

// Footswitches
constexpr int Footswitch1Pin = 22;
constexpr int Footswitch2Pin = 27;

// UI timing
constexpr uint16_t UiFrameMs = 33;        // ~30 FPS target
constexpr uint16_t TouchPollMs = 16;      // ~60 Hz
constexpr uint16_t LvglTickMs = 5;        // LVGL handler

// Footswitch timing
constexpr uint16_t FootswitchPollMs = 4;
constexpr uint16_t FootswitchDebounceMs = 15;
constexpr uint16_t FootswitchLongPressMs = 500;
constexpr uint16_t FootswitchDoublePressMs = 300;

// Telemetry rates
constexpr uint16_t MeterRateHz = 30;
constexpr uint16_t PitchRateHz = 20;
constexpr uint16_t DspStatusRateHz = 5;
constexpr uint16_t HeartbeatRateHz = 2;

// LVGL buffer configuration
constexpr uint16_t LvglBufferLines = 20;
constexpr size_t LvglBufferSize = ScreenWidth * LvglBufferLines * 2;  // RGB565

}  // namespace VoxCydConfig

#endif  // CYD_CONFIG_H
