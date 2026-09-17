#include "EffectEditScreen.h"
#include "../UiApp.h"
#include "../UiTheme.h"
#include <cstdio>

static lv_obj_t* edit_container = nullptr;
static lv_obj_t* title_label = nullptr;
static lv_obj_t* enable_btn = nullptr;
static lv_obj_t* enable_label = nullptr;
static int current_edit_id = -1;

static const char* effect_names[] = {"HARMONY", "REVERB", "DELAY", "LIMITER"};

void effect_edit_screen_init(lv_obj_t* parent) {
    edit_container = lv_obj_create(parent);
    lv_obj_set_size(edit_container, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(edit_container, COLOR_BG_DARK, 0);
    lv_obj_set_style_pad_all(edit_container, SPACING_M, 0);
    lv_obj_set_style_border_width(edit_container, 0, 0);
    lv_obj_set_flex_flow(edit_container, LV_FLEX_FLOW_COLUMN);

    // HEADER
    lv_obj_t* header = lv_obj_create(edit_container);
    lv_obj_set_size(header, LV_PCT(100), 32);
    lv_obj_set_style_bg_color(header, COLOR_BG_HEADER, 0);
    lv_obj_set_style_radius(header, 0, 0);
    lv_obj_set_style_border_width(header, 0, 0);
    lv_obj_set_style_pad_hor(header, SPACING_M, 0);
    lv_obj_set_flex_flow(header, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(header, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    // Back Button
    lv_obj_t* back_btn = lv_btn_create(header);
    lv_obj_set_size(back_btn, 40, 24);
    lv_obj_set_style_bg_color(back_btn, COLOR_BG_SURFACE, 0);
    lv_obj_t* back_label = lv_label_create(back_btn);
    lv_label_set_text(back_label, "<");
    lv_obj_center(back_label);
    lv_obj_add_event_cb(back_btn, [](lv_event_t* e) {
        ui_navigate_to(UiScreenId::FX_CHAIN);
    }, LV_EVENT_CLICKED, NULL);

    // Title
    title_label = lv_label_create(header);
    lv_label_set_text(title_label, "EDIT");
    lv_obj_set_style_text_color(title_label, COLOR_TEXT_PRIMARY, 0);
    lv_obj_set_style_text_font(title_label, FONT_EMPHASIS, 0);

    // Enable Toggle
    enable_btn = lv_btn_create(header);
    lv_obj_set_size(enable_btn, 48, 24);
    lv_obj_set_style_bg_color(enable_btn, COLOR_BG_SURFACE, 0);
    enable_label = lv_label_create(enable_btn);
    lv_label_set_text(enable_label, "OFF");
    lv_obj_center(enable_label);
    lv_obj_add_event_cb(enable_btn, [](lv_event_t* e) {
        if (current_edit_id >= 0) {
            UiAction action = { UiActionType::ToggleEffect, (uint16_t)current_edit_id, 0 };
            ui_emit_action(action);
        }
    }, LV_EVENT_CLICKED, NULL);

    // PARAMETERS (Mocked for now)
    lv_obj_t* params_area = lv_obj_create(edit_container);
    lv_obj_set_size(params_area, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(params_area, COLOR_BG_DARK, 0);
    lv_obj_set_style_border_width(params_area, 0, 0);
    lv_obj_set_style_pad_all(params_area, 0, 0);
    lv_obj_set_flex_flow(params_area, LV_FLEX_FLOW_COLUMN);

    lv_obj_t* param1_row = lv_obj_create(params_area);
    lv_obj_set_size(param1_row, LV_PCT(100), 40);
    lv_obj_set_style_bg_color(param1_row, COLOR_BG_SURFACE, 0);
    lv_obj_set_style_border_width(param1_row, 0, 0);
    lv_obj_t* p1_label = lv_label_create(param1_row);
    lv_label_set_text(p1_label, "Param 1");
    lv_obj_align(p1_label, LV_ALIGN_LEFT_MID, 8, 0);
    lv_obj_t* p1_slider = lv_slider_create(param1_row);
    lv_obj_set_size(p1_slider, 160, 10);
    lv_obj_align(p1_slider, LV_ALIGN_RIGHT_MID, -8, 0);
}

void effect_edit_load_effect(int effectId) {
    if (effectId >= 0 && effectId < 4) {
        current_edit_id = effectId;
        if (title_label) {
            lv_label_set_text(title_label, effect_names[effectId]);
        }
    }
}
