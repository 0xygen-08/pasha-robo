#ifndef PASHA_BLE_SERVICE_H
#define PASHA_BLE_SERVICE_H

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include "types.h"
#include "config.h"

class BleService {
public:
    static BleService& getInstance();

    void begin();
    void update();

    bool isConnected() const { return deviceConnected; }
    void notifyStatus();
    void notifySettings();

    // Callback olayları
    void onConnect();
    void onDisconnect();
    void onTimeReceived(const PashaTimePacket& packet);
    void onWeatherReceived(const PashaWeatherPacket& packet);
    void onExpressionReceived(const PashaExpressionPacket& packet);
    void onSettingsReceived(const PashaSettingsPacket& packet);

private:
    BleService();
    ~BleService() = default;

    bool deviceConnected;
    bool oldDeviceConnected;
    uint32_t lastStatusNotifyMs;

    BLEServer* pServer;
    BLEService* pService;
    BLECharacteristic* pTimeChar;
    BLECharacteristic* pWeatherChar;
    BLECharacteristic* pExpressionChar;
    BLECharacteristic* pSettingsChar;
    BLECharacteristic* pStatusChar;
};

#endif // PASHA_BLE_SERVICE_H
