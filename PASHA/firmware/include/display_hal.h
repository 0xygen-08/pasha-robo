#ifndef PASHA_DISPLAY_HAL_H
#define PASHA_DISPLAY_HAL_H

#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>
#include "config.h"

class DisplayHAL {
public:
    static DisplayHAL& getInstance();

    bool begin();
    void clear();
    void display();
    void setContrast(uint8_t contrast);
    void setPowerSave(bool isPowerSave);

    U8G2& getU8g2() { return u8g2; }

    // Çizim yardımcıları
    void drawSmoothHeart(int16_t cx, int16_t cy, float scale);
    void drawEyeBrow(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint8_t thickness = 2);

private:
    DisplayHAL();
    ~DisplayHAL() = default;

#if defined(USE_SH1106)
    U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2;
#elif defined(USE_SSD1306)
    U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2;
#else
    #error "Lutfen config.h icinde USE_SH1106 veya USE_SSD1306 tanimlayiniz!"
#endif
};

#endif // PASHA_DISPLAY_HAL_H
