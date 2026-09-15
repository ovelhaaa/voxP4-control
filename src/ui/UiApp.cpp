#include "UiApp.h"
#include "UiTheme.h"
#include "screens/PerformanceScreen.h"
#include <lvgl.h>

static lv_obj_t* main_screen = nullptr;
static UiScreenId current_screen = UiScreenId::PERFORMANCE;
static UiAppState app_state = {0};

// LVGL display driver
static lv_disp_drv_t disp_drv;
static lv_color_t buf1[VoxCydConfig::LvglBufferSize / 2];
static lv_color_t buf2[VoxCydConfig::LvglBufferSize / 2];

// Display flush callback
static void disp_flush(lv_disp_drv_t* disp, const lv_area_t* area, lv_color_t* color_p) {
    // Will be implemented with LovyanGFX integration
    lgfx::LGFX_Device* lcd = nullptr;  // Will be set during init
    if (lcd) {
        lcd->startWrite();
        lcd->setAddrWindow(area->x1, area->y1, area->x2 - area->x1 + 1, area->y2 - area->y1 + 1);
        lcd->writePixels((lgfx::rgb565_t*)color_p, lv_area_get_width(area) * lv_area_get_height(area), false, false);
        lcd->endWrite();
    }
    lv_disp_flush_ready(disp);
}

void ui_app_init(void) {
    lv_init();
    
    // Initialize theme styles
    ui_theme_init();
    
    // Initialize LVGL display buffer
    lv_disp_draw_buf_init(&disp_drv.draw_buf, buf1, buf2, VoxCydConfig::ScreenWidth, VoxCydConfig::LvglBufferLines);
    
    // Initialize display driver
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = VoxCydConfig::ScreenWidth;
    disp_drv.ver_res = VoxCydConfig::ScreenHeight;
    disp_drv.flush_cb = disp_flush;
    disp_drv.draw_buf = &disp_drv.draw_buf;
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
    
    // Set initial state
    app_state.currentScreen = UiScreenId::PERFORMANCE;
    app_state.linkUp = false;
    strncpy(app_state.presetName, "Connecting...", sizeof(app_state.presetName) - 1);
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
    // Update UI indicator
}

void ui_update_preset(uint16_t id, const char* name) {
    app_state.presetId = id;
    strncpy(app_state.presetName, name, sizeof(app_state.presetName) - 1);
    // Update preset display
}

void ui_update_effect_state(int effectId, bool enabled) {
    switch (effectId) {
        case 0: app_state.harmonyEnabled = enabled; break;
        case 1: app_state.reverbEnabled = enabled; break;
        case 2: app_state.limiterEnabled = enabled; break;
        case 3: app_state.delayEnabled = enabled; break;
    }
    // Update effect button states
}

void ui_update_meters(float inputDb, float outputDb) {
    app_state.inputPeakDb = inputDb;
    app_state.outputPeakDb = outputDb;
    // Update meter widgets
}

void ui_update_pitch(float freqHz, int note, bool voiced) {
    app_state.pitchFreqHz = freqHz;
    app_state.detectedNote = note;
    app_state.voiced = voiced;
    // Update pitch display
}
