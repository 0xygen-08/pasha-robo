#include "clock_screen.h"
#include <math.h>

ClockScreen& ClockScreen::getInstance() {
    static ClockScreen instance;
    return instance;
}

ClockScreen::ClockScreen() :
    hasValidTime(false),
    baseEpoch(0),
    syncLocalMillis(0),
    currentStyle(CLOCK_STYLE_BIG_DIGITAL)
{
}

void ClockScreen::begin() {
    hasValidTime = false;
}

void ClockScreen::setTime(uint32_t epoch, int16_t tzOffsetMinutes) {
    baseEpoch = epoch + (tzOffsetMinutes * 60);
    syncLocalMillis = millis();
    hasValidTime = true;
}

void ClockScreen::setStyle(ClockStyle style) {
    if (style < CLOCK_STYLE_COUNT) {
        currentStyle = style;
    }
}

void ClockScreen::getCurrentTime(uint8_t& hours, uint8_t& minutes, uint8_t& seconds) {
    if (!hasValidTime) {
        hours = 0;
        minutes = 0;
        seconds = 0;
        return;
    }

    uint32_t elapsedSec = (millis() - syncLocalMillis) / 1000;
    uint32_t currentSec = baseEpoch + elapsedSec;

    seconds = currentSec % 60;
    minutes = (currentSec / 60) % 60;
    hours = (currentSec / 3600) % 24;
}

void ClockScreen::draw(int16_t offsetX) {
    if (!hasValidTime) {
        drawNoTimeSync(offsetX);
        return;
    }

    uint8_t hours, minutes, seconds;
    getCurrentTime(hours, minutes, seconds);

    switch (currentStyle) {
        case CLOCK_STYLE_BIG_DIGITAL:
            drawBigDigital(hours, minutes, seconds, offsetX);
            break;
        case CLOCK_STYLE_DIGITAL_MINI_FACE:
            drawDigitalMiniFace(hours, minutes, seconds, offsetX);
            break;
        case CLOCK_STYLE_ANALOG:
            drawAnalog(hours, minutes, seconds, offsetX);
            break;
        case CLOCK_STYLE_RETRO_7SEG:
            drawRetro7Seg(hours, minutes, seconds, offsetX);
            break;
        default:
            drawBigDigital(hours, minutes, seconds, offsetX);
            break;
    }
}

// Saat senkronizasyonu yoksa sevimli BLE bağlan ikonu
void ClockScreen::drawNoTimeSync(int16_t offsetX) {
    U8G2& u8g2 = DisplayHAL::getInstance().getU8g2();

    int16_t cx = 64 + offsetX;
    int16_t cy = 24;

    // Bluetooth sembolü
    u8g2.drawLine(cx, cy - 14, cx, cy + 14);
    u8g2.drawLine(cx, cy, cx + 7, cy - 7);
    u8g2.drawLine(cx + 7, cy - 7, cx, cy - 14);
    u8g2.drawLine(cx, cy, cx + 7, cy + 7);
    u8g2.drawLine(cx + 7, cy + 7, cx, cy + 14);
    u8g2.drawLine(cx - 5, cy - 5, cx + 5, cy + 5);
    u8g2.drawLine(cx - 5, cy + 5, cx + 5, cy - 5);

    // Animasyonlu radyo dalgaları
    uint8_t wave = (millis() / 400) % 3;
    if (wave >= 1) {
        u8g2.drawCircle(cx, cy, 18, U8G2_DRAW_UPPER_RIGHT | U8G2_DRAW_LOWER_RIGHT);
        u8g2.drawCircle(cx, cy, 18, U8G2_DRAW_UPPER_LEFT | U8G2_DRAW_LOWER_LEFT);
    }
    if (wave >= 2) {
        u8g2.drawCircle(cx, cy, 24, U8G2_DRAW_UPPER_RIGHT | U8G2_DRAW_LOWER_RIGHT);
        u8g2.drawCircle(cx, cy, 24, U8G2_DRAW_UPPER_LEFT | U8G2_DRAW_LOWER_LEFT);
    }

    u8g2.setFont(u8g2_font_6x12_tf);
    const char* msg = "Pasha'ya Baglan";
    int16_t tw = u8g2.getStrWidth(msg);
    u8g2.drawStr(cx - (tw / 2), 56, msg);
}

