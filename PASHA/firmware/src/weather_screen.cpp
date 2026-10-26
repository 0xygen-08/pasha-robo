#include "weather_screen.h"
#include <math.h>

WeatherScreen& WeatherScreen::getInstance() {
    static WeatherScreen instance;
    return instance;
}

WeatherScreen::WeatherScreen() :
    hasValidWeather(false),
    currentTemp(22),
    currentWeatherCode(WEATHER_CLEAR),
    tempMin(18),
    tempMax(26),
    currentHumidity(50),
    lastUpdateEpoch(0),
    currentStyle(WEATHER_STYLE_BIG_ICON)
{
}

void WeatherScreen::begin() {
    hasValidWeather = false;
}

void WeatherScreen::setWeatherData(int8_t temp, uint8_t weatherCode, int8_t minTemp, int8_t maxTemp, uint8_t humidity) {
    currentTemp = temp;
    currentWeatherCode = (weatherCode <= WEATHER_FOG) ? static_cast<WeatherCode>(weatherCode) : WEATHER_CLEAR;
    tempMin = minTemp;
    tempMax = maxTemp;
    currentHumidity = humidity;
    lastUpdateEpoch = millis() / 1000;
    hasValidWeather = true;
}

bool WeatherScreen::isStale() const {
    if (!hasValidWeather) return false;
    uint32_t elapsedSec = (millis() / 1000) - lastUpdateEpoch;
    return (elapsedSec >= WEATHER_STALE_THRESHOLD_SEC);
}

void WeatherScreen::setStyle(WeatherStyle style) {
    if (style < WEATHER_STYLE_COUNT) {
        currentStyle = style;
    }
}

const char* WeatherScreen::getWeatherName(WeatherCode code) {
    switch (code) {
        case WEATHER_CLEAR:       return "Gunesli";
        case WEATHER_FEW_CLOUDS:  return "Parcali";
        case WEATHER_CLOUDY:      return "Bulutlu";
        case WEATHER_RAIN:        return "Yagmurlu";
        case WEATHER_SNOW:        return "Kar Yagisli";
        case WEATHER_THUNDER:     return "Firtinali";
        case WEATHER_FOG:         return "Sisli";
        default:                  return "Hava";
    }
}

void WeatherScreen::draw(int16_t offsetX) {
    if (!hasValidWeather) {
        drawNoWeatherData(offsetX);
        return;
    }

    switch (currentStyle) {
        case WEATHER_STYLE_BIG_ICON:
            drawBigIconStyle(offsetX);
            break;
        case WEATHER_STYLE_ANIMATED_ICON:
            drawAnimatedIconStyle(offsetX);
            break;
        case WEATHER_STYLE_COMPACT_TEXT:
            drawCompactTextStyle(offsetX);
            break;
        default:
            drawBigIconStyle(offsetX);
            break;
    }

    // Bayat veri uyarısı (3 saatten eskiyse sağ üstte yanıp sönen nokta)
    if (isStale()) {
        U8G2& u8g2 = DisplayHAL::getInstance().getU8g2();
        if ((millis() / 500) % 2 == 0) {
            u8g2.drawDisc(124 + offsetX, 4, 2, U8G2_DRAW_ALL);
        }
    }
}

// Hava verisi yoksa şirin Pasha "?_?" ifadesi
void WeatherScreen::drawNoWeatherData(int16_t offsetX) {
    U8G2& u8g2 = DisplayHAL::getInstance().getU8g2();

    int16_t cx = 64 + offsetX;

    // Şaşkın/meraklı mini gözler
    u8g2.drawCircle(cx - 16, 24, 7, U8G2_DRAW_ALL);
    u8g2.drawCircle(cx + 16, 24, 7, U8G2_DRAW_ALL);
    u8g2.drawDisc(cx - 16, 24, 2, U8G2_DRAW_ALL);
    u8g2.drawDisc(cx + 16, 24, 2, U8G2_DRAW_ALL);

    // Minik meraklı ağız '_'
    u8g2.drawHLine(cx - 6, 32, 12);

    u8g2.setFont(u8g2_font_helvB10_tf);
    const char* msg = "Hava Verisi Yok";
    int16_t tw = u8g2.getStrWidth(msg);
    u8g2.drawStr(cx - (tw / 2), 52, msg);

    u8g2.setFont(u8g2_font_5x7_tf);
    const char* sub = "Telefondan guncelleyin";
    int16_t sw = u8g2.getStrWidth(sub);
    u8g2.drawStr(cx - (sw / 2), 62, sub);
}

