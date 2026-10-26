#ifndef PASHA_TYPES_H
#define PASHA_TYPES_H

#include <Arduino.h>

// 1. Duygu İfadeleri
enum ExpressionId : uint8_t {
    EXPR_NORMAL = 0,     // Normal / Nötr sevimli bakış
    EXPR_HAPPY = 1,      // Mutlu (hilal şeklinde gözler)
    EXPR_SWEET = 2,      // Tatlı / Sevecen (hafif mahcup, allıklı)
    EXPR_ANGRY = 3,      // Kızgın (çatık kaşlar)
    EXPR_GRUMPY = 4,     // Sinirli / Huysuz (düz kaş, dar gözler)
    EXPR_SURPRISED = 5,  // Şaşırmış (geniş açık yuvarlak gözler, kalkık kaş)
    EXPR_IN_LOVE = 6,    // Aşık (kalp şeklinde gözler + hafif kalp atışı)
    EXPR_SAD = 7,        // Üzgün (aşağı eğimli gözler / gözyaşı damlası)
    EXPR_SLEEPY = 8,     // Uykulu (yarı kapalı, Zzz balonu)
    EXPR_EXCITED = 9,    // Sevinçli / Heyecanlı (yıldız parıltılı / zıplayan)
    EXPR_SUSPICIOUS = 10,// Şüpheli (biri kısık, diğeri açık göz)
    EXPR_WINK = 11,      // Göz kırpma (tek göz kapalı, tebessüm)
    EXPR_COUNT = 12
};

// 2. Yüz Stilleri
enum FaceStyle : uint8_t {
    FACE_STYLE_DOTS = 0,        // Minimal yuvarlak nokta gözler
    FACE_STYLE_ROUND_RECT = 1,  // Klasik robot (oval/yuvarlatılmış dikdörtgen + kaş)
    FACE_STYLE_KAWAII = 2,     // Kawaii: büyük gözler + parlama + allık + sevimli ağız
    FACE_STYLE_PIXEL = 3,       // Retro piksel / 8-bit robot gözleri
    FACE_STYLE_COUNT = 4
};

// 3. Saat Stilleri
enum ClockStyle : uint8_t {
    CLOCK_STYLE_BIG_DIGITAL = 0,        // Büyük dijital saat
    CLOCK_STYLE_DIGITAL_MINI_FACE = 1,  // Dijital saat + mini Pasha yüzü
    CLOCK_STYLE_ANALOG = 2,             // Analog akrep-yelkovan kadranı
    CLOCK_STYLE_RETRO_7SEG = 3,         // Retro 7-segment dijital saat
    CLOCK_STYLE_COUNT = 4
};

// 4. Hava Durumu Stilleri
enum WeatherStyle : uint8_t {
    WEATHER_STYLE_BIG_ICON = 0,         // Büyük ikon + derece
    WEATHER_STYLE_ANIMATED_ICON = 1,    // Hareketli ikon (ışınlar, damlalar) + derece
    WEATHER_STYLE_COMPACT_TEXT = 2,     // Kompakt satır (derece, min/max, nem)
    WEATHER_STYLE_COUNT = 3
};

// 5. Hava Durumu Kodları (WMO standart eşlemesi)
enum WeatherCode : uint8_t {
    WEATHER_CLEAR = 0,          // Güneşli / Açık
    WEATHER_FEW_CLOUDS = 1,     // Parçalı Bulutlu
    WEATHER_CLOUDY = 2,         // Bulutlu / Kapalı
    WEATHER_RAIN = 3,           // Yağmurlu / Çisenti
    WEATHER_SNOW = 4,           // Karlı
    WEATHER_THUNDER = 5,        // Fırtınalı / Şimşekli
    WEATHER_FOG = 6,            // Sisli / Puslu
    WEATHER_UNKNOWN = 255       // Veri yok
};