// 1. Stil: Büyük Dijital Saat
void ClockScreen::drawBigDigital(uint8_t hours, uint8_t minutes, uint8_t seconds, int16_t offsetX) {
    U8G2& u8g2 = DisplayHAL::getInstance().getU8g2();

    char timeBuf[12];
    snprintf(timeBuf, sizeof(timeBuf), "%02d:%02d", hours, minutes);

    u8g2.setFont(u8g2_font_logisoso26_tn);
    int16_t tw = u8g2.getStrWidth(timeBuf);
    int16_t x = 64 - (tw / 2) - 8 + offsetX;
    int16_t y = 46;

    // Saniyeye göre iki noktanın yanıp sönmesi
    bool showColon = (seconds % 2 == 0);
    if (!showColon) {
        // İki noktayı boşlukla değiştir
        timeBuf[2] = ' ';
    }
    u8g2.drawStr(x, y, timeBuf);

    // Sağ altta minik saniye
    char secBuf[8];
    snprintf(secBuf, sizeof(secBuf), ":%02d", seconds);
    u8g2.setFont(u8g2_font_helvB10_tf);
    u8g2.drawStr(x + tw + 2, 46, secBuf);

    // Üstte zarif dekoratif çizgi
    u8g2.drawHLine(20 + offsetX, 10, 88);
}

// 2. Stil: Dijital Saat + Mini Pasha Yüzü
void ClockScreen::drawDigitalMiniFace(uint8_t hours, uint8_t minutes, uint8_t seconds, int16_t offsetX) {
    U8G2& u8g2 = DisplayHAL::getInstance().getU8g2();

    // Üstte Saat
    char timeBuf[16];
    snprintf(timeBuf, sizeof(timeBuf), "%02d:%02d:%02d", hours, minutes, seconds);
    u8g2.setFont(u8g2_font_helvB12_tf);
    int16_t tw = u8g2.getStrWidth(timeBuf);
    u8g2.drawStr(64 - (tw / 2) + offsetX, 20, timeBuf);

    // Ayırıcı çizgi
    u8g2.drawHLine(28 + offsetX, 26, 72);

    // Altta Sevimli Mini Pasha Gözleri
    int16_t eyeY = 46;
    int16_t leftEyeX = 50 + offsetX;
    int16_t rightEyeX = 78 + offsetX;

    // Arada mini göz kırpma
    bool blink = (seconds % 8 == 0);
    if (blink) {
        u8g2.drawHLine(leftEyeX - 6, eyeY, 13);
        u8g2.drawHLine(rightEyeX - 6, eyeY, 13);
    } else {
        u8g2.drawRBox(leftEyeX - 6, eyeY - 8, 12, 16, 4);
        u8g2.drawRBox(rightEyeX - 6, eyeY - 8, 12, 16, 4);

        // Sevimli minik yanaklar
        u8g2.drawLine(leftEyeX - 12, eyeY + 4, leftEyeX - 8, eyeY + 2);
        u8g2.drawLine(rightEyeX + 8, eyeY + 2, rightEyeX + 12, eyeY + 4);
    }
}

