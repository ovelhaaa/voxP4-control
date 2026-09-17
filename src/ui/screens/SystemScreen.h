#ifndef SYSTEM_SCREEN_H
#define SYSTEM_SCREEN_H

#include <lvgl.h>

void system_screen_init(lv_obj_t* parent);
void system_screen_update_info(const char* p4Firmware, const char* cydFirmware, 
                                int sampleRate, int blockSize, float cpuLoad,
                                int underruns, int uartErrors, int crcErrors,
                                int reconnectCount, uint32_t heapFree, uint32_t uptimeSec);

#endif  // SYSTEM_SCREEN_H
