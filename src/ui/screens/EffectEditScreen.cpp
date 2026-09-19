#include "EffectEditScreen.h"
#include "../UiApp.h"
#include "../UiTheme.h"
#include "ui/params/UiParamModel.h"
#include "voxlink/VoxLinkUi.h"
#include <cmath>
#include <cstdio>
#include <cstring>
#if defined(DEBUG_BUILD)
#include <Arduino.h>
#endif

static lv_obj_t* edit_container = nullptr;
static lv_obj_t* title_label = nullptr;
static lv_obj_t* enable_btn = nullptr;
static lv_obj_t* enable_label = nullptr;
static lv_obj_t* params_area = nullptr;
static int current_edit_id = -1;
static bool pending_rebuild = false;

static const char* effect_names[] = {"HARMONY", "REVERB", "DELAY", "LIMITER"};

struct RowBinding {
    UiParamId id;
    UiControlType type;
    lv_obj_t* control;
    lv_obj_t* value_label;
};
static constexpr int kMaxRows = 24;
static RowBinding s_rows[kMaxRows];
static int s_row_count = 0;

static void set_enable_visual(bool enabled);
static void rebuild_params(void);

static void bind_row(UiParamId id, UiControlType type, lv_obj_t* control,
                     lv_obj_t* value_label) {
    if (s_row_count >= kMaxRows) return;
    s_rows[s_row_count].id = id;
    s_rows[s_row_count].type = type;
    s_rows[s_row_count].control = control;
    s_rows[s_row_count].value_label = value_label;
    ++s_row_count;
}

static void emit_set(const UiParamDescriptor& d, float value) {
    UiAction action = {UiActionType::SetParameter, static_cast<uint16_t>(d.id),
                       value};
    ui_emit_action(action);
}

static void format_label(const UiParamDescriptor& d, float value,
                         lv_obj_t* label) {
    if (!label) return;
    char buf[24];
    ui_format_parameter_value(d, value, buf, sizeof(buf));
    lv_label_set_text(label, buf);
}

static int slider_count(const UiParamDescriptor& d) {
    int count = (int)std::lround((d.maxValue - d.minValue) / d.uiStep);
    return count < 1 ? 1 : count;
}

static int slider_index(const UiParamDescriptor& d, float value) {
    int index = (int)std::lround((value - d.minValue) / d.uiStep);
    int count = slider_count(d);
    if (index < 0) index = 0;
    if (index > count) index = count;
    return index;
}

static int stepper_count(const UiParamDescriptor& d) {
    if (d.options && d.optionCount > 0) return d.optionCount;
    return slider_count(d) + 1;
}

// --- Shared visual helpers -------------------------------------------------
// Used by creation, local events and remote (authoritative) updates, so that a
// P4 PARAM_CHANGED produces exactly the same visuals as a local tap.

static void set_toggle_visual(lv_obj_t* btn, lv_obj_t* label, bool on) {
    if (btn) {
        lv_obj_set_style_bg_color(btn, on ? COLOR_ACCENT_DARK : COLOR_SURFACE, 0);
        lv_obj_set_style_border_width(btn, on ? 0 : 1, 0);
        lv_obj_set_style_border_color(btn, COLOR_SEPARATOR, 0);
    }
    if (label) {
        lv_label_set_text(label, on ? "ON" : "OFF");
        lv_obj_set_style_text_color(label,
                                    on ? COLOR_TEXT_PRIMARY : COLOR_TEXT_MUTED, 0);
    }
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
            lv_obj_set_style_text_color(
                label, active ? COLOR_ACCENT_BRIGHT : COLOR_TEXT_SECONDARY, 0);
        }
    }
}

// --- Event callbacks -------------------------------------------------------

static void slider_event(lv_event_t* e) {
    lv_obj_t* slider = lv_event_get_target(e);
    const UiParamDescriptor* d =
        static_cast<const UiParamDescriptor*>(lv_obj_get_user_data(slider));
    if (!d) return;
    const float value = d->minValue + lv_slider_get_value(slider) * d->uiStep;
    for (int i = 0; i < s_row_count; i++) {
        if (s_rows[i].id == d->id) {
            format_label(*d, value, s_rows[i].value_label);
            break;
        }
    }
    emit_set(*d, value);
}

static void toggle_event(lv_event_t* e) {
    lv_obj_t* btn = lv_event_get_target(e);
    const UiParamDescriptor* d =
        static_cast<const UiParamDescriptor*>(lv_obj_get_user_data(btn));
    if (!d) return;
    const bool on = ui_get_parameter(d->id) < 0.5f;
    set_toggle_visual(btn, lv_obj_get_child(btn, 0), on);
    emit_set(*d, on ? 1.0f : 0.0f);
}

