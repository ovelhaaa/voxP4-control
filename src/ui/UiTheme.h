#ifndef UI_THEME_H
#define UI_THEME_H

#include <lvgl.h>

// Color palette - dark theme optimized for stage use
namespace VoxUiTheme {

// Primary colors
constexpr lv_color_t COLOR_BG_DARK = LV_COLOR_MAKE(0x12, 0x12, 0x12);      // Main background
constexpr lv_color_t COLOR_BG_CARD = LV_COLOR_MAKE(0x1E, 0x1E, 0x2E);      // Card background
constexpr lv_color_t COLOR_BG_HEADER = LV_COLOR_MAKE(0x2A, 0x2A, 0x3A);    // Header bar

// Accent colors
constexpr lv_color_t COLOR_ACCENT = LV_COLOR_MAKE(0x4A, 0x90, 0xD9);       // Blue accent
constexpr lv_color_t COLOR_ACCENT_BRIGHT = LV_COLOR_MAKE(0x64B5F6);        // Bright blue

// Status colors
constexpr lv_color_t COLOR_SUCCESS = LV_COLOR_MAKE(0x4CAF50);              // Green - ON/active
constexpr lv_color_t COLOR_WARNING = LV_COLOR_MAKE(0xFFA726);              // Orange - warning
constexpr lv_color_t COLOR_ERROR = LV_COLOR_MAKE(0xEF5350);                // Red - error/off
constexpr lv_color_t COLOR_LINK_OK = LV_COLOR_MAKE(0x66BB6A);              // Link active
constexpr lv_color_t COLOR_LINK_LOST = LV_COLOR_MAKE(0xEF5350);            // Link lost

// Text colors
constexpr lv_color_t COLOR_TEXT_PRIMARY = LV_COLOR_MAKE(0xFF, 0xFF, 0xFF);  // White
constexpr lv_color_t COLOR_TEXT_SECONDARY = LV_COLOR_MAKE(0xB0, 0xB0, 0xB0); // Light gray
constexpr lv_color_t COLOR_TEXT_MUTED = LV_COLOR_MAKE(0x70, 0x70, 0x70);   // Dark gray

// Meter colors
constexpr lv_color_t COLOR_METER_LOW = LV_COLOR_MAKE(0x4CAF50);            // Green
constexpr lv_color_t COLOR_METER_MID = LV_COLOR_MAKE(0xFFEB3B);            // Yellow
constexpr lv_color_t COLOR_METER_HIGH = LV_COLOR_MAKE(0xF44336);           // Red

// Effect states
constexpr lv_color_t COLOR_EFFECT_ON = LV_COLOR_MAKE(0x4CAF50);            // Effect enabled
constexpr lv_color_t COLOR_EFFECT_OFF = LV_COLOR_MAKE(0x424242);           // Effect disabled
constexpr lv_color_t COLOR_EFFECT_PRESSED = LV_COLOR_MAKE(0xFFA726);       // Footswitch pressed

}  // namespace VoxUiTheme

// Style helpers
void ui_theme_init(void);
lv_style_t* ui_style_get_button(void);
lv_style_t* ui_style_get_card(void);
lv_style_t* ui_style_get_header(void);
lv_style_t* ui_style_get_meter(void);
lv_style_t* ui_style_get_text_primary(void);
lv_style_t* ui_style_get_text_secondary(void);

#endif  // UI_THEME_H
