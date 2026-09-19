#include "SystemScreen.h"
#include "../UiTheme.h"
#include <cstdio>

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

static lv_obj_t* link_label = nullptr;
static lv_obj_t* baud_label = nullptr;
static lv_obj_t* rx_label = nullptr;
static lv_obj_t* tx_label = nullptr;

static lv_obj_t* add_section(lv_obj_t* parent, const char* title, lv_color_t color) {
    lv_obj_t* label = lv_label_create(parent);
    lv_label_set_text(label, title);
    lv_obj_set_style_text_color(label, color, 0);
    lv_obj_set_style_text_font(label, FONT_TINY, 0);
    lv_obj_set_style_pad_top(label, SPACING_XS, 0);
    return label;
}

static lv_obj_t* add_value_row(lv_obj_t* parent, const char* name) {
    lv_obj_t* row = lv_obj_create(parent);
    lv_obj_set_size(row, LV_PCT(100), 15);
    lv_obj_set_style_bg_opa(row, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(row, 0, 0);
    lv_obj_set_style_pad_all(row, 0, 0);
    lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(row, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t* label = lv_label_create(row);
    lv_label_set_text(label, name);
    lv_obj_set_style_text_color(label, COLOR_TEXT_SECONDARY, 0);
    lv_obj_set_style_text_font(label, FONT_TINY, 0);

    lv_obj_t* value = lv_label_create(row);
    lv_label_set_text(value, "--");
    lv_obj_set_style_text_color(value, COLOR_TEXT_PRIMARY, 0);
    lv_obj_set_style_text_font(value, FONT_TINY, 0);
    return value;
}

// Values are neutral by default; colour appears only when there is a reason.
static void set_value_alert(lv_obj_t* label, bool warning, bool error) {
    if (!label) return;
    lv_obj_set_style_text_color(label,
        error ? COLOR_ERROR : (warning ? COLOR_WARNING : COLOR_TEXT_PRIMARY), 0);
}

void system_screen_init(lv_obj_t* parent) {
    lv_obj_t* container = lv_obj_create(parent);
    lv_obj_set_size(container, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(container, COLOR_BG, 0);
    lv_obj_set_style_pad_all(container, SPACING_XS, 0);
    lv_obj_set_style_pad_row(container, SPACING_XS, 0);
    lv_obj_set_style_border_width(container, 0, 0);
    lv_obj_set_flex_flow(container, LV_FLEX_FLOW_COLUMN);

    // === HEADER ===
    lv_obj_t* header = lv_obj_create(container);
    lv_obj_set_size(header, LV_PCT(100), 24);
    lv_obj_set_style_bg_color(header, COLOR_HEADER, 0);
    lv_obj_set_style_radius(header, RADIUS_S, 0);
    lv_obj_set_style_border_width(header, 0, 0);
    lv_obj_set_style_pad_hor(header, SPACING_M, 0);
    lv_obj_set_style_pad_ver(header, 0, 0);
    lv_obj_set_flex_flow(header, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(header, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t* title = lv_label_create(header);
    lv_label_set_text(title, "SYSTEM");
    lv_obj_set_style_text_color(title, COLOR_TEXT_PRIMARY, 0);
    lv_obj_set_style_text_font(title, FONT_BODY, 0);

    // === SCROLLABLE BODY ===
    lv_obj_t* body = lv_obj_create(container);
    lv_obj_set_size(body, LV_PCT(100), LV_PCT(100));
    lv_obj_set_flex_grow(body, 1);
    lv_obj_set_style_bg_color(body, COLOR_BG, 0);
    lv_obj_set_style_border_width(body, 0, 0);
    lv_obj_set_style_pad_all(body, 0, 0);
    lv_obj_set_style_pad_row(body, 0, 0);
    lv_obj_set_flex_flow(body, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_scroll_dir(body, LV_DIR_VER);
    lv_obj_set_scrollbar_mode(body, LV_SCROLLBAR_MODE_AUTO);

    add_section(body, "VOXLINK", COLOR_AUDIO);
    link_label = add_value_row(body, "LINK");
    baud_label = add_value_row(body, "BAUD");
    rx_label = add_value_row(body, "RX");
    tx_label = add_value_row(body, "TX");
    p4_fw_label = add_value_row(body, "P4 FW");
    cyd_fw_label = add_value_row(body, "CYD FW");

    add_section(body, "AUDIO", COLOR_AUDIO);
    sample_rate_label = add_value_row(body, "RATE");
    block_size_label = add_value_row(body, "BLOCK");

    add_section(body, "PERFORMANCE", COLOR_TEXT_SECONDARY);
    cpu_load_label = add_value_row(body, "CPU");
    underruns_label = add_value_row(body, "UNDERRUNS");

    add_section(body, "COMM", COLOR_TEXT_SECONDARY);
    uart_errors_label = add_value_row(body, "UART ERR");
    crc_errors_label = add_value_row(body, "CRC ERR");
    reconnect_label = add_value_row(body, "RECONNECT");

    add_section(body, "SYSTEM", COLOR_TEXT_SECONDARY);
    heap_free_label = add_value_row(body, "HEAP");
    uptime_label = add_value_row(body, "UPTIME");
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
        if (sampleRate >= 1000 && (sampleRate % 1000) == 0) {
            snprintf(buf, sizeof(buf), "%d kHz", sampleRate / 1000);
        } else {
            snprintf(buf, sizeof(buf), "%d Hz", sampleRate);
        }
        lv_label_set_text(sample_rate_label, buf);
    }
    
    if (block_size_label) {
        snprintf(buf, sizeof(buf), "%d", blockSize);
        lv_label_set_text(block_size_label, buf);
    }
    
    if (cpu_load_label) {
        snprintf(buf, sizeof(buf), "%.0f %%", cpuLoad);
        lv_label_set_text(cpu_load_label, buf);
        set_value_alert(cpu_load_label, cpuLoad > 85.0f, cpuLoad > 95.0f);
    }
    
    if (underruns_label) {
        snprintf(buf, sizeof(buf), "%d", underruns);
        lv_label_set_text(underruns_label, buf);
        set_value_alert(underruns_label, underruns > 0, underruns >= 5);
    }
    
    if (uart_errors_label) {
        snprintf(buf, sizeof(buf), "%d", uartErrors);
        lv_label_set_text(uart_errors_label, buf);
        set_value_alert(uart_errors_label, uartErrors > 0, false);
    }
    
    if (crc_errors_label) {
        snprintf(buf, sizeof(buf), "%d", crcErrors);
        lv_label_set_text(crc_errors_label, buf);
        set_value_alert(crc_errors_label, crcErrors > 0, false);
    }
    
    if (reconnect_label) {
        snprintf(buf, sizeof(buf), "%d", reconnectCount);
        lv_label_set_text(reconnect_label, buf);
        set_value_alert(reconnect_label, reconnectCount > 0, false);
    }
    
    if (heap_free_label) {
        snprintf(buf, sizeof(buf), "%lu KB", (unsigned long)(heapFree / 1024));
        lv_label_set_text(heap_free_label, buf);
    }
    
    if (uptime_label) {
        if (uptimeSec < 3600) {
            snprintf(buf, sizeof(buf), "%02lu:%02lu", (unsigned long)(uptimeSec / 60), (unsigned long)(uptimeSec % 60));
        } else {
            snprintf(buf, sizeof(buf), "%lu:%02lu:%02lu",
                     (unsigned long)(uptimeSec / 3600),
                     (unsigned long)((uptimeSec % 3600) / 60),
                     (unsigned long)(uptimeSec % 60));
        }
        lv_label_set_text(uptime_label, buf);
    }
}

void system_screen_update_link(bool active, uint32_t baud, uint32_t rxFrames,
                               uint32_t txFrames, uint32_t crcErrors,
                               uint32_t parseErrors, uint32_t reconnects,
                               uint32_t pending, uint32_t heapFree,
                               uint32_t uptimeSec) {
    char buf[32];
    if (link_label) {
        if (active)
            lv_label_set_text(link_label, "ACTIVE");
        else
            lv_label_set_text(link_label, pending ? "..." : "OFF");
        lv_obj_set_style_text_color(
            link_label, active ? COLOR_AUDIO : COLOR_TEXT_MUTED, 0);
    }
    if (baud_label) {
        snprintf(buf, sizeof(buf), "%u", (unsigned)baud);
        lv_label_set_text(baud_label, buf);
    }
    if (rx_label) {
        snprintf(buf, sizeof(buf), "%u", (unsigned)rxFrames);
        lv_label_set_text(rx_label, buf);
    }
    if (tx_label) {
        snprintf(buf, sizeof(buf), "%u", (unsigned)txFrames);
        lv_label_set_text(tx_label, buf);
    }
    if (crc_errors_label) {
        snprintf(buf, sizeof(buf), "%u", (unsigned)crcErrors);
        lv_label_set_text(crc_errors_label, buf);
        set_value_alert(crc_errors_label, crcErrors > 0, false);
    }
    if (uart_errors_label) {
        snprintf(buf, sizeof(buf), "%u", (unsigned)parseErrors);
        lv_label_set_text(uart_errors_label, buf);
        set_value_alert(uart_errors_label, parseErrors > 0, false);
    }
    if (reconnect_label) {
        snprintf(buf, sizeof(buf), "%u", (unsigned)reconnects);
        lv_label_set_text(reconnect_label, buf);
        set_value_alert(reconnect_label, reconnects > 0, false);
    }
    if (heap_free_label) {
        snprintf(buf, sizeof(buf), "%lu KB", (unsigned long)(heapFree / 1024));
        lv_label_set_text(heap_free_label, buf);
    }
    if (uptime_label) {
        snprintf(buf, sizeof(buf), "%lu:%02lu", (unsigned long)(uptimeSec / 60),
                 (unsigned long)(uptimeSec % 60));
        lv_label_set_text(uptime_label, buf);
    }
}
