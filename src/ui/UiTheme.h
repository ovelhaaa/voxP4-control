#ifndef UI_THEME_H
#define UI_THEME_H

#include <lvgl.h>

// Enum para acesso programático às cores
typedef enum {
    THEME_COLOR_BG_DARK,
    THEME_COLOR_BG_CARD,
    THEME_COLOR_BG_HEADER,
    THEME_COLOR_ACCENT,
    THEME_COLOR_ACCENT_BRIGHT,
    THEME_COLOR_ACCENT_GREEN,
    THEME_COLOR_SUCCESS,
    THEME_COLOR_WARNING,
    THEME_COLOR_ERROR,
    THEME_COLOR_LINK_OK,
    THEME_COLOR_LINK_LOST,
    THEME_COLOR_DISABLED,
    THEME_COLOR_BORDER,
    THEME_COLOR_TEXT_PRIMARY,
    THEME_COLOR_TEXT_SECONDARY,
    THEME_COLOR_TEXT_MUTED,
    THEME_COLOR_METER_LOW,
    THEME_COLOR_METER_MID,
    THEME_COLOR_METER_HIGH,
    THEME_COLOR_EFFECT_ON,
    THEME_COLOR_EFFECT_OFF
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

using VoxUiTheme::COLOR_BG_DARK;
using VoxUiTheme::COLOR_BG_CARD;
using VoxUiTheme::COLOR_BG_HEADER;
using VoxUiTheme::COLOR_ACCENT;
using VoxUiTheme::COLOR_ACCENT_BRIGHT;
using VoxUiTheme::COLOR_ACCENT_GREEN;
using VoxUiTheme::COLOR_SUCCESS;
using VoxUiTheme::COLOR_WARNING;
using VoxUiTheme::COLOR_ERROR;
using VoxUiTheme::COLOR_LINK_OK;
using VoxUiTheme::COLOR_LINK_LOST;
using VoxUiTheme::COLOR_DISABLED;
using VoxUiTheme::COLOR_BORDER;
using VoxUiTheme::COLOR_TEXT_PRIMARY;
using VoxUiTheme::COLOR_TEXT_SECONDARY;
using VoxUiTheme::COLOR_TEXT_MUTED;
using VoxUiTheme::COLOR_METER_LOW;
using VoxUiTheme::COLOR_METER_MID;
using VoxUiTheme::COLOR_METER_HIGH;
using VoxUiTheme::COLOR_EFFECT_ON;
using VoxUiTheme::COLOR_EFFECT_OFF;

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
