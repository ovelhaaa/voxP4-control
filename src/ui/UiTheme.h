#ifndef UI_THEME_H
#define UI_THEME_H

#include <lvgl.h>

// Enum para acesso programático às cores
typedef enum {
    COLOR_BG_DARK,
    COLOR_BG_CARD,
    COLOR_BG_HEADER,
    COLOR_ACCENT,
    COLOR_ACCENT_BRIGHT,
    COLOR_ACCENT_GREEN,
    COLOR_SUCCESS,
    COLOR_WARNING,
    COLOR_ERROR,
    COLOR_LINK_OK,
    COLOR_LINK_LOST,
    COLOR_DISABLED,
    COLOR_BORDER,
    COLOR_TEXT_PRIMARY,
    COLOR_TEXT_SECONDARY,
    COLOR_TEXT_MUTED,
    COLOR_METER_LOW,
    COLOR_METER_MID,
    COLOR_METER_HIGH,
    COLOR_EFFECT_ON,
    COLOR_EFFECT_OFF
} UiThemeColor;

// Color palette - dark theme optimized for stage use
namespace VoxUiTheme {

// Primary colors
constexpr lv_color_t COLOR_BG_DARK = LV_COLOR_MAKE(0x12, 0x12, 0x12);      // Main background
constexpr lv_color_t COLOR_BG_CARD = LV_COLOR_MAKE(0x1E, 0x1E, 0x2E);      // Card background
constexpr lv_color_t COLOR_BG_HEADER = LV_COLOR_MAKE(0x2A, 0x2A, 0x3A);    // Header bar

// Accent colors
constexpr lv_color_t COLOR_ACCENT = LV_COLOR_MAKE(0x4A, 0x90, 0xD9);       // Blue accent
constexpr lv_color_t COLOR_ACCENT_BRIGHT = LV_COLOR_MAKE(0x64, 0xB5, 0xF6);        // Bright blue
constexpr lv_color_t COLOR_ACCENT_GREEN = LV_COLOR_MAKE(0x00, 0xE6, 0x76);         // Green for ON state

// Status colors
constexpr lv_color_t COLOR_SUCCESS = LV_COLOR_MAKE(0x4C, 0xAF, 0x50);              // Green - ON/active
constexpr lv_color_t COLOR_WARNING = LV_COLOR_MAKE(0xFF, 0xA7, 0x26);              // Orange - warning
constexpr lv_color_t COLOR_ERROR = LV_COLOR_MAKE(0xEF, 0x53, 0x50);                // Red - error/off
constexpr lv_color_t COLOR_LINK_OK = LV_COLOR_MAKE(0x66, 0xBB, 0x6A);              // Link active
constexpr lv_color_t COLOR_LINK_LOST = LV_COLOR_MAKE(0xEF, 0x53, 0x50);            // Link lost
constexpr lv_color_t COLOR_DISABLED = LV_COLOR_MAKE(0x42, 0x42, 0x42);             // Disabled state
constexpr lv_color_t COLOR_BORDER = LV_COLOR_MAKE(0x3A, 0x3A, 0x4A);               // Border color

// Text colors
constexpr lv_color_t COLOR_TEXT_PRIMARY = LV_COLOR_MAKE(0xFF, 0xFF, 0xFF);  // White
constexpr lv_color_t COLOR_TEXT_SECONDARY = LV_COLOR_MAKE(0xB0, 0xB0, 0xB0); // Light gray
constexpr lv_color_t COLOR_TEXT_MUTED = LV_COLOR_MAKE(0x70, 0x70, 0x70);   // Dark gray

// Meter colors
constexpr lv_color_t COLOR_METER_LOW = LV_COLOR_MAKE(0x4C, 0xAF, 0x50);            // Green
constexpr lv_color_t COLOR_METER_MID = LV_COLOR_MAKE(0xFF, 0xEB, 0x3B);            // Yellow
constexpr lv_color_t COLOR_METER_HIGH = LV_COLOR_MAKE(0xF4, 0x43, 0x36);           // Red

// Effect states
constexpr lv_color_t COLOR_EFFECT_ON = LV_COLOR_MAKE(0x4C, 0xAF, 0x50);            // Effect enabled
constexpr lv_color_t COLOR_EFFECT_OFF = LV_COLOR_MAKE(0x42, 0x42, 0x42);           // Effect disabled
constexpr lv_color_t COLOR_EFFECT_PRESSED = LV_COLOR_MAKE(0xFF, 0xA7, 0x26);       // Footswitch pressed

}  // namespace VoxUiTheme

// Theme API
void ui_theme_init(void);
lv_color_t UiTheme_get_color(UiThemeColor color);
lv_style_t* ui_style_get_button(void);
lv_style_t* ui_style_get_card(void);
lv_style_t* ui_style_get_header(void);
lv_style_t* ui_style_get_meter(void);
lv_style_t* ui_style_get_text_primary(void);
lv_style_t* ui_style_get_text_secondary(void);

#endif  // UI_THEME_H
