#ifndef PASHA_POWER_MANAGER_H
#define PASHA_POWER_MANAGER_H

#include <Arduino.h>
#include "types.h"
#include "config.h"

class PowerManager {
public:
    static PowerManager& getInstance();

    void begin();
    void update();

    void resetActivity();
    void setSleepTimeout(uint8_t minutes);
    void setBrightness(uint8_t brightness);

    bool isSleeping() const { return isScreenSleeping; }
    void wakeUp();

    void onBleConnected();
    void onBleDisconnected();

private:
    PowerManager();
    ~PowerManager() = default;

    uint8_t sleepTimeoutMin;
    uint8_t currentBrightness;
    uint32_t lastActivityMs;
    bool isScreenSleeping;
    bool isBleConnected;
};

#endif // PASHA_POWER_MANAGER_H
