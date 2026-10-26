#include "scheduler.h"
#include "face_engine.h"
#include "clock_screen.h"
#include "weather_screen.h"
#include "display_hal.h"
#include <math.h>

Scheduler& Scheduler::getInstance() {
    static Scheduler instance;
    return instance;
}

Scheduler::Scheduler() :
    currentScreen(SCREEN_STATE_FACE),
    nextScreen(SCREEN_STATE_FACE),
    inTransition(false),
    transitionStartMs(0),
    transitionDurationMs(350),
    currentSlideOffset(0),
    screenEnterMs(0),
    cycleIntervalMs(DEFAULT_CYCLE_INTERVAL_SEC * 1000),
    clockDurationMs(DEFAULT_CLOCK_DURATION_SEC * 1000),
    weatherDurationMs(DEFAULT_WEATHER_DUR_SEC * 1000),
    clockEnabled(DEFAULT_CLOCK_ENABLED == 1),
    weatherEnabled(DEFAULT_WEATHER_ENABLED == 1)
{
}

void Scheduler::begin() {
    currentScreen = SCREEN_STATE_FACE;
    inTransition = false;
    screenEnterMs = millis();
}

void Scheduler::configure(uint16_t cycleIntervalSec, uint8_t clockDurSec, uint8_t weatherDurSec, bool clockEn, bool weatherEn) {
    cycleIntervalMs = (cycleIntervalSec > 0) ? (cycleIntervalSec * 1000) : 10000;
    clockDurationMs = (clockDurSec > 0) ? (clockDurSec * 1000) : 5000;
    weatherDurationMs = (weatherDurSec > 0) ? (weatherDurSec * 1000) : 5000;
    clockEnabled = clockEn;
    weatherEnabled = weatherEn;
}

void Scheduler::forceScreen(ScreenState screen) {
    if (currentScreen != screen) {
        startTransition(currentScreen, screen);
    }
}

void Scheduler::startTransition(ScreenState from, ScreenState to) {
    if (from == to) return;
    currentScreen = from;
    nextScreen = to;
    inTransition = true;
    transitionStartMs = millis();
    currentSlideOffset = 0;
}

void Scheduler::update() {
    uint32_t now = millis();

    // 1. Geçiş animasyonu devam ediyorsa
    if (inTransition) {
        uint32_t elapsed = now - transitionStartMs;
        if (elapsed >= transitionDurationMs) {
            inTransition = false;
            currentScreen = nextScreen;
            screenEnterMs = now;
            currentSlideOffset = 0;
        } else {
            // Sinüzoidal yumuşak slide ease-out
            float t = (float)elapsed / (float)transitionDurationMs;
            float ease = sinf(t * 1.570796f);
            currentSlideOffset = (int16_t)(ease * 128.0f);
        }
        return;
    }

    // 2. Yüz ekranı güncellemeleri
    if (currentScreen == SCREEN_STATE_FACE) {
        FaceEngine::getInstance().update();

        // Döngü süresi dolunca Saat veya Hava'ya geç
        if (now - screenEnterMs >= cycleIntervalMs) {
            if (clockEnabled) {
                startTransition(SCREEN_STATE_FACE, SCREEN_STATE_CLOCK);
            } else if (weatherEnabled) {
                startTransition(SCREEN_STATE_FACE, SCREEN_STATE_WEATHER);
            } else {
                screenEnterMs = now; // İkisi de kapalıysa süreyi sıfırla
            }
        }
    }
    // 3. Saat ekranı
    else if (currentScreen == SCREEN_STATE_CLOCK) {
        if (now - screenEnterMs >= clockDurationMs) {
            if (weatherEnabled) {
                startTransition(SCREEN_STATE_CLOCK, SCREEN_STATE_WEATHER);
            } else {
                startTransition(SCREEN_STATE_CLOCK, SCREEN_STATE_FACE);
            }
        }
    }
    // 4. Hava durumu ekranı
    else if (currentScreen == SCREEN_STATE_WEATHER) {
        if (now - screenEnterMs >= weatherDurationMs) {
            startTransition(SCREEN_STATE_WEATHER, SCREEN_STATE_FACE);
        }
    }
}

void Scheduler::renderScreen(ScreenState screen, int16_t offsetX) {
    switch (screen) {
        case SCREEN_STATE_FACE:
            // Yüz motoru normal çizilir
            FaceEngine::getInstance().draw();
            break;
        case SCREEN_STATE_CLOCK:
            ClockScreen::getInstance().draw(offsetX);
            break;
        case SCREEN_STATE_WEATHER:
            WeatherScreen::getInstance().draw(offsetX);
            break;
        default:
            FaceEngine::getInstance().draw();
            break;
    }
}

void Scheduler::render() {
    DisplayHAL& hal = DisplayHAL::getInstance();
    hal.clear();

    if (inTransition) {
        // Eski ekran sola kayar (-currentSlideOffset)
        renderScreen(currentScreen, -currentSlideOffset);
        // Yeni ekran sağdan girer (128 - currentSlideOffset)
        renderScreen(nextScreen, 128 - currentSlideOffset);
    } else {
        renderScreen(currentScreen, 0);
    }

    hal.display();
}
