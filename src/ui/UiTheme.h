#ifndef UI_THEME_H
#define UI_THEME_H

#include <lvgl.h>

// VoxP4 control surface design tokens.
// Identity: dark industrial + warm orange + turquoise audio feedback.
// Optimized for a 320x240 stage controller; the orange accent is reserved for
// selection / active effect / primary action, turquoise for live audio data.
namespace VoxUiTheme {

// Backgrounds
constexpr lv_color_t COLOR_BG             = LV_COLOR_MAKE(0x09, 0x0A, 0x0E);  // #090A0E
constexpr lv_color_t COLOR_SURFACE        = LV_COLOR_MAKE(0x14, 0x15, 0x1B);  // #14151B
constexpr lv_color_t COLOR_SURFACE_ELEV   = LV_COLOR_MAKE(0x1C, 0x1D, 0x24);  // #1C1D24
constexpr lv_color_t COLOR_HEADER         = LV_COLOR_MAKE(0x10, 0x11, 0x16);  // #101116
constexpr lv_color_t COLOR_NAV            = LV_COLOR_MAKE(0x10, 0x11, 0x16);  // #101116
constexpr lv_color_t COLOR_PANEL          = LV_COLOR_MAKE(0x20, 0x21, 0x28);  // #202128
constexpr lv_color_t COLOR_BORDER         = LV_COLOR_MAKE(0x34, 0x34, 0x3C);  // #34343C
constexpr lv_color_t COLOR_SEPARATOR      = LV_COLOR_MAKE(0x29, 0x2A, 0x30);  // #292A30

// Text
constexpr lv_color_t COLOR_TEXT_PRIMARY   = LV_COLOR_MAKE(0xF0, 0xED, 0xE5);  // #F0EDE5
constexpr lv_color_t COLOR_TEXT_SECONDARY = LV_COLOR_MAKE(0xB1, 0xAC, 0xA3);  // #B1ACA3
constexpr lv_color_t COLOR_TEXT_MUTED     = LV_COLOR_MAKE(0x71, 0x6E, 0x69);  // #716E69
constexpr lv_color_t COLOR_TEXT_FAINT     = LV_COLOR_MAKE(0x4A, 0x49, 0x47);  // #4A4947

// Main accent - burnt orange (selection, active control, primary action)
constexpr lv_color_t COLOR_ACCENT          = LV_COLOR_MAKE(0xF4, 0x51, 0x26);  // #F45126
constexpr lv_color_t COLOR_ACCENT_BRIGHT   = LV_COLOR_MAKE(0xFF, 0x60, 0x30);  // #FF6030
constexpr lv_color_t COLOR_ACCENT_DARK     = LV_COLOR_MAKE(0xB8, 0x3A, 0x1C);  // #B83A1C

// Audio / live feedback accent - turquoise (pitch, meters, telemetry)
constexpr lv_color_t COLOR_AUDIO           = LV_COLOR_MAKE(0x20, 0xD6, 0xC7);  // #20D6C7
constexpr lv_color_t COLOR_AUDIO_BRIGHT    = LV_COLOR_MAKE(0x55, 0xEF, 0xE2);  // #55EFE2
constexpr lv_color_t COLOR_AUDIO_DARK      = LV_COLOR_MAKE(0x14, 0x8E, 0x87);  // #148E87

// Semantic
constexpr lv_color_t COLOR_WARNING         = LV_COLOR_MAKE(0xE6, 0xA6, 0x3A);  // #E6A63A
constexpr lv_color_t COLOR_ERROR           = LV_COLOR_MAKE(0xE6, 0x50, 0x50);  // #E65050
constexpr lv_color_t COLOR_DISABLED        = LV_COLOR_MAKE(0x41, 0x41, 0x47);  // #414147

}  // namespace VoxUiTheme

using VoxUiTheme::COLOR_BG;
using VoxUiTheme::COLOR_SURFACE;
using VoxUiTheme::COLOR_SURFACE_ELEV;
using VoxUiTheme::COLOR_HEADER;
using VoxUiTheme::COLOR_NAV;
using VoxUiTheme::COLOR_PANEL;
using VoxUiTheme::COLOR_BORDER;
using VoxUiTheme::COLOR_SEPARATOR;
using VoxUiTheme::COLOR_TEXT_PRIMARY;
using VoxUiTheme::COLOR_TEXT_SECONDARY;
using VoxUiTheme::COLOR_TEXT_MUTED;
using VoxUiTheme::COLOR_TEXT_FAINT;
using VoxUiTheme::COLOR_ACCENT;
using VoxUiTheme::COLOR_ACCENT_BRIGHT;
using VoxUiTheme::COLOR_ACCENT_DARK;
using VoxUiTheme::COLOR_AUDIO;
using VoxUiTheme::COLOR_AUDIO_BRIGHT;
using VoxUiTheme::COLOR_AUDIO_DARK;
using VoxUiTheme::COLOR_WARNING;
using VoxUiTheme::COLOR_ERROR;
using VoxUiTheme::COLOR_DISABLED;

// Font definitions - LVGL built-in Montserrat fonts already compiled in.
#define FONT_TINY       &lv_font_montserrat_10
#define FONT_SMALL      &lv_font_montserrat_12
#define FONT_BODY       &lv_font_montserrat_14
#define FONT_EMPHASIS   &lv_font_montserrat_16
#define FONT_HERO       &lv_font_montserrat_16

// Spacing tokens (pixels)
#define SPACING_XS      4
#define SPACING_S       6
#define SPACING_M       8
#define SPACING_L       10
#define MARGIN_SCREEN   6

// Radius tokens (pixels) - restrained, hardware-like
#define RADIUS_S        3
#define RADIUS_M        5
#define RADIUS_L        6

// Touch target sizes
#define TOUCH_TARGET_MIN    40
#define TOUCH_TARGET_IDEAL  44

// Theme API
void ui_theme_init(void);
lv_style_t* ui_style_get_bg(void);
lv_style_t* ui_style_get_surface(void);
lv_style_t* ui_style_get_elevated(void);
lv_style_t* ui_style_get_header(void);
lv_style_t* ui_style_get_text_primary(void);
lv_style_t* ui_style_get_text_secondary(void);
lv_style_t* ui_style_get_text_muted(void);

// Instant, cheap pressed-state feedback. No animation, shadow or scale.
void ui_apply_pressed(lv_obj_t* obj, lv_color_t bg, lv_color_t border);

// Marks a control as unavailable: no click, no pressed feedback, muted border
// and child labels, dimmed. Used for controls whose backend is not wired yet so
// they never look "falsely alive".
void ui_apply_disabled(lv_obj_t* obj);

#endif  // UI_THEME_H
