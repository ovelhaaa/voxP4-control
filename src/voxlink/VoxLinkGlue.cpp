// VoxLink client glue for the ESP32 CYD: owns the single client instance from a
// dedicated task and exchanges intents/events with the UI context through small
// fixed FreeRTOS queues. LVGL is only ever touched by the UI task.
#if defined(ARDUINO)

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>

#include "voxlink/VoxLinkClient.h"
#include "voxlink/VoxLinkGlue.h"
#include "voxlink/VoxLinkProtocol.h"
#include "voxlink/VoxLinkUi.h"

#include "board/CYD_Config.h"
#include "ui/UiApp.h"
#include "ui/params/UiParamModel.h"
#include "ui/screens/SystemScreen.h"

namespace {
voxlink::VoxLinkClient g_client;
constexpr size_t kIntentQueueDepth = 16;
// Must hold a full 71-parameter snapshot plus LinkActive with margin. The
// FreeRTOS queue is also a head/tail ring with usable depth = depth-1.
constexpr size_t kEventQueueDepth = 80;

struct Intent {
    uint16_t id;
    float value;
};

QueueHandle_t g_intent_queue = nullptr;
QueueHandle_t g_event_queue = nullptr;
uint32_t g_ui_queue_drops = 0; // events the FreeRTOS bridge could not enqueue

// Published capability snapshot for UI-thread queries (guarded by a spinlock).
portMUX_TYPE g_caps_mux = portMUX_INITIALIZER_UNLOCKED;
constexpr size_t kPublishedIds = 64;
struct PublishedCaps {
    bool valid = false;
    bool presets = false;
    bool bypass_available = true;
    bool active = false;
    uint16_t count = 0;
    uint16_t ids[kPublishedIds];
    uint32_t rx = 0;
    uint32_t tx = 0;
    uint32_t crc = 0;
    uint32_t parse = 0;
    uint32_t reconnects = 0;
    uint32_t pending = 0;
    uint32_t event_drops = 0;
    uint32_t ui_queue_drops = 0;
};
PublishedCaps g_published;

void publish_caps(bool active) {
    PublishedCaps next;
    const voxlink::CapsSnapshot &caps = g_client.caps();
    const voxlink::Counters &counters = g_client.counters();
    next.valid = caps.valid;
    next.presets = caps.valid && ((caps.caps & voxlink::kCapPresets) != 0);
    next.bypass_available = !active; // global bypass has no v1 backend
    next.active = active;
    next.count = 0;
    if (caps.valid) {
        for (const auto &p : caps.params) {
            if (p.used && next.count < kPublishedIds)
                next.ids[next.count++] = p.id;
        }
    }
    next.rx = counters.frames_rx;
    next.tx = counters.frames_tx;
    next.crc = counters.crc_errors;
    next.parse = counters.parse_errors + counters.length_errors +
                 counters.version_errors;
    next.reconnects = counters.reconnects;
    next.pending = static_cast<uint32_t>(g_client.pending_count());
    next.event_drops = counters.event_drops;
    next.ui_queue_drops = g_ui_queue_drops;
    portENTER_CRITICAL(&g_caps_mux);
    g_published = next;
    portEXIT_CRITICAL(&g_caps_mux);
}

void task_entry(void *) {
    for (;;) {
        Intent intent;
        while (xQueueReceive(g_intent_queue, &intent, 0) == pdTRUE) {
            g_client.set_parameter(intent.id, intent.value, millis());
        }

        const int available = Serial2.available();
        if (available > 0) {
            uint8_t buffer[128];
            const size_t want =
                (static_cast<size_t>(available) < sizeof(buffer)) ? available
                                                                  : sizeof(buffer);
            const size_t n = Serial2.readBytes(buffer, want);
            if (n > 0) g_client.feed(buffer, n, millis());
        }

        g_client.tick(millis());

        uint8_t tx[128];
        size_t n = 0;
        while ((n = g_client.take_tx(tx, sizeof(tx))) > 0) {
            Serial2.write(tx, n);
        }

        voxlink::Event ev;
        while (g_client.take_event(&ev)) {
            if (xQueueSend(g_event_queue, &ev, 0) != pdTRUE) ++g_ui_queue_drops;
        }

        publish_caps(g_client.active());

#if defined(DEBUG_BUILD)
        // Log only abnormal saturation events, never normal traffic.
        static voxlink::Counters prev;
        const voxlink::Counters &c = g_client.counters();
        if (c.event_drops != prev.event_drops)
            Serial.printf("[VL] event queue full (drops=%u)\n", (unsigned)c.event_drops);
        if (c.pending_full != prev.pending_full)
            Serial.printf("[VL] pending pool full (drops=%u)\n", (unsigned)c.pending_full);
        if (c.tx_drops != prev.tx_drops)
            Serial.printf("[VL] tx ring full (drops=%u)\n", (unsigned)c.tx_drops);
        if (c.caps_overflow != prev.caps_overflow)
            Serial.printf("[VL] caps overflow\n");
        if (c.snapshot_overflow != prev.snapshot_overflow)
            Serial.printf("[VL] snapshot overflow\n");
        if (c.queue_full_exhausted != prev.queue_full_exhausted)
            Serial.printf("[VL] retries exhausted (count=%u)\n",
                          (unsigned)c.queue_full_exhausted);
        prev = c;
#endif

        vTaskDelay(pdMS_TO_TICKS(2));
    }
}
} // namespace