static void seg_event(lv_event_t* e) {
    lv_obj_t* btn = lv_event_get_target(e);
    const int index = (int)(intptr_t)lv_obj_get_user_data(btn);
    lv_obj_t* seg = lv_obj_get_parent(btn);
    const UiParamDescriptor* d =
        static_cast<const UiParamDescriptor*>(lv_obj_get_user_data(seg));
    if (!d) return;
    set_segmented_visual(seg, index);
    emit_set(*d, (float)index);
}

static void stepper_event(lv_event_t* e) {
    lv_obj_t* btn = lv_event_get_target(e);
    const int delta = (int)(intptr_t)lv_obj_get_user_data(btn);
    lv_obj_t* stepper = lv_obj_get_parent(btn);
    const UiParamDescriptor* d =
        static_cast<const UiParamDescriptor*>(lv_obj_get_user_data(stepper));
    if (!d) return;
    const int count = stepper_count(*d);
    const int index =
        (int)std::lround((ui_get_parameter(d->id) - d->minValue) / d->uiStep);
    const int next = ui_step_index(index, delta, count, d->wraparound);
    const float value = d->minValue + next * d->uiStep;
    format_label(*d, value, lv_obj_get_child(stepper, 1));
    emit_set(*d, value);
}

// --- Layout helpers --------------------------------------------------------

