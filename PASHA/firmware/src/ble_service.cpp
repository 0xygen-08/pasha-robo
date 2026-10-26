#include "ble_service.h"
#include "face_engine.h"
#include "clock_screen.h"
#include "weather_screen.h"
#include "scheduler.h"
#include "settings_storage.h"
#include "power_manager.h"

// ==========================================
// BLE SERVER VE KARAKTERİSTİK GERİ ÇAĞIRIMLARI
// ==========================================
class ServerCallbacks : public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) override {
        BleService::getInstance().onConnect();
    }

    void onDisconnect(BLEServer* pServer) override {
        BleService::getInstance().onDisconnect();
    }
};

class TimeCallbacks : public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic* pCharacteristic) override {
        String value = pCharacteristic->getValue();
        if (value.length() >= sizeof(PashaTimePacket)) {
            PashaTimePacket packet;
            memcpy(&packet, value.c_str(), sizeof(PashaTimePacket));
            BleService::getInstance().onTimeReceived(packet);
        }
    }
};

class WeatherCallbacks : public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic* pCharacteristic) override {
        String value = pCharacteristic->getValue();
        if (value.length() >= sizeof(PashaWeatherPacket)) {
            PashaWeatherPacket packet;
            memcpy(&packet, value.c_str(), sizeof(PashaWeatherPacket));
            BleService::getInstance().onWeatherReceived(packet);
        }
    }
};

class ExpressionCallbacks : public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic* pCharacteristic) override {
        String value = pCharacteristic->getValue();
        if (value.length() >= sizeof(PashaExpressionPacket)) {
            PashaExpressionPacket packet;
            memcpy(&packet, value.c_str(), sizeof(PashaExpressionPacket));
            BleService::getInstance().onExpressionReceived(packet);
        }
    }
};

class SettingsCallbacks : public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic* pCharacteristic) override {
        String value = pCharacteristic->getValue();
        if (value.length() >= sizeof(PashaSettingsPacket)) {
            PashaSettingsPacket packet;
            memcpy(&packet, value.c_str(), sizeof(PashaSettingsPacket));
            BleService::getInstance().onSettingsReceived(packet);
        }
    }
};

// ==========================================
// BLE SERVICE SINIF METODLARI
// ==========================================
BleService& BleService::getInstance() {
    static BleService instance;
    return instance;
}

BleService::BleService() :
    deviceConnected(false),
    oldDeviceConnected(false),
    lastStatusNotifyMs(0),
    pServer(nullptr),
    pService(nullptr),
    pTimeChar(nullptr),
    pWeatherChar(nullptr),
    pExpressionChar(nullptr),
    pSettingsChar(nullptr),
    pStatusChar(nullptr)
{
}

void BleService::begin() {
    BLEDevice::init(BLE_DEVICE_NAME);

    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new ServerCallbacks());

    pService = pServer->createService(PASHA_SERVICE_UUID);

    // 1. TIME Karakteristiği (Write)
    pTimeChar = pService->createCharacteristic(
        CHAR_TIME_UUID,
        BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_WRITE_NR
    );
    pTimeChar->setCallbacks(new TimeCallbacks());

    // 2. WEATHER Karakteristiği (Write)
    pWeatherChar = pService->createCharacteristic(
        CHAR_WEATHER_UUID,
        BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_WRITE_NR
    );
    pWeatherChar->setCallbacks(new WeatherCallbacks());

    // 3. EXPRESSION Karakteristiği (Write)
    pExpressionChar = pService->createCharacteristic(
        CHAR_EXPRESSION_UUID,
        BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_WRITE_NR
    );
    pExpressionChar->setCallbacks(new ExpressionCallbacks());

    // 4. SETTINGS Karakteristiği (Read, Write, Notify)
    pSettingsChar = pService->createCharacteristic(
        CHAR_SETTINGS_UUID,
        BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_NOTIFY
    );
    pSettingsChar->setCallbacks(new SettingsCallbacks());
    pSettingsChar->addDescriptor(new BLE2902());

    // 5. STATUS Karakteristiği (Read, Notify)
    pStatusChar = pService->createCharacteristic(
        CHAR_STATUS_UUID,
        BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY
    );
    pStatusChar->addDescriptor(new BLE2902());

    pService->start();

    // Başlangıç değerlerini karakteristiklere yükle
    notifySettings();
    notifyStatus();

    // Reklam (Advertising) Başlat
    BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(PASHA_SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06);
    pAdvertising->setMinPreferred(0x12);
    pAdvertising->start();
}

