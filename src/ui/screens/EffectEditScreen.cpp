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

static void set_enable_visual(bool enabled);

// --- Reusable parameter components -------------------------------------------------

static lv_obj_t* create_section_header(lv_obj_t* parent, const char* text) {
    lv_obj_t* label = lv_label_create(parent);
    lv_label_set_text(label, text);
    lv_obj_set_style_text_color(label, COLOR_TEXT_MUTED, 0);
    lv_obj_set_style_text_font(label, FONT_TINY, 0);
    return label;
}

// A labelled parameter row with a value readout and an orange horizontal slider.
static lv_obj_t* create_param_slider(lv_obj_t* parent, const char* label,
                                     const char* value, int min, int max, int cur) {
    lv_obj_t* row = lv_obj_create(parent);
    lv_obj_set_size(row, LV_PCT(100), 38);
    lv_obj_set_style_bg_color(row, COLOR_SURFACE, 0);
    lv_obj_set_style_radius(row, RADIUS_S, 0);
    lv_obj_set_style_border_width(row, 0, 0);
    lv_obj_set_style_pad_hor(row, SPACING_M, 0);
    lv_obj_set_style_pad_ver(row, SPACING_XS, 0);
    lv_obj_set_style_pad_row(row, 2, 0);
    lv_obj_set_flex_flow(row, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(row, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);

    lv_obj_t* top = lv_obj_create(row);
    lv_obj_set_size(top, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(top, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(top, 0, 0);
    lv_obj_set_style_pad_all(top, 0, 0);
    lv_obj_set_flex_flow(top, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(top, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t* name = lv_label_create(top);
    lv_label_set_text(name, label);
    lv_obj_set_style_text_color(name, COLOR_TEXT_MUTED, 0);
    lv_obj_set_style_text_font(name, FONT_TINY, 0);

    lv_obj_t* val = lv_label_create(top);
    lv_label_set_text(val, value);
    lv_obj_set_style_text_color(val, COLOR_TEXT_PRIMARY, 0);
    lv_obj_set_style_text_font(val, FONT_SMALL, 0);

    lv_obj_t* slider = lv_slider_create(row);
    lv_obj_set_size(slider, LV_PCT(100), 8);
    lv_slider_set_range(slider, min, max);
    lv_slider_set_value(slider, cur, LV_ANIM_OFF);
    lv_obj_set_ext_click_area(slider, 12);
    lv_obj_set_style_bg_color(slider, COLOR_PANEL, 0);
    lv_obj_set_style_radius(slider, RADIUS_S, 0);
    lv_obj_set_style_bg_color(slider, COLOR_ACCENT, LV_PART_INDICATOR);
    lv_obj_set_style_radius(slider, RADIUS_S, LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(slider, COLOR_ACCENT_BRIGHT, LV_PART_KNOB);
    lv_obj_set_style_radius(slider, LV_RADIUS_CIRCLE, LV_PART_KNOB);
    lv_obj_set_style_pad_all(slider, 2, LV_PART_KNOB);
    return row;
}

// A labelled enum selector rendered as a compact segmented control.
static lv_obj_t* create_param_enum(lv_obj_t* parent, const char* label, const char* value,
                                   const char* const* options, int count, int selected) {
    lv_obj_t* row = lv_obj_create(parent);
    lv_obj_set_size(row, LV_PCT(100), 48);
    lv_obj_set_style_bg_color(row, COLOR_SURFACE, 0);
    lv_obj_set_style_radius(row, RADIUS_S, 0);
    lv_obj_set_style_border_width(row, 0, 0);
    lv_obj_set_style_pad_hor(row, SPACING_M, 0);
    lv_obj_set_style_pad_ver(row, SPACING_XS, 0);
    lv_obj_set_style_pad_row(row, 2, 0);
    lv_obj_set_flex_flow(row, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(row, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);

    lv_obj_t* top = lv_obj_create(row);
    lv_obj_set_size(top, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(top, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(top, 0, 0);
    lv_obj_set_style_pad_all(top, 0, 0);
    lv_obj_set_flex_flow(top, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(top, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t* name = lv_label_create(top);
    lv_label_set_text(name, label);
    lv_obj_set_style_text_color(name, COLOR_TEXT_MUTED, 0);
    lv_obj_set_style_text_font(name, FONT_TINY, 0);

    lv_obj_t* val = lv_label_create(top);
    lv_label_set_text(val, value);
    lv_obj_set_style_text_color(val, COLOR_TEXT_PRIMARY, 0);
    lv_obj_set_style_text_font(val, FONT_SMALL, 0);

    lv_obj_t* seg = lv_obj_create(row);
    lv_obj_set_size(seg, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(seg, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(seg, 0, 0);
    lv_obj_set_style_pad_all(seg, 0, 0);
    lv_obj_set_style_pad_column(seg, 3, 0);
    lv_obj_set_flex_flow(seg, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(seg, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    for (int i = 0; i < count; i++) {
        bool active = (i == selected);
        lv_obj_t* btn = lv_btn_create(seg);
        lv_obj_set_height(btn, 24);
        lv_obj_set_flex_grow(btn, 1);
        lv_obj_set_style_radius(btn, RADIUS_S, 0);
        lv_obj_set_style_shadow_width(btn, 0, 0);
        lv_obj_set_style_border_width(btn, 1, 0);
        lv_obj_set_style_bg_color(btn, active ? COLOR_SURFACE_ELEV : COLOR_PANEL, 0);
        lv_obj_set_style_border_color(btn, active ? COLOR_ACCENT : COLOR_SEPARATOR, 0);

        lv_obj_t* opt = lv_label_create(btn);
        lv_label_set_text(opt, options[i]);
        lv_obj_center(opt);
        lv_obj_set_style_text_font(opt, FONT_SMALL, 0);
        lv_obj_set_style_text_color(opt, active ? COLOR_ACCENT_BRIGHT : COLOR_TEXT_SECONDARY, 0);
    }
    return row;
}

// --- Screen ------------------------------------------------------------------------

void effect_edit_screen_init(lv_obj_t* parent) {
    edit_container = lv_obj_create(parent);
    lv_obj_set_size(edit_container, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(edit_container, COLOR_BG, 0);
    lv_obj_set_style_pad_all(edit_container, SPACING_XS, 0);
    lv_obj_set_style_pad_row(edit_container, SPACING_XS, 0);
    lv_obj_set_style_border_width(edit_container, 0, 0);
    lv_obj_set_flex_flow(edit_container, LV_FLEX_FLOW_COLUMN);

    // HEADER: <  NAME  [toggle]
    lv_obj_t* header = lv_obj_create(edit_container);
    lv_obj_set_size(header, LV_PCT(100), 28);
    lv_obj_set_style_bg_color(header, COLOR_HEADER, 0);
    lv_obj_set_style_radius(header, RADIUS_S, 0);
    lv_obj_set_style_border_width(header, 0, 0);
    lv_obj_set_style_pad_hor(header, SPACING_S, 0);
    lv_obj_set_style_pad_ver(header, 0, 0);
    lv_obj_set_flex_flow(header, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(header, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t* back_btn = lv_btn_create(header);
    lv_obj_set_size(back_btn, 32, 22);
    lv_obj_set_style_bg_color(back_btn, COLOR_SURFACE, 0);
    lv_obj_set_style_border_width(back_btn, 1, 0);
    lv_obj_set_style_border_color(back_btn, COLOR_SEPARATOR, 0);
    lv_obj_set_style_radius(back_btn, RADIUS_S, 0);
    lv_obj_set_style_shadow_width(back_btn, 0, 0);
    lv_obj_t* back_label = lv_label_create(back_btn);
    lv_label_set_text(back_label, "<");
    lv_obj_center(back_label);
    lv_obj_set_style_text_color(back_label, COLOR_TEXT_SECONDARY, 0);
    lv_obj_set_style_text_font(back_label, FONT_SMALL, 0);
    lv_obj_add_event_cb(back_btn, [](lv_event_t* e) {
        ui_navigate_to(UiScreenId::FX_CHAIN);
    }, LV_EVENT_CLICKED, NULL);

    title_label = lv_label_create(header);
    lv_label_set_text(title_label, "EDIT");
    lv_obj_set_style_text_color(title_label, COLOR_TEXT_PRIMARY, 0);
    lv_obj_set_style_text_font(title_label, FONT_BODY, 0);
    lv_obj_set_flex_grow(title_label, 1);
    lv_obj_set_style_text_align(title_label, LV_TEXT_ALIGN_CENTER, 0);

    enable_btn = lv_btn_create(header);
    lv_obj_set_size(enable_btn, 44, 22);
    lv_obj_set_style_radius(enable_btn, RADIUS_S, 0);
    lv_obj_set_style_shadow_width(enable_btn, 0, 0);
    enable_label = lv_label_create(enable_btn);
    lv_label_set_text(enable_label, "OFF");
    lv_obj_center(enable_label);
    lv_obj_set_style_text_font(enable_label, FONT_TINY, 0);
    lv_obj_add_event_cb(enable_btn, [](lv_event_t* e) {
        if (current_edit_id >= 0) {
            UiAction action = { UiActionType::ToggleEffect, (uint16_t)current_edit_id, 0 };
            ui_emit_action(action);
        }
    }, LV_EVENT_CLICKED, NULL);
    set_enable_visual(false);

    // PARAMETERS: scrollable, ready for more rows / a second voice
    lv_obj_t* params_area = lv_obj_create(edit_container);
    lv_obj_set_size(params_area, LV_PCT(100), LV_PCT(100));
    lv_obj_set_flex_grow(params_area, 1);
    lv_obj_set_style_bg_color(params_area, COLOR_BG, 0);
    lv_obj_set_style_border_width(params_area, 0, 0);
    lv_obj_set_style_pad_all(params_area, 0, 0);
    lv_obj_set_style_pad_row(params_area, SPACING_XS, 0);
    lv_obj_set_flex_flow(params_area, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_scroll_dir(params_area, LV_DIR_VER);
    lv_obj_set_scrollbar_mode(params_area, LV_SCROLLBAR_MODE_AUTO);

    create_section_header(params_area, "VOICE 1");

    static const char* pan_opts[] = {"L", "C", "R"};
    create_param_slider(params_area, "INTERVAL", "+3rd", -12, 12, 3);
    create_param_slider(params_area, "LEVEL", "-9.0 dB", -60, 6, -9);
    create_param_enum(params_area, "PAN", "C", pan_opts, 3, 1);
    create_param_slider(params_area, "MIX", "65%", 0, 100, 65);
}

static void set_enable_visual(bool enabled) {
    if (!enable_btn || !enable_label) return;
    lv_obj_set_style_bg_color(enable_btn, enabled ? COLOR_ACCENT_DARK : COLOR_SURFACE, 0);
    lv_obj_set_style_border_width(enable_btn, enabled ? 0 : 1, 0);
    lv_obj_set_style_border_color(enable_btn, COLOR_SEPARATOR, 0);
    lv_label_set_text(enable_label, enabled ? "ON" : "OFF");
    lv_obj_set_style_text_color(enable_label, enabled ? COLOR_TEXT_PRIMARY : COLOR_TEXT_MUTED, 0);
}

void effect_edit_load_effect(int effectId) {
    if (effectId >= 0 && effectId < 4) {
        current_edit_id = effectId;
        if (title_label) {
            lv_label_set_text(title_label, effect_names[effectId]);
        }
    }
}
