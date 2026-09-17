#include "UiApp.h"
#include "UiTheme.h"
#include "screens/PerformanceScreen.h"
#include "screens/FxChainScreen.h"
#include "screens/PresetsScreen.h"
#include "screens/FootswitchScreen.h"
#include "screens/SystemScreen.h"
#include "board/LGFX_CYD.h"
#include <lvgl.h>

using namespace VoxUiTheme;

// Helper to convert MIDI note number to note name string
static const char* note_name_from_midi(int note) {
    static const char* notes[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
    if (note < 0 || note > 127) return "?";
    return notes[note % 12];
}

// Shell components
static lv_obj_t* content_area = nullptr;
static lv_obj_t* nav_bar = nullptr;
static lv_obj_t* nav_tabs[4] = {nullptr, nullptr, nullptr, nullptr};
static const char* nav_labels[] = {"PERF", "FX", "PRESET", "SET"};

// Screen containers (4 main tabs)
static lv_obj_t* screen_containers[5] = {nullptr, nullptr, nullptr, nullptr, nullptr}; // 0-3: main tabs, 4: SYSTEM
static UiScreenId current_screen = UiScreenId::PERFORMANCE;
static UiAppState app_state = {};

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
        lcd_device->writePixels((lgfx::rgb565_t*)color_p, lv_area_get_width(area) * lv_area_get_height(area), false);
        lcd_device->endWrite();
    }
    lv_disp_flush_ready(disp);
}

// Navigation tab click callback
static void nav_tab_clicked(lv_event_t* e) {
    lv_obj_t* tab = lv_event_get_target(e);
    int index = (int)(intptr_t)lv_obj_get_user_data(tab);
    
    if (index >= 0 && index < 4) {
        ui_navigate_to(static_cast<UiScreenId>(index));
    }
}

