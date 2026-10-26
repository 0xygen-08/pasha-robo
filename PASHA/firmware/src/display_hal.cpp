#include "display_hal.h"

DisplayHAL& DisplayHAL::getInstance() {
    static DisplayHAL instance;
    return instance;
}

DisplayHAL::DisplayHAL() :
#if defined(USE_SH1106)
    u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE)
#elif defined(USE_SSD1306)
    u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE)
#endif
{
}

bool DisplayHAL::begin() {
    // I2C pinlerini ESP32-C3 Super Mini yapılandırmasına göre başlat
    Wire.begin(OLED_SDA_PIN, OLED_SCL_PIN, OLED_I2C_FREQ);

    u8g2.begin();
    u8g2.setContrast(DEFAULT_BRIGHTNESS);
    u8g2.setBusClock(OLED_I2C_FREQ);
    u8g2.clearBuffer();
    u8g2.sendBuffer();
    return true;
}

void DisplayHAL::clear() {
    u8g2.clearBuffer();
}

void DisplayHAL::display() {
    u8g2.sendBuffer();
}

void DisplayHAL::setContrast(uint8_t contrast) {
    u8g2.setContrast(contrast);
}

void DisplayHAL::setPowerSave(bool isPowerSave) {
    u8g2.setPowerSave(isPowerSave ? 1 : 0);
}

// Sevimli kalp çizim fonksiyonu (Nabız ve Aşık ifadesi için)
void DisplayHAL::drawSmoothHeart(int16_t cx, int16_t cy, float scale) {
    if (scale <= 0.1f) return;

    // Kalp boyutu scale ile ölçeklenir (varsayılan yarıçap ~8-12px)
    int16_t r = (int16_t)(7.0f * scale);
    if (r < 2) r = 2;

    int16_t leftCx = cx - r;
    int16_t rightCx = cx + r;
    int16_t topCy = cy - (int16_t)(3.0f * scale);

    // İki üst daire
    u8g2.drawDisc(leftCx, topCy, r, U8G2_DRAW_ALL);
    u8g2.drawDisc(rightCx, topCy, r, U8G2_DRAW_ALL);

    // Alt üçgen gövdesi
    int16_t bottomY = cy + (int16_t)(10.0f * scale);
    int16_t leftX = leftCx - r;
    int16_t rightX = rightCx + r;

    u8g2.drawTriangle(leftX, topCy, rightX, topCy, cx, bottomY);
    // Dolgu boşluğunu kapatmak için iç üçgenler
    u8g2.drawTriangle(leftX + 1, topCy, rightX - 1, topCy, cx, bottomY);
    u8g2.drawBox(leftX + 2, topCy, (rightX - leftX) - 3, (int16_t)(4.0f * scale));
}

// Kalın kaş çizimi
void DisplayHAL::drawEyeBrow(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint8_t thickness) {
    for (uint8_t i = 0; i < thickness; i++) {
        u8g2.drawLine(x1, y1 + i, x2, y2 + i);
    }
}
