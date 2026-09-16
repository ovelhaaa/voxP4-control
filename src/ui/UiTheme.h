#ifndef UI_THEME_H
#define UI_THEME_H

#include <lvgl.h>

/**
 * VoxP4 Control Surface - Design System
 * 
 * Paleta inspirada em equipamentos musicais profissionais modernos:
 * - Flat, escuro, compacto e musical
 * - Sem skeuomorphism, sem texturas pesadas
 * - Hierarquia visual clara para leitura instantânea no palco
 * - Cores com significado semântico (verde = sucesso/link, não apenas "ON")
 */

// Enum para acesso programático às cores
typedef enum {
    // Backgrounds
    THEME_COLOR_BG,
    THEME_COLOR_SURFACE,
    THEME_COLOR_SURFACE_ELEVATED,
    THEME_COLOR_SURFACE_SELECTED,
    
    // Borders
    THEME_COLOR_BORDER_SUBTLE,
    THEME_COLOR_BORDER_ACTIVE,
    
    // Text
    THEME_COLOR_TEXT_PRIMARY,
    THEME_COLOR_TEXT_SECONDARY,
    THEME_COLOR_TEXT_MUTED,
    
    // Accents
    THEME_COLOR_ACCENT_PRIMARY,
    THEME_COLOR_ACCENT_SECONDARY,
    THEME_COLOR_ACCENT_BRIGHT,
    
    // Status semantic colors
    THEME_COLOR_SUCCESS,
    THEME_COLOR_WARNING,
    THEME_COLOR_ERROR,
    THEME_COLOR_LINK_OK,
    THEME_COLOR_LINK_LOST,
    
    // Meter zones
    THEME_COLOR_METER_SAFE,
    THEME_COLOR_METER_WARNING,
    THEME_COLOR_METER_CLIP,
    
    // Effect states
    THEME_COLOR_EFFECT_ACTIVE,
    THEME_COLOR_EFFECT_INACTIVE,
    THEME_COLOR_EFFECT_PRESSED,
    
    // Footswitch states
    THEME_COLOR_FS_PRESSED,
    
    // Misc
    THEME_COLOR_DISABLED
} UiThemeColor;

// Fontes por hierarquia de informação
typedef enum {
    THEME_FONT_HERO,        // 28-32px - dados musicais principais (nota)
    THEME_FONT_EMPHASIS,    // 20-24px - valores importantes (frequência)
    THEME_FONT_BODY,        // 14-16px - labels primários
    THEME_FONT_SMALL,       // 12-13px - labels secundários
    THEME_FONT_TINY         // 10-11px - diagnósticos mínimos
} UiThemeFont;

// Spacing tokens (px)
#define THEME_SPACING_XS    4
#define THEME_SPACING_S     6
#define THEME_SPACING_M     8
#define THEME_SPACING_L     12

// Radius tokens (px)
#define THEME_RADIUS_S      4
#define THEME_RADIUS_M      6
#define THEME_RADIUS_L      8

// Dimensões de touch targets
#define THEME_TOUCH_MIN     40
#define THEME_TOUCH_IDEAL   44

// Altura do header (não desperdiçar espaço)
#define THEME_HEADER_HEIGHT 28

// Altura da nav bar
#define THEME_NAV_HEIGHT    36