// 3. Stil: Analog Kadran
void ClockScreen::drawAnalog(uint8_t hours, uint8_t minutes, uint8_t seconds, int16_t offsetX) {
    U8G2& u8g2 = DisplayHAL::getInstance().getU8g2();

    int16_t cx = 64 + offsetX;
    int16_t cy = 32;
    int16_t radius = 28;

    // Dış çember
    u8g2.drawCircle(cx, cy, radius, U8G2_DRAW_ALL);

    // 12, 3, 6, 9 Saat işaretleri
    u8g2.drawPixel(cx, cy - radius + 2);
    u8g2.drawPixel(cx + radius - 3, cy);
    u8g2.drawPixel(cx, cy + radius - 3);
    u8g2.drawPixel(cx - radius + 2, cy);

    // Yelkovan (Dakika): 0-60 dk -> 0 - 360 derece
    float minAngle = (minutes + (seconds / 60.0f)) * (6.28318f / 60.0f) - 1.57079f;
    int16_t minLen = radius - 6;
    int16_t mx = cx + (int16_t)(cosf(minAngle) * minLen);
    int16_t my = cy + (int16_t)(sinf(minAngle) * minLen);
    u8g2.drawLine(cx, cy, mx, my);

    // Akrep (Saat): 0-12 saat -> 0 - 360 derece
    float hrAngle = ((hours % 12) + (minutes / 60.0f)) * (6.28318f / 12.0f) - 1.57079f;
    int16_t hrLen = radius - 12;
    int16_t hx = cx + (int16_t)(cosf(hrAngle) * hrLen);
    int16_t hy = cy + (int16_t)(sinf(hrAngle) * hrLen);
    // Kalın akrep
    u8g2.drawLine(cx, cy, hx, hy);
    u8g2.drawLine(cx + 1, cy, hx + 1, hy);

    // Saniye ibresi
    float secAngle = seconds * (6.28318f / 60.0f) - 1.57079f;
    int16_t sx = cx + (int16_t)(cosf(secAngle) * (radius - 4));
    int16_t sy = cy + (int16_t)(sinf(secAngle) * (radius - 4));
    u8g2.drawLine(cx, cy, sx, sy);

    // Merkez pimi
    u8g2.drawDisc(cx, cy, 2, U8G2_DRAW_ALL);
}

// 4. Stil: Retro 7-Segment Dijital Saat
void ClockScreen::draw7SegDigit(int16_t x, int16_t y, uint8_t digit, uint8_t segW, uint8_t segH) {
    U8G2& u8g2 = DisplayHAL::getInstance().getU8g2();

    // 7-segment bitmask (A, B, C, D, E, F, G)
    // Bit 0: A (üst), Bit 1: B (üst sağ), Bit 2: C (alt sağ), Bit 3: D (alt)
    // Bit 4: E (alt sol), Bit 5: F (üst sol), Bit 6: G (orta)
    static const uint8_t segMap[10] = {
        0b00111111, // 0
        0b00000110, // 1
        0b01011011, // 2
        0b01001111, // 3
        0b01100110, // 4
        0b01101101, // 5
        0b01111101, // 6
        0b00000111, // 7
        0b01111111, // 8
        0b01101111  // 9
    };

    uint8_t mask = (digit <= 9) ? segMap[digit] : 0;
    uint8_t t = 3; // Çizgi kalınlığı

    // A: Üst yatay
    if (mask & (1 << 0)) u8g2.drawBox(x + 1, y, segW - 2, t);
    // B: Üst sağ dikey
    if (mask & (1 << 1)) u8g2.drawBox(x + segW - t, y + 1, t, segH - 1);
    // C: Alt sağ dikey
    if (mask & (1 << 2)) u8g2.drawBox(x + segW - t, y + segH, t, segH - 1);
    // D: Alt yatay
    if (mask & (1 << 3)) u8g2.drawBox(x + 1, y + (segH * 2) - t, segW - 2, t);
    // E: Alt sol dikey
    if (mask & (1 << 4)) u8g2.drawBox(x, y + segH, t, segH - 1);
    // F: Üst sol dikey
    if (mask & (1 << 5)) u8g2.drawBox(x, y + 1, t, segH - 1);
    // G: Orta yatay
    if (mask & (1 << 6)) u8g2.drawBox(x + 1, y + segH - (t / 2), segW - 2, t);
}

void ClockScreen::drawRetro7Seg(uint8_t hours, uint8_t minutes, uint8_t seconds, int16_t offsetX) {
    U8G2& u8g2 = DisplayHAL::getInstance().getU8g2();

    uint8_t h1 = hours / 10;
    uint8_t h2 = hours % 10;
    uint8_t m1 = minutes / 10;
    uint8_t m2 = minutes % 10;

    uint8_t segW = 16;
    uint8_t segH = 15;
    int16_t startX = 14 + offsetX;
    int16_t startY = 17;

    draw7SegDigit(startX, startY, h1, segW, segH);
    draw7SegDigit(startX + 22, startY, h2, segW, segH);

    // İki nokta
    if (seconds % 2 == 0) {
        u8g2.drawBox(startX + 44, startY + 7, 3, 3);
        u8g2.drawBox(startX + 44, startY + 19, 3, 3);
    }

    draw7SegDigit(startX + 52, startY, m1, segW, segH);
    draw7SegDigit(startX + 74, startY, m2, segW, segH);
}
