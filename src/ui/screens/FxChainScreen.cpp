#include "FxChainScreen.h"
#include "../UiApp.h"
#include "../UiTheme.h"

// Module card geometry. The rack holds seven modules and scrolls horizontally
// with snap, so touch targets stay large instead of shrinking to fit.
#define FX_CARD_W 64
#define FX_CARD_H 84
#define FX_ACTION_H 44
#define FX_CARD_GAP 6

// Each effect is presented as a hardware-like processing module, not a settings
// row. Active state is shown by a top accent rail + LED + stronger text, on a
// neutral card outline. The whole card is one large touch target.
typedef struct {
    lv_obj_t* card;
    lv_obj_t* rail;
    lv_obj_t* led;
    lv_obj_t* name;
    lv_obj_t* value;
    lv_obj_t* meta;
} FxModule_t;

static FxModule_t fx_modules[kUiEffectCount] = {};
static lv_obj_t* bypass_button = nullptr;

// Global bypass is a local flag only; this updates its visual state without
// touching individual effect enabled states.
void fx_chain_set_bypass(bool active) {
    if (!bypass_button) return;
    lv_obj_set_style_bg_color(bypass_button, active ? COLOR_ACCENT_DARK : COLOR_SURFACE, 0);
    lv_obj_set_style_bg_opa(bypass_button, active ? LV_OPA_COVER : LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_color(bypass_button, COLOR_ACCENT, 0);
}

void fx_chain_set_bypass_available(bool available) {
    if (!bypass_button) return;
    if (available) {
        lv_obj_add_flag(bypass_button, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_style_opa(bypass_button, LV_OPA_COVER, 0);
        lv_obj_set_style_border_color(bypass_button, COLOR_ACCENT, 0);
    } else {
        ui_apply_disabled(bypass_button);
    }
}

static void module_clicked(lv_event_t* e) {
    lv_obj_t* card = lv_event_get_target(e);
    int effect_id = (int)(intptr_t)lv_obj_get_user_data(card);
    UiAction action = { UiActionType::OpenEffect, (uint16_t)effect_id, 0 };
    ui_emit_action(action);
}

static lv_obj_t* make_line(lv_obj_t* parent) {
    lv_obj_t* line = lv_obj_create(parent);
    lv_obj_set_size(line, LV_PCT(100), 1);
    lv_obj_clear_flag(line, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(line, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(line, COLOR_SEPARATOR, 0);
    lv_obj_set_style_border_width(line, 0, 0);
    lv_obj_set_style_radius(line, 0, 0);
    lv_obj_set_style_pad_all(line, 0, 0);
    return line;
}

static void create_module(lv_obj_t* parent, int index, const char* name) {
    lv_obj_t* card = lv_obj_create(parent);
    lv_obj_set_size(card, FX_CARD_W, FX_CARD_H);
    lv_obj_clear_flag(card, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(card, COLOR_SURFACE, 0);
    lv_obj_set_style_border_width(card, 1, 0);
    lv_obj_set_style_border_color(card, COLOR_SEPARATOR, 0);
    lv_obj_set_style_radius(card, RADIUS_M, 0);
    lv_obj_set_style_clip_corner(card, true, 0);
    lv_obj_set_style_pad_all(card, 0, 0);
    lv_obj_set_flex_flow(card, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(card, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_add_flag(card, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_user_data(card, (void*)(intptr_t)index);
    lv_obj_add_event_cb(card, module_clicked, LV_EVENT_CLICKED, NULL);
    ui_apply_pressed(card, COLOR_SURFACE_ELEV, COLOR_BORDER);

    // Top accent rail: lit when the module is active. Kept in the layout even
    // when OFF (blended into the surface) so the value never shifts position.
    lv_obj_t* rail = lv_obj_create(card);
    lv_obj_set_size(rail, LV_PCT(100), 3);
    lv_obj_clear_flag(rail, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(rail, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(rail, COLOR_SURFACE, 0);
    lv_obj_set_style_border_width(rail, 0, 0);
    lv_obj_set_style_radius(rail, RADIUS_M, 0);
    lv_obj_set_style_pad_all(rail, 0, 0);

    // Padded content
    lv_obj_t* body = lv_obj_create(card);
    lv_obj_set_size(body, LV_PCT(100), LV_PCT(100));
    lv_obj_set_flex_grow(body, 1);
    lv_obj_clear_flag(body, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(body, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_opa(body, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(body, 0, 0);
    lv_obj_set_style_pad_all(body, SPACING_XS, 0);
    lv_obj_set_style_pad_row(body, SPACING_XS, 0);
    lv_obj_set_flex_flow(body, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(body, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    // Top row: status LED + effect name
    lv_obj_t* top = lv_obj_create(body);
    lv_obj_set_size(top, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_clear_flag(top, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(top, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_opa(top, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(top, 0, 0);
    lv_obj_set_style_pad_all(top, 0, 0);
    lv_obj_set_style_pad_column(top, SPACING_XS, 0);
    lv_obj_set_flex_flow(top, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(top, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t* led = lv_obj_create(top);
    lv_obj_set_size(led, 8, 8);
    lv_obj_clear_flag(led, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(led, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(led, COLOR_DISABLED, 0);
    lv_obj_set_style_radius(led, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_border_width(led, 0, 0);
    lv_obj_set_style_pad_all(led, 0, 0);

    lv_obj_t* name_label = lv_label_create(top);
    lv_label_set_text(name_label, name);
    lv_obj_set_style_text_font(name_label, FONT_TINY, 0);
    lv_obj_set_style_text_color(name_label, COLOR_TEXT_SECONDARY, 0);

    // Dominant readout, vertically centered in the free space
    lv_obj_t* value_box = lv_obj_create(body);
    lv_obj_set_width(value_box, LV_PCT(100));
    lv_obj_set_flex_grow(value_box, 1);
    lv_obj_clear_flag(value_box, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(value_box, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_opa(value_box, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(value_box, 0, 0);
    lv_obj_set_style_pad_all(value_box, 0, 0);
    lv_obj_set_flex_flow(value_box, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(value_box, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t* value_label = lv_label_create(value_box);
    lv_label_set_text(value_label, "--");
    lv_obj_set_style_text_font(value_label, FONT_EMPHASIS, 0);
    lv_obj_set_style_text_color(value_label, COLOR_TEXT_MUTED, 0);

    make_line(body);

    lv_obj_t* meta_label = lv_label_create(body);
    lv_label_set_text(meta_label, "--");
    lv_obj_set_style_text_font(meta_label, FONT_TINY, 0);
    lv_obj_set_style_text_color(meta_label, COLOR_TEXT_FAINT, 0);

    fx_modules[index].card = card;
    fx_modules[index].rail = rail;
    fx_modules[index].led = led;
    fx_modules[index].name = name_label;
    fx_modules[index].value = value_label;
    fx_modules[index].meta = meta_label;
}

// Global action: title + subtitle, outline treatment with a subtle pressed state.
static lv_obj_t* create_global_action(lv_obj_t* parent, const char* title, const char* subtitle,
                                      lv_color_t border, lv_color_t title_color, bool filled) {
    lv_obj_t* btn = lv_btn_create(parent);
    lv_obj_set_size(btn, 150, FX_ACTION_H);
    lv_obj_set_style_radius(btn, RADIUS_S, 0);
    lv_obj_set_style_shadow_width(btn, 0, 0);
    lv_obj_set_style_border_width(btn, 1, 0);
    lv_obj_set_style_border_color(btn, border, 0);
    lv_obj_set_style_bg_color(btn, COLOR_SURFACE, 0);
    lv_obj_set_style_bg_opa(btn, filled ? LV_OPA_COVER : LV_OPA_TRANSP, 0);
    ui_apply_pressed(btn, COLOR_SURFACE_ELEV, border);

    lv_obj_t* col = lv_obj_create(btn);
    lv_obj_set_size(col, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_clear_flag(col, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(col, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_opa(col, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(col, 0, 0);
    lv_obj_set_style_pad_all(col, 0, 0);
    lv_obj_set_style_pad_row(col, 1, 0);
    lv_obj_set_flex_flow(col, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(col, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_center(col);

    lv_obj_t* title_label = lv_label_create(col);
    lv_label_set_text(title_label, title);
    lv_obj_set_style_text_font(title_label, FONT_BODY, 0);
    lv_obj_set_style_text_color(title_label, title_color, 0);

    lv_obj_t* subtitle_label = lv_label_create(col);
    lv_label_set_text(subtitle_label, subtitle);
    lv_obj_set_style_text_font(subtitle_label, FONT_TINY, 0);
    lv_obj_set_style_text_color(subtitle_label, filled ? COLOR_TEXT_MUTED : COLOR_TEXT_FAINT, 0);

    return btn;
}

void fx_chain_screen_init(lv_obj_t* parent) {
    lv_obj_t* fx_container = lv_obj_create(parent);
    lv_obj_set_size(fx_container, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(fx_container, COLOR_BG, 0);
    lv_obj_set_style_pad_all(fx_container, SPACING_XS, 0);
    lv_obj_set_style_pad_row(fx_container, SPACING_S, 0);
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

    // === MODULE RACK (horizontal, snap-scrolling over 7 modules) ===
    lv_obj_t* rack = lv_obj_create(fx_container);
    lv_obj_set_size(rack, LV_PCT(100), LV_PCT(100));
    lv_obj_set_flex_grow(rack, 1);
    lv_obj_add_flag(rack, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scroll_dir(rack, LV_DIR_HOR);
    lv_obj_set_scroll_snap_x(rack, LV_SCROLL_SNAP_CENTER);
    lv_obj_set_scrollbar_mode(rack, LV_SCROLLBAR_MODE_AUTO);
    lv_obj_set_style_bg_color(rack, COLOR_BG, 0);
    lv_obj_set_style_border_width(rack, 0, 0);
    lv_obj_set_style_pad_all(rack, 0, 0);
    lv_obj_set_style_pad_column(rack, FX_CARD_GAP, 0);
    lv_obj_set_flex_flow(rack, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(rack, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    for (size_t i = 0; i < kUiEffectCount; ++i) {
        create_module(rack, (int)i, ui_effect_name(static_cast<UiEffectId>(i)));
    }

    // === GLOBAL ACTIONS ===
    make_line(fx_container);

    lv_obj_t* actions_row = lv_obj_create(fx_container);
    lv_obj_set_size(actions_row, LV_PCT(100), FX_ACTION_H);
    lv_obj_clear_flag(actions_row, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(actions_row, COLOR_BG, 0);
    lv_obj_set_style_border_width(actions_row, 0, 0);
    lv_obj_set_style_pad_all(actions_row, 0, 0);
    lv_obj_set_style_pad_column(actions_row, SPACING_M, 0);
    lv_obj_set_flex_flow(actions_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(actions_row, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t* all_on_btn = create_global_action(actions_row, "ALL ON", "ENABLE ALL EFFECTS",
                                                COLOR_AUDIO_DARK, COLOR_AUDIO, false);
    lv_obj_add_event_cb(all_on_btn, [](lv_event_t* e) {
        UiAction action = { UiActionType::AllEffectsOn, 0, 0 };
        ui_emit_action(action);
    }, LV_EVENT_CLICKED, NULL);

    bypass_button = create_global_action(actions_row, "BYPASS", "GLOBAL BYPASS",
                                         COLOR_ACCENT, COLOR_TEXT_PRIMARY, true);
    lv_obj_add_event_cb(bypass_button, [](lv_event_t* e) {
        UiAction action = { UiActionType::GlobalBypass, 0, 0 };
        ui_emit_action(action);
    }, LV_EVENT_CLICKED, NULL);
}

void fx_chain_update_effect_state(UiEffectId effect, bool enabled,
                                  const char* mainValue, const char* metadata) {
    const int effectId = (int)effect;
    if (effectId < 0 || effectId >= static_cast<int>(kUiEffectCount) || !fx_modules[effectId].card) return;

    FxModule_t& m = fx_modules[effectId];

    // ON is communicated by rail + LED + stronger text only. The card outline
    // stays neutral so the rack does not read as four outlined boxes.
    lv_obj_set_style_bg_color(m.rail, enabled ? COLOR_ACCENT : COLOR_SURFACE, 0);
    lv_obj_set_style_bg_color(m.led, enabled ? COLOR_ACCENT : COLOR_DISABLED, 0);
    lv_obj_set_style_text_color(m.name, enabled ? COLOR_TEXT_PRIMARY : COLOR_TEXT_SECONDARY, 0);
    lv_obj_set_style_text_color(m.value, enabled ? COLOR_TEXT_PRIMARY : COLOR_TEXT_MUTED, 0);
    lv_obj_set_style_text_color(m.meta, enabled ? COLOR_TEXT_SECONDARY : COLOR_TEXT_FAINT, 0);

    if (mainValue) lv_label_set_text(m.value, mainValue);
    if (metadata) lv_label_set_text(m.meta, metadata);
}