void voxlink_glue_init() {
    g_intent_queue = xQueueCreate(kIntentQueueDepth, sizeof(Intent));
    g_event_queue = xQueueCreate(kEventQueueDepth, sizeof(voxlink::Event));
    if (g_intent_queue == nullptr || g_event_queue == nullptr) return;

    Serial2.begin(VoxCydConfig::VoxUartBaud, SERIAL_8N1, VoxCydConfig::VoxUartRx,
                  VoxCydConfig::VoxUartTx);
    // Bound any blocking read to 2 ms even if fewer bytes than requested arrive;
    // the task loop runs every ~2 ms and must not stall heartbeat/coalescing.
    Serial2.setTimeout(2);

    // UI intents -> client task.
    ui_set_wire_intent_callback([](uint16_t wire_id, float value) {
        if (g_intent_queue == nullptr) return;
        Intent intent{wire_id, value};
        xQueueSend(g_intent_queue, &intent, 0);
    });

    g_client.begin(millis());
    publish_caps(false);

    xTaskCreatePinnedToCore(task_entry, "voxlink", 4096, nullptr, 4, nullptr, 1);
}

void voxlink_glue_tick() {
    if (g_event_queue == nullptr) return;
    voxlink::Event ev;
    while (xQueueReceive(g_event_queue, &ev, 0) == pdTRUE) {
        switch (ev.type) {
            case voxlink::EventType::LinkActive: {
                bool presets = false;
                bool bypass = false;
                portENTER_CRITICAL(&g_caps_mux);
                presets = g_published.presets;
                bypass = g_published.bypass_available;
                portEXIT_CRITICAL(&g_caps_mux);
                ui_set_link_capabilities(presets, bypass);
                ui_update_link_state(true);
                // The editor may have been built before CAPS arrived.
                ui_refresh_capability_gated_controls();
                break;
            }
            case voxlink::EventType::LinkDown:
                ui_set_link_capabilities(false, true);
                ui_update_link_state(false);
                break;
            case voxlink::EventType::ParamAuthoritative:
            case voxlink::EventType::ParamRevert: {
                // The controller state is keyed by wire ID, so every canonical
                // parameter (including effect enables) flows through the same
                // path. The P4 remains the authority.
                if (ev.type == voxlink::EventType::ParamRevert)
                    ui_revert_parameter(ev.id);
                else
                    ui_apply_parameter_authoritative(ev.id, ev.value);
                break;
            }
        }
    }

    // Compact System diagnostics at a harmless rate.
    static uint32_t last_system_ms = 0;
    const uint32_t now = millis();
    if (now - last_system_ms >= 500) {
        last_system_ms = now;
        PublishedCaps snapshot;
        portENTER_CRITICAL(&g_caps_mux);
        snapshot = g_published;
        portEXIT_CRITICAL(&g_caps_mux);
        system_screen_update_link(snapshot.active, VoxCydConfig::VoxUartBaud,
                                  snapshot.rx, snapshot.tx, snapshot.crc,
                                  snapshot.parse, snapshot.reconnects,
                                  snapshot.pending, esp_get_free_heap_size(),
                                  now / 1000);
    }
}

bool voxlink_param_supported(uint16_t voxlink_id) {
    bool valid = false;
    bool found = false;
    portENTER_CRITICAL(&g_caps_mux);
    valid = g_published.valid;
    if (valid) {
        for (uint16_t i = 0; i < g_published.count; ++i) {
            if (g_published.ids[i] == voxlink_id) {
                found = true;
                break;
            }
        }
    }
    portEXIT_CRITICAL(&g_caps_mux);
    if (!valid) return true; // offline local development
    return found;
}

#endif // ARDUINO