void BleService::onConnect() {
    deviceConnected = true;
    PowerManager::getInstance().onBleConnected();
    notifyStatus();
}

void BleService::onDisconnect() {
    deviceConnected = false;
    PowerManager::getInstance().onBleDisconnected();

    // Yeniden reklam başlat
    BLEDevice::startAdvertising();
}

void BleService::onTimeReceived(const PashaTimePacket& packet) {
    ClockScreen::getInstance().setTime(packet.epoch, packet.tzOffsetMinutes);
    PowerManager::getInstance().resetActivity();
    notifyStatus();
}

void BleService::onWeatherReceived(const PashaWeatherPacket& packet) {
    WeatherScreen::getInstance().setWeatherData(
        packet.temperature,
        packet.weatherCode,
        packet.tempMin,
        packet.tempMax,
        packet.humidity
    );
    PowerManager::getInstance().resetActivity();
    notifyStatus();
}

void BleService::onExpressionReceived(const PashaExpressionPacket& packet) {
    bool isAuto = (packet.mode == 1);
    FaceEngine::getInstance().setExpression(static_cast<ExpressionId>(packet.expressionId), !isAuto);
    FaceEngine::getInstance().setAutoMode(isAuto);
    PowerManager::getInstance().resetActivity();
    notifyStatus();
}

void BleService::onSettingsReceived(const PashaSettingsPacket& packet) {
    // 1. Yüz Stili
    FaceEngine::getInstance().setFaceStyle(static_cast<FaceStyle>(packet.faceStyle));

    // 2. Saat Stili
    ClockScreen::getInstance().setStyle(static_cast<ClockStyle>(packet.clockStyle));

    // 3. Hava Durumu Stili
    WeatherScreen::getInstance().setStyle(static_cast<WeatherStyle>(packet.weatherStyle));

    // 4. Scheduler Zamanlamaları
    Scheduler::getInstance().configure(
        packet.cycleIntervalSec,
        packet.clockDurationSec,
        packet.weatherDurSec,
        packet.clockEnabled == 1,
        packet.weatherEnabled == 1
    );

    // 5. Güç & Parlaklık Ayarları
    PowerManager::getInstance().setBrightness(packet.brightness);
    PowerManager::getInstance().setSleepTimeout(packet.sleepTimeoutMin);

    // 6. Kalıcı hafızaya kaydet
    SettingsStorage::getInstance().saveSettings(packet);

    PowerManager::getInstance().resetActivity();
    notifySettings();
    notifyStatus();
}

void BleService::notifySettings() {
    if (!pSettingsChar) return;

    PashaSettingsPacket s;
    SettingsStorage::getInstance().loadSettings(s);
    pSettingsChar->setValue((uint8_t*)&s, sizeof(PashaSettingsPacket));
    if (deviceConnected) {
        pSettingsChar->notify();
    }
}

void BleService::notifyStatus() {
    if (!pStatusChar) return;

    PashaStatusPacket status;
    status.fwMajor = FIRMWARE_VERSION_MAJOR;
    status.fwMinor = FIRMWARE_VERSION_MINOR;
    status.isConnected = deviceConnected ? 1 : 0;
    status.hasTime = ClockScreen::getInstance().hasTime() ? 1 : 0;
    status.hasWeather = WeatherScreen::getInstance().hasData() ? 1 : 0;
    status.currentExpression = static_cast<uint8_t>(FaceEngine::getInstance().getExpression());
    status.currentScreen = static_cast<uint8_t>(Scheduler::getInstance().getCurrentScreen());

    pStatusChar->setValue((uint8_t*)&status, sizeof(PashaStatusPacket));
    if (deviceConnected) {
        pStatusChar->notify();
    }
}

void BleService::update() {
    uint32_t now = millis();

    // 2 saniyede bir durum bildirimini güncelle
    if (deviceConnected && (now - lastStatusNotifyMs >= 2000)) {
        lastStatusNotifyMs = now;
        notifyStatus();
    }

    // Bağlantı durum değişim kontrolü
    if (!deviceConnected && oldDeviceConnected) {
        delay(20);
        BLEDevice::startAdvertising();
        oldDeviceConnected = deviceConnected;
    }
    if (deviceConnected && !oldDeviceConnected) {
        oldDeviceConnected = deviceConnected;
    }
}
