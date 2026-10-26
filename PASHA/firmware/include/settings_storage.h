#ifndef PASHA_SETTINGS_STORAGE_H
#define PASHA_SETTINGS_STORAGE_H

#include <Arduino.h>
#include <Preferences.h>
#include "types.h"
#include "config.h"

class SettingsStorage {
public:
    static SettingsStorage& getInstance();

    void begin();
    void loadSettings(PashaSettingsPacket& settings);
    void saveSettings(const PashaSettingsPacket& settings);
    void resetDefaults(PashaSettingsPacket& settings);

private:
    SettingsStorage();
    ~SettingsStorage() = default;

    Preferences prefs;
};

#endif // PASHA_SETTINGS_STORAGE_H
