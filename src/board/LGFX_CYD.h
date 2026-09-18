#ifndef LGFX_CYD_H
#define LGFX_CYD_H

#include <LovyanGFX.hpp>
#include "board/CYD_Config.h"

class LGFX_CYD : public lgfx::LGFX_Device {
public:
    lgfx::Panel_ST7789 _panel_instance;
    lgfx::Bus_SPI _bus_instance;
    lgfx::Light_PWM _light_instance;
    lgfx::Touch_XPT2046 _touch_instance;

    LGFX_CYD(void) {
        {
            auto cfg = _bus_instance.config();

            // SPI configuration for display
            cfg.spi_host = VSPI_HOST;
            cfg.spi_mode = 0;
            cfg.freq_write = 40000000;
            cfg.freq_read = 16000000;
            cfg.spi_3wire = false;
            cfg.use_lock = true;
            cfg.dma_channel = SPI_DMA_CH_AUTO;

            // Pin assignment
            cfg.pin_sclk = VoxCydConfig::TftSclk;
            cfg.pin_miso = VoxCydConfig::TftMiso;
            cfg.pin_mosi = VoxCydConfig::TftMosi;
            cfg.pin_dc = VoxCydConfig::TftDc;

            _bus_instance.config(cfg);
            _panel_instance.setBus(&_bus_instance);
        }

        {
            auto cfg = _panel_instance.config();

            cfg.pin_cs = VoxCydConfig::TftCs;
            cfg.pin_rst = VoxCydConfig::TftRst;

            cfg.readable = true;
            cfg.bus_shared = false;

            // Required panel configurations for this ST7789 variant
            cfg.panel_width = 240;
            cfg.panel_height = 320;
            cfg.offset_x = 0;
            cfg.offset_y = 0;
            cfg.offset_rotation = 0;
            cfg.dummy_read_pixel = 8;
            cfg.dummy_read_bits = 1;
            cfg.invert = true;
            cfg.rgb_order = false;
            cfg.dlen_16bit = false;

            _panel_instance.config(cfg);
            _panel_instance.setRotation(VoxCydConfig::ScreenRotation);
        }

        {
            auto cfg = _light_instance.config();
            cfg.pin_bl = VoxCydConfig::TftBacklight;
            cfg.invert = false;
            cfg.freq = 44100;
            cfg.pwm_channel = 7;

            _light_instance.config(cfg);
            _panel_instance.setLight(&_light_instance);
        }

        {
            auto cfg = _touch_instance.config();

            cfg.x_min = VoxCydConfig::TouchMinX;
            cfg.x_max = VoxCydConfig::TouchMaxX;
            cfg.y_min = VoxCydConfig::TouchMinY;
            cfg.y_max = VoxCydConfig::TouchMaxY;
            cfg.offset_rotation = 2;
            cfg.pin_sclk = VoxCydConfig::TouchSclk;
            cfg.pin_miso = VoxCydConfig::TouchMiso;
            cfg.pin_mosi = VoxCydConfig::TouchMosi;
            cfg.pin_cs = VoxCydConfig::TouchCs;

            cfg.bus_shared = false;
            cfg.freq = 1000000;
            cfg.spi_host = -1;  // Software SPI

            _touch_instance.config(cfg);
            _panel_instance.setTouch(&_touch_instance);
        }

        setPanel(&_panel_instance);
    }
};

#endif  // LGFX_CYD_H
