#ifndef PASHA_WEATHER_SCREEN_H
#define PASHA_WEATHER_SCREEN_H

#include <Arduino.h>
#include "types.h"
#include "config.h"
#include "display_hal.h"

class WeatherScreen {
public:
    static WeatherScreen& getInstance();

    void begin();
    void setWeatherData(int8_t temp, uint8_t weatherCode, int8_t minTemp, int8_t maxTemp, uint8_t humidity);
    bool hasData() const { return hasValidWeather; }
    bool isStale() const;

    void setStyle(WeatherStyle style);
    WeatherStyle getStyle() const { return currentStyle; }

    void draw(int16_t offsetX = 0);

private:
    WeatherScreen();
    ~WeatherScreen() = default;

    void drawNoWeatherData(int16_t offsetX);
    void drawBigIconStyle(int16_t offsetX);
    void drawAnimatedIconStyle(int16_t offsetX);
    void drawCompactTextStyle(int16_t offsetX);

    // İkon çizim yardımcıları
    void drawSunIcon(int16_t cx, int16_t cy, bool animated);
    void drawCloudIcon(int16_t cx, int16_t cy);
    void drawRainIcon(int16_t cx, int16_t cy, bool animated);
    void drawSnowIcon(int16_t cx, int16_t cy, bool animated);
    void drawThunderIcon(int16_t cx, int16_t cy, bool animated);
    void drawFogIcon(int16_t cx, int16_t cy);

    void drawWeatherIcon(WeatherCode code, int16_t cx, int16_t cy, bool animated);
    const char* getWeatherName(WeatherCode code);

    bool hasValidWeather;
    int8_t currentTemp;
    WeatherCode currentWeatherCode;
    int8_t tempMin;
    int8_t tempMax;
    uint8_t currentHumidity;
    uint32_t lastUpdateEpoch;

    WeatherStyle currentStyle;
};

#endif // PASHA_WEATHER_SCREEN_H
