#include "UiApp.h"
#include "UiTheme.h"
#include "screens/PerformanceScreen.h"
#include "screens/FxChainScreen.h"
#include "screens/PresetsScreen.h"
#include "screens/FootswitchScreen.h"
#include "screens/SystemScreen.h"
#include "screens/EffectEditScreen.h"
#include "screens/SettingsScreen.h"
#include "screens/MasterScreen.h"
#include "board/LGFX_CYD.h"
#include "control/FootswitchManager.h"
#include "model/TapTempo.h"
#include "session/PerformanceSession.h"
#include "storage/LibraryStorage.h"
#include <lvgl.h>
#include <Arduino.h>
#include <cstdio>
#include <cctype>
#include <cstring>
#include <cmath>

using namespace VoxUiTheme;

static UiAppState app_state = {};

// Performance Session & Library
static Library s_library;
static PerformanceSession s_session(&s_library);

PerformanceSession* ui_get_performance_session() {
    return &s_session;
}

// VoxLink bridge state (see VoxLinkGlue.cpp).
static UiWireIntentFn s_wire_intent = nullptr;
static bool s_presets_available = true;
static bool s_bypass_available = true;

// Tap Tempo estimator. Pure C++, owned by the controller.
static TapTempo s_tap_tempo;

// Footswitch snapshot kept by the App layer so a physical event never needs to
// consult a screen. Labels are derived once from the FootswitchManager config.
static char fs_short_label[2][16] = {"--", "--"};
static bool fs_pressed[2] = {false, false};

#if defined(DEBUG_BUILD)
static void ui_log_heap(const char* stage) {
    Serial.printf("[UI] %s created, heap: %u (min %u)\n", stage,
                  (unsigned)esp_get_free_heap_size(),
                  (unsigned)esp_get_minimum_free_heap_size());
}
#endif

static bool effect_enabled_from_state(UiEffectId effect) {
    return app_state.params.get(ui_effect_enable_wire(effect)) >= 0.5f;
}

// Recompute and fan out the summary for a single effect. Only the affected
// effect's Performance card and FX Chain module are touched.
static void update_effect_summary(UiEffectId effect) {
    if (effect >= UiEffectId::Count) return;
    char main_value[24];
    char metadata[24];
    ui_build_effect_summary(effect, app_state.params, main_value,
                            sizeof(main_value), metadata, sizeof(metadata));
    performance_update_effect_value(effect, main_value);
    fx_chain_update_effect_state(effect, effect_enabled_from_state(effect),
                                 main_value, metadata);
}

static void refresh_effect_enable(UiEffectId effect) {
    performance_update_effect(effect, effect_enabled_from_state(effect));
    effect_edit_set_enabled(effect, effect_enabled_from_state(effect));
    update_effect_summary(effect);
}

static void fan_out_parameter(uint16_t wireId, float value);
static void apply_session_deltas(const std::vector<ParamDelta>& deltas);

// Action dispatcher for view intent. This is the boundary between screens and
// state authority.
void ui_emit_action(const UiAction& action) {
    switch (action.type) {
        case UiActionType::ToggleEffect: {
            if (action.id >= kUiEffectCount) return;
            const UiEffectId effect = static_cast<UiEffectId>(action.id);
            ui_set_effect_enable_local(effect, !effect_enabled_from_state(effect));
            break;
        }

        case UiActionType::OpenEffect:
            // Local navigation action, not sent to P4.
            effect_edit_load_effect(static_cast<UiEffectId>(action.id));
            effect_edit_set_enabled(static_cast<UiEffectId>(action.id),
                                    (action.id < kUiEffectCount)
                                        ? effect_enabled_from_state(
                                              static_cast<UiEffectId>(action.id))
                                        : false);
            ui_navigate_to(UiScreenId::EFFECT_EDIT);
            break;

        case UiActionType::AllEffectsOn:
            for (size_t i = 0; i < kUiEffectCount; ++i) {
                ui_set_effect_enable_local(static_cast<UiEffectId>(i), true);
            }
            break;

        case UiActionType::GlobalBypass:
            // Global bypass has no VoxLink v1 backend. It stays a local-only
            // flag and is unavailable while a real link is active.
            if (!s_bypass_available) break;
            app_state.globalBypass = !app_state.globalBypass;
            fx_chain_set_bypass(app_state.globalBypass);
            break;

        case UiActionType::LoadPreset:
            if (action.id > 0) app_state.selectedPresetId = action.id;
            ui_load_selected_preset();
            break;

        case UiActionType::SavePreset:
        case UiActionType::CommitEdits:
            ui_commit_edits();
            break;

        case UiActionType::RevertEdits:
            ui_revert_edits();
            break;

        case UiActionType::NextSubscene:
            apply_session_deltas(s_session.nextSubscene());
            break;

        case UiActionType::PrevSubscene:
            apply_session_deltas(s_session.previousSubscene());
            break;

        case UiActionType::NextScene:
            apply_session_deltas(s_session.nextScene());
            break;

        case UiActionType::PrevScene:
            apply_session_deltas(s_session.previousScene());
            break;

        case UiActionType::SetParameter:
            // action.id is a VoxLink wire ID. Optimistic local edit; produces a
            // VoxLink intent when connected.
            ui_set_parameter_local(action.id, action.value);
            break;

        case UiActionType::SetFootswitchConfig:
            // Reserved for a future milestone.
            break;
    }
}

