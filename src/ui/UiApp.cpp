#include "UiApp.h"
#include "UiTheme.h"
#include "screens/PerformanceScreen.h"
#include "board/LGFX_CYD.h"
#include <lvgl.h>

// Helper to convert MIDI note number to note name string
static const char* note_name_from_midi(int note) {
    static const char* notes[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
    if (note < 0 || note > 127) return "?";
    return notes[note % 12];
}

static lv_obj_t* main_screen = nullptr;
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
    
    // Create main screen container
    main_screen = lv_obj_create(lv_scr_act());
    lv_obj_set_size(main_screen, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(main_screen, lv_color_make(0x12, 0x12, 0x12), 0);
    lv_obj_set_style_pad_all(main_screen, 0, 0);
    lv_obj_set_style_border_width(main_screen, 0, 0);
    
    // Initialize performance screen (default)
    performance_screen_init(main_screen);
    
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
    // Screen switching logic will be implemented per screen module
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
