#ifndef PASHA_CONFIG_H
#define PASHA_CONFIG_H

#include <Arduino.h>

// ==========================================
// 1. DONANIM & PIN TANIMLARI
// ==========================================
// ESP32-C3 Super Mini varsayılan I2C pinleri
#define OLED_SDA_PIN 8
#define OLED_SCL_PIN 9
#define OLED_I2C_FREQ 400000 // 400 kHz Fast I2C

// Ekran sürücüsü seçimi:
// Varsayılan: 1.3" OLED için SH1106
// Eğer 0.96" veya SSD1306 bazlı 1.3" kullanıyorsanız, USE_SH1106'yı kapatıp USE_SSD1306'yı açın.
#define USE_SH1106
// #define USE_SSD1306

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define TARGET_FPS 30
#define FRAME_TIME_MS (1000 / TARGET_FPS)

// ==========================================
// 2. BLE YAPILANDIRMASI & UUID'LER
// ==========================================
#define BLE_DEVICE_NAME "Pasha"

// Pasha Özel GATT Servis & Karakteristik UUID'leri
#define PASHA_SERVICE_UUID         "0000beef-7273-6861-6e64-657669636531"
#define CHAR_TIME_UUID             "0000bee1-7273-6861-6e64-657669636531"
#define CHAR_WEATHER_UUID          "0000bee2-7273-6861-6e64-657669636531"
#define CHAR_EXPRESSION_UUID       "0000bee3-7273-6861-6e64-657669636531"
#define CHAR_SETTINGS_UUID         "0000bee4-7273-6861-6e64-657669636531"
#define CHAR_STATUS_UUID           "0000bee5-7273-6861-6e64-657669636531"

// ==========================================
// 3. VARSAYILAN YAZILIM AYARLARI
// ==========================================
#define FIRMWARE_VERSION_MAJOR 1
#define FIRMWARE_VERSION_MINOR 0

#define DEFAULT_FACE_STYLE         1   // RoundRect (Klasik Robot)
#define DEFAULT_CLOCK_STYLE        0   // Big Digital
#define DEFAULT_WEATHER_STYLE      0   // Big Icon + Temp
#define DEFAULT_CYCLE_INTERVAL_SEC 30  // Ekranlar arası döngü periyodu (sn)
#define DEFAULT_CLOCK_DURATION_SEC 5   // Saatin ekranda kalma süresi (sn)
#define DEFAULT_WEATHER_DUR_SEC    5   // Hava durumunun ekranda kalma süresi (sn)
#define DEFAULT_CLOCK_ENABLED      1   // 1: Açık, 0: Kapalı
#define DEFAULT_WEATHER_ENABLED    1   // 1: Açık, 0: Kapalı
#define DEFAULT_BRIGHTNESS         255 // 0-255 U8g2 Kontrast
#define DEFAULT_SLEEP_TIMEOUT_MIN  10  // Ekran otomatik uyku süresi (dakika, 0=kapalı)
#define DEFAULT_EXPR_MODE          1   // 1: Otomatik duygu modu, 0: Manuel mod

// Otomatik mod rastgele duygu değişim süreleri (ms)
#define AUTO_EXPR_MIN_MS 5000
#define AUTO_EXPR_MAX_MS 15000

// Canlılık / Kırpma / Bakış süreleri (ms)
#define BLINK_INTERVAL_MIN_MS 2500
#define BLINK_INTERVAL_MAX_MS 6000
#define BLINK_DURATION_MS 160

#define GAZE_INTERVAL_MIN_MS 3000
#define GAZE_INTERVAL_MAX_MS 8000

// Hava verisi bayatlama eşiği (3 saat = 10800 saniye)
#define WEATHER_STALE_THRESHOLD_SEC 10800

#endif // PASHA_CONFIG_H
