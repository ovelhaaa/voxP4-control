#ifndef PERFORMANCE_SCREEN_H
#define PERFORMANCE_SCREEN_H

#include <lvgl.h>
#include "ui/params/UiParamModel.h"

// Initialize the performance screen
void performance_screen_init(lv_obj_t* parent);

// Update functions for dynamic content
void performance_update_meters(float inputDb, float outputDb);
void performance_update_pitch(float freqHz, const char* noteName, bool voiced);
void performance_update_effect(UiEffectId effect, bool enabled);
void performance_update_effect_value(UiEffectId effect, const char* mainValue);
void performance_update_preset(const char* name);
void performance_update_scene_status(const char* sceneName, const char* subsceneName, int subIdx, int subTotal, int setIdx, int setTotal, bool isDirty);
void performance_update_link(bool connected);
void performance_update_tempo(float bpm);
void performance_update_footswitch(int fsIndex, const char* label, bool pressed);

#endif  // PERFORMANCE_SCREEN_H
