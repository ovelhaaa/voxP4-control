#ifndef MASTER_SCREEN_H
#define MASTER_SCREEN_H

#include <lvgl.h>
#include <cstdint>

// Global MASTER / ROUTING area. Keeps global controls off the Performance
// screen while remaining one tap away. It edits the canonical parameters
// (TempoBpm, LimiterCeiling, OutputMuteDry, OutputSpatialRouting,
// OutputSpatialSource) through the same VoxLink parameter path as any effect.
void master_screen_init(lv_obj_t* parent);

// Full resync from the canonical state (on entry / CAPS change).
void master_screen_refresh(void);

// Apply a single remote/local parameter change to the visible widgets.
void master_screen_notify(uint16_t wireId, float value);

// Reflect the latest Tap Tempo estimate in the TAP affordance.
void master_screen_refresh_tap(bool hasEstimate, float bpm);

#endif // MASTER_SCREEN_H
