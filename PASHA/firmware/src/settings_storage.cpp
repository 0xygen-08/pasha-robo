#include "settings_storage.h"

SettingsStorage& SettingsStorage::getInstance() {
    static SettingsStorage instance;
    return instance;
}

SettingsStorage::SettingsStorage() {
}

void SettingsStorage::begin() {
    prefs.begin("pasha_cfg", false);
}

void SettingsStorage::resetDefaults(PashaSettingsPacket& settings) {
    settings.faceStyle = DEFAULT_FACE_STYLE;
    settings.clockStyle = DEFAULT_CLOCK_STYLE;
    settings.weatherStyle = DEFAULT_WEATHER_STYLE;
    settings.cycleIntervalSec = DEFAULT_CYCLE_INTERVAL_SEC;
    settings.clockDurationSec = DEFAULT_CLOCK_DURATION_SEC;
    settings.weatherDurSec = DEFAULT_WEATHER_DUR_SEC;
    settings.clockEnabled = DEFAULT_CLOCK_ENABLED;
    settings.weatherEnabled = DEFAULT_WEATHER_ENABLED;
    settings.brightness = DEFAULT_BRIGHTNESS;
    settings.sleepTimeoutMin = DEFAULT_SLEEP_TIMEOUT_MIN;
    settings.autoExprMode = DEFAULT_EXPR_MODE;

    saveSettings(settings);
}

void SettingsStorage::loadSettings(PashaSettingsPacket& settings) {
    if (!prefs.isKey("initialized")) {
        resetDefaults(settings);
        prefs.putBool("initialized", true);
        return;
    }

    settings.faceStyle = prefs.getUChar("f_style", DEFAULT_FACE_STYLE);
    settings.clockStyle = prefs.getUChar("c_style", DEFAULT_CLOCK_STYLE);
    settings.weatherStyle = prefs.getUChar("w_style", DEFAULT_WEATHER_STYLE);
    settings.cycleIntervalSec = prefs.getUShort("cycle_int", DEFAULT_CYCLE_INTERVAL_SEC);
    settings.clockDurationSec = prefs.getUChar("c_dur", DEFAULT_CLOCK_DURATION_SEC);
    settings.weatherDurSec = prefs.getUChar("w_dur", DEFAULT_WEATHER_DUR_SEC);
    settings.clockEnabled = prefs.getUChar("c_en", DEFAULT_CLOCK_ENABLED);
    settings.weatherEnabled = prefs.getUChar("w_en", DEFAULT_WEATHER_ENABLED);
    settings.brightness = prefs.getUChar("bright", DEFAULT_BRIGHTNESS);
    settings.sleepTimeoutMin = prefs.getUChar("sleep_t", DEFAULT_SLEEP_TIMEOUT_MIN);
    settings.autoExprMode = prefs.getUChar("auto_expr", DEFAULT_EXPR_MODE);
}

void SettingsStorage::saveSettings(const PashaSettingsPacket& settings) {
    prefs.putUChar("f_style", settings.faceStyle);
    prefs.putUChar("c_style", settings.clockStyle);
    prefs.putUChar("w_style", settings.weatherStyle);
    prefs.putUShort("cycle_int", settings.cycleIntervalSec);
    prefs.putUChar("c_dur", settings.clockDurationSec);
    prefs.putUChar("w_dur", settings.weatherDurSec);
    prefs.putUChar("c_en", settings.clockEnabled);
    prefs.putUChar("w_en", settings.weatherEnabled);
    prefs.putUChar("bright", settings.brightness);
    prefs.putUChar("sleep_t", settings.sleepTimeoutMin);
    prefs.putUChar("auto_expr", settings.autoExprMode);
}