// Color palette - dark, musical, professional
namespace VoxUiTheme {

// Backgrounds - paleta escura profunda
constexpr lv_color_t COLOR_BG                 = LV_COLOR_MAKE(0x09, 0x0B, 0x0F);  // Background principal
constexpr lv_color_t COLOR_SURFACE            = LV_COLOR_MAKE(0x12, 0x16, 0x20);  // Cards/surfaces
constexpr lv_color_t COLOR_SURFACE_ELEVATED   = LV_COLOR_MAKE(0x19, 0x1E, 0x2A);  // Elements elevados
constexpr lv_color_t COLOR_SURFACE_SELECTED   = LV_COLOR_MAKE(0x1F, 0x26, 0x33);  // Seleção/active

// Borders
constexpr lv_color_t COLOR_BORDER_SUBTLE      = LV_COLOR_MAKE(0x29, 0x2F, 0x3D);  // Bordas sutis
constexpr lv_color_t COLOR_BORDER_ACTIVE      = LV_COLOR_MAKE(0x3D, 0x4A, 0x5C);  // Bordas ativas

// Text hierarchy
constexpr lv_color_t COLOR_TEXT_PRIMARY       = LV_COLOR_MAKE(0xF2, 0xF5, 0xF7);  // Texto principal
constexpr lv_color_t COLOR_TEXT_SECONDARY     = LV_COLOR_MAKE(0xA5, 0xAD, 0xBA);  // Labels secundários
constexpr lv_color_t COLOR_TEXT_MUTED         = LV_COLOR_MAKE(0x68, 0x71, 0x81);  // Texto desativado/muted

// Accent colors - cyan/blue-cyan elétrico
constexpr lv_color_t COLOR_ACCENT_PRIMARY     = LV_COLOR_MAKE(0x06, 0xB6, 0xD4);  // Cyan primary
constexpr lv_color_t COLOR_ACCENT_SECONDARY   = LV_COLOR_MAKE(0x00, 0xD4, 0xB8);  // Teal secondary
constexpr lv_color_t COLOR_ACCENT_BRIGHT      = LV_COLOR_MAKE(0x64, 0xF0, 0xFF);  // Electric cyan highlight

// Semantic status colors
constexpr lv_color_t COLOR_SUCCESS            = LV_COLOR_MAKE(0x4C, 0xAF, 0x50);  // Success/link OK
constexpr lv_color_t COLOR_WARNING            = LV_COLOR_MAKE(0xFF, 0xA7, 0x26);  // Warning/pressed
constexpr lv_color_t COLOR_ERROR              = LV_COLOR_MAKE(0xEF, 0x53, 0x50);  // Error/clip
constexpr lv_color_t COLOR_LINK_OK            = LV_COLOR_MAKE(0x66, 0xBB, 0x6A);  // Link connected
constexpr lv_color_t COLOR_LINK_LOST          = LV_COLOR_MAKE(0xEF, 0x53, 0x50);  // Link lost

// Meter zones - gradient de verde para vermelho
constexpr lv_color_t COLOR_METER_SAFE         = LV_COLOR_MAKE(0x00, 0xE6, 0x76);  // -60 a -12 dB (green)
constexpr lv_color_t COLOR_METER_WARNING      = LV_COLOR_MAKE(0xFF, 0xD7, 0x40);  // -12 a -3 dB (yellow)
constexpr lv_color_t COLOR_METER_CLIP         = LV_COLOR_MAKE(0xFF, 0x52, 0x52);  // -3 a 0 dB (red)

// Effect states - NÃO usar verde grande para ON, usar accent border/LED
constexpr lv_color_t COLOR_EFFECT_ACTIVE      = LV_COLOR_MAKE(0x19, 0x1E, 0x2A);  // Card ativo (sutil)
constexpr lv_color_t COLOR_EFFECT_INACTIVE    = LV_COLOR_MAKE(0x12, 0x16, 0x20);  // Card inativo
constexpr lv_color_t COLOR_EFFECT_PRESSED     = LV_COLOR_MAKE(0x2A, 0x2F, 0x3D);  // Card sendo pressionado

// Footswitch
constexpr lv_color_t COLOR_FS_PRESSED         = LV_COLOR_MAKE(0xFF, 0xA7, 0x26);  // Amber quando pressionado

// Misc
constexpr lv_color_t COLOR_DISABLED           = LV_COLOR_MAKE(0x2A, 0x2F, 0x3D);  // Disabled

// Legacy aliases para compatibilidade durante transição
constexpr lv_color_t COLOR_BG_DARK            = COLOR_BG;
constexpr lv_color_t COLOR_BG_CARD            = COLOR_SURFACE;
constexpr lv_color_t COLOR_BG_HEADER          = COLOR_SURFACE_ELEVATED;
constexpr lv_color_t COLOR_ACCENT             = COLOR_ACCENT_PRIMARY;
constexpr lv_color_t COLOR_ACCENT_GREEN       = COLOR_SUCCESS;
constexpr lv_color_t COLOR_METER_LOW          = COLOR_METER_SAFE;
constexpr lv_color_t COLOR_METER_MID          = COLOR_METER_WARNING;
constexpr lv_color_t COLOR_METER_HIGH         = COLOR_METER_CLIP;
constexpr lv_color_t COLOR_EFFECT_ON          = COLOR_EFFECT_ACTIVE;
constexpr lv_color_t COLOR_EFFECT_OFF         = COLOR_EFFECT_INACTIVE;

}  // namespace VoxUiTheme

