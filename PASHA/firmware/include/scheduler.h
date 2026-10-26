#ifndef PASHA_SCHEDULER_H
#define PASHA_SCHEDULER_H

#include <Arduino.h>
#include "types.h"
#include "config.h"

enum TransitionType : uint8_t {
    TRANSITION_SLIDE_LEFT = 0,
    TRANSITION_NONE = 1
};

class Scheduler {
public:
    static Scheduler& getInstance();

    void begin();
    void update();
    void render();

    // Zamanlama ve aktiflik yapılandırması
    void configure(uint16_t cycleIntervalSec, uint8_t clockDurSec, uint8_t weatherDurSec, bool clockEn, bool weatherEn);

    ScreenState getCurrentScreen() const { return currentScreen; }
    void forceScreen(ScreenState screen);

private:
    Scheduler();
    ~Scheduler() = default;

    void startTransition(ScreenState from, ScreenState to);
    void renderScreen(ScreenState screen, int16_t offsetX);

    ScreenState currentScreen;
    ScreenState nextScreen;
    bool inTransition;
    uint32_t transitionStartMs;
    uint32_t transitionDurationMs;
    int16_t currentSlideOffset;

    uint32_t screenEnterMs;
    uint16_t cycleIntervalMs;
    uint32_t clockDurationMs;
    uint32_t weatherDurationMs;
    bool clockEnabled;
    bool weatherEnabled;
};

#endif // PASHA_SCHEDULER_H
