#include "FxChainScreen.h"
#include "../UiApp.h"
#include "../UiTheme.h"

static lv_obj_t* fx_rows[4] = {nullptr};
static lv_obj_t* fx_leds[4] = {nullptr};
static lv_obj_t* fx_name_labels[4] = {nullptr};
static lv_obj_t* fx_param_labels[4] = {nullptr};
static lv_obj_t* fx_state_labels[4] = {nullptr};

static const char* effect_names[] = {"HARMONY", "REVERB", "DELAY", "LIMITER"};

static void fx_row_clicked(lv_event_t* e) {
    lv_obj_t* row = lv_event_get_target(e);
    int effect_id = (int)(intptr_t)lv_obj_get_user_data(row);
    UiAction action = { UiActionType::OpenEffect, (uint16_t)effect_id, 0 };
    ui_emit_action(action);
}

void fx_chain_screen_init(lv_obj_t* parent) {
    lv_obj_t* fx_container = lv_obj_create(parent);
    lv_obj_set_size(fx_container, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(fx_container, COLOR_BG, 0);
    lv_obj_set_style_pad_all(fx_container, SPACING_XS, 0);
    lv_obj_set_style_pad_row(fx_container, SPACING_XS, 0);
    lv_obj_set_style_border_width(fx_container, 0, 0);
    lv_obj_set_flex_flow(fx_container, LV_FLEX_FLOW_COLUMN);

    // === HEADER ===
    lv_obj_t* header = lv_obj_create(fx_container);
    lv_obj_set_size(header, LV_PCT(100), 24);
    lv_obj_set_style_bg_color(header, COLOR_HEADER, 0);
    lv_obj_set_style_radius(header, RADIUS_S, 0);
    lv_obj_set_style_border_width(header, 0, 0);
    lv_obj_set_style_pad_hor(header, SPACING_M, 0);
    lv_obj_set_style_pad_ver(header, 0, 0);
    lv_obj_set_flex_flow(header, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(header, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t* title = lv_label_create(header);
    lv_label_set_text(title, "FX CHAIN");
    lv_obj_set_style_text_color(title, COLOR_TEXT_PRIMARY, 0);
    lv_obj_set_style_text_font(title, FONT_BODY, 0);

    // === MODULE RACK ===
    lv_obj_t* rack = lv_obj_create(fx_container);
    lv_obj_set_size(rack, LV_PCT(100), LV_PCT(100));
    lv_obj_set_flex_grow(rack, 1);
    lv_obj_set_style_bg_color(rack, COLOR_BG, 0);
    lv_obj_set_style_border_width(rack, 0, 0);
    lv_obj_set_style_pad_all(rack, 0, 0);
    lv_obj_set_style_pad_row(rack, 3, 0);
    lv_obj_set_flex_flow(rack, LV_FLEX_FLOW_COLUMN);

    for (int i = 0; i < 4; i++) {
        lv_obj_t* row = lv_obj_create(rack);
        lv_obj_set_size(row, LV_PCT(100), 28);
        lv_obj_set_style_bg_color(row, COLOR_SURFACE, 0);
        lv_obj_set_style_radius(row, RADIUS_S, 0);
        lv_obj_set_style_border_width(row, 1, 0);
        lv_obj_set_style_border_color(row, COLOR_SEPARATOR, 0);
        lv_obj_set_style_pad_hor(row, SPACING_M, 0);
        lv_obj_set_style_pad_ver(row, 0, 0);
        lv_obj_set_style_pad_column(row, SPACING_S, 0);
        lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(row, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_add_flag(row, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_user_data(row, (void*)(intptr_t)i);
        lv_obj_add_event_cb(row, fx_row_clicked, LV_EVENT_CLICKED, NULL);

        lv_obj_t* led = lv_obj_create(row);
        lv_obj_set_size(led, 7, 7);
        lv_obj_set_style_bg_color(led, COLOR_DISABLED, 0);
        lv_obj_set_style_radius(led, LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_border_width(led, 0, 0);
        lv_obj_set_style_pad_all(led, 0, 0);

        lv_obj_t* name_label = lv_label_create(row);
        lv_label_set_text(name_label, effect_names[i]);
        lv_obj_set_style_text_color(name_label, COLOR_TEXT_SECONDARY, 0);
        lv_obj_set_style_text_font(name_label, FONT_SMALL, 0);
        lv_obj_set_width(name_label, 78);

        lv_obj_t* param_label = lv_label_create(row);
        lv_label_set_text(param_label, "--");
        lv_obj_set_style_text_color(param_label, COLOR_TEXT_MUTED, 0);
        lv_obj_set_style_text_font(param_label, FONT_SMALL, 0);
        lv_obj_set_flex_grow(param_label, 1);

        lv_obj_t* state_label = lv_label_create(row);
        lv_label_set_text(state_label, "OFF");
        lv_obj_set_style_text_color(state_label, COLOR_TEXT_MUTED, 0);
        lv_obj_set_style_text_font(state_label, FONT_TINY, 0);

        fx_rows[i] = row;
        fx_leds[i] = led;
        fx_name_labels[i] = name_label;
        fx_param_labels[i] = param_label;
        fx_state_labels[i] = state_label;
    }

    // === GLOBAL ACTIONS ===
    lv_obj_t* actions_row = lv_obj_create(fx_container);
    lv_obj_set_size(actions_row, LV_PCT(100), 32);
    lv_obj_set_style_bg_color(actions_row, COLOR_BG, 0);
    lv_obj_set_style_border_width(actions_row, 0, 0);
    lv_obj_set_style_pad_all(actions_row, 0, 0);
    lv_obj_set_style_pad_column(actions_row, SPACING_S, 0);
    lv_obj_set_flex_flow(actions_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(actions_row, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t* all_on_btn = lv_btn_create(actions_row);
    lv_obj_set_size(all_on_btn, 130, 32);
    lv_obj_set_style_bg_opa(all_on_btn, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(all_on_btn, 1, 0);
    lv_obj_set_style_border_color(all_on_btn, COLOR_AUDIO_DARK, 0);
    lv_obj_set_style_radius(all_on_btn, RADIUS_S, 0);
    lv_obj_set_style_shadow_width(all_on_btn, 0, 0);
    lv_obj_t* all_on_label = lv_label_create(all_on_btn);
    lv_label_set_text(all_on_label, "ALL ON");
    lv_obj_center(all_on_label);
    lv_obj_set_style_text_color(all_on_label, COLOR_AUDIO, 0);
    lv_obj_set_style_text_font(all_on_label, FONT_SMALL, 0);
    lv_obj_add_event_cb(all_on_btn, [](lv_event_t* e) {
        UiAction action = { UiActionType::AllEffectsOn, 0, 0 };
        ui_emit_action(action);
    }, LV_EVENT_CLICKED, NULL);

    lv_obj_t* bypass_btn = lv_btn_create(actions_row);
    lv_obj_set_size(bypass_btn, 130, 32);
    lv_obj_set_style_bg_color(bypass_btn, COLOR_ACCENT_DARK, 0);
    lv_obj_set_style_border_width(bypass_btn, 0, 0);
    lv_obj_set_style_radius(bypass_btn, RADIUS_S, 0);
    lv_obj_set_style_shadow_width(bypass_btn, 0, 0);
    lv_obj_t* bypass_label = lv_label_create(bypass_btn);
    lv_label_set_text(bypass_label, "BYPASS");
    lv_obj_center(bypass_label);
    lv_obj_set_style_text_color(bypass_label, COLOR_TEXT_PRIMARY, 0);
    lv_obj_set_style_text_font(bypass_label, FONT_SMALL, 0);
    lv_obj_add_event_cb(bypass_btn, [](lv_event_t* e) {
        UiAction action = { UiActionType::GlobalBypass, 0, 0 };
        ui_emit_action(action);
    }, LV_EVENT_CLICKED, NULL);
}

void fx_chain_update_effect_state(int effectId, bool enabled, const char* paramName, const char* paramValue) {
    if (effectId < 0 || effectId >= 4 || !fx_rows[effectId]) return;

    lv_obj_set_style_border_color(fx_rows[effectId], enabled ? COLOR_ACCENT : COLOR_SEPARATOR, 0);
    lv_obj_set_style_bg_color(fx_leds[effectId], enabled ? COLOR_ACCENT : COLOR_DISABLED, 0);
    lv_obj_set_style_text_color(fx_name_labels[effectId],
        enabled ? COLOR_TEXT_PRIMARY : COLOR_TEXT_SECONDARY, 0);
    lv_obj_set_style_text_color(fx_state_labels[effectId],
        enabled ? COLOR_ACCENT_BRIGHT : COLOR_TEXT_MUTED, 0);
    lv_label_set_text(fx_state_labels[effectId], enabled ? "ON" : "OFF");

    (void)paramName;
    if (paramValue) {
        lv_label_set_text(fx_param_labels[effectId], paramValue);
        lv_obj_set_style_text_color(fx_param_labels[effectId],
            enabled ? COLOR_TEXT_PRIMARY : COLOR_TEXT_MUTED, 0);
    }
}
