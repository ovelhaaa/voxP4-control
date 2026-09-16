#ifndef UI_THEME_H
#define UI_THEME_H

#include <lvgl.h>

// Enum para acesso programático às cores
typedef enum {
    THEME_COLOR_BG_DARK,
    THEME_COLOR_BG_SURFACE,
    THEME_COLOR_BG_ELEVATED,
    THEME_COLOR_BG_HEADER,
    THEME_COLOR_ACCENT_PRIMARY,
    THEME_COLOR_ACCENT_BRIGHT,
    THEME_COLOR_ACCENT_CYAN,
    THEME_COLOR_SUCCESS,
    THEME_COLOR_WARNING,
    THEME_COLOR_ERROR,
    THEME_COLOR_LINK_OK,
    THEME_COLOR_LINK_LOST,
    THEME_COLOR_DISABLED,
    THEME_COLOR_BORDER_SUBTLE,
    THEME_COLOR_BORDER_ACTIVE,
    THEME_COLOR_TEXT_PRIMARY,
    THEME_COLOR_TEXT_SECONDARY,
    THEME_COLOR_TEXT_MUTED,
    THEME_COLOR_METER_SAFE,
    THEME_COLOR_METER_WARNING,
    THEME_COLOR_METER_CLIP,
    THEME_COLOR_METER_PEAK,
    THEME_COLOR_EFFECT_LED_OFF,
    THEME_COLOR_EFFECT_LED_ON,
    THEME_COLOR_FS_PRESSED
} UiThemeColor;

// Color palette - dark theme optimized for stage use
// Professional vocal processor aesthetic - flat, musical, high contrast
namespace VoxUiTheme {

// Background colors - deep dark for stage use
constexpr lv_color_t COLOR_BG_DARK = LV_COLOR_MAKE(0x09, 0x0B, 0x0F);      // Main background (#090B0F)
constexpr lv_color_t COLOR_BG_SURFACE = LV_COLOR_MAKE(0x12, 0x16, 0x20);   // Card/surface background (#121620)
constexpr lv_color_t COLOR_BG_ELEVATED = LV_COLOR_MAKE(0x19, 0x1E, 0x2A);  // Elevated surfaces (#191E2A)
constexpr lv_color_t COLOR_BG_HEADER = LV_COLOR_MAKE(0x1E, 0x23, 0x30);    // Header bar (#1E2330)

// Accent colors - electric cyan/blue for active states
constexpr lv_color_t COLOR_ACCENT_PRIMARY = LV_COLOR_MAKE(0x06, 0xB6, 0xD4);  // Primary accent cyan (#06B6D4)
constexpr lv_color_t COLOR_ACCENT_BRIGHT = LV_COLOR_MAKE(0x22, 0xD5, 0xF0);   // Bright cyan (#22D5F0)
constexpr lv_color_t COLOR_ACCENT_CYAN = LV_COLOR_MAKE(0x00, 0xE5, 0xFF);     // Electric cyan LED (#00E5FF)

// Status colors - semantic meaning
constexpr lv_color_t COLOR_SUCCESS = LV_COLOR_MAKE(0x4C, 0xAF, 0x50);         // Success/active (#4CAF50)
constexpr lv_color_t COLOR_WARNING = LV_COLOR_MAKE(0xFF, 0xA7, 0x26);         // Warning/pressed (#FFA726)
constexpr lv_color_t COLOR_ERROR = LV_COLOR_MAKE(0xEF, 0x53, 0x50);           // Error (#EF5350)
constexpr lv_color_t COLOR_LINK_OK = LV_COLOR_MAKE(0x66, 0xBB, 0x6A);         // Link connected (#66BB6A)
constexpr lv_color_t COLOR_LINK_LOST = LV_COLOR_MAKE(0xEF, 0x53, 0x50);       // Link lost (#EF5350)
constexpr lv_color_t COLOR_DISABLED = LV_COLOR_MAKE(0x42, 0x42, 0x42);        // Disabled state (#424242)

// Border colors
constexpr lv_color_t COLOR_BORDER_SUBTLE = LV_COLOR_MAKE(0x29, 0x2F, 0x3D);   // Subtle borders (#292F3D)
constexpr lv_color_t COLOR_BORDER_ACTIVE = LV_COLOR_MAKE(0x06, 0xB6, 0xD4);   // Active border cyan (#06B6D4)

// Text colors - legible on stage
constexpr lv_color_t COLOR_TEXT_PRIMARY = LV_COLOR_MAKE(0xF2, 0xF5, 0xF7);    // Primary text (#F2F5F7)
constexpr lv_color_t COLOR_TEXT_SECONDARY = LV_COLOR_MAKE(0xA5, 0xAD, 0xBA);  // Secondary text (#A5ADBA)
constexpr lv_color_t COLOR_TEXT_MUTED = LV_COLOR_MAKE(0x68, 0x71, 0x81);      // Muted text (#687181)

// Meter colors - dB zones
constexpr lv_color_t COLOR_METER_SAFE = LV_COLOR_MAKE(0x00, 0xE6, 0x76);      // Safe zone: -60 to -12dB (#00E676)
constexpr lv_color_t COLOR_METER_WARNING = LV_COLOR_MAKE(0xFF, 0xD7, 0x40);   // Warning: -12 to -3dB (#FFD740)
constexpr lv_color_t COLOR_METER_CLIP = LV_COLOR_MAKE(0xFF, 0x52, 0x52);      // Clip: -3 to 0dB (#FF5252)
constexpr lv_color_t COLOR_METER_PEAK = LV_COLOR_MAKE(0xFF, 0x00, 0x00);      // Peak hold marker (#FF0000)

// Effect state colors - subtle, not full green cards
constexpr lv_color_t COLOR_EFFECT_LED_OFF = LV_COLOR_MAKE(0x42, 0x42, 0x42);  // LED off (#424242)
constexpr lv_color_t COLOR_EFFECT_LED_ON = LV_COLOR_MAKE(0x64, 0xF0, 0xFF);   // LED on bright cyan (#64F0FF)

// Footswitch pressed
constexpr lv_color_t COLOR_FS_PRESSED = LV_COLOR_MAKE(0xFF, 0xA7, 0x26);      // Amber when pressed (#FFA726)

}  // namespace VoxUiTheme

