#include "PerformanceScreen.h"
#include "../UiApp.h"
#include "../UiTheme.h"
#include "../widgets/VuMeter.h"
#include "../widgets/EffectCard.h"
#include <cstdio>

static lv_obj_t* performance_container = nullptr;
static lv_obj_t* preset_label = nullptr;
static lv_obj_t* link_indicator = nullptr;
static VuMeter_t* input_meter = nullptr;
static VuMeter_t* output_meter = nullptr;
static lv_obj_t* pitch_note_label = nullptr;
static lv_obj_t* pitch_freq_label = nullptr;
static lv_obj_t* voiced_label = nullptr;
static EffectCard_t* effect_cards[kUiEffectCount] = {nullptr};
static lv_obj_t* fs_containers[2] = {nullptr};
static lv_obj_t* fs_labels[2] = {nullptr};

void performance_screen_init(lv_obj_t* parent) {
    performance_container = lv_obj_create(parent);
    lv_obj_set_size(performance_container, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(performance_container, COLOR_BG, 0);
    lv_obj_set_style_pad_all(performance_container, SPACING_XS, 0);
    lv_obj_set_style_pad_row(performance_container, SPACING_XS, 0);
    lv_obj_set_style_border_width(performance_container, 0, 0);
    lv_obj_set_flex_flow(performance_container, LV_FLEX_FLOW_COLUMN);

    // === HEADER (24px): preset takes visual priority, link is secondary ===
    lv_obj_t* header = lv_obj_create(performance_container);
    lv_obj_set_size(header, LV_PCT(100), 24);
    lv_obj_set_style_bg_color(header, COLOR_HEADER, 0);
    lv_obj_set_style_radius(header, RADIUS_S, 0);
    lv_obj_set_style_border_width(header, 0, 0);
    lv_obj_set_style_pad_hor(header, SPACING_M, 0);
    lv_obj_set_style_pad_ver(header, 0, 0);
    lv_obj_set_flex_flow(header, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(header, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    preset_label = lv_label_create(header);
    lv_label_set_text(preset_label, "P--  CONNECTING");
    lv_obj_set_style_text_color(preset_label, COLOR_TEXT_PRIMARY, 0);
    lv_obj_set_style_text_font(preset_label, FONT_BODY, 0);

    link_indicator = lv_label_create(header);
    lv_label_set_text(link_indicator, "○ LINK");
    lv_obj_set_style_text_color(link_indicator, COLOR_TEXT_MUTED, 0);
    lv_obj_set_style_text_font(link_indicator, FONT_SMALL, 0);

    // === PITCH (40px): the note is the fastest-read musical information ===
    lv_obj_t* pitch_container = lv_obj_create(performance_container);
    lv_obj_set_size(pitch_container, LV_PCT(100), 40);
    lv_obj_set_style_bg_color(pitch_container, COLOR_SURFACE_ELEV, 0);
    lv_obj_set_style_radius(pitch_container, RADIUS_M, 0);
    lv_obj_set_style_border_width(pitch_container, 1, 0);
    lv_obj_set_style_border_color(pitch_container, COLOR_SEPARATOR, 0);
    lv_obj_set_style_pad_hor(pitch_container, SPACING_L, 0);
    lv_obj_set_style_pad_ver(pitch_container, SPACING_XS, 0);
    lv_obj_set_flex_flow(pitch_container, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(pitch_container, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    pitch_note_label = lv_label_create(pitch_container);
    lv_label_set_text(pitch_note_label, "--");
    lv_obj_set_style_text_color(pitch_note_label, COLOR_AUDIO, 0);
    lv_obj_set_style_text_font(pitch_note_label, FONT_EMPHASIS, 0);

    lv_obj_t* pitch_info = lv_obj_create(pitch_container);
    lv_obj_set_size(pitch_info, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(pitch_info, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(pitch_info, 0, 0);
    lv_obj_set_style_pad_all(pitch_info, 0, 0);
    lv_obj_set_flex_flow(pitch_info, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(pitch_info, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_END, LV_FLEX_ALIGN_END);

    pitch_freq_label = lv_label_create(pitch_info);
    lv_label_set_text(pitch_freq_label, "-- Hz");
    lv_obj_set_style_text_color(pitch_freq_label, COLOR_TEXT_SECONDARY, 0);
    lv_obj_set_style_text_font(pitch_freq_label, FONT_SMALL, 0);

    voiced_label = lv_label_create(pitch_info);
    lv_label_set_text(voiced_label, "UNVOICED");
    lv_obj_set_style_text_color(voiced_label, COLOR_TEXT_MUTED, 0);
    lv_obj_set_style_text_font(voiced_label, FONT_TINY, 0);

    // === METERS (34px): turquoise bar, orange warning, red clip ===
    lv_obj_t* meters_container = lv_obj_create(performance_container);
    lv_obj_set_size(meters_container, LV_PCT(100), 34);
    lv_obj_set_style_bg_color(meters_container, COLOR_BG, 0);
    lv_obj_set_style_border_width(meters_container, 0, 0);
    lv_obj_set_style_pad_all(meters_container, 0, 0);
    lv_obj_set_style_pad_row(meters_container, 2, 0);
    lv_obj_set_flex_flow(meters_container, LV_FLEX_FLOW_COLUMN);

    for (int i = 0; i < 2; i++) {
        lv_obj_t* row = lv_obj_create(meters_container);
        lv_obj_set_size(row, LV_PCT(100), 16);
        lv_obj_set_style_bg_color(row, COLOR_BG, 0);
        lv_obj_set_style_border_width(row, 0, 0);
        lv_obj_set_style_pad_all(row, 0, 0);
        lv_obj_set_style_pad_column(row, SPACING_XS, 0);
        lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(row, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

        lv_obj_t* label = lv_label_create(row);
        lv_label_set_text(label, i == 0 ? "IN" : "OUT");
        lv_obj_set_style_text_color(label, COLOR_TEXT_MUTED, 0);
        lv_obj_set_style_text_font(label, FONT_TINY, 0);
        lv_obj_set_width(label, 22);

        VuMeter_t* meter = vu_meter_create(row, 0, 0, 16, "");
        if (meter) {
            lv_obj_set_height(vu_meter_get_container(meter), 16);
            lv_obj_set_flex_grow(vu_meter_get_container(meter), 1);
        }
        if (i == 0) input_meter = meter; else output_meter = meter;
    }

    // === EFFECT CARDS (46px) ===
    lv_obj_t* effects_container = lv_obj_create(performance_container);
    lv_obj_set_size(effects_container, LV_PCT(100), 46);
    lv_obj_set_style_bg_color(effects_container, COLOR_BG, 0);
    lv_obj_set_style_border_width(effects_container, 0, 0);
    lv_obj_set_style_pad_all(effects_container, 0, 0);
    lv_obj_set_style_pad_column(effects_container, SPACING_XS, 0);
    lv_obj_set_flex_flow(effects_container, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(effects_container, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    for (int i = 0; i < static_cast<int>(kUiEffectCount); i++) {
        effect_cards[i] = effect_card_create(effects_container, 0, 0, (EffectType_t)i);
        if (!effect_cards[i]) continue;

        // Value is filled by the App from the parameter model on init.
        effect_card_set_param(effect_cards[i], "--");
        effect_card_set_enabled(effect_cards[i], false);

        lv_obj_add_flag(effect_cards[i]->card, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_user_data(effect_cards[i]->card, (void*)(intptr_t)i);
        lv_obj_add_event_cb(effect_cards[i]->card, [](lv_event_t* e) {
            lv_obj_t* card = lv_event_get_target(e);
            int effect_id = (int)(intptr_t)lv_obj_get_user_data(card);
            UiAction action = { UiActionType::ToggleEffect, (uint16_t)effect_id, 0 };
            ui_emit_action(action);
        }, LV_EVENT_CLICKED, NULL);
    }

    // === FOOTSWITCH SUMMARY (32px) ===
    lv_obj_t* fs_container = lv_obj_create(performance_container);
    lv_obj_set_size(fs_container, LV_PCT(100), 32);
    lv_obj_set_style_bg_color(fs_container, COLOR_BG, 0);
    lv_obj_set_style_border_width(fs_container, 0, 0);
    lv_obj_set_style_pad_all(fs_container, 0, 0);
    lv_obj_set_style_pad_column(fs_container, SPACING_XS, 0);
    lv_obj_set_flex_flow(fs_container, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(fs_container, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    for (int i = 0; i < 2; i++) {
        fs_containers[i] = lv_obj_create(fs_container);
        lv_obj_set_size(fs_containers[i], LV_PCT(49), LV_PCT(100));
        lv_obj_set_style_bg_color(fs_containers[i], COLOR_SURFACE, 0);
        lv_obj_set_style_radius(fs_containers[i], RADIUS_S, 0);
        lv_obj_set_style_border_width(fs_containers[i], 1, 0);
        lv_obj_set_style_border_color(fs_containers[i], COLOR_SEPARATOR, 0);
        lv_obj_set_style_pad_hor(fs_containers[i], SPACING_S, 0);
        lv_obj_set_style_pad_ver(fs_containers[i], 0, 0);
        lv_obj_set_flex_flow(fs_containers[i], LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(fs_containers[i], LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

        char fs_num[8];
        snprintf(fs_num, sizeof(fs_num), "FS%d", i + 1);
        lv_obj_t* num_label = lv_label_create(fs_containers[i]);
        lv_label_set_text(num_label, fs_num);
        lv_obj_set_style_text_color(num_label, COLOR_TEXT_MUTED, 0);
        lv_obj_set_style_text_font(num_label, FONT_TINY, 0);

        fs_labels[i] = lv_label_create(fs_containers[i]);
        // Neutral until the App feeds the real FootswitchManager config.
        lv_label_set_text(fs_labels[i], "--");
        lv_obj_set_style_text_color(fs_labels[i], COLOR_TEXT_SECONDARY, 0);
        lv_obj_set_style_text_font(fs_labels[i], FONT_SMALL, 0);
    }
}

void performance_update_meters(float inputDb, float outputDb) {
    if (input_meter) vu_meter_update(input_meter, inputDb);
    if (output_meter) vu_meter_update(output_meter, outputDb);
}

void performance_update_pitch(float freqHz, const char* noteName, bool voiced) {
    if (pitch_note_label) {
        lv_label_set_text(pitch_note_label, noteName);
        lv_obj_set_style_text_color(pitch_note_label, voiced ? COLOR_AUDIO_BRIGHT : COLOR_AUDIO_DARK, 0);
    }
    if (pitch_freq_label) {
        char buf[16];
        snprintf(buf, sizeof(buf), "%.1f Hz", freqHz);
        lv_label_set_text(pitch_freq_label, buf);
    }
    if (voiced_label) {
        lv_label_set_text(voiced_label, voiced ? "VOICED" : "UNVOICED");
        lv_obj_set_style_text_color(voiced_label, voiced ? COLOR_AUDIO : COLOR_TEXT_MUTED, 0);
    }
}

void performance_update_effect(int effectIndex, bool enabled) {
    if (effectIndex < 0 || effectIndex >= static_cast<int>(kUiEffectCount)) return;
    if (effect_cards[effectIndex]) {
        effect_card_set_enabled(effect_cards[effectIndex], enabled);
    }
}

void performance_update_effect_value(int effectIndex, const char* mainValue) {
    if (effectIndex < 0 || effectIndex >= static_cast<int>(kUiEffectCount)) return;
    if (effect_cards[effectIndex] && mainValue) {
        effect_card_set_param(effect_cards[effectIndex], mainValue);
    }
}

void performance_update_preset(const char* name) {
    if (preset_label) lv_label_set_text(preset_label, name);
}

void performance_update_scene_status(const char* sceneName, const char* subsceneName,
                                     int subIdx, int subTotal, int setIdx, int setTotal,
                                     bool isDirty) {
    if (!preset_label) return;
    char buf[64];
    if (setTotal > 0 && subTotal > 0) {
        snprintf(buf, sizeof(buf), "[%02d/%02d] %s | %s [%d/%d]%s",
                 setIdx, setTotal, sceneName ? sceneName : "--",
                 subsceneName ? subsceneName : "--", subIdx, subTotal,
                 isDirty ? " *" : "");
    } else if (subTotal > 0) {
        snprintf(buf, sizeof(buf), "%s | %s [%d/%d]%s",
                 sceneName ? sceneName : "--",
                 subsceneName ? subsceneName : "--", subIdx, subTotal,
                 isDirty ? " *" : "");
    } else if (sceneName) {
        snprintf(buf, sizeof(buf), "%s%s", sceneName, isDirty ? " *" : "");
    } else {
        snprintf(buf, sizeof(buf), "--%s", isDirty ? " *" : "");
    }
    lv_label_set_text(preset_label, buf);
}

void performance_update_link(bool connected) {
    if (link_indicator) {
        lv_label_set_text(link_indicator, connected ? "● LINK" : "○ LINK");
        lv_obj_set_style_text_color(link_indicator,
            connected ? COLOR_AUDIO : COLOR_TEXT_MUTED, 0);
    }
}

void performance_update_footswitch(int fsIndex, const char* label, bool pressed) {
    if (fsIndex < 0 || fsIndex > 1) return;

    if (fs_labels[fsIndex]) {
        lv_label_set_text(fs_labels[fsIndex], label);
        lv_obj_set_style_text_color(fs_labels[fsIndex],
            pressed ? COLOR_ACCENT_BRIGHT : COLOR_TEXT_SECONDARY, 0);

        if (fs_containers[fsIndex]) {
            lv_obj_set_style_border_width(fs_containers[fsIndex], 1, 0);
            lv_obj_set_style_border_color(fs_containers[fsIndex],
                pressed ? COLOR_ACCENT : COLOR_SEPARATOR, 0);
            lv_obj_set_style_bg_color(fs_containers[fsIndex],
                pressed ? COLOR_SURFACE_ELEV : COLOR_SURFACE, 0);
        }
    }
}
