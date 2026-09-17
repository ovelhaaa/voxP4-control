#include "SystemScreen.h"
#include "../UiTheme.h"
#include <cstdio>

static lv_obj_t* sys_container = nullptr;
static lv_obj_t* p4_fw_label = nullptr;
static lv_obj_t* cyd_fw_label = nullptr;
static lv_obj_t* sample_rate_label = nullptr;
static lv_obj_t* block_size_label = nullptr;
static lv_obj_t* cpu_load_label = nullptr;
static lv_obj_t* underruns_label = nullptr;
static lv_obj_t* uart_errors_label = nullptr;
static lv_obj_t* crc_errors_label = nullptr;
static lv_obj_t* reconnect_label = nullptr;
static lv_obj_t* heap_free_label = nullptr;
static lv_obj_t* uptime_label = nullptr;

void system_screen_init(lv_obj_t* parent) {
    // Create main container with padding
    sys_container = lv_obj_create(parent);
    lv_obj_set_size(sys_container, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(sys_container, COLOR_BG_DARK, 0);
    lv_obj_set_style_pad_all(sys_container, SPACING_M, 0);
    lv_obj_set_style_border_width(sys_container, 0, 0);
    lv_obj_set_flex_flow(sys_container, LV_FLEX_FLOW_COLUMN);
    
    // === HEADER ===
    lv_obj_t* header = lv_obj_create(sys_container);
    lv_obj_set_size(header, LV_PCT(100), 28);
    lv_obj_set_style_bg_color(header, COLOR_BG_HEADER, 0);
    lv_obj_set_style_radius(header, 0, 0);
    lv_obj_set_style_border_width(header, 0, 0);
    lv_obj_set_style_pad_hor(header, SPACING_M, 0);
    lv_obj_set_flex_flow(header, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(header, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    
    lv_obj_t* title = lv_label_create(header);
    lv_label_set_text(title, "SYSTEM INFO");
    lv_obj_set_style_text_color(title, COLOR_TEXT_PRIMARY, 0);
    lv_obj_set_style_text_font(title, FONT_EMPHASIS, 0);
    
    // === VOXLINK SECTION ===
    lv_obj_t* link_section = lv_obj_create(sys_container);
    lv_obj_set_size(link_section, LV_PCT(100), 52);
    lv_obj_set_style_bg_color(link_section, COLOR_BG_SURFACE, 0);
    lv_obj_set_style_radius(link_section, RADIUS_M, 0);
    lv_obj_set_style_border_width(link_section, 0, 0);
    lv_obj_set_style_pad_all(link_section, SPACING_M, 0);
    lv_obj_set_flex_flow(link_section, LV_FLEX_FLOW_COLUMN);
    
    lv_obj_t* link_title = lv_label_create(link_section);
    lv_label_set_text(link_title, "VoxLink Status");
    lv_obj_set_style_text_color(link_title, COLOR_ACCENT_PRIMARY, 0);
    lv_obj_set_style_text_font(link_title, FONT_BODY, 0);
    
    // P4 Firmware
    lv_obj_t* p4_row = lv_obj_create(link_section);
    lv_obj_set_size(p4_row, LV_PCT(100), 20);
    lv_obj_set_style_bg_color(p4_row, COLOR_BG_SURFACE, 0);
    lv_obj_set_style_border_width(p4_row, 0, 0);
    lv_obj_set_flex_flow(p4_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(p4_row, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    
    lv_obj_t* p4_label = lv_label_create(p4_row);
    lv_label_set_text(p4_label, "P4 Firmware:");
    lv_obj_set_style_text_color(p4_label, COLOR_TEXT_SECONDARY, 0);
    lv_obj_set_style_text_font(p4_label, FONT_SMALL, 0);
    
    p4_fw_label = lv_label_create(p4_row);
    lv_label_set_text(p4_fw_label, "?.?.?");
    lv_obj_set_style_text_color(p4_fw_label, COLOR_TEXT_PRIMARY, 0);
    lv_obj_set_style_text_font(p4_fw_label, FONT_SMALL, 0);
    
    // CYD Firmware
    lv_obj_t* cyd_row = lv_obj_create(link_section);
    lv_obj_set_size(cyd_row, LV_PCT(100), 20);
    lv_obj_set_style_bg_color(cyd_row, COLOR_BG_SURFACE, 0);
    lv_obj_set_style_border_width(cyd_row, 0, 0);
    lv_obj_set_flex_flow(cyd_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(cyd_row, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    
    lv_obj_t* cyd_label = lv_label_create(cyd_row);
    lv_label_set_text(cyd_label, "CYD Firmware:");
    lv_obj_set_style_text_color(cyd_label, COLOR_TEXT_SECONDARY, 0);
    lv_obj_set_style_text_font(cyd_label, FONT_SMALL, 0);
    
    cyd_fw_label = lv_label_create(cyd_row);
    lv_label_set_text(cyd_fw_label, "?.?.?");
    lv_obj_set_style_text_color(cyd_fw_label, COLOR_TEXT_PRIMARY, 0);
    lv_obj_set_style_text_font(cyd_fw_label, FONT_SMALL, 0);
    
    // === AUDIO SECTION ===
    lv_obj_t* audio_section = lv_obj_create(sys_container);
    lv_obj_set_size(audio_section, LV_PCT(100), 52);
    lv_obj_set_style_bg_color(audio_section, COLOR_BG_SURFACE, 0);
    lv_obj_set_style_radius(audio_section, RADIUS_M, 0);
    lv_obj_set_style_border_width(audio_section, 0, 0);
    lv_obj_set_style_pad_all(audio_section, SPACING_M, 0);
    lv_obj_set_flex_flow(audio_section, LV_FLEX_FLOW_COLUMN);
    
    lv_obj_t* audio_title = lv_label_create(audio_section);
    lv_label_set_text(audio_title, "Audio");
    lv_obj_set_style_text_color(audio_title, COLOR_ACCENT_PRIMARY, 0);
    lv_obj_set_style_text_font(audio_title, FONT_BODY, 0);
    
    // Sample Rate
    lv_obj_t* sr_row = lv_obj_create(audio_section);
    lv_obj_set_size(sr_row, LV_PCT(100), 20);
    lv_obj_set_style_bg_color(sr_row, COLOR_BG_SURFACE, 0);
    lv_obj_set_style_border_width(sr_row, 0, 0);
    lv_obj_set_flex_flow(sr_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(sr_row, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    
    lv_obj_t* sr_label = lv_label_create(sr_row);
    lv_label_set_text(sr_label, "Sample Rate:");
    lv_obj_set_style_text_color(sr_label, COLOR_TEXT_SECONDARY, 0);
    lv_obj_set_style_text_font(sr_label, FONT_SMALL, 0);
    
    sample_rate_label = lv_label_create(sr_row);
    lv_label_set_text(sample_rate_label, "48000 Hz");
    lv_obj_set_style_text_color(sample_rate_label, COLOR_TEXT_PRIMARY, 0);
    lv_obj_set_style_text_font(sample_rate_label, FONT_SMALL, 0);
    
    // Block Size
    lv_obj_t* bs_row = lv_obj_create(audio_section);
    lv_obj_set_size(bs_row, LV_PCT(100), 20);
    lv_obj_set_style_bg_color(bs_row, COLOR_BG_SURFACE, 0);
    lv_obj_set_style_border_width(bs_row, 0, 0);
    lv_obj_set_flex_flow(bs_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(bs_row, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    
    lv_obj_t* bs_label = lv_label_create(bs_row);
    lv_label_set_text(bs_label, "Block Size:");
    lv_obj_set_style_text_color(bs_label, COLOR_TEXT_SECONDARY, 0);
    lv_obj_set_style_text_font(bs_label, FONT_SMALL, 0);
    
    block_size_label = lv_label_create(bs_row);
    lv_label_set_text(block_size_label, "64");
    lv_obj_set_style_text_color(block_size_label, COLOR_TEXT_PRIMARY, 0);
    lv_obj_set_style_text_font(block_size_label, FONT_SMALL, 0);
    
    // === PERFORMANCE SECTION ===
    lv_obj_t* perf_section = lv_obj_create(sys_container);
    lv_obj_set_size(perf_section, LV_PCT(100), 52);
    lv_obj_set_style_bg_color(perf_section, COLOR_BG_SURFACE, 0);
    lv_obj_set_style_radius(perf_section, RADIUS_M, 0);
    lv_obj_set_style_border_width(perf_section, 0, 0);
    lv_obj_set_style_pad_all(perf_section, SPACING_M, 0);
    lv_obj_set_flex_flow(perf_section, LV_FLEX_FLOW_COLUMN);
    
    lv_obj_t* perf_title = lv_label_create(perf_section);
    lv_label_set_text(perf_title, "Performance");
    lv_obj_set_style_text_color(perf_title, COLOR_ACCENT_PRIMARY, 0);
    lv_obj_set_style_text_font(perf_title, FONT_BODY, 0);
    
    // CPU Load
    lv_obj_t* cpu_row = lv_obj_create(perf_section);
    lv_obj_set_size(cpu_row, LV_PCT(100), 20);
    lv_obj_set_style_bg_color(cpu_row, COLOR_BG_SURFACE, 0);
    lv_obj_set_style_border_width(cpu_row, 0, 0);
    lv_obj_set_flex_flow(cpu_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(cpu_row, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    
    lv_obj_t* cpu_label = lv_label_create(cpu_row);
    lv_label_set_text(cpu_label, "CPU Load:");
    lv_obj_set_style_text_color(cpu_label, COLOR_TEXT_SECONDARY, 0);
    lv_obj_set_style_text_font(cpu_label, FONT_SMALL, 0);
    
    cpu_load_label = lv_label_create(cpu_row);
    lv_label_set_text(cpu_load_label, "0.0%");
    lv_obj_set_style_text_color(cpu_load_label, COLOR_TEXT_PRIMARY, 0);
    lv_obj_set_style_text_font(cpu_load_label, FONT_SMALL, 0);
    
    // Underruns
    lv_obj_t* und_row = lv_obj_create(perf_section);
    lv_obj_set_size(und_row, LV_PCT(100), 20);
    lv_obj_set_style_bg_color(und_row, COLOR_BG_SURFACE, 0);
    lv_obj_set_style_border_width(und_row, 0, 0);
    lv_obj_set_flex_flow(und_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(und_row, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    
    lv_obj_t* und_label = lv_label_create(und_row);
    lv_label_set_text(und_label, "Underruns:");
    lv_obj_set_style_text_color(und_label, COLOR_TEXT_SECONDARY, 0);
    lv_obj_set_style_text_font(und_label, FONT_SMALL, 0);
    
    underruns_label = lv_label_create(und_row);
    lv_label_set_text(underruns_label, "0");
    lv_obj_set_style_text_color(underruns_label, COLOR_TEXT_PRIMARY, 0);
    lv_obj_set_style_text_font(underruns_label, FONT_SMALL, 0);
    
    // === ERRORS SECTION ===
    lv_obj_t* err_section = lv_obj_create(sys_container);
    lv_obj_set_size(err_section, LV_PCT(100), 52);
    lv_obj_set_style_bg_color(err_section, COLOR_BG_SURFACE, 0);
    lv_obj_set_style_radius(err_section, RADIUS_M, 0);
    lv_obj_set_style_border_width(err_section, 0, 0);
    lv_obj_set_style_pad_all(err_section, SPACING_M, 0);
    lv_obj_set_flex_flow(err_section, LV_FLEX_FLOW_COLUMN);
    
    lv_obj_t* err_title = lv_label_create(err_section);
    lv_label_set_text(err_title, "Errors");
    lv_obj_set_style_text_color(err_title, COLOR_ACCENT_PRIMARY, 0);
    lv_obj_set_style_text_font(err_title, FONT_BODY, 0);
    
    // UART Errors
    lv_obj_t* uart_row = lv_obj_create(err_section);
    lv_obj_set_size(uart_row, LV_PCT(100), 20);
    lv_obj_set_style_bg_color(uart_row, COLOR_BG_SURFACE, 0);
    lv_obj_set_style_border_width(uart_row, 0, 0);
    lv_obj_set_flex_flow(uart_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(uart_row, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    
    lv_obj_t* uart_label = lv_label_create(uart_row);
    lv_label_set_text(uart_label, "UART Errors:");
    lv_obj_set_style_text_color(uart_label, COLOR_TEXT_SECONDARY, 0);
    lv_obj_set_style_text_font(uart_label, FONT_SMALL, 0);
    
    uart_errors_label = lv_label_create(uart_row);
    lv_label_set_text(uart_errors_label, "0");
    lv_obj_set_style_text_color(uart_errors_label, COLOR_TEXT_PRIMARY, 0);
    lv_obj_set_style_text_font(uart_errors_label, FONT_SMALL, 0);
    
    // CRC Errors
    lv_obj_t* crc_row = lv_obj_create(err_section);
    lv_obj_set_size(crc_row, LV_PCT(100), 20);
    lv_obj_set_style_bg_color(crc_row, COLOR_BG_SURFACE, 0);
    lv_obj_set_style_border_width(crc_row, 0, 0);
    lv_obj_set_flex_flow(crc_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(crc_row, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    
    lv_obj_t* crc_label = lv_label_create(crc_row);
    lv_label_set_text(crc_label, "CRC Errors:");
    lv_obj_set_style_text_color(crc_label, COLOR_TEXT_SECONDARY, 0);
    lv_obj_set_style_text_font(crc_label, FONT_SMALL, 0);
    
    crc_errors_label = lv_label_create(crc_row);
    lv_label_set_text(crc_errors_label, "0");
    lv_obj_set_style_text_color(crc_errors_label, COLOR_TEXT_PRIMARY, 0);
    lv_obj_set_style_text_font(crc_errors_label, FONT_SMALL, 0);
    
    // Reconnect Count
    lv_obj_t* rec_row = lv_obj_create(err_section);
    lv_obj_set_size(rec_row, LV_PCT(100), 20);
    lv_obj_set_style_bg_color(rec_row, COLOR_BG_SURFACE, 0);
    lv_obj_set_style_border_width(rec_row, 0, 0);
    lv_obj_set_flex_flow(rec_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(rec_row, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    
    lv_obj_t* rec_label = lv_label_create(rec_row);
    lv_label_set_text(rec_label, "Reconnects:");
    lv_obj_set_style_text_color(rec_label, COLOR_TEXT_SECONDARY, 0);
    lv_obj_set_style_text_font(rec_label, FONT_SMALL, 0);
    
    reconnect_label = lv_label_create(rec_row);
    lv_label_set_text(reconnect_label, "0");
    lv_obj_set_style_text_color(reconnect_label, COLOR_TEXT_PRIMARY, 0);
    lv_obj_set_style_text_font(reconnect_label, FONT_SMALL, 0);
    
    // === MEMORY & UPTIME ===
    lv_obj_t* mem_section = lv_obj_create(sys_container);
    lv_obj_set_size(mem_section, LV_PCT(100), 52);
    lv_obj_set_style_bg_color(mem_section, COLOR_BG_SURFACE, 0);
    lv_obj_set_style_radius(mem_section, RADIUS_M, 0);
    lv_obj_set_style_border_width(mem_section, 0, 0);
    lv_obj_set_style_pad_all(mem_section, SPACING_M, 0);
    lv_obj_set_flex_flow(mem_section, LV_FLEX_FLOW_COLUMN);
    
    lv_obj_t* mem_title = lv_label_create(mem_section);
    lv_label_set_text(mem_title, "Memory / Uptime");
    lv_obj_set_style_text_color(mem_title, COLOR_ACCENT_PRIMARY, 0);
    lv_obj_set_style_text_font(mem_title, FONT_BODY, 0);
    
    // Heap Free
    lv_obj_t* heap_row = lv_obj_create(mem_section);
    lv_obj_set_size(heap_row, LV_PCT(100), 20);
    lv_obj_set_style_bg_color(heap_row, COLOR_BG_SURFACE, 0);
    lv_obj_set_style_border_width(heap_row, 0, 0);
    lv_obj_set_flex_flow(heap_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(heap_row, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    
    lv_obj_t* heap_label = lv_label_create(heap_row);
    lv_label_set_text(heap_label, "Heap Free:");
    lv_obj_set_style_text_color(heap_label, COLOR_TEXT_SECONDARY, 0);
    lv_obj_set_style_text_font(heap_label, FONT_SMALL, 0);
    
    heap_free_label = lv_label_create(heap_row);
    lv_label_set_text(heap_free_label, "0 KB");
    lv_obj_set_style_text_color(heap_free_label, COLOR_TEXT_PRIMARY, 0);
    lv_obj_set_style_text_font(heap_free_label, FONT_SMALL, 0);
    
    // Uptime
    lv_obj_t* up_row = lv_obj_create(mem_section);
    lv_obj_set_size(up_row, LV_PCT(100), 20);
    lv_obj_set_style_bg_color(up_row, COLOR_BG_SURFACE, 0);
    lv_obj_set_style_border_width(up_row, 0, 0);
    lv_obj_set_flex_flow(up_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(up_row, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    
    lv_obj_t* up_label = lv_label_create(up_row);
    lv_label_set_text(up_label, "Uptime:");
    lv_obj_set_style_text_color(up_label, COLOR_TEXT_SECONDARY, 0);
    lv_obj_set_style_text_font(up_label, FONT_SMALL, 0);
    
    uptime_label = lv_label_create(up_row);
    lv_label_set_text(uptime_label, "0s");
    lv_obj_set_style_text_color(uptime_label, COLOR_TEXT_PRIMARY, 0);
    lv_obj_set_style_text_font(uptime_label, FONT_SMALL, 0);
}

void system_screen_update_info(const char* p4Firmware, const char* cydFirmware, 
                                int sampleRate, int blockSize, float cpuLoad,
                                int underruns, int uartErrors, int crcErrors,
                                int reconnectCount, uint32_t heapFree, uint32_t uptimeSec) {
    char buf[32];
    
    if (p4_fw_label && p4Firmware) {
        lv_label_set_text(p4_fw_label, p4Firmware);
    }
    
    if (cyd_fw_label && cydFirmware) {
        lv_label_set_text(cyd_fw_label, cydFirmware);
    }
    
    if (sample_rate_label) {
        snprintf(buf, sizeof(buf), "%d Hz", sampleRate);
        lv_label_set_text(sample_rate_label, buf);
    }
    
    if (block_size_label) {
        snprintf(buf, sizeof(buf), "%d", blockSize);
        lv_label_set_text(block_size_label, buf);
    }
    
    if (cpu_load_label) {
        snprintf(buf, sizeof(buf), "%.1f%%", cpuLoad);
        lv_label_set_text(cpu_load_label, buf);
    }
    
    if (underruns_label) {
        snprintf(buf, sizeof(buf), "%d", underruns);
        lv_label_set_text(underruns_label, buf);
    }
    
    if (uart_errors_label) {
        snprintf(buf, sizeof(buf), "%d", uartErrors);
        lv_label_set_text(uart_errors_label, buf);
    }
    
    if (crc_errors_label) {
        snprintf(buf, sizeof(buf), "%d", crcErrors);
        lv_label_set_text(crc_errors_label, buf);
    }
    
    if (reconnect_label) {
        snprintf(buf, sizeof(buf), "%d", reconnectCount);
        lv_label_set_text(reconnect_label, buf);
    }
    
    if (heap_free_label) {
        snprintf(buf, sizeof(buf), "%lu KB", (unsigned long)(heapFree / 1024));
        lv_label_set_text(heap_free_label, buf);
    }
    
    if (uptime_label) {
        if (uptimeSec < 60) {
            snprintf(buf, sizeof(buf), "%lus", uptimeSec);
        } else if (uptimeSec < 3600) {
            snprintf(buf, sizeof(buf), "%lum %lus", uptimeSec / 60, uptimeSec % 60);
        } else {
            snprintf(buf, sizeof(buf), "%luh %lum", uptimeSec / 3600, (uptimeSec % 3600) / 60);
        }
        lv_label_set_text(uptime_label, buf);
    }
}
