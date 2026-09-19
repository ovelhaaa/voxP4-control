#include "UiApp.h"
#include "UiTheme.h"
#include "screens/PerformanceScreen.h"
#include "screens/FxChainScreen.h"
#include "screens/PresetsScreen.h"
#include "screens/FootswitchScreen.h"
#include "screens/SystemScreen.h"
#include "screens/EffectEditScreen.h"
#include "screens/SettingsScreen.h"
#include "board/LGFX_CYD.h"
#include <lvgl.h>
#include <cstdio>
#include <cctype>

using namespace VoxUiTheme;

static UiAppState app_state = {};

// Canonical effect display metadata. Keep this list in sync with the effect
// index order used everywhere: 0 HARMONY, 1 REVERB, 2 DELAY, 3 LIMITER.
static const char* kEffectNames[4]     = {"HARMONY", "REVERB", "DELAY", "LIMITER"};
static const char* kEffectMainValues[4] = {"+3rd", "18%", "--", "-3 dB"};
static const char* kEffectMetadata[4]   = {"KEY AUTO", "PLATE", "1/4", "THRESHOLD"};

const char* ui_effect_name(int effectId) {
    if (effectId < 0 || effectId >= 4) return "--";
    return kEffectNames[effectId];
}

const char* ui_effect_main_value(int effectId) {
    if (effectId < 0 || effectId >= 4) return "--";
    return kEffectMainValues[effectId];
}

const char* ui_effect_metadata(int effectId) {
    if (effectId < 0 || effectId >= 4) return "";
    return kEffectMetadata[effectId];
}

static bool effect_enabled_from_state(int effectId) {
    switch (effectId) {
        case 0: return app_state.harmonyEnabled;
        case 1: return app_state.reverbEnabled;
        case 2: return app_state.delayEnabled;
        case 3: return app_state.limiterEnabled;
        default: return false;
    }
}

// Placeholder for action handling. In the future this will dispatch to VoxLink.
void ui_emit_action(const UiAction& action) {
    // This provides a clear boundary between view intent and state authority.
    // Until VoxLink is wired, actions update the local optimistic state, which
    // is then fanned out to every screen that represents an effect.
    if (action.type == UiActionType::ToggleEffect) {
        if (action.id >= 4) return;
        ui_update_effect_state(action.id, !effect_enabled_from_state(action.id));
    } else if (action.type == UiActionType::OpenEffect) {
        // Local navigation action, not sent to P4.
        effect_edit_load_effect(action.id);
        if (action.id < 4) {
            effect_edit_set_enabled(action.id, effect_enabled_from_state(action.id));
        }
        ui_navigate_to(UiScreenId::EFFECT_EDIT);
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
static lv_obj_t* screen_containers[7] = {nullptr};
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
        // Must use the uint16_t overload: the void* overload treats the buffer
        // as 3-byte RGB888, which garbles the RGB565 framebuffer.
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
        // The LGFX_CYD config already handles rotation correctly
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

// Create navigation bar at bottom
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
        lv_obj_set_user_data(nav_tabs[i], (void*)(intptr_t)i);
        lv_obj_add_event_cb(nav_tabs[i], nav_tab_clicked, LV_EVENT_CLICKED, NULL);
        
        lv_obj_t* label = lv_label_create(nav_tabs[i]);
        lv_label_set_text(label, nav_labels[i]);
        lv_obj_set_style_text_color(label, COLOR_TEXT_MUTED, 0);
        lv_obj_set_style_text_font(label, FONT_SMALL, 0);
        lv_obj_center(label);
    }
    
    // Update active tab visual
    update_nav_bar(current_screen);
}

// Update navigation bar to show active tab
void update_nav_bar(UiScreenId active_screen) {
    int active_idx = (int)active_screen;
    
    // Handle subscreens - show parent tab as active
    if (active_screen == UiScreenId::EFFECT_EDIT) {
        active_idx = 1; // FX_CHAIN tab
    } else if (active_screen == UiScreenId::SYSTEM || active_screen == UiScreenId::FOOTSWITCH || active_screen == UiScreenId::SETTINGS) {
        active_idx = 3; // SET tab
    }
    
    for (int i = 0; i < 4; i++) {
        if (!nav_tabs[i]) continue;
        
        bool is_active = (i == active_idx);
        lv_obj_set_style_bg_color(nav_tabs[i], is_active ? COLOR_BG : COLOR_NAV, 0);
        lv_obj_set_style_border_side(nav_tabs[i], is_active ? LV_BORDER_SIDE_TOP : LV_BORDER_SIDE_NONE, 0);
        lv_obj_set_style_border_width(nav_tabs[i], is_active ? 3 : 0, 0);
        lv_obj_set_style_border_color(nav_tabs[i], COLOR_ACCENT, 0);
        
        lv_obj_t* label = lv_obj_get_child(nav_tabs[i], 0);
        if (label) {
            lv_obj_set_style_text_color(label, is_active ? COLOR_TEXT_PRIMARY : COLOR_TEXT_MUTED, 0);
        }
    }
}

