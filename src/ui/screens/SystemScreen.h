#ifndef SYSTEM_SCREEN_H
#define SYSTEM_SCREEN_H

#include <lvgl.h>

void system_screen_init(lv_obj_t* parent);
void system_screen_update_info(const char* p4Firmware, const char* cydFirmware, 
                                int sampleRate, int blockSize, float cpuLoad,
                                int underruns, int uartErrors, int crcErrors,
                                int reconnectCount, uint32_t heapFree, uint32_t uptimeSec);

// Compact VoxLink link/diagnostics update (a few times per second at most).
void system_screen_update_link(bool active, uint32_t baud, uint32_t rxFrames,
                               uint32_t txFrames, uint32_t crcErrors,
                               uint32_t parseErrors, uint32_t reconnects,
                               uint32_t pending, uint32_t heapFree,
                               uint32_t uptimeSec);

#endif  // SYSTEM_SCREEN_H
