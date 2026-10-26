#include "power_manager.h"
#include "display_hal.h"
#include <esp32-hal-cpu.h>

PowerManager& PowerManager::getInstance() {
    static PowerManager instance;
    return instance;
}

PowerManager::PowerManager() :
    sleepTimeoutMin(DEFAULT_SLEEP_TIMEOUT_MIN),
    currentBrightness(DEFAULT_BRIGHTNESS),
    lastActivityMs(0),
    isScreenSleeping(false),
    isBleConnected(false)
{
}

void PowerManager::begin() {
    lastActivityMs = millis();
    isScreenSleeping = false;
    isBleConnected = false;

    // Başlangıçta güç tasarrufu için 80 MHz CPU frekansı
    setCpuFrequencyMhz(80);
}

void PowerManager::resetActivity() {
    lastActivityMs = millis();
    if (isScreenSleeping) {
        wakeUp();
    }
}

void PowerManager::setSleepTimeout(uint8_t minutes) {
    sleepTimeoutMin = minutes;
    resetActivity();
}

void PowerManager::setBrightness(uint8_t brightness) {
    currentBrightness = brightness;
    DisplayHAL::getInstance().setContrast(brightness);
}

void PowerManager::wakeUp() {
    if (isScreenSleeping) {
        DisplayHAL::getInstance().setPowerSave(false);
        isScreenSleeping = false;
    }
    lastActivityMs = millis();
}

void PowerManager::onBleConnected() {
    isBleConnected = true;
    // Bağlıyken pürüzsüz animasyon ve BLE performansı için 160 MHz
    setCpuFrequencyMhz(160);
    resetActivity();
}

void PowerManager::onBleDisconnected() {
    isBleConnected = false;
    // Boştayken pil tasarrufu için 80 MHz
    setCpuFrequencyMhz(80);
    resetActivity();
}

void PowerManager::update() {
    // Uyku kapalıysa (0) kontrol etme
    if (sleepTimeoutMin == 0) return;

    // Eğer BLE bağlıysa ekran uyumasın
    if (isBleConnected) return;

    uint32_t now = millis();
    uint32_t timeoutMs = (uint32_t)sleepTimeoutMin * 60UL * 1000UL;

    if (!isScreenSleeping && (now - lastActivityMs >= timeoutMs)) {
        // Ekranı uyku moduna al
        DisplayHAL::getInstance().setPowerSave(true);
        isScreenSleeping = true;
    }
}
