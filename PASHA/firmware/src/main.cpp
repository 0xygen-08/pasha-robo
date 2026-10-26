#include <Arduino.h>
#include "config.h"
#include "types.h"
#include "display_hal.h"
#include "face_engine.h"
#include "clock_screen.h"
#include "weather_screen.h"
#include "scheduler.h"
#include "settings_storage.h"
#include "power_manager.h"
#include "ble_service.h"

static uint32_t lastFrameMs = 0;

void setup() {
    Serial.begin(115200);
    // ESP32-C3 USB CDC başlatma payı
    delay(500);
    Serial.println("\n[PASHA] Robot baslatiliyor...");

    // 1. Ekran Donanım Katmanı
    DisplayHAL::getInstance().begin();
    Serial.println("[PASHA] Ekran baslatildi.");

    // 2. Kalıcı Ayarlar (NVS)
    SettingsStorage::getInstance().begin();
    PashaSettingsPacket settings;
    SettingsStorage::getInstance().loadSettings(settings);
    Serial.println("[PASHA] Ayarlar yuklendi.");

    // 3. Yüz Motoru
    FaceEngine::getInstance().begin();
    FaceEngine::getInstance().setFaceStyle(static_cast<FaceStyle>(settings.faceStyle));
    FaceEngine::getInstance().setAutoMode(settings.autoExprMode == 1);

    // 4. Saat ve Hava Ekranları
    ClockScreen::getInstance().begin();
    ClockScreen::getInstance().setStyle(static_cast<ClockStyle>(settings.clockStyle));

    WeatherScreen::getInstance().begin();
    WeatherScreen::getInstance().setStyle(static_cast<WeatherStyle>(settings.weatherStyle));

    // 5. Zamanlayıcı & Geçiş Motoru
    Scheduler::getInstance().begin();
    Scheduler::getInstance().configure(
        settings.cycleIntervalSec,
        settings.clockDurationSec,
        settings.weatherDurSec,
        settings.clockEnabled == 1,
        settings.weatherEnabled == 1
    );

    // 6. Güç Yöneticisi
    PowerManager::getInstance().begin();
    PowerManager::getInstance().setBrightness(settings.brightness);
    PowerManager::getInstance().setSleepTimeout(settings.sleepTimeoutMin);

    // 7. BLE Servisi
    BleService::getInstance().begin();
    Serial.println("[PASHA] BLE yayini baslatildi: " BLE_DEVICE_NAME);

    lastFrameMs = millis();
}

void loop() {
    uint32_t now = millis();

    // 30 FPS Hedef Çerçeve Zamanlaması (~33.3 ms)
    if (now - lastFrameMs >= FRAME_TIME_MS) {
        lastFrameMs = now;

        // Arka plan servis ve durum güncellemeleri
        PowerManager::getInstance().update();
        BleService::getInstance().update();
        Scheduler::getInstance().update();

        // Ekran uyku modunda değilse yeni kareyi çiz
        if (!PowerManager::getInstance().isSleeping()) {
            Scheduler::getInstance().render();
        }
    }

    // CPU yükünü dengelemek için kısa dinlenme
    yield();
}