// 1. Stil: Büyük İkon + Derece
void WeatherScreen::drawBigIconStyle(int16_t offsetX) {
    U8G2& u8g2 = DisplayHAL::getInstance().getU8g2();

    // Sol tarafta statik ikon (cx = 36, cy = 32)
    drawWeatherIcon(currentWeatherCode, 36 + offsetX, 32, false);

    // Sağ tarafta büyük sıcaklık
    char tempBuf[8];
    snprintf(tempBuf, sizeof(tempBuf), "%d", currentTemp);

    u8g2.setFont(u8g2_font_logisoso26_tn);
    u8g2.drawStr(70 + offsetX, 38, tempBuf);

    int16_t tw = u8g2.getStrWidth(tempBuf);
    u8g2.setFont(u8g2_font_helvB10_tf);
    u8g2.drawStr(72 + tw + offsetX, 24, "C");
    u8g2.drawCircle(72 + tw - 3 + offsetX, 16, 2, U8G2_DRAW_ALL);

    // Min / Max aralığı
    char minMaxBuf[16];
    snprintf(minMaxBuf, sizeof(minMaxBuf), "%d / %d", tempMin, tempMax);
    u8g2.setFont(u8g2_font_6x12_tf);
    u8g2.drawStr(72 + offsetX, 54, minMaxBuf);
}

// 2. Stil: Hareketli İkon + Derece
void WeatherScreen::drawAnimatedIconStyle(int16_t offsetX) {
    U8G2& u8g2 = DisplayHAL::getInstance().getU8g2();

    // Sol tarafta hareketli hava ikonu
    drawWeatherIcon(currentWeatherCode, 36 + offsetX, 32, true);

    // Sağ tarafta sıcaklık ve durum adı
    char tempBuf[8];
    snprintf(tempBuf, sizeof(tempBuf), "%d", currentTemp);

    u8g2.setFont(u8g2_font_logisoso24_tn);
    u8g2.drawStr(70 + offsetX, 34, tempBuf);

    int16_t tw = u8g2.getStrWidth(tempBuf);
    u8g2.setFont(u8g2_font_helvB10_tf);
    u8g2.drawStr(72 + tw + offsetX, 22, "C");
    u8g2.drawCircle(72 + tw - 3 + offsetX, 14, 2, U8G2_DRAW_ALL);

    // Hava durumu adı (Güneşli, Yağmurlu vb.)
    u8g2.setFont(u8g2_font_6x12_tf);
    u8g2.drawStr(70 + offsetX, 52, getWeatherName(currentWeatherCode));
}

// 3. Stil: Kompakt Metin Satırı
void WeatherScreen::drawCompactTextStyle(int16_t offsetX) {
    U8G2& u8g2 = DisplayHAL::getInstance().getU8g2();

    int16_t cx = 64 + offsetX;

    // Üstte Durum Başlığı
    u8g2.setFont(u8g2_font_helvB10_tf);
    const char* name = getWeatherName(currentWeatherCode);
    int16_t nw = u8g2.getStrWidth(name);
    u8g2.drawStr(cx - (nw / 2), 16, name);

    // Ortada Büyük Sıcaklık
    char tempBuf[12];
    snprintf(tempBuf, sizeof(tempBuf), "%d C", currentTemp);
    u8g2.setFont(u8g2_font_logisoso22_tn);
    int16_t tw = u8g2.getStrWidth(tempBuf);
    u8g2.drawStr(cx - (tw / 2) - 4, 42, tempBuf);
    u8g2.drawCircle(cx + (tw / 2) - 20, 26, 2, U8G2_DRAW_ALL);

    // Altta Min/Max ve Nem
    char detailsBuf[36];
    snprintf(detailsBuf, sizeof(detailsBuf), "Min:%d Max:%d Nem:%%%d", tempMin, tempMax, currentHumidity);
    u8g2.setFont(u8g2_font_5x8_tf);
    int16_t dw = u8g2.getStrWidth(detailsBuf);
    u8g2.drawStr(cx - (dw / 2), 58, detailsBuf);
}

// ==========================================
// HAVA DURUMU İKON ÇİZİMLERİ & ANİMASYONLARI
// ==========================================
void WeatherScreen::drawSunIcon(int16_t cx, int16_t cy, bool animated) {
    U8G2& u8g2 = DisplayHAL::getInstance().getU8g2();

    // Merkez güneş diski
    u8g2.drawDisc(cx, cy, 8, U8G2_DRAW_ALL);

    // Işınlar (8 adet)
    float baseAngle = animated ? (millis() * 0.002f) : 0.0f;
    for (int i = 0; i < 8; i++) {
        float angle = baseAngle + (i * 0.785398f); // 45 derece aralık
        int16_t r1 = 11;
        int16_t r2 = 16;
        int16_t x1 = cx + (int16_t)(cosf(angle) * r1);
        int16_t y1 = cy + (int16_t)(sinf(angle) * r1);
        int16_t x2 = cx + (int16_t)(cosf(angle) * r2);
        int16_t y2 = cy + (int16_t)(sinf(angle) * r2);
        u8g2.drawLine(x1, y1, x2, y2);
    }
}

