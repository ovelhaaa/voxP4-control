#ifndef LV_CONF_H
#define LV_CONF_H

#include <stdint.h>

// Graphics configuration
#define LV_COLOR_DEPTH 16
#define LV_COLOR_16_SWAP 0
#define LV_COLOR_SCREEN_TRANSP 0

// Buffer configuration
#define LV_MEM_CUSTOM 1
#define LV_MEM_SIZE (48U * 1024U)

// Font configuration
#define LV_FONT_MONTSERRAT_10 1
#define LV_FONT_MONTSERRAT_12 1
#define LV_FONT_MONTSERRAT_14 1
#define LV_FONT_MONTSERRAT_16 1
#define LV_FONT_MONTSERRAT_18 0
#define LV_FONT_MONTSERRAT_20 0
#define LV_FONT_MONTSERRAT_22 0
#define LV_FONT_MONTSERRAT_24 0
#define LV_FONT_MONTSERRAT_26 0
#define LV_FONT_MONTSERRAT_28 0
#define LV_FONT_MONTSERRAT_30 0
#define LV_FONT_MONTSERRAT_32 0
#define LV_FONT_MONTSERRAT_34 0
#define LV_FONT_MONTSERRAT_36 0
#define LV_FONT_MONTSERRAT_38 0
#define LV_FONT_MONTSERRAT_40 0
#define LV_FONT_MONTSERRAT_42 0
#define LV_FONT_MONTSERRAT_44 0
#define LV_FONT_MONTSERRAT_46 0
#define LV_FONT_MONTSERRAT_48 0

// Text encoding
#define LV_TXT_ENC LV_TXT_ENC_UTF8

// Feature enablement
#define LV_USE_LOG 0
#define LV_USE_ASSERT_NULL 0
#define LV_USE_ASSERT_MALLOC 0
#define LV_USE_ASSERT_MEMCPY 0
#define LV_USE_ASSERT_STR 0
#define LV_USE_ASSERT_STYLE 0
#define LV_USE_ASSERT_OBJ 0

// Modules
#define LV_USE_ANIMATION 1
#define LV_USE_FS_STDIO 0
#define LV_USE_GPU 0
#define LV_USE_GRADIENT_SIMPLE 1
#define LV_USE_IMG_TRANSFORM 0
#define LV_USE_SNAPSHOT 0
#define LV_USE_MONKEY 0
#define LV_USE_GRIDNAV 0
#define LV_USE_FRAGMENT 0
#define LV_USE_IMGFONT 0
#define LV_USE_SVG 0

// Objects
#define LV_USE_ARC 1
#define LV_USE_ANIMIMG 1
#define LV_USE_BAR 1
#define LV_USE_BTN 1
#define LV_USE_BTNMATRIX 1
#define LV_USE_CALENDAR 1
#define LV_USE_CANVAS 1
#define LV_USE_CHECKBOX 1
#define LV_USE_DROPDOWN 1
#define LV_USE_IMAGE 1
#define LV_USE_IMAGEBUTTON 1
#define LV_USE_KEYBOARD 1
#define LV_USE_LABEL 1
#define LV_USE_LED 1
#define LV_USE_LINE 1
#define LV_USE_ROLLER 1
#define LV_USE_SLIDER 1
#define LV_USE_SWITCH 1
#define LV_USE_TEXTAREA 1
#define LV_USE_TABLE 1
#define LV_USE_CHART 1
#define LV_USE_METER 1

// Others
#define LV_USE_MSG 0
#define LV_USE_LIST 1
#define LV_USE_MENU 1
#define LV_USE_TILEVIEW 1
#define LV_USE_WIN 1
#define LV_USE_SPAN 1
#define LV_USE_SPINBOX 1
#define LV_USE_SPINNER 1
#define LV_USE_TABVIEW 1
#define LV_USE_VIEWPORT 0

// Themes
#define LV_THEME_DEFAULT_GROW 0
#define LV_THEME_DEFAULT_DARK 1

// Layouts
#define LV_USE_FLEX 1
#define LV_USE_GRID 1

#endif /* LV_CONF_H */