// Export all colors
using VoxUiTheme::COLOR_BG;
using VoxUiTheme::COLOR_SURFACE;
using VoxUiTheme::COLOR_SURFACE_ELEVATED;
using VoxUiTheme::COLOR_SURFACE_SELECTED;
using VoxUiTheme::COLOR_BORDER_SUBTLE;
using VoxUiTheme::COLOR_BORDER_ACTIVE;
using VoxUiTheme::COLOR_TEXT_PRIMARY;
using VoxUiTheme::COLOR_TEXT_SECONDARY;
using VoxUiTheme::COLOR_TEXT_MUTED;
using VoxUiTheme::COLOR_ACCENT_PRIMARY;
using VoxUiTheme::COLOR_ACCENT_SECONDARY;
using VoxUiTheme::COLOR_ACCENT_BRIGHT;
using VoxUiTheme::COLOR_SUCCESS;
using VoxUiTheme::COLOR_WARNING;
using VoxUiTheme::COLOR_ERROR;
using VoxUiTheme::COLOR_LINK_OK;
using VoxUiTheme::COLOR_LINK_LOST;
using VoxUiTheme::COLOR_METER_SAFE;
using VoxUiTheme::COLOR_METER_WARNING;
using VoxUiTheme::COLOR_METER_CLIP;
using VoxUiTheme::COLOR_EFFECT_ACTIVE;
using VoxUiTheme::COLOR_EFFECT_INACTIVE;
using VoxUiTheme::COLOR_EFFECT_PRESSED;
using VoxUiTheme::COLOR_FS_PRESSED;
using VoxUiTheme::COLOR_DISABLED;

// Legacy aliases
using VoxUiTheme::COLOR_BG_DARK;
using VoxUiTheme::COLOR_BG_CARD;
using VoxUiTheme::COLOR_BG_HEADER;
using VoxUiTheme::COLOR_ACCENT;
using VoxUiTheme::COLOR_ACCENT_BRIGHT;
using VoxUiTheme::COLOR_ACCENT_GREEN;
using VoxUiTheme::COLOR_METER_LOW;
using VoxUiTheme::COLOR_METER_MID;
using VoxUiTheme::COLOR_METER_HIGH;
using VoxUiTheme::COLOR_EFFECT_ON;
using VoxUiTheme::COLOR_EFFECT_OFF;

// Theme API - inicialização e acesso
void ui_theme_init(void);
lv_color_t UiTheme_get_color(UiThemeColor color);

// Estilos predefinidos
lv_style_t* ui_style_get_bg(void);
lv_style_t* ui_style_get_surface(void);
lv_style_t* ui_style_get_surface_elevated(void);
lv_style_t* ui_style_get_card(void);
lv_style_t* ui_style_get_card_active(void);
lv_style_t* ui_style_get_header(void);
lv_style_t* ui_style_get_navbar(void);
lv_style_t* ui_style_get_meter_bg(void);
lv_style_t* ui_style_get_text_primary(void);
lv_style_t* ui_style_get_text_secondary(void);
lv_style_t* ui_style_get_text_muted(void);
lv_style_t* ui_style_get_text_hero(void);
lv_style_t* ui_style_get_button_primary(void);
lv_style_t* ui_style_get_button_touch(void);
lv_style_t* ui_style_get_effect_card(void);
lv_style_t* ui_style_get_effect_card_active(void);

// Helpers de fonte
const lv_font_t* ui_theme_get_font(UiThemeFont font);

#endif  // UI_THEME_H