// Create navigation bar at bottom
static void create_nav_bar(lv_obj_t* parent) {
    nav_bar = lv_obj_create(parent);
    lv_obj_set_size(nav_bar, LV_PCT(100), 36);
    lv_obj_set_style_bg_color(nav_bar, COLOR_BG_NAV, 0);
    lv_obj_set_style_radius(nav_bar, 0, 0);
    lv_obj_set_style_border_width(nav_bar, 0, 0);
    lv_obj_set_style_pad_all(nav_bar, 0, 0);
    lv_obj_align(nav_bar, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_flex_flow(nav_bar, LV_FLEX_FLOW_ROW);
    
    for (int i = 0; i < 4; i++) {
        nav_tabs[i] = lv_btn_create(nav_bar);
        lv_obj_set_size(nav_tabs[i], LV_PCT(25), LV_PCT(100));
        lv_obj_set_style_bg_color(nav_tabs[i], COLOR_BG_NAV, 0);
        lv_obj_set_style_radius(nav_tabs[i], 0, 0);
        lv_obj_set_style_border_width(nav_tabs[i], 0, 0);
        lv_obj_set_flex_grow(nav_tabs[i], 1);
        lv_obj_set_user_data(nav_tabs[i], (void*)(intptr_t)i);
        lv_obj_add_event_cb(nav_tabs[i], nav_tab_clicked, LV_EVENT_CLICKED, NULL);
        
        lv_obj_t* label = lv_label_create(nav_tabs[i]);
        lv_label_set_text(label, nav_labels[i]);
        lv_obj_set_style_text_color(label, COLOR_TEXT_SECONDARY, 0);
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
    } else if (active_screen == UiScreenId::SYSTEM) {
        active_idx = 3; // FOOTSWITCH/SET tab
    }
    
    for (int i = 0; i < 4; i++) {
        if (!nav_tabs[i]) continue;
        
        bool is_active = (i == active_idx);
        lv_obj_set_style_bg_color(nav_tabs[i], is_active ? COLOR_BG_SURFACE : COLOR_BG_NAV, 0);
        lv_obj_set_style_border_side(nav_tabs[i], is_active ? LV_BORDER_SIDE_TOP : LV_BORDER_SIDE_NONE, 0);
        lv_obj_set_style_border_width(nav_tabs[i], is_active ? 3 : 0, 0);
        lv_obj_set_style_border_color(nav_tabs[i], COLOR_ACCENT_PRIMARY, 0);
        
        lv_obj_t* label = lv_obj_get_child(nav_tabs[i], 0);
        if (label) {
            lv_obj_set_style_text_color(label, is_active ? COLOR_ACCENT_PRIMARY : COLOR_TEXT_SECONDARY, 0);
        }
    }
}

// Show/hide screen containers using LVGL 8.x API
static void show_screen(UiScreenId screen_id) {
    int idx = (int)screen_id;
    
    // Handle subscreens by showing their parent tab
    if (screen_id == UiScreenId::EFFECT_EDIT) {
        idx = 1; // FX_CHAIN tab
    }
    
    for (int i = 0; i < 5; i++) {
        if (screen_containers[i]) {
            if (i == idx) {
                lv_obj_clear_flag(screen_containers[i], LV_OBJ_FLAG_HIDDEN);
            } else {
                lv_obj_add_flag(screen_containers[i], LV_OBJ_FLAG_HIDDEN);
            }
        }
    }
}

void ui_app_init(void) {
    lv_init();
    
    // Initialize theme styles
    ui_theme_init();
    
    // Initialize LovyanGFX display device
    lcd_device = new LGFX_CYD();
    lcd_device->init();
    lcd_device->setRotation(VoxCydConfig::ScreenRotation);
    lcd_device->fillScreen(TFT_BLACK);
    
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
    
    // Create content area (top portion, leaving space for nav bar)
    content_area = lv_obj_create(lv_scr_act());
    lv_obj_set_size(content_area, LV_PCT(100), 204); // 240 - 36 = 204
    lv_obj_set_style_bg_color(content_area, COLOR_BG_DARK, 0);
    lv_obj_set_style_pad_all(content_area, 0, 0);
    lv_obj_set_style_border_width(content_area, 0, 0);
    lv_obj_align(content_area, LV_ALIGN_TOP_MID, 0, 0);
    
    // Create screen containers (5 total: 4 main tabs + SYSTEM)
    screen_containers[0] = lv_obj_create(content_area); // PERF
    screen_containers[1] = lv_obj_create(content_area); // FX
    screen_containers[2] = lv_obj_create(content_area); // PRESET
    screen_containers[3] = lv_obj_create(content_area); // FOOTSWITCH
    screen_containers[4] = lv_obj_create(content_area); // SYSTEM
    
    for (int i = 0; i < 5; i++) {
        lv_obj_set_size(screen_containers[i], LV_PCT(100), LV_PCT(100));
        lv_obj_set_style_bg_color(screen_containers[i], COLOR_BG_DARK, 0);
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
    
    // Create navigation bar
    create_nav_bar(content_area);
    
    // Set initial state and hydrate UI
    app_state.currentScreen = UiScreenId::PERFORMANCE;
    app_state.linkUp = false;
    strncpy(app_state.presetName, "Connecting...", sizeof(app_state.presetName) - 1);
    
    // Hydrate UI with current state
    performance_update_preset(app_state.presetName);
    performance_update_link(app_state.linkUp);
    performance_update_effect(0, app_state.harmonyEnabled);
    performance_update_effect(1, app_state.reverbEnabled);
    performance_update_effect(2, app_state.limiterEnabled);
    performance_update_effect(3, app_state.delayEnabled);
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
    strncpy(app_state.presetName, name, sizeof(app_state.presetName) - 1);
    performance_update_preset(name);
}

void ui_update_effect_state(int effectId, bool enabled) {
    switch (effectId) {
        case 0: app_state.harmonyEnabled = enabled; break;
        case 1: app_state.reverbEnabled = enabled; break;
        case 2: app_state.limiterEnabled = enabled; break;
        case 3: app_state.delayEnabled = enabled; break;
    }
    performance_update_effect(effectId, enabled);
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
