#include "SettingsScreen.h"
#include "../UiApp.h"
#include "../UiTheme.h"

static lv_obj_t* create_menu_entry(lv_obj_t* parent, const char* title, const char* subtitle,
                                   UiScreenId target) {
    lv_obj_t* entry = lv_obj_create(parent);
    lv_obj_set_size(entry, LV_PCT(100), 72);
    lv_obj_set_style_bg_color(entry, COLOR_SURFACE, 0);
    lv_obj_set_style_radius(entry, RADIUS_M, 0);
    lv_obj_set_style_border_width(entry, 1, 0);
    lv_obj_set_style_border_color(entry, COLOR_SEPARATOR, 0);
    lv_obj_set_style_pad_hor(entry, SPACING_L, 0);
    lv_obj_set_style_pad_ver(entry, SPACING_M, 0);
    lv_obj_set_flex_flow(entry, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(entry, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_add_flag(entry, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_user_data(entry, (void*)(intptr_t)target);
    lv_obj_add_event_cb(entry, [](lv_event_t* e) {
        lv_obj_t* obj = lv_event_get_target(e);
        UiScreenId target = (UiScreenId)(intptr_t)lv_obj_get_user_data(obj);
        ui_navigate_to(target);
    }, LV_EVENT_CLICKED, NULL);

    lv_obj_t* text = lv_obj_create(entry);
    lv_obj_set_size(text, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(text, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(text, 0, 0);
    lv_obj_set_style_pad_all(text, 0, 0);
    lv_obj_set_style_pad_row(text, 2, 0);
    lv_obj_set_flex_flow(text, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(text, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);

    lv_obj_t* title_label = lv_label_create(text);
    lv_label_set_text(title_label, title);
    lv_obj_set_style_text_color(title_label, COLOR_TEXT_PRIMARY, 0);
    lv_obj_set_style_text_font(title_label, FONT_BODY, 0);

    lv_obj_t* subtitle_label = lv_label_create(text);
    lv_label_set_text(subtitle_label, subtitle);
    lv_obj_set_style_text_color(subtitle_label, COLOR_TEXT_MUTED, 0);
    lv_obj_set_style_text_font(subtitle_label, FONT_TINY, 0);

    lv_obj_t* chevron = lv_label_create(entry);
    lv_label_set_text(chevron, ">");
    lv_obj_set_style_text_color(chevron, COLOR_TEXT_MUTED, 0);
    lv_obj_set_style_text_font(chevron, FONT_EMPHASIS, 0);

    return entry;
}

void settings_screen_init(lv_obj_t* parent) {
    lv_obj_t* container = lv_obj_create(parent);
    lv_obj_set_size(container, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(container, COLOR_BG, 0);
    lv_obj_set_style_pad_all(container, SPACING_XS, 0);
    lv_obj_set_style_pad_row(container, SPACING_S, 0);
    lv_obj_set_style_border_width(container, 0, 0);
    lv_obj_set_flex_flow(container, LV_FLEX_FLOW_COLUMN);

    lv_obj_t* header = lv_obj_create(container);
    lv_obj_set_size(header, LV_PCT(100), 24);
    lv_obj_set_style_bg_color(header, COLOR_HEADER, 0);
    lv_obj_set_style_radius(header, RADIUS_S, 0);
    lv_obj_set_style_border_width(header, 0, 0);
    lv_obj_set_style_pad_hor(header, SPACING_M, 0);
    lv_obj_set_style_pad_ver(header, 0, 0);
    lv_obj_set_flex_flow(header, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(header, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t* title = lv_label_create(header);
    lv_label_set_text(title, "SETTINGS");
    lv_obj_set_style_text_color(title, COLOR_TEXT_PRIMARY, 0);
    lv_obj_set_style_text_font(title, FONT_BODY, 0);

    create_menu_entry(container, "FOOTSWITCHES", "assign actions and behavior", UiScreenId::FOOTSWITCH);
    create_menu_entry(container, "SYSTEM", "link, audio and diagnostics", UiScreenId::SYSTEM);
}
