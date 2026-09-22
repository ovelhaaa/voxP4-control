#ifndef EFFECT_EDIT_SCREEN_H
#define EFFECT_EDIT_SCREEN_H

#include <lvgl.h>
#include "ui/params/UiParamModel.h"

// Initialize the data-driven effect editor. Header/back/enable are persistent;
// only the parameter body is rebuilt per effect (and per mode / sync change).
void effect_edit_screen_init(lv_obj_t* parent);

// Load an effect and rebuild its parameter body from the canonical state.
// BASIC is shown by default; ADVANCED stays collapsed until opened.
void effect_edit_load_effect(UiEffectId effect);

// Sync the compact ON/OFF toggle with the authoritative effect state.
void effect_edit_set_enabled(UiEffectId effect, bool enabled);

// Called by the App when a parameter value changes. Updates the affected row
// immediately, or schedules a body rebuild for structural controls (mode/sync).
void effect_edit_notify(uint16_t wireId, float value);

// Performs a pending rebuild outside LVGL event dispatch. Called from ui_app_run.
void effect_edit_tick(void);

// Requests a rebuild of the open editor body (e.g. after a CAPS snapshot
// changes capability gating). Safe to call outside LVGL event dispatch.
void effect_edit_refresh(void);

#endif // EFFECT_EDIT_SCREEN_H