static lv_obj_t* create_row(lv_obj_t* parent, int height) {
    lv_obj_t* row = lv_obj_create(parent);
    lv_obj_set_size(row, LV_PCT(100), height);
    lv_obj_clear_flag(row, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(row, COLOR_SURFACE, 0);
    lv_obj_set_style_radius(row, RADIUS_S, 0);
    lv_obj_set_style_border_width(row, 0, 0);
    lv_obj_set_style_pad_hor(row, SPACING_M, 0);
    lv_obj_set_style_pad_ver(row, SPACING_XS, 0);
    lv_obj_set_style_pad_row(row, 2, 0);
    lv_obj_set_flex_flow(row, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(row, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START,
                          LV_FLEX_ALIGN_START);
    return row;
}

static void add_row_header(lv_obj_t* row, const char* label, const char* value,
                           lv_obj_t** value_out) {
    lv_obj_t* top = lv_obj_create(row);
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

    if (value != nullptr) {
        lv_obj_t* val = lv_label_create(top);
        lv_label_set_text(val, value);
        lv_obj_set_style_text_color(val, COLOR_TEXT_PRIMARY, 0);
        lv_obj_set_style_text_font(val, FONT_SMALL, 0);
        if (value_out) *value_out = val;
    }
}

static void create_section_header(lv_obj_t* parent, const char* text) {
    lv_obj_t* label = lv_label_create(parent);
    lv_label_set_text(label, text);
    lv_obj_set_style_text_color(label, COLOR_TEXT_MUTED, 0);
    lv_obj_set_style_text_font(label, FONT_TINY, 0);
    lv_obj_set_style_pad_top(label, SPACING_XS, 0);
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

// --- Row builders ----------------------------------------------------------

static void create_slider_row(const UiParamDescriptor& d) {
    lv_obj_t* row = create_row(params_area, 40);
    char buf[24];
    ui_format_parameter_value(d, ui_get_parameter(d.id), buf, sizeof(buf));
    lv_obj_t* value_label = nullptr;
    add_row_header(row, d.label, buf, &value_label);

    lv_obj_t* slider = lv_slider_create(row);
    lv_obj_set_size(slider, LV_PCT(100), 8);
    lv_slider_set_range(slider, 0, slider_count(d));
    lv_slider_set_value(slider, slider_index(d, ui_get_parameter(d.id)),
                        LV_ANIM_OFF);
    lv_obj_set_ext_click_area(slider, 12);
    style_slider(slider);
    lv_obj_set_user_data(slider, (void*)&d);
    lv_obj_add_event_cb(slider, slider_event, LV_EVENT_VALUE_CHANGED, NULL);
    bind_row(d.id, UiControlType::Slider, slider, value_label);
}

static void create_toggle_row(const UiParamDescriptor& d) {
    lv_obj_t* row = create_row(params_area, 40);
    lv_obj_t* top = lv_obj_create(row);
    lv_obj_set_size(top, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_clear_flag(top, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_opa(top, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(top, 0, 0);
    lv_obj_set_style_pad_all(top, 0, 0);
    lv_obj_set_flex_flow(top, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(top, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER,
                          LV_FLEX_ALIGN_CENTER);

    lv_obj_t* name = lv_label_create(top);
    lv_label_set_text(name, d.label);
    lv_obj_set_style_text_color(name, COLOR_TEXT_MUTED, 0);
    lv_obj_set_style_text_font(name, FONT_TINY, 0);

    const bool on = ui_get_parameter(d.id) >= 0.5f;
    lv_obj_t* btn = lv_btn_create(top);
    lv_obj_set_size(btn, 56, 26);
    lv_obj_set_ext_click_area(btn, 6);
    lv_obj_set_style_radius(btn, RADIUS_S, 0);
    lv_obj_set_style_shadow_width(btn, 0, 0);
    ui_apply_pressed(btn, COLOR_SURFACE_ELEV, COLOR_BORDER);

    lv_obj_t* bl = lv_label_create(btn);
    lv_obj_center(bl);
    lv_obj_set_style_text_font(bl, FONT_TINY, 0);
    set_toggle_visual(btn, bl, on);

    lv_obj_set_user_data(btn, (void*)&d);
    lv_obj_add_event_cb(btn, toggle_event, LV_EVENT_CLICKED, NULL);
    bind_row(d.id, UiControlType::Toggle, btn, bl);
}

static void create_segmented_row(const UiParamDescriptor& d) {
    lv_obj_t* row = create_row(params_area, 46);
    char buf[24];
    ui_format_parameter_value(d, ui_get_parameter(d.id), buf, sizeof(buf));
    lv_obj_t* value_label = nullptr;
    add_row_header(row, d.label, buf, &value_label);

    lv_obj_t* seg = lv_obj_create(row);
    lv_obj_set_size(seg, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_clear_flag(seg, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_opa(seg, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(seg, 0, 0);
    lv_obj_set_style_pad_all(seg, 0, 0);
    lv_obj_set_style_pad_column(seg, 3, 0);
    lv_obj_set_flex_flow(seg, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(seg, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER,
                          LV_FLEX_ALIGN_CENTER);

    const int selected = (int)std::lround(ui_get_parameter(d.id));
    for (int i = 0; i < d.optionCount; i++) {
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
        lv_label_set_text(opt, d.options[i]);
        lv_obj_center(opt);
        lv_obj_set_style_text_font(opt, FONT_SMALL, 0);

        lv_obj_set_user_data(btn, (void*)(intptr_t)i);
        lv_obj_add_event_cb(btn, seg_event, LV_EVENT_CLICKED, NULL);
    }
    set_segmented_visual(seg, selected);
    lv_obj_set_user_data(seg, (void*)&d);
    bind_row(d.id, UiControlType::Segmented, seg, value_label);
}

static void create_stepper_row(const UiParamDescriptor& d) {
    lv_obj_t* row = create_row(params_area, 48);
    add_row_header(row, d.label, nullptr, nullptr);

    lv_obj_t* stepper = lv_obj_create(row);
    lv_obj_set_size(stepper, LV_PCT(100), 30);
    lv_obj_clear_flag(stepper, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_opa(stepper, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(stepper, 0, 0);
    lv_obj_set_style_pad_all(stepper, 0, 0);
    lv_obj_set_flex_flow(stepper, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(stepper, LV_FLEX_ALIGN_SPACE_BETWEEN,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t* minus = lv_btn_create(stepper);
    lv_obj_set_size(minus, 44, 28);
    lv_obj_set_ext_click_area(minus, 4);
    lv_obj_set_style_radius(minus, RADIUS_S, 0);
    lv_obj_set_style_shadow_width(minus, 0, 0);
    lv_obj_set_style_border_width(minus, 1, 0);
    lv_obj_set_style_border_color(minus, COLOR_SEPARATOR, 0);
    lv_obj_set_style_bg_color(minus, COLOR_PANEL, 0);
    ui_apply_pressed(minus, COLOR_SURFACE_ELEV, COLOR_ACCENT);
    lv_obj_t* minus_label = lv_label_create(minus);
    lv_label_set_text(minus_label, "<");
    lv_obj_center(minus_label);
    lv_obj_set_style_text_color(minus_label, COLOR_TEXT_SECONDARY, 0);
    lv_obj_set_user_data(minus, (void*)(intptr_t)-1);
    lv_obj_add_event_cb(minus, stepper_event, LV_EVENT_CLICKED, NULL);

    char buf[24];
    ui_format_parameter_value(d, ui_get_parameter(d.id), buf, sizeof(buf));
    lv_obj_t* value = lv_label_create(stepper);
    lv_label_set_text(value, buf);
    lv_obj_set_flex_grow(value, 1);
    lv_obj_set_style_text_align(value, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_color(value, COLOR_TEXT_PRIMARY, 0);
    lv_obj_set_style_text_font(value, FONT_SMALL, 0);

    lv_obj_t* plus = lv_btn_create(stepper);
    lv_obj_set_size(plus, 44, 28);
    lv_obj_set_ext_click_area(plus, 4);
    lv_obj_set_style_radius(plus, RADIUS_S, 0);
    lv_obj_set_style_shadow_width(plus, 0, 0);
    lv_obj_set_style_border_width(plus, 1, 0);
    lv_obj_set_style_border_color(plus, COLOR_SEPARATOR, 0);
    lv_obj_set_style_bg_color(plus, COLOR_PANEL, 0);
    ui_apply_pressed(plus, COLOR_SURFACE_ELEV, COLOR_ACCENT);
    lv_obj_t* plus_label = lv_label_create(plus);
    lv_label_set_text(plus_label, ">");
    lv_obj_center(plus_label);
    lv_obj_set_style_text_color(plus_label, COLOR_TEXT_SECONDARY, 0);
    lv_obj_set_user_data(plus, (void*)(intptr_t)1);
    lv_obj_add_event_cb(plus, stepper_event, LV_EVENT_CLICKED, NULL);

    lv_obj_set_user_data(stepper, (void*)&d);
    bind_row(d.id, UiControlType::Stepper, stepper, value);
}

// --- Rebuild ---------------------------------------------------------------

static void rebuild_params(void) {
    pending_rebuild = false;
    if (!params_area) return;
    lv_obj_clean(params_area);
    s_row_count = 0;
    if (current_edit_id < 0) return;

    const UiEffectId effect = static_cast<UiEffectId>(current_edit_id);
    const uint8_t mode =
        (uint8_t)std::lround(ui_get_parameter(UiParamId::HarmonyMode));
    size_t count = 0;
    const UiParamDescriptor* table = ui_effect_descriptors(effect, &count);
    if (!table) return;

    const char* last_section = nullptr;
    for (size_t i = 0; i < count; i++) {
        const UiParamDescriptor& d = table[i];
        if (!ui_descriptor_applies(d, mode)) continue;
        // Capability gating: skip parameters the connected P4 does not expose.
        const uint16_t wire_id = ui_param_voxlink_id(d.id);
        if (wire_id != 0 && !voxlink_param_supported(wire_id)) continue;
        if (d.section &&
            (last_section == nullptr || std::strcmp(d.section, last_section) != 0)) {
            create_section_header(params_area, d.section);
            last_section = d.section;
        }
        switch (d.type) {
            case UiControlType::Slider: create_slider_row(d); break;
            case UiControlType::Toggle: create_toggle_row(d); break;
            case UiControlType::Segmented: create_segmented_row(d); break;
            case UiControlType::Stepper: create_stepper_row(d); break;
        }
    }
#if defined(DEBUG_BUILD)
    // Lets a hardware run confirm that switching effects repeatedly does not
    // progressively consume heap (the body is lv_obj_clean'd on each rebuild).
    Serial.printf("[UI] Effect Edit '%s' rebuilt, heap: %u\n",
                  effect_names[current_edit_id], (unsigned)esp_get_free_heap_size());
#endif
}

// --- Public API ------------------------------------------------------------

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
    lv_obj_set_flex_align(header, LV_FLEX_ALIGN_SPACE_BETWEEN,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t* back_btn = lv_btn_create(header);
    lv_obj_set_size(back_btn, 34, 24);
    lv_obj_set_ext_click_area(back_btn, 6);
    lv_obj_set_style_bg_color(back_btn, COLOR_SURFACE, 0);
    lv_obj_set_style_border_width(back_btn, 1, 0);
    lv_obj_set_style_border_color(back_btn, COLOR_SEPARATOR, 0);
    lv_obj_set_style_radius(back_btn, RADIUS_S, 0);
    lv_obj_set_style_shadow_width(back_btn, 0, 0);
    ui_apply_pressed(back_btn, COLOR_SURFACE_ELEV, COLOR_BORDER);
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
    lv_obj_set_size(enable_btn, 48, 24);
    lv_obj_set_ext_click_area(enable_btn, 6);
    lv_obj_set_style_radius(enable_btn, RADIUS_S, 0);
    lv_obj_set_style_shadow_width(enable_btn, 0, 0);
    ui_apply_pressed(enable_btn, COLOR_SURFACE_ELEV, COLOR_ACCENT);
    enable_label = lv_label_create(enable_btn);
    lv_label_set_text(enable_label, "OFF");
    lv_obj_center(enable_label);
    lv_obj_set_style_text_font(enable_label, FONT_TINY, 0);
    lv_obj_add_event_cb(enable_btn, [](lv_event_t* e) {
        if (current_edit_id >= 0) {
            UiAction action = {UiActionType::ToggleEffect,
                               (uint16_t)current_edit_id, 0.0f};
            ui_emit_action(action);
        }
    }, LV_EVENT_CLICKED, NULL);
    set_enable_visual(false);

    // PARAMETERS: scrollable body, rebuilt per effect.
    params_area = lv_obj_create(edit_container);
    lv_obj_set_size(params_area, LV_PCT(100), LV_PCT(100));
    lv_obj_set_flex_grow(params_area, 1);
    lv_obj_set_style_bg_color(params_area, COLOR_BG, 0);
    lv_obj_set_style_border_width(params_area, 0, 0);
    lv_obj_set_style_pad_all(params_area, 0, 0);
    lv_obj_set_style_pad_row(params_area, SPACING_XS, 0);
    lv_obj_set_flex_flow(params_area, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_scroll_dir(params_area, LV_DIR_VER);
    lv_obj_set_scrollbar_mode(params_area, LV_SCROLLBAR_MODE_AUTO);
}

static void set_enable_visual(bool enabled) {
    if (!enable_btn || !enable_label) return;
    lv_obj_set_style_bg_color(enable_btn, enabled ? COLOR_ACCENT_DARK : COLOR_SURFACE, 0);
    lv_obj_set_style_border_width(enable_btn, enabled ? 0 : 1, 0);
    lv_obj_set_style_border_color(enable_btn, COLOR_SEPARATOR, 0);
    lv_label_set_text(enable_label, enabled ? "ON" : "OFF");
    lv_obj_set_style_text_color(enable_label,
                                enabled ? COLOR_TEXT_PRIMARY : COLOR_TEXT_MUTED, 0);
}

void effect_edit_load_effect(int effectId) {
    if (effectId < 0 || effectId >= 4) return;
    current_edit_id = effectId;
    if (title_label) lv_label_set_text(title_label, effect_names[effectId]);
    rebuild_params();
}

void effect_edit_set_enabled(int effectId, bool enabled) {
    if (effectId != current_edit_id) return;
    set_enable_visual(enabled);
}

void effect_edit_notify(UiParamId id, float value) {
    if (current_edit_id < 0) return;
    const UiParamDescriptor* d = ui_param_descriptor(id);
    if (!d) return;

    // Only the currently open effect should react to a value change.
    if (ui_effect_of(id) != static_cast<UiEffectId>(current_edit_id)) return;

    // Only Harmony MODE changes the structure of the page. Defer that rebuild to
    // the next tick so the widget dispatching the event is not deleted mid-event.
    // Other Segmented controls (e.g. NON-SCALE) update in place and must NOT
    // rebuild or reset the scroll position.
    if (id == UiParamId::HarmonyMode) {
        pending_rebuild = true;
        return;
    }

    for (int i = 0; i < s_row_count; i++) {
        if (s_rows[i].id != id) continue;
        switch (s_rows[i].type) {
            case UiControlType::Slider:
                if (s_rows[i].control) {
                    lv_slider_set_value(s_rows[i].control, slider_index(*d, value),
                                        LV_ANIM_OFF);
                }
                format_label(*d, value, s_rows[i].value_label);
                break;
            case UiControlType::Toggle:
                set_toggle_visual(s_rows[i].control, s_rows[i].value_label,
                                  value >= 0.5f);
                break;
            case UiControlType::Stepper:
                format_label(*d, value, s_rows[i].value_label);
                break;
            case UiControlType::Segmented:
                // s_rows[i].control is the segmented container.
                set_segmented_visual(s_rows[i].control,
                                     (int)std::lround(value));
                format_label(*d, value, s_rows[i].value_label);
                break;
        }
        return;
    }
}

void effect_edit_tick(void) {
    if (pending_rebuild) rebuild_params();
}

void effect_edit_refresh(void) {
    if (current_edit_id >= 0) pending_rebuild = true;
}
