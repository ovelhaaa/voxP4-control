#include "SettingsScreen.h"
#include "../UiApp.h"
#include "../UiTheme.h"

static lv_obj_t* settings_container = nullptr;

void settings_screen_init(lv_obj_t* parent) {
    settings_container = lv_obj_create(parent);
    lv_obj_set_size(settings_container, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(settings_container, COLOR_BG_DARK, 0);
    lv_obj_set_style_pad_all(settings_container, SPACING_M, 0);
    lv_obj_set_style_border_width(settings_container, 0, 0);
    lv_obj_set_flex_flow(settings_container, LV_FLEX_FLOW_COLUMN);

    // HEADER
    lv_obj_t* header = lv_obj_create(settings_container);
    lv_obj_set_size(header, LV_PCT(100), 28);
    lv_obj_set_style_bg_color(header, COLOR_BG_HEADER, 0);
    lv_obj_set_style_radius(header, 0, 0);
    lv_obj_set_style_border_width(header, 0, 0);
    lv_obj_set_style_pad_hor(header, SPACING_M, 0);
    lv_obj_set_flex_flow(header, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(header, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t* title = lv_label_create(header);
    lv_label_set_text(title, "SETTINGS");
    lv_obj_set_style_text_color(title, COLOR_TEXT_PRIMARY, 0);
    lv_obj_set_style_text_font(title, FONT_EMPHASIS, 0);

    // CONTENT
    lv_obj_t* content = lv_obj_create(settings_container);
    lv_obj_set_size(content, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(content, COLOR_BG_DARK, 0);
    lv_obj_set_style_border_width(content, 0, 0);
    lv_obj_set_style_pad_all(content, 0, 0);
    lv_obj_set_flex_flow(content, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(content, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    // Footswitch Menu Button
    lv_obj_t* fs_btn = lv_btn_create(content);
    lv_obj_set_size(fs_btn, 160, 48);
    lv_obj_set_style_bg_color(fs_btn, COLOR_BG_SURFACE, 0);
    lv_obj_set_style_radius(fs_btn, RADIUS_M, 0);
    lv_obj_t* fs_label = lv_label_create(fs_btn);
    lv_label_set_text(fs_label, "FOOTSWITCHES");
    lv_obj_center(fs_label);
    lv_obj_add_event_cb(fs_btn, [](lv_event_t* e) {
        ui_navigate_to(UiScreenId::FOOTSWITCH);
    }, LV_EVENT_CLICKED, NULL);

    // System Menu Button
    lv_obj_t* sys_btn = lv_btn_create(content);
    lv_obj_set_size(sys_btn, 160, 48);
    lv_obj_set_style_bg_color(sys_btn, COLOR_BG_SURFACE, 0);
    lv_obj_set_style_radius(sys_btn, RADIUS_M, 0);
    lv_obj_t* sys_label = lv_label_create(sys_btn);
    lv_label_set_text(sys_label, "SYSTEM INFO");
    lv_obj_center(sys_label);
    lv_obj_add_event_cb(sys_btn, [](lv_event_t* e) {
        ui_navigate_to(UiScreenId::SYSTEM);
    }, LV_EVENT_CLICKED, NULL);
}
