#include "MasterScreen.h"
#include "../UiApp.h"
#include "../UiTheme.h"
#include <cmath>
#include <cstdio>

using namespace VoxUiTheme;

static lv_obj_t* master_container = nullptr;
static lv_obj_t* tempo_slider = nullptr;
static lv_obj_t* tempo_value = nullptr;
static lv_obj_t* tap_label = nullptr;
static lv_obj_t* ceiling_slider = nullptr;
static lv_obj_t* ceiling_value = nullptr;
static lv_obj_t* mute_btn = nullptr;
static lv_obj_t* mute_label = nullptr;
static lv_obj_t* routing_seg = nullptr;
static lv_obj_t* source_seg = nullptr;

static constexpr int kTempoSteps = 270; // 30..300 @ 1
static constexpr int kCeilingSteps = 90; // 0.10..1.00 @ 0.01

static void emit_set(uint16_t wireId, float value) {
    ui_emit_action({UiActionType::SetParameter, wireId, value});
}

static void style_slider(lv_obj_t* slider) {
    lv_obj_set_style_bg_color(slider, COLOR_PANEL, 0);
    lv_obj_set_style_radius(slider, RADIUS_S, 0);
    lv_obj_set_style_bg_color(slider, COLOR_ACCENT, LV_PART_INDICATOR);
    lv_obj_set_style_radius(slider, RADIUS_S, LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(slider, COLOR_ACCENT_BRIGHT, LV_PART_KNOB);
    lv_obj_set_style_radius(slider, LV_RADIUS_CIRCLE, LV_PART_KNOB);
    lv_obj_set_style_pad_all(slider, 2, LV_PART_KNOB);
}

static lv_obj_t* make_card(lv_obj_t* parent, int height) {
    lv_obj_t* card = lv_obj_create(parent);
    lv_obj_set_size(card, LV_PCT(100), height);
    lv_obj_clear_flag(card, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(card, COLOR_SURFACE, 0);
    lv_obj_set_style_radius(card, RADIUS_S, 0);
    lv_obj_set_style_border_width(card, 0, 0);
    lv_obj_set_style_pad_hor(card, SPACING_M, 0);
    lv_obj_set_style_pad_ver(card, SPACING_XS, 0);
    lv_obj_set_style_pad_row(card, 2, 0);
    lv_obj_set_flex_flow(card, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(card, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START,
                          LV_FLEX_ALIGN_START);
    return card;
}

static lv_obj_t* make_row_header(lv_obj_t* card, const char* label, const char* value,
                                 lv_obj_t** value_out) {
    lv_obj_t* top = lv_obj_create(card);
    lv_obj_set_size(top, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_clear_flag(top, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_opa(top, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(top, 0, 0);
    lv_obj_set_style_pad_all(top, 0, 0);
    lv_obj_set_flex_flow(top, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(top, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER,
                          LV_FLEX_ALIGN_CENTER);

    lv_obj_t* name = lv_label_create(top);
    lv_label_set_text(name, label);
    lv_obj_set_style_text_color(name, COLOR_TEXT_MUTED, 0);
    lv_obj_set_style_text_font(name, FONT_TINY, 0);

    lv_obj_t* val = lv_label_create(top);
    lv_label_set_text(val, value);
    lv_obj_set_style_text_color(val, COLOR_TEXT_PRIMARY, 0);
    lv_obj_set_style_text_font(val, FONT_SMALL, 0);
    if (value_out) *value_out = val;
    return top;
}

static void set_mute_visual(bool on) {
    if (!mute_btn || !mute_label) return;
    lv_obj_set_style_bg_color(mute_btn, on ? COLOR_ACCENT_DARK : COLOR_SURFACE_ELEV, 0);
    lv_obj_set_style_border_width(mute_btn, on ? 0 : 1, 0);
    lv_obj_set_style_border_color(mute_btn, COLOR_SEPARATOR, 0);
    lv_label_set_text(mute_label, on ? "MUTED" : "OFF");
    lv_obj_set_style_text_color(mute_label, on ? COLOR_TEXT_PRIMARY : COLOR_TEXT_MUTED, 0);
}

static void set_segmented_visual(lv_obj_t* seg, int index) {
    if (!seg) return;
    const uint32_t count = lv_obj_get_child_cnt(seg);
    for (uint32_t i = 0; i < count; i++) {
        lv_obj_t* opt = lv_obj_get_child(seg, i);
        const bool active = ((int)i == index);
        lv_obj_set_style_bg_color(opt, active ? COLOR_SURFACE_ELEV : COLOR_PANEL, 0);
        lv_obj_set_style_border_color(opt, active ? COLOR_ACCENT : COLOR_SEPARATOR, 0);
        lv_obj_t* label = lv_obj_get_child(opt, 0);
        if (label) {
            lv_obj_set_style_text_color(label,
                active ? COLOR_ACCENT_BRIGHT : COLOR_TEXT_SECONDARY, 0);
        }
    }
}

static void tempo_event(lv_event_t* e) {
    lv_obj_t* slider = lv_event_get_target(e);
    const float value = 30.0f + lv_slider_get_value(slider);
    emit_set(VOXP4_PARAM_TEMPO_BPM, value);
}

static void ceiling_event(lv_event_t* e) {
    lv_obj_t* slider = lv_event_get_target(e);
    const float value = 0.10f + lv_slider_get_value(slider) * 0.01f;
    emit_set(VOXP4_PARAM_LIMITER_CEILING, value);
}

static void mute_event(lv_event_t* e) {
    const bool on = ui_get_parameter(VOXP4_PARAM_OUTPUT_MUTE_DRY) < 0.5f;
    set_mute_visual(on);
    emit_set(VOXP4_PARAM_OUTPUT_MUTE_DRY, on ? 1.0f : 0.0f);
}

struct SegBinding {
    lv_obj_t* seg;
    uint16_t wireId;
    UiParamMeta meta;
};
static SegBinding s_routing;
static SegBinding s_source;

static void seg_event(lv_event_t* e) {
    lv_obj_t* btn = lv_event_get_target(e);
    const int index = (int)(intptr_t)lv_obj_get_user_data(btn);
    lv_obj_t* seg = lv_obj_get_parent(btn);
    const SegBinding* binding =
        static_cast<const SegBinding*>(lv_obj_get_user_data(seg));
    if (!binding) return;
    set_segmented_visual(seg, index);
    emit_set(binding->wireId, (float)index);
}

static lv_obj_t* build_segmented(lv_obj_t* card, const UiParamMeta& meta,
                                 SegBinding* binding) {
    lv_obj_t* seg = lv_obj_create(card);
    lv_obj_set_size(seg, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_clear_flag(seg, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_opa(seg, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(seg, 0, 0);
    lv_obj_set_style_pad_all(seg, 0, 0);
    lv_obj_set_style_pad_column(seg, 3, 0);
    lv_obj_set_flex_flow(seg, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(seg, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER,
                          LV_FLEX_ALIGN_CENTER);

    for (int i = 0; i < meta.optionCount; i++) {
        lv_obj_t* btn = lv_btn_create(seg);
        lv_obj_set_height(btn, 24);
        lv_obj_set_flex_grow(btn, 1);
        lv_obj_set_style_radius(btn, RADIUS_S, 0);
        lv_obj_set_style_shadow_width(btn, 0, 0);
        lv_obj_set_style_border_width(btn, 1, 0);
        lv_obj_set_style_bg_color(btn, COLOR_PANEL, 0);
        lv_obj_set_style_border_color(btn, COLOR_SEPARATOR, 0);
        ui_apply_pressed(btn, COLOR_SURFACE_ELEV, COLOR_BORDER);

        lv_obj_t* opt = lv_label_create(btn);
        lv_label_set_text(opt, meta.options[i]);
        lv_obj_center(opt);
        lv_obj_set_style_text_font(opt, FONT_TINY, 0);

        lv_obj_set_user_data(btn, (void*)(intptr_t)i);
        lv_obj_add_event_cb(btn, seg_event, LV_EVENT_CLICKED, NULL);
    }
    if (binding) {
        binding->seg = seg;
        binding->wireId = meta.wireId;
        binding->meta = meta;
        lv_obj_set_user_data(seg, (void*)binding);
    }
    return seg;
}

void master_screen_init(lv_obj_t* parent) {
    master_container = lv_obj_create(parent);
    lv_obj_set_size(master_container, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(master_container, COLOR_BG, 0);
    lv_obj_set_style_pad_all(master_container, SPACING_XS, 0);
    lv_obj_set_style_pad_row(master_container, SPACING_XS, 0);
    lv_obj_set_style_border_width(master_container, 0, 0);
    lv_obj_set_flex_flow(master_container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_scroll_dir(master_container, LV_DIR_VER);
    lv_obj_set_scrollbar_mode(master_container, LV_SCROLLBAR_MODE_AUTO);

    lv_obj_t* header = lv_obj_create(master_container);
    lv_obj_set_size(header, LV_PCT(100), 24);
    lv_obj_set_style_bg_color(header, COLOR_HEADER, 0);
    lv_obj_set_style_radius(header, RADIUS_S, 0);
    lv_obj_set_style_border_width(header, 0, 0);
    lv_obj_set_style_pad_hor(header, SPACING_M, 0);
    lv_obj_set_style_pad_ver(header, 0, 0);
    lv_obj_set_flex_flow(header, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(header, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER,
                          LV_FLEX_ALIGN_CENTER);

    lv_obj_t* title = lv_label_create(header);
    lv_label_set_text(title, "MASTER / ROUTING");
    lv_obj_set_style_text_color(title, COLOR_TEXT_PRIMARY, 0);
    lv_obj_set_style_text_font(title, FONT_BODY, 0);

    tap_label = lv_label_create(header);
    lv_label_set_text(tap_label, "TAP TEMPO");
    lv_obj_set_style_text_color(tap_label, COLOR_ACCENT_BRIGHT, 0);
    lv_obj_set_style_text_font(tap_label, FONT_TINY, 0);
    lv_obj_add_flag(tap_label, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_ext_click_area(tap_label, 12);
    lv_obj_add_event_cb(tap_label, [](lv_event_t* e) {
        ui_tap_tempo();
    }, LV_EVENT_CLICKED, NULL);

    // TEMPO
    lv_obj_t* tempo_card = make_card(master_container, 48);
    make_row_header(tempo_card, "TEMPO", "120 BPM", &tempo_value);
    tempo_slider = lv_slider_create(tempo_card);
    lv_obj_set_size(tempo_slider, LV_PCT(100), 8);
    lv_slider_set_range(tempo_slider, 0, kTempoSteps);
    lv_obj_set_ext_click_area(tempo_slider, 12);
    style_slider(tempo_slider);
    lv_obj_add_event_cb(tempo_slider, tempo_event, LV_EVENT_VALUE_CHANGED, NULL);

    // MASTER LIMITER CEILING (there is no enable parameter; only the ceiling)
    lv_obj_t* ceiling_card = make_card(master_container, 48);
    make_row_header(ceiling_card, "LIMITER CEILING", "-0.4 dB", &ceiling_value);
    ceiling_slider = lv_slider_create(ceiling_card);
    lv_obj_set_size(ceiling_slider, LV_PCT(100), 8);
    lv_slider_set_range(ceiling_slider, 0, kCeilingSteps);
    lv_obj_set_ext_click_area(ceiling_slider, 12);
    style_slider(ceiling_slider);
    lv_obj_add_event_cb(ceiling_slider, ceiling_event, LV_EVENT_VALUE_CHANGED, NULL);

    // MUTE DRY
    lv_obj_t* mute_card = make_card(master_container, 44);
    lv_obj_t* mute_top = make_row_header(mute_card, "MUTE DRY", nullptr, nullptr);
    mute_btn = lv_btn_create(mute_top);
    lv_obj_set_size(mute_btn, 64, 26);
    lv_obj_set_ext_click_area(mute_btn, 6);
    lv_obj_set_style_radius(mute_btn, RADIUS_S, 0);
    lv_obj_set_style_shadow_width(mute_btn, 0, 0);
    ui_apply_pressed(mute_btn, COLOR_SURFACE_ELEV, COLOR_BORDER);
    mute_label = lv_label_create(mute_btn);
    lv_obj_center(mute_label);
    lv_obj_set_style_text_font(mute_label, FONT_TINY, 0);
    lv_obj_add_event_cb(mute_btn, mute_event, LV_EVENT_CLICKED, NULL);

    // ROUTING
    const UiParamMeta* routing = ui_param_meta(VOXP4_PARAM_OUTPUT_SPATIAL_ROUTING);
    if (routing) {
        lv_obj_t* card = make_card(master_container, 52);
        make_row_header(card, "SPATIAL ROUTING", nullptr, nullptr);
        routing_seg = build_segmented(card, *routing, &s_routing);
    }

    // SOURCE
    const UiParamMeta* source = ui_param_meta(VOXP4_PARAM_OUTPUT_SPATIAL_SOURCE);
    if (source) {
        lv_obj_t* card = make_card(master_container, 52);
        make_row_header(card, "SPATIAL SOURCE", nullptr, nullptr);
        source_seg = build_segmented(card, *source, &s_source);
    }

    master_screen_refresh();
}

void master_screen_refresh(void) {
    // TEMPO
    if (tempo_slider) {
        const float bpm = ui_get_parameter(VOXP4_PARAM_TEMPO_BPM);
        int idx = (int)lround(bpm - 30.0f);
        if (idx < 0) idx = 0;
        if (idx > kTempoSteps) idx = kTempoSteps;
        lv_slider_set_value(tempo_slider, idx, LV_ANIM_OFF);
        if (tempo_value) {
            char buf[16];
            snprintf(buf, sizeof(buf), "%d BPM", (int)(bpm + 0.5f));
            lv_label_set_text(tempo_value, buf);
        }
    }
    // CEILING
    if (ceiling_slider) {
        const float ceiling = ui_get_parameter(VOXP4_PARAM_LIMITER_CEILING);
        int idx = (int)lround((ceiling - 0.10f) / 0.01f);
        if (idx < 0) idx = 0;
        if (idx > kCeilingSteps) idx = kCeilingSteps;
        lv_slider_set_value(ceiling_slider, idx, LV_ANIM_OFF);
        if (ceiling_value) {
            const UiParamMeta* m = ui_param_meta(VOXP4_PARAM_LIMITER_CEILING);
            char buf[16];
            if (m) ui_format_parameter_value(*m, ceiling, buf, sizeof(buf));
            else snprintf(buf, sizeof(buf), "--");
            lv_label_set_text(ceiling_value, buf);
        }
    }
    // MUTE DRY
    set_mute_visual(ui_get_parameter(VOXP4_PARAM_OUTPUT_MUTE_DRY) >= 0.5f);
    // ROUTING / SOURCE
    set_segmented_visual(routing_seg,
        (int)lround(ui_get_parameter(VOXP4_PARAM_OUTPUT_SPATIAL_ROUTING)));
    set_segmented_visual(source_seg,
        (int)lround(ui_get_parameter(VOXP4_PARAM_OUTPUT_SPATIAL_SOURCE)));
}

void master_screen_notify(uint16_t wireId, float value) {
    switch (wireId) {
        case VOXP4_PARAM_TEMPO_BPM:
            if (tempo_slider) {
                int idx = (int)lround(value - 30.0f);
                if (idx < 0) idx = 0;
                if (idx > kTempoSteps) idx = kTempoSteps;
                lv_slider_set_value(tempo_slider, idx, LV_ANIM_OFF);
            }
            if (tempo_value) {
                char buf[16];
                snprintf(buf, sizeof(buf), "%d BPM", (int)(value + 0.5f));
                lv_label_set_text(tempo_value, buf);
            }
            break;
        case VOXP4_PARAM_LIMITER_CEILING:
            if (ceiling_slider) {
                int idx = (int)lround((value - 0.10f) / 0.01f);
                if (idx < 0) idx = 0;
                if (idx > kCeilingSteps) idx = kCeilingSteps;
                lv_slider_set_value(ceiling_slider, idx, LV_ANIM_OFF);
            }
            if (ceiling_value) {
                const UiParamMeta* m = ui_param_meta(VOXP4_PARAM_LIMITER_CEILING);
                char buf[16];
                if (m) ui_format_parameter_value(*m, value, buf, sizeof(buf));
                else snprintf(buf, sizeof(buf), "--");
                lv_label_set_text(ceiling_value, buf);
            }
            break;
        case VOXP4_PARAM_OUTPUT_MUTE_DRY:
            set_mute_visual(value >= 0.5f);
            break;
        case VOXP4_PARAM_OUTPUT_SPATIAL_ROUTING:
            set_segmented_visual(routing_seg, (int)lround(value));
            break;
        case VOXP4_PARAM_OUTPUT_SPATIAL_SOURCE:
            set_segmented_visual(source_seg, (int)lround(value));
            break;
        default:
            break;
    }
}

void master_screen_refresh_tap(bool hasEstimate, float bpm) {
    if (!tap_label) return;
    char buf[24];
    if (hasEstimate) {
        snprintf(buf, sizeof(buf), "%d BPM", (int)(bpm + 0.5f));
        lv_label_set_text(tap_label, buf);
    } else {
        lv_label_set_text(tap_label, "TAP TEMPO");
    }
}