// 6. Bakış Yönü
enum GazeDirection : uint8_t {
    GAZE_CENTER = 0,
    GAZE_LEFT = 1,
    GAZE_RIGHT = 2,
    GAZE_UP = 3,
    GAZE_DOWN = 4,
    GAZE_UP_LEFT = 5,
    GAZE_UP_RIGHT = 6,
    GAZE_DOWN_LEFT = 7,
    GAZE_DOWN_RIGHT = 8
};

// 7. Ekran Durumları (Scheduler)
enum ScreenState : uint8_t {
    SCREEN_STATE_FACE = 0,
    SCREEN_STATE_CLOCK = 1,
    SCREEN_STATE_WEATHER = 2,
    SCREEN_STATE_TRANSITION = 3,
    SCREEN_STATE_SLEEP = 4
};

// 8. Paket Veri Yapıları (Binary BLE uyumlu)
#pragma pack(push, 1)

// TIME Characteristic (6 byte)
struct PashaTimePacket {
    uint32_t epoch;          // Unix epoch saniyesi (UTC)
    int16_t tzOffsetMinutes; // Dakika cinsinden ofset (+180 = UTC+3)
};

// WEATHER Characteristic (5 byte)
struct PashaWeatherPacket {
    int8_t temperature;      // °C (-50 .. 60)
    uint8_t weatherCode;     // WeatherCode enum
    int8_t tempMin;          // Günün en düşük °C
    int8_t tempMax;          // Günün en yüksek °C
    uint8_t humidity;        // Bağıl nem % (0-100)
};

// EXPRESSION Characteristic (2 byte)
struct PashaExpressionPacket {
    uint8_t expressionId;    // ExpressionId enum (0-11)
    uint8_t mode;            // 0: Manuel (sabit), 1: Otomatik rastgele
};

// SETTINGS Characteristic (11 byte)
struct PashaSettingsPacket {
    uint8_t faceStyle;           // 0-3
    uint8_t clockStyle;          // 0-3
    uint8_t weatherStyle;        // 0-2
    uint16_t cycleIntervalSec;   // Döngü aralığı (sn)
    uint8_t clockDurationSec;    // Saat süresi (sn)
    uint8_t weatherDurSec;       // Hava süresi (sn)
    uint8_t clockEnabled;        // 1: Açık, 0: Kapalı
    uint8_t weatherEnabled;      // 1: Açık, 0: Kapalı
    uint8_t brightness;          // 0-255 (U8g2 kontrast)
    uint8_t sleepTimeoutMin;     // 0-60 dk (0 = uyuma yok)
    uint8_t autoExprMode;        // 1: Otomatik duygu modu, 0: Manuel mod
};

// STATUS Characteristic (7 byte)
struct PashaStatusPacket {
    uint8_t fwMajor;
    uint8_t fwMinor;
    uint8_t isConnected;         // 1: BLE bağlı, 0: Boşta
    uint8_t hasTime;             // 1: Saat senkronize, 0: Yok
    uint8_t hasWeather;          // 1: Hava durumu verisi var, 0: Yok
    uint8_t currentExpression;   // Anlık gösterilen duygu
    uint8_t currentScreen;       // ScreenState enum
};

#pragma pack(pop)

// Parametrik Göz Yapısı (Interpolasyon için)
struct EyeParameters {
    float x;             // Merkez X (piksel)
    float y;             // Merkez Y (piksel)
    float width;         // Genişlik (piksel)
    float height;        // Yükseklik (piksel)
    float radius;        // Köşe yuvarlama yarıçapı
    float eyelidTop;     // Üst göz kapağı kapanma oranı (0.0 açık, 1.0 tam kapalı)
    float eyelidBottom;  // Alt göz kapağı yükselme oranı (0.0 açık, 1.0 tam kapalı)
    float pupilOffsetX;  // Gözbebeği / bakış ofseti X
    float pupilOffsetY;  // Gözbebeği / bakış ofseti Y
    float browAngle;     // Kaş açısı (-30 .. +30 derece)
    float browOffsetY;   // Kaş dikey ofseti
    bool isHeart;        // Kalp göz mü?
    bool isSquint;       // Çizgi şeklinde kısık mı?
    bool isWink;         // Göz kırpık mı?
};

#endif // PASHA_TYPES_H