// Helper to convert MIDI note number to note name string (e.g. "A3")
static const char* note_name_from_midi(int note) {
    static const char* notes[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
    static char buf[8];
    if (note < 0 || note > 127) return "--";
    snprintf(buf, sizeof(buf), "%s%d", notes[note % 12], note / 12 - 1);
    return buf;
}

// Shell components
static lv_obj_t* content_area = nullptr;
static lv_obj_t* nav_bar = nullptr;
static lv_obj_t* nav_tabs[4] = {nullptr, nullptr, nullptr, nullptr};
static const char* nav_labels[] = {"PERF", "FX", "PRESET", "SET"};

// Screen containers
static constexpr int kScreenContainerCount = 8;
static lv_obj_t* screen_containers[kScreenContainerCount] = {nullptr};
static UiScreenId current_screen = UiScreenId::PERFORMANCE;

// LVGL display driver
static lv_disp_drv_t disp_drv;
static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf1[VoxCydConfig::LvglBufferSize / 2];
static lv_color_t buf2[VoxCydConfig::LvglBufferSize / 2];

// LovyanGFX device instance
static LGFX_CYD* lcd_device = nullptr;

// Display flush callback
static void disp_flush(lv_disp_drv_t* disp, const lv_area_t* area, lv_color_t* color_p) {
    if (lcd_device) {
        lcd_device->startWrite();
        lcd_device->setAddrWindow(area->x1, area->y1, area->x2 - area->x1 + 1, area->y2 - area->y1 + 1);
        lcd_device->writePixels(reinterpret_cast<const uint16_t*>(color_p),
                               lv_area_get_width(area) * lv_area_get_height(area), true);
        lcd_device->endWrite();
    }
    lv_disp_flush_ready(disp);
}

// Touch read callback
static void touchpad_read(lv_indev_drv_t* indev_drv, lv_indev_data_t* data) {
    if (!lcd_device) return;

    uint16_t touchX, touchY;
    bool touched = lcd_device->getTouch(&touchX, &touchY);

    if (!touched) {
        data->state = LV_INDEV_STATE_REL;
    } else {
        data->state = LV_INDEV_STATE_PR;
        data->point.x = touchX;
        data->point.y = touchY;
    }
}

// Navigation tab click callback
static void nav_tab_clicked(lv_event_t* e) {
    lv_obj_t* tab = lv_event_get_target(e);
    int index = (int)(intptr_t)lv_obj_get_user_data(tab);

    if (index >= 0 && index < 4) {
        if (index == 3) {
            ui_navigate_to(UiScreenId::SETTINGS);
        } else {
            ui_navigate_to(static_cast<UiScreenId>(index));
        }
    }
}

static void create_nav_bar(lv_obj_t* parent) {
    nav_bar = lv_obj_create(parent);
    lv_obj_set_size(nav_bar, LV_PCT(100), 36);
    lv_obj_set_style_bg_color(nav_bar, COLOR_NAV, 0);
    lv_obj_set_style_radius(nav_bar, 0, 0);
    lv_obj_set_style_border_width(nav_bar, 0, 0);
    lv_obj_set_style_pad_all(nav_bar, 0, 0);
    lv_obj_align(nav_bar, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_flex_flow(nav_bar, LV_FLEX_FLOW_ROW);

    for (int i = 0; i < 4; i++) {
        nav_tabs[i] = lv_btn_create(nav_bar);
        lv_obj_set_size(nav_tabs[i], LV_PCT(25), LV_PCT(100));
        lv_obj_set_style_bg_color(nav_tabs[i], COLOR_NAV, 0);
        lv_obj_set_style_radius(nav_tabs[i], 0, 0);
        lv_obj_set_style_border_width(nav_tabs[i], 0, 0);
        lv_obj_set_style_shadow_width(nav_tabs[i], 0, 0);
        lv_obj_set_flex_grow(nav_tabs[i], 1);
        ui_apply_pressed(nav_tabs[i], COLOR_SURFACE_ELEV, COLOR_BORDER);
        lv_obj_set_user_data(nav_tabs[i], (void*)(intptr_t)i);
        lv_obj_add_event_cb(nav_tabs[i], nav_tab_clicked, LV_EVENT_CLICKED, NULL);

        lv_obj_t* label = lv_label_create(nav_tabs[i]);
        lv_label_set_text(label, nav_labels[i]);
        lv_obj_set_style_text_color(label, COLOR_TEXT_MUTED, 0);
        lv_obj_set_style_text_font(label, FONT_SMALL, 0);
        lv_obj_center(label);
    }

    update_nav_bar(current_screen);
}

void update_nav_bar(UiScreenId active_screen) {
    int active_idx = (int)active_screen;

    if (active_screen == UiScreenId::EFFECT_EDIT) {
        active_idx = 1; // FX_CHAIN tab
    } else if (active_screen == UiScreenId::SYSTEM ||
               active_screen == UiScreenId::FOOTSWITCH ||
               active_screen == UiScreenId::SETTINGS ||
               active_screen == UiScreenId::MASTER) {
        active_idx = 3; // SET tab
    }

    for (int i = 0; i < 4; i++) {
        if (!nav_tabs[i]) continue;

        bool is_active = (i == active_idx);
        lv_obj_set_style_bg_color(nav_tabs[i], COLOR_NAV, 0);
        lv_obj_set_style_border_side(nav_tabs[i], is_active ? LV_BORDER_SIDE_TOP : LV_BORDER_SIDE_NONE, 0);
        lv_obj_set_style_border_width(nav_tabs[i], is_active ? 2 : 0, 0);
        lv_obj_set_style_border_color(nav_tabs[i], COLOR_ACCENT, 0);

        lv_obj_t* label = lv_obj_get_child(nav_tabs[i], 0);
        if (label) {
            lv_obj_set_style_text_color(label, is_active ? COLOR_TEXT_PRIMARY : COLOR_TEXT_MUTED, 0);
        }
    }
}

static void show_screen(UiScreenId screen_id) {
    int idx = (int)screen_id;

    for (int i = 0; i < kScreenContainerCount; i++) {
        if (screen_containers[i]) {
            if (i == idx) {
                lv_obj_clear_flag(screen_containers[i], LV_OBJ_FLAG_HIDDEN);
            } else {
                lv_obj_add_flag(screen_containers[i], LV_OBJ_FLAG_HIDDEN);
            }
        }
    }
}

void ui_app_init(LGFX_CYD& display) {
    lv_init();
    ui_theme_init();

    lcd_device = &display;

    lv_disp_draw_buf_init(&draw_buf, buf1, buf2, VoxCydConfig::ScreenWidth * VoxCydConfig::LvglBufferLines);

    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = VoxCydConfig::ScreenWidth;
    disp_drv.ver_res = VoxCydConfig::ScreenHeight;
    disp_drv.flush_cb = disp_flush;
    disp_drv.draw_buf = &draw_buf;
    disp_drv.full_refresh = 0;
    disp_drv.direct_mode = 0;
    lv_disp_drv_register(&disp_drv);

    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = touchpad_read;
    lv_indev_drv_register(&indev_drv);

    content_area = lv_obj_create(lv_scr_act());
    lv_obj_set_size(content_area, LV_PCT(100), 204);
    lv_obj_set_style_bg_color(content_area, COLOR_BG, 0);
    lv_obj_set_style_pad_all(content_area, 0, 0);
    lv_obj_set_style_border_width(content_area, 0, 0);
    lv_obj_align(content_area, LV_ALIGN_TOP_MID, 0, 0);

    for (int i = 0; i < kScreenContainerCount; i++) {
        screen_containers[i] = lv_obj_create(content_area);
        lv_obj_set_size(screen_containers[i], LV_PCT(100), LV_PCT(100));
        lv_obj_set_style_bg_color(screen_containers[i], COLOR_BG, 0);
        lv_obj_set_style_pad_all(screen_containers[i], 0, 0);
        lv_obj_set_style_border_width(screen_containers[i], 0, 0);
        if (i != 0) {
            lv_obj_add_flag(screen_containers[i], LV_OBJ_FLAG_HIDDEN);
        }
    }

#if defined(DEBUG_BUILD)
    Serial.printf("[UI] heap before screens: %u\n", (unsigned)esp_get_free_heap_size());
#endif
    performance_screen_init(screen_containers[0]);
#if defined(DEBUG_BUILD)
    ui_log_heap("Performance");
#endif
    fx_chain_screen_init(screen_containers[1]);
#if defined(DEBUG_BUILD)
    ui_log_heap("FX Chain");
#endif
    presets_screen_init(screen_containers[2]);
#if defined(DEBUG_BUILD)
    ui_log_heap("Presets");
#endif
    footswitch_screen_init(screen_containers[3]);
#if defined(DEBUG_BUILD)
    ui_log_heap("Footswitch");
#endif
    system_screen_init(screen_containers[4]);
#if defined(DEBUG_BUILD)
    ui_log_heap("System");
#endif
    effect_edit_screen_init(screen_containers[5]);
#if defined(DEBUG_BUILD)
    ui_log_heap("Effect Edit");
#endif
    settings_screen_init(screen_containers[6]);
#if defined(DEBUG_BUILD)
    ui_log_heap("Settings");
#endif
    master_screen_init(screen_containers[7]);
#if defined(DEBUG_BUILD)
    ui_log_heap("Master");
#endif

    create_nav_bar(lv_scr_act());
#if defined(DEBUG_BUILD)
    ui_log_heap("Nav bar");
#endif

    app_state.currentScreen = UiScreenId::PERFORMANCE;
    app_state.linkUp = false;
    app_state.globalBypass = false;
    app_state.selectedPresetId = 0;
    app_state.presetName[0] = '\0';

    performance_update_link(app_state.linkUp);

    // The canonical state already holds registry defaults (including effect
    // enables). Fan them out to every view; never rely on zero initialization.
    for (size_t i = 0; i < kUiEffectCount; ++i) {
        refresh_effect_enable(static_cast<UiEffectId>(i));
    }
    master_screen_refresh();
    performance_update_tempo(app_state.params.get(VOXP4_PARAM_TEMPO_BPM));

    // Footswitch config comes from the real FootswitchManager, never hardcoded.
    for (int i = 0; i < 2; i++) {
        FootswitchConfig* cfg = footswitch_get_config(i);
        if (cfg) {
            ui_update_footswitch_config(i, (uint8_t)cfg->mode, (uint8_t)cfg->pressAction);
            footswitch_update_edit(i, (uint8_t)cfg->mode, (uint8_t)cfg->pressAction,
                                   (uint8_t)cfg->releaseAction,
                                   (uint8_t)cfg->longPressAction,
                                   (uint8_t)cfg->doublePressAction);
        }
    }

    // Initialize library & session
    LibraryStorage::init();
    LibraryStorage::loadLibrary(s_library);
    s_session.setLibrary(&s_library);

    // Apply resolved state from session (canonical, wire-id keyed).
    const ResolvedState& resolved = s_session.getResolvedState();
    for (size_t i = 0; i < ParameterRegistry::kParamCount; ++i) {
        const uint16_t wid = ParameterRegistry::denseIndexToWireId(i);
        if (wid == 0) continue;
        app_state.params.applyAuthoritative(wid, resolved.getByWireId(wid).asFloat());
    }
    for (size_t i = 0; i < kUiEffectCount; ++i) {
        refresh_effect_enable(static_cast<UiEffectId>(i));
    }
    master_screen_refresh();
    performance_update_tempo(app_state.params.get(VOXP4_PARAM_TEMPO_BPM));

    if (!s_library.scenes.empty()) {
        const char* sceneNames[16];
        int count = 0;
        for (size_t i = 0; i < s_library.scenes.size() && count < 16; ++i) {
            sceneNames[count++] = s_library.scenes[i].name.c_str();
        }
        presets_update_list(sceneNames, count);
        presets_update_current(1, sceneNames[0]);
    } else {
        ui_update_preset(1, presets_get_name(0));
    }
    ui_update_performance_header();

    performance_update_meters(app_state.inputPeakDb, app_state.outputPeakDb);
    const char* noteName = note_name_from_midi(app_state.detectedNote);
    performance_update_pitch(app_state.pitchFreqHz, noteName, app_state.voiced);
    fx_chain_set_bypass(app_state.globalBypass);
}

void ui_app_run(void) {
    // Deferred editor rebuilds (mode changes) run outside LVGL event dispatch.
    effect_edit_tick();
    lv_timer_handler();
    delay(VoxCydConfig::LvglTickMs);
}

void ui_navigate_to(UiScreenId screen) {
    if (current_screen == screen) return;

    current_screen = screen;
    show_screen(screen);
    update_nav_bar(screen);

    // Master screen values may have changed while it was hidden.
    if (screen == UiScreenId::MASTER) master_screen_refresh();
}

UiScreenId ui_get_current_screen(void) {
    return current_screen;
}

void ui_update_link_state(bool connected) {
    app_state.linkUp = connected;
    performance_update_link(connected);
}

void ui_update_preset(uint16_t id, const char* name) {
    app_state.currentPresetId = id;
    app_state.selectedPresetId = id;

    const char* safe_name = (name && name[0]) ? name : "--";
    char buf[40];
    snprintf(buf, sizeof(buf), "P%02u  %s", (unsigned)id, safe_name);
    for (char* p = buf; *p; ++p) *p = (char)toupper((unsigned char)*p);
    strncpy(app_state.presetName, buf, sizeof(app_state.presetName) - 1);
    app_state.presetName[sizeof(app_state.presetName) - 1] = '\0';

    performance_update_preset(app_state.presetName);
    presets_update_current(id, safe_name);
    presets_select_preset((int)id - 1);
}

void ui_select_preset(int index) {
    if (index < 0) return;
    app_state.selectedPresetId = (uint16_t)(index + 1);
    presets_select_preset(index);
}

void ui_load_selected_preset(void) {
    const uint16_t id = app_state.selectedPresetId;
    if (id > 0 && id <= s_library.scenes.size()) {
        apply_session_deltas(s_session.selectScene(s_library.scenes[id - 1].id));
    } else if (id > 0) {
        ui_update_preset(id, presets_get_name((int)id - 1));
    }
}

bool ui_is_global_bypass(void) {
    return app_state.globalBypass;
}

const char* ui_footswitch_short_label(uint8_t action) {
    const char* full = footswitch_action_name((FootswitchAction)action);
    if (full == nullptr) return "NONE";
    if (strstr(full, "SUBSCENE NEXT")) return "SUB NEXT";
    if (strstr(full, "SUBSCENE PREV")) return "SUB PREV";
    if (strstr(full, "SCENE NEXT")) return "SCENE NEXT";
    if (strstr(full, "SCENE PREV")) return "SCENE PREV";
    if (strstr(full, "HARMONY")) return "HARMONY";
    if (strstr(full, "REVERB")) return "REVERB";
    if (strstr(full, "DELAY")) return "DELAY";
    if (strstr(full, "DRIVE")) return "DRIVE";
    if (strstr(full, "GATE")) return "GATE";
    if (strstr(full, "COMPRESSOR")) return "COMP";
    if (strstr(full, "MODULATION")) return "MOD";
    if (strstr(full, "TAP TEMPO")) return "TAP";
    if (strstr(full, "PRESET NEXT")) return "NEXT";
    if (strstr(full, "PRESET PREV")) return "PREV";
    if (strstr(full, "GLOBAL BYPASS")) return "BYPASS";
    return "NONE";
}

void ui_update_footswitch_config(int index, uint8_t mode, uint8_t action) {
    if (index < 0 || index > 1) return;
    snprintf(fs_short_label[index], sizeof(fs_short_label[index]), "%s",
             ui_footswitch_short_label(action));
    footswitch_update_config(index, mode, fs_short_label[index]);
    performance_update_footswitch(index, fs_short_label[index], fs_pressed[index]);
}

uint8_t ui_get_footswitch_field(int index, UiFootswitchField field) {
    if (index < 0 || index > 1) return 0;
    FootswitchConfig* cfg = footswitch_get_config(index);
    if (cfg == nullptr) return 0;
    switch (field) {
        case UiFootswitchField::Mode: return (uint8_t)cfg->mode;
        case UiFootswitchField::Press: return (uint8_t)cfg->pressAction;
        case UiFootswitchField::Release: return (uint8_t)cfg->releaseAction;
        case UiFootswitchField::LongPress: return (uint8_t)cfg->longPressAction;
        case UiFootswitchField::DoublePress: return (uint8_t)cfg->doublePressAction;
        default: return 0;
    }
}

void ui_set_footswitch_field(int index, UiFootswitchField field, uint8_t value) {
    if (index < 0 || index > 1) return;
    FootswitchConfig* cfg = footswitch_get_config(index);
    if (cfg == nullptr) return;
    switch (field) {
        case UiFootswitchField::Mode: cfg->mode = (FootswitchMode)value; break;
        case UiFootswitchField::Press: cfg->pressAction = (FootswitchAction)value; break;
        case UiFootswitchField::Release: cfg->releaseAction = (FootswitchAction)value; break;
        case UiFootswitchField::LongPress: cfg->longPressAction = (FootswitchAction)value; break;
        case UiFootswitchField::DoublePress: cfg->doublePressAction = (FootswitchAction)value; break;
        default: return;
    }
    // Fan the new configuration out to every representation.
    ui_update_footswitch_config(index, (uint8_t)cfg->mode, (uint8_t)cfg->pressAction);
    footswitch_update_edit(index, (uint8_t)cfg->mode,
                           (uint8_t)cfg->pressAction, (uint8_t)cfg->releaseAction,
                           (uint8_t)cfg->longPressAction,
                           (uint8_t)cfg->doublePressAction);
}

// Apply a single footswitch press action. Release handling for momentary
// actions is done in ui_update_footswitch_state.
static void execute_footswitch_action(int index, FootswitchAction action) {
    switch (action) {
        case FS_ACTION_SUBSCENE_NEXT:
            ui_emit_action({ UiActionType::NextSubscene, 0, 0.0f });
            break;
        case FS_ACTION_SUBSCENE_PREV:
            ui_emit_action({ UiActionType::PrevSubscene, 0, 0.0f });
            break;
        case FS_ACTION_SCENE_NEXT:
        case FS_ACTION_PRESET_NEXT:
            ui_emit_action({ UiActionType::NextScene, 0, 0.0f });
            break;
        case FS_ACTION_SCENE_PREV:
        case FS_ACTION_PRESET_PREV:
            ui_emit_action({ UiActionType::PrevScene, 0, 0.0f });
            break;
        case FS_ACTION_HARMONY_TOGGLE:
        case FS_ACTION_HARMONY_MOMENTARY:
            ui_set_effect_enable_local(UiEffectId::Harmony,
                                       action == FS_ACTION_HARMONY_MOMENTARY
                                           ? true
                                           : !ui_effect_enabled(UiEffectId::Harmony));
            break;
        case FS_ACTION_REVERB_TOGGLE:
            ui_set_effect_enable_local(UiEffectId::Reverb,
                                       !ui_effect_enabled(UiEffectId::Reverb));
            break;
        case FS_ACTION_DELAY_TOGGLE:
            ui_set_effect_enable_local(UiEffectId::Delay,
                                       !ui_effect_enabled(UiEffectId::Delay));
            break;
        case FS_ACTION_MODULATION_TOGGLE:
            ui_set_effect_enable_local(UiEffectId::Modulation,
                                       !ui_effect_enabled(UiEffectId::Modulation));
            break;
        case FS_ACTION_DRIVE_TOGGLE:
            ui_set_effect_enable_local(UiEffectId::Drive,
                                       !ui_effect_enabled(UiEffectId::Drive));
            break;
        case FS_ACTION_GATE_TOGGLE:
            ui_set_effect_enable_local(UiEffectId::Gate,
                                       !ui_effect_enabled(UiEffectId::Gate));
            break;
        case FS_ACTION_COMPRESSOR_TOGGLE:
            ui_set_effect_enable_local(UiEffectId::Compressor,
                                       !ui_effect_enabled(UiEffectId::Compressor));
            break;
        case FS_ACTION_TAP_TEMPO:
            ui_tap_tempo();
            break;
        case FS_ACTION_GLOBAL_BYPASS:
            ui_emit_action({ UiActionType::GlobalBypass, 0, 0.0f });
            break;
        default:
            (void)index;
            break;
    }
}

void ui_update_footswitch_state(int index, bool pressed) {
    if (index < 0 || index > 1) return;
    fs_pressed[index] = pressed;
    performance_update_footswitch(index, fs_short_label[index], pressed);
    footswitch_update_state(index, pressed);

    FootswitchConfig* cfg = footswitch_get_config(index);
    if (cfg == nullptr) return;

    if (pressed) {
        execute_footswitch_action(index, cfg->pressAction);
    } else {
        // Momentary actions release on the falling edge.
        if (cfg->pressAction == FS_ACTION_HARMONY_MOMENTARY) {
            ui_set_effect_enable_local(UiEffectId::Harmony, false);
        }
        if (cfg->releaseAction != FS_ACTION_NONE) {
            execute_footswitch_action(index, cfg->releaseAction);
        }
    }
}

// ---------------------------------------------------------------------------
// Parameter model
// ---------------------------------------------------------------------------
float ui_get_parameter(uint16_t wireId) {
    return app_state.params.get(wireId);
}

bool ui_parameter_is_valid(uint16_t wireId) {
    return app_state.params.isValid(wireId);
}

UiValueAuthority ui_parameter_authority(uint16_t wireId) {
    return app_state.params.authority(wireId);
}

float ui_get_authoritative_parameter(uint16_t wireId) {
    return app_state.params.authoritative(wireId);
}

const UiParamState& ui_get_param_state(void) {
    return app_state.params;
}

float ui_get_tempo_bpm(void) {
    return app_state.params.get(VOXP4_PARAM_TEMPO_BPM);
}

void ui_format_effect_summary(UiEffectId effect, char* mainValue, size_t mainSize,
                              char* metadata, size_t metaSize) {
    if (effect >= UiEffectId::Count) return;
    ui_build_effect_summary(effect, app_state.params, mainValue, mainSize,
                            metadata, metaSize);
}

bool ui_effect_enabled(UiEffectId effect) {
    return effect_enabled_from_state(effect);
}

static void fan_out_parameter(uint16_t wireId, float value) {
    (void)value;
    UiEffectId effect;
    if (ui_effect_from_enable_wire(wireId, &effect)) {
        refresh_effect_enable(effect);
    } else {
        effect = ui_effect_of_wire(wireId);
        if (effect < UiEffectId::Count) update_effect_summary(effect);
    }
    if (wireId == VOXP4_PARAM_TEMPO_BPM) {
        performance_update_tempo(app_state.params.get(VOXP4_PARAM_TEMPO_BPM));
    }
    effect_edit_notify(wireId, value);
    master_screen_notify(wireId, value);
}

void ui_update_performance_header(void) {
    const Scene* sc = s_session.getActiveScene();
    const Subscene* sub = s_session.getActiveSubscene();
    const Setlist* sl = s_session.getActiveSetlist();

    const char* sceneName = sc ? sc->name.c_str() : "No Scene";
    const char* subName = sub ? sub->name.c_str() : (sc ? "Default" : "--");

    int subIdx = 0, subTotal = 0;
    if (sc && !sc->subscenes.empty()) {
        subTotal = static_cast<int>(sc->subscenes.size());
        for (size_t i = 0; i < sc->subscenes.size(); ++i) {
            if (sc->subscenes[i].id == s_session.getActiveSubsceneId()) {
                subIdx = static_cast<int>(i) + 1;
                break;
            }
        }
    }

    int setIdx = 0, setTotal = 0;
    if (sl && !sl->entries.empty()) {
        setTotal = static_cast<int>(sl->entries.size());
        setIdx = s_session.getActiveEntryIndex() + 1;
    }

    performance_update_scene_status(sceneName, subName, subIdx, subTotal, setIdx, setTotal, s_session.isDirty());
}

static void apply_session_deltas(const std::vector<ParamDelta>& deltas) {
    for (const auto& delta : deltas) {
        app_state.params.applyAuthoritative(delta.wireId, delta.value.asFloat());
        fan_out_parameter(delta.wireId, delta.value.asFloat());
        if (s_wire_intent != nullptr) {
            s_wire_intent(delta.wireId, delta.value.asFloat());
        }
    }
    ui_update_performance_header();
}

void ui_commit_edits(void) {
    if (s_session.commitTemporaryEdits()) {
        LibraryStorage::saveLibraryAtomic(s_library);
        ui_update_performance_header();
    }
}

void ui_revert_edits(void) {
    std::vector<ParamDelta> deltas = s_session.revertTemporaryEdits();
    apply_session_deltas(deltas);
}

void ui_set_parameter_local(uint16_t wireId, float value) {
    const UiParamMeta* meta = ui_param_meta(wireId);
    float clamped = value;
    if (meta != nullptr) {
        clamped = ui_clamp_parameter(*meta, value);
    }
    app_state.params.setLocal(wireId, clamped);
    const float applied = app_state.params.get(wireId);
    fan_out_parameter(wireId, applied);

    s_session.applyTemporaryEdit(wireId, ParameterValue::makeFloat(applied));
    ui_update_performance_header();

    if (s_wire_intent != nullptr) s_wire_intent(wireId, applied);
}

void ui_apply_parameter_authoritative(uint16_t wireId, float acceptedValue) {
    app_state.params.applyAuthoritative(wireId, acceptedValue);
    fan_out_parameter(wireId, app_state.params.get(wireId));
}

void ui_revert_parameter(uint16_t wireId) {
    app_state.params.revert(wireId);
    fan_out_parameter(wireId, app_state.params.get(wireId));
}

void ui_set_effect_enable_local(UiEffectId effect, bool enabled) {
    const uint16_t wireId = ui_effect_enable_wire(effect);
    if (wireId == 0) return;
    ui_set_parameter_local(wireId, enabled ? 1.0f : 0.0f);
}

void ui_tap_tempo(void) {
    float bpm = 0.0f;
    if (s_tap_tempo.tap(millis(), &bpm)) {
        ui_set_parameter_local(VOXP4_PARAM_TEMPO_BPM, bpm);
    }
    master_screen_refresh_tap(s_tap_tempo.hasEstimate(), s_tap_tempo.bpm());
}

void ui_set_wire_intent_callback(UiWireIntentFn fn) { s_wire_intent = fn; }

void ui_refresh_capability_gated_controls() {
    if (current_screen == UiScreenId::EFFECT_EDIT) effect_edit_refresh();
    master_screen_refresh();
}

void ui_set_link_capabilities(bool presetsAvailable, bool bypassAvailable) {
    s_presets_available = presetsAvailable;
    s_bypass_available = bypassAvailable;
    presets_set_available(presetsAvailable);
    fx_chain_set_bypass_available(bypassAvailable);
    if (!bypassAvailable) {
        app_state.globalBypass = false;
        fx_chain_set_bypass(false);
    }
}

void ui_update_meters(float inputDb, float outputDb) {
    app_state.inputPeakDb = inputDb;
    app_state.outputPeakDb = outputDb;
    performance_update_meters(inputDb, outputDb);
}

void ui_update_pitch(float freqHz, int note, bool voiced) {
    app_state.pitchFreqHz = freqHz;
    app_state.detectedNote = note;
    app_state.voiced = voiced;
    const char* noteName = note_name_from_midi(note);
    performance_update_pitch(freqHz, noteName, voiced);
}
