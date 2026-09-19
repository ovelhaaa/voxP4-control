/**
 * VoxP4 Control Surface - Main Entry Point
 * 
 * Firmware para ESP32 Cheap Yellow Display (CYD)
 * Interface de controle para o VoxP4 vocal effects processor
 * 
 * Hardware: ESP32-2432S028R (2-USB variant)
 * Display: ST7789 320x240 landscape
 * Touch: XPT2046 resistivo
 * Footswitches: 2 entradas GPIO
 * Comunicação: UART VoxLink com ESP32-P4
 */

#include <Arduino.h>
#include <LovyanGFX.hpp>
#include <lvgl.h>

#include "board/CYD_Config.h"
#include "board/LGFX_CYD.h"
#include "ui/UiApp.h"
#include "ui/UiTheme.h"
#include "control/FootswitchManager.h"

// Instância global do display
LGFX_CYD lcd;

// Flag para indicar inicialização completa
static bool system_initialized = false;

/**
 * Inicialização do hardware
 */
void setup() {
    // Serial para debug
    Serial.begin(115200);
    while (!Serial && (millis() < 2000)) {
        delay(10);
    }
    
    Serial.println();
    Serial.println("╔════════════════════════════════════╗");
    Serial.println("║     VoxP4 Control Surface          ║");
    Serial.println("║     ESP32 CYD Firmware             ║");
    Serial.println("╚════════════════════════════════════╝");
    Serial.printf("Hardware: ESP32-2432S028R (2-USB)\n");
    Serial.printf("Display: ST7789 %dx%d\n", VoxCydConfig::ScreenWidth, VoxCydConfig::ScreenHeight);
    Serial.printf("Touch: XPT2046\n");
    Serial.printf("UART VoxLink: TX=%d RX=%d @ %d baud\n", 
                  VoxCydConfig::VoxUartTx, 
                  VoxCydConfig::VoxUartRx, 
                  VoxCydConfig::VoxUartBaud);
    
    // Inicializar display
    Serial.println("[INIT] Initializing display...");
    if (!lcd.init()) {
        Serial.println("[ERROR] Display initialization failed!");
        while (true) {
            delay(1000);
        }
    }
    
    lcd.setRotation(VoxCydConfig::ScreenRotation);

    // Themed boot splash: short, quiet, then replaced by the main UI.
    lcd.fillScreen((uint16_t)lv_color_to16(COLOR_BG));
    lcd.setTextDatum(lgfx::middle_center);
    lcd.setTextColor((uint16_t)lv_color_to16(COLOR_TEXT_PRIMARY));
    lcd.setTextSize(3);
    lcd.drawString("VOXP4", 160, 92);
    lcd.setTextColor((uint16_t)lv_color_to16(COLOR_ACCENT));
    lcd.setTextSize(2);
    lcd.drawString("CONTROL", 160, 128);
    lcd.fillRect(122, 150, 48, 3, (uint16_t)lv_color_to16(COLOR_ACCENT));
    lcd.fillRect(172, 150, 26, 3, (uint16_t)lv_color_to16(COLOR_AUDIO));
    lcd.setTextColor((uint16_t)lv_color_to16(COLOR_TEXT_MUTED));
    lcd.setTextSize(1);
    lcd.drawString("initializing...", 160, 176);
    
    // Inicializar footswitches
    Serial.println("[INIT] Initializing footswitches...");
    footswitch_init();
    footswitch_set_event_callback([](const FootswitchEvent* ev) {
        // Forward physical events to the App layer. main.cpp must not know
        // about screen functions or label formatting.
        switch (ev->type) {
            case FS_EVENT_PRESS:
                ui_update_footswitch_state(ev->index, true);
                break;
            case FS_EVENT_RELEASE:
                ui_update_footswitch_state(ev->index, false);
                break;
            default:
                break;
        }
    });
    
    // Inicializar UI (LVGL)
    Serial.println("[INIT] Initializing LVGL UI...");
    ui_app_init(lcd);
    
    Serial.println("[INIT] System ready!");
    
    // Print memory info
    Serial.printf("[MEM] Free heap: %u bytes\n", esp_get_free_heap_size());
    Serial.printf("[MEM] Min free heap: %u bytes\n", esp_get_minimum_free_heap_size());

    system_initialized = true;
}

/**
 * Loop principal
 * 
 * Responsabilidades:
 * - Processar eventos LVGL (UI)
 * - Polling dos footswitches
 * - Comunicação UART com VoxP4 (futuro)
 */
void loop() {
    // Processar eventos da UI (LVGL)
    ui_app_run();
    
    // Processar footswitches
    footswitch_poll();
    
    // Futuro: Processar comunicação UART
    // voxlink_process();
    
    // Pequeno delay para evitar busy-wait excessivo
    // O timing é gerenciado principalmente pelo LVGL e footswitch polling
    delay(1);
}

/**
 * Callbacks e handlers serão implementados nos módulos específicos:
 * 
 * - ui/UiApp.cpp: Eventos de touch e navegação
 * - control/FootswitchManager.cpp: Eventos dos footswitches
 * - protocol/VoxLink.cpp: Mensagens UART
 * - transport/VoxUart.cpp: Driver UART
 */