void WeatherScreen::drawCloudIcon(int16_t cx, int16_t cy) {
    U8G2& u8g2 = DisplayHAL::getInstance().getU8g2();

    // Sevimli pufidik bulut
    u8g2.drawDisc(cx - 8, cy + 2, 7, U8G2_DRAW_ALL);
    u8g2.drawDisc(cx + 2, cy - 3, 9, U8G2_DRAW_ALL);
    u8g2.drawDisc(cx + 11, cy + 3, 6, U8G2_DRAW_ALL);
    u8g2.drawBox(cx - 8, cy + 2, 19, 7);
}

void WeatherScreen::drawRainIcon(int16_t cx, int16_t cy, bool animated) {
    U8G2& u8g2 = DisplayHAL::getInstance().getU8g2();

    // Üstte bulut
    drawCloudIcon(cx, cy - 6);

    // Altta yağmur damlaları
    int16_t animOffset = animated ? ((millis() / 50) % 8) : 0;
    for (int i = -1; i <= 1; i++) {
        int16_t rx = cx + (i * 9);
        int16_t ry = cy + 8 + animOffset;
        u8g2.drawLine(rx, ry, rx - 2, ry + 4);
    }
}

void WeatherScreen::drawSnowIcon(int16_t cx, int16_t cy, bool animated) {
    U8G2& u8g2 = DisplayHAL::getInstance().getU8g2();

    drawCloudIcon(cx, cy - 6);

    // Kar taneleri
    int16_t animOffset = animated ? ((millis() / 70) % 8) : 0;
    for (int i = -1; i <= 1; i++) {
        int16_t sx = cx + (i * 9);
        int16_t sy = cy + 9 + animOffset;
        u8g2.drawPixel(sx, sy);
        u8g2.drawPixel(sx - 1, sy);
        u8g2.drawPixel(sx + 1, sy);
        u8g2.drawPixel(sx, sy - 1);
        u8g2.drawPixel(sx, sy + 1);
    }
}

void WeatherScreen::drawThunderIcon(int16_t cx, int16_t cy, bool animated) {
    U8G2& u8g2 = DisplayHAL::getInstance().getU8g2();

    drawCloudIcon(cx, cy - 6);

    // Yıldırım / şimşek oku
    bool flash = animated ? ((millis() / 150) % 2 == 0) : true;
    if (flash) {
        u8g2.drawLine(cx + 2, cy + 5, cx - 3, cy + 12);
        u8g2.drawLine(cx - 3, cy + 12, cx + 1, cy + 12);
        u8g2.drawLine(cx + 1, cy + 12, cx - 2, cy + 20);
    }
}

void WeatherScreen::drawFogIcon(int16_t cx, int16_t cy) {
    U8G2& u8g2 = DisplayHAL::getInstance().getU8g2();

    for (int i = -2; i <= 2; i++) {
        int16_t fy = cy + (i * 5);
        int16_t fw = (i % 2 == 0) ? 28 : 20;
        u8g2.drawHLine(cx - (fw / 2), fy, fw);
    }
}

void WeatherScreen::drawWeatherIcon(WeatherCode code, int16_t cx, int16_t cy, bool animated) {
    // Bulutun hafifçe salınması
    if (animated && (code == WEATHER_CLOUDY || code == WEATHER_FEW_CLOUDS)) {
        cx += (int16_t)(sinf(millis() * 0.003f) * 3.0f);
    }

    switch (code) {
        case WEATHER_CLEAR:
            drawSunIcon(cx, cy, animated);
            break;
        case WEATHER_FEW_CLOUDS:
            drawSunIcon(cx - 5, cy - 4, animated);
            drawCloudIcon(cx + 4, cy + 4);
            break;
        case WEATHER_CLOUDY:
            drawCloudIcon(cx, cy);
            break;
        case WEATHER_RAIN:
            drawRainIcon(cx, cy, animated);
            break;
        case WEATHER_SNOW:
            drawSnowIcon(cx, cy, animated);
            break;
        case WEATHER_THUNDER:
            drawThunderIcon(cx, cy, animated);
            break;
        case WEATHER_FOG:
            drawFogIcon(cx, cy);
            break;
        default:
            drawSunIcon(cx, cy, animated);
            break;
    }
}