// Show/hide screen containers using LVGL 8.x API
static void show_screen(UiScreenId screen_id) {
    int idx = (int)screen_id;
    
    for (int i = 0; i < 7; i++) {
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
    
    // Initialize theme styles
    ui_theme_init();
    
    // Store display reference
    lcd_device = &display;
    
    // Initialize LVGL display buffer
    lv_disp_draw_buf_init(&draw_buf, buf1, buf2, VoxCydConfig::ScreenWidth * VoxCydConfig::LvglBufferLines);
    
    // Initialize display driver
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = VoxCydConfig::ScreenWidth;
    disp_drv.ver_res = VoxCydConfig::ScreenHeight;
    disp_drv.flush_cb = disp_flush;
    disp_drv.draw_buf = &draw_buf;
    disp_drv.full_refresh = 0;
    disp_drv.direct_mode = 0;
    lv_disp_drv_register(&disp_drv);
    
    // Initialize input device (Touchpad)
    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = touchpad_read;
    lv_indev_drv_register(&indev_drv);

    // Create content area (top portion, leaving space for nav bar)
    content_area = lv_obj_create(lv_scr_act());
    lv_obj_set_size(content_area, LV_PCT(100), 204); // 240 - 36 = 204
    lv_obj_set_style_bg_color(content_area, COLOR_BG, 0);
    lv_obj_set_style_pad_all(content_area, 0, 0);
    lv_obj_set_style_border_width(content_area, 0, 0);
    lv_obj_align(content_area, LV_ALIGN_TOP_MID, 0, 0);
    
    // Create screen containers
    screen_containers[0] = lv_obj_create(content_area); // PERF
    screen_containers[1] = lv_obj_create(content_area); // FX
    screen_containers[2] = lv_obj_create(content_area); // PRESET
    screen_containers[3] = lv_obj_create(content_area); // FOOTSWITCH
    screen_containers[4] = lv_obj_create(content_area); // SYSTEM
    screen_containers[5] = lv_obj_create(content_area); // EFFECT_EDIT
    screen_containers[6] = lv_obj_create(content_area); // SETTINGS
    
    for (int i = 0; i < 7; i++) {
        lv_obj_set_size(screen_containers[i], LV_PCT(100), LV_PCT(100));
        lv_obj_set_style_bg_color(screen_containers[i], COLOR_BG, 0);
        lv_obj_set_style_pad_all(screen_containers[i], 0, 0);
        lv_obj_set_style_border_width(screen_containers[i], 0, 0);
        if (i != 0) {
            lv_obj_add_flag(screen_containers[i], LV_OBJ_FLAG_HIDDEN);
        }
    }
    
    // Initialize all screens
    performance_screen_init(screen_containers[0]);
    fx_chain_screen_init(screen_containers[1]);
    presets_screen_init(screen_containers[2]);
    footswitch_screen_init(screen_containers[3]);
    system_screen_init(screen_containers[4]);
    effect_edit_screen_init(screen_containers[5]);
    settings_screen_init(screen_containers[6]);
    
    // Create navigation bar (sibling to content_area on root)
    create_nav_bar(lv_scr_act());
    
    // Set initial state and hydrate UI
    app_state.currentScreen = UiScreenId::PERFORMANCE;
    app_state.linkUp = false;
    strncpy(app_state.presetName, "P--  CONNECTING", sizeof(app_state.presetName) - 1);
    
    // Hydrate UI with current state
    performance_update_preset(app_state.presetName);
    performance_update_link(app_state.linkUp);
    for (int i = 0; i < 4; i++) {
        ui_update_effect_state(i, effect_enabled_from_state(i));
    }
    performance_update_meters(app_state.inputPeakDb, app_state.outputPeakDb);
    const char* noteName = note_name_from_midi(app_state.detectedNote);
    performance_update_pitch(app_state.pitchFreqHz, noteName, app_state.voiced);
}

void ui_app_run(void) {
    lv_timer_handler();
    delay(VoxCydConfig::LvglTickMs);
}

void ui_navigate_to(UiScreenId screen) {
    if (current_screen == screen) return;
    
    current_screen = screen;
    
    // Show the selected screen
    show_screen(screen);
    
    // Update navigation bar visual
    update_nav_bar(screen);
}

UiScreenId ui_get_current_screen(void) {
    return current_screen;
}

void ui_update_link_state(bool connected) {
    app_state.linkUp = connected;
    performance_update_link(connected);
}

void ui_update_preset(uint16_t id, const char* name) {
    app_state.presetId = id;
    char buf[40];
    snprintf(buf, sizeof(buf), "P%02u  %s", (unsigned)id, name ? name : "");
    for (char* p = buf; *p; ++p) *p = (char)toupper((unsigned char)*p);
    strncpy(app_state.presetName, buf, sizeof(app_state.presetName) - 1);
    app_state.presetName[sizeof(app_state.presetName) - 1] = '\0';
    performance_update_preset(app_state.presetName);
}

void ui_update_effect_state(int effectId, bool enabled) {
    switch (effectId) {
        case 0: app_state.harmonyEnabled = enabled; break;
        case 1: app_state.reverbEnabled = enabled; break;
        case 2: app_state.delayEnabled = enabled; break;
        case 3: app_state.limiterEnabled = enabled; break;
        default: return;
    }
    // Fan out one logical state change to every representation so the UI can
    // never show conflicting effect states across screens.
    performance_update_effect(effectId, enabled);
    fx_chain_update_effect_state(effectId, enabled,
                                 ui_effect_main_value(effectId),
                                 ui_effect_metadata(effectId));
    effect_edit_set_enabled(effectId, enabled);
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
