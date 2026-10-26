#ifndef PASHA_CLOCK_SCREEN_H
#define PASHA_CLOCK_SCREEN_H

#include <Arduino.h>
#include "types.h"
#include "display_hal.h"

class ClockScreen {
public:
    static ClockScreen& getInstance();

    void begin();
    void setTime(uint32_t epoch, int16_t tzOffsetMinutes);
    bool hasTime() const { return hasValidTime; }

    void setStyle(ClockStyle style);
    ClockStyle getStyle() const { return currentStyle; }

    void draw(int16_t offsetX = 0);

private:
    ClockScreen();
    ~ClockScreen() = default;

    void getCurrentTime(uint8_t& hours, uint8_t& minutes, uint8_t& seconds);

    void drawNoTimeSync(int16_t offsetX);
    void drawBigDigital(uint8_t hours, uint8_t minutes, uint8_t seconds, int16_t offsetX);
    void drawDigitalMiniFace(uint8_t hours, uint8_t minutes, uint8_t seconds, int16_t offsetX);
    void drawAnalog(uint8_t hours, uint8_t minutes, uint8_t seconds, int16_t offsetX);
    void drawRetro7Seg(uint8_t hours, uint8_t minutes, uint8_t seconds, int16_t offsetX);

    void draw7SegDigit(int16_t x, int16_t y, uint8_t digit, uint8_t segW, uint8_t segH);

    bool hasValidTime;
    uint32_t baseEpoch;
    uint32_t syncLocalMillis;
    ClockStyle currentStyle;
};

#endif // PASHA_CLOCK_SCREEN_H