using VoxUiTheme::COLOR_BG_DARK;
using VoxUiTheme::COLOR_BG_SURFACE;
using VoxUiTheme::COLOR_BG_ELEVATED;
using VoxUiTheme::COLOR_BG_HEADER;
using VoxUiTheme::COLOR_ACCENT_PRIMARY;
using VoxUiTheme::COLOR_ACCENT_BRIGHT;
using VoxUiTheme::COLOR_ACCENT_CYAN;
using VoxUiTheme::COLOR_SUCCESS;
using VoxUiTheme::COLOR_WARNING;
using VoxUiTheme::COLOR_ERROR;
using VoxUiTheme::COLOR_LINK_OK;
using VoxUiTheme::COLOR_LINK_LOST;
using VoxUiTheme::COLOR_DISABLED;
using VoxUiTheme::COLOR_BORDER_SUBTLE;
using VoxUiTheme::COLOR_BORDER_ACTIVE;
using VoxUiTheme::COLOR_TEXT_PRIMARY;
using VoxUiTheme::COLOR_TEXT_SECONDARY;
using VoxUiTheme::COLOR_TEXT_MUTED;
using VoxUiTheme::COLOR_METER_SAFE;
using VoxUiTheme::COLOR_METER_WARNING;
using VoxUiTheme::COLOR_METER_CLIP;
using VoxUiTheme::COLOR_METER_PEAK;
using VoxUiTheme::COLOR_EFFECT_LED_OFF;
using VoxUiTheme::COLOR_EFFECT_LED_ON;
using VoxUiTheme::COLOR_FS_PRESSED;

// Font definitions - LVGL built-in Montserrat fonts
#define FONT_TINY       &lv_font_montserrat_10
#define FONT_SMALL      &lv_font_montserrat_12
#define FONT_BODY       &lv_font_montserrat_14
#define FONT_EMPHASIS   &lv_font_montserrat_16
#define FONT_HERO       &lv_font_montserrat_24

// Spacing tokens (pixels)
#define SPACING_XS      4
#define SPACING_S       6
#define SPACING_M       8
#define SPACING_L       12

// Radius tokens (pixels)
#define RADIUS_S        4
#define RADIUS_M        6
#define RADIUS_L        8

// Touch target sizes
#define TOUCH_TARGET_MIN    40
#define TOUCH_TARGET_IDEAL  44

// Theme API
void ui_theme_init(void);
lv_color_t UiTheme_get_color(UiThemeColor color);
lv_style_t* ui_style_get_bg(void);
lv_style_t* ui_style_get_surface(void);
lv_style_t* ui_style_get_elevated(void);
lv_style_t* ui_style_get_card(void);
lv_style_t* ui_style_get_header(void);
lv_style_t* ui_style_get_meter(void);
lv_style_t* ui_style_get_text_primary(void);
lv_style_t* ui_style_get_text_secondary(void);
lv_style_t* ui_style_get_text_muted(void);
lv_style_t* ui_style_get_button(void);
lv_style_t* ui_style_get_nav_tab(void);

#endif  // UI_THEME_H
