# PASHA Donanım Bağlantı Rehberi (WIRING)

Bu dokümanda **ESP32 Super Mini (ESP32-C3)** ve **1.3" I2C OLED (SH1106 / SSD1306)** ekranın pin bağlantıları detaylandırılmıştır.

---

## 1. Bağlantı Tablosu

| OLED Ekran Pini | ESP32-C3 Super Mini Pini | Açıklama |
| :--- | :--- | :--- |
| **VCC** | **3V3** (3.3V) | Ekran beslemesi (ESP32 regülatör çıkışı) |
| **GND** | **GND** | Ortak toprak hattı |
| **SDA** | **GPIO 8** | I2C Veri Hattı (Data) |
| **SCL** | **GPIO 9** | I2C Saat Hattı (Clock) |

> [!CAUTION]
> 1.3" OLED ekranınızı asla ESP32'nin **5V** hattına doğrudan bağlamayınız. Bazı OLED kartlarında 3.3V regülatör olsa bile ESP32-C3'ün GPIO pinleri 3.3V toleranslıdır. Her zaman **3V3** pinini kullanınız.

---

## 2. ESP32-C3 Super Mini Pinout Görünümü

```text
               +-------------------+
         3V3 --| [ ]           [ ] |-- 5V
         GND --| [ ]   ESP32   [ ] |-- GND
       GPIO8 --| [ ]   - C3    [ ] |-- GPIO7
       GPIO9 --| [ ]   SUPER   [ ] |-- GPIO6
      GPIO10 --| [ ]   MINI    [ ] |-- GPIO5
      GPIO20 --| [ ]           [ ] |-- GPIO4
      GPIO21 --| [ ]   [USB]   [ ] |-- GPIO3
               +-------------------+
```

---

## 3. ASCII Bağlantı Şeması

```text
   +-----------------------+              +--------------------+
   | 1.3" OLED (128x64)    |              | ESP32-C3 SuperMini |
   |                       |              |                    |
   | [VCC] ----------------+--------------+--> 3V3             |
   | [GND] ----------------+--------------+--> GND             |
   | [SDA] ----------------+--------------+--> GPIO 8          |
   | [SCL] ----------------+--------------+--> GPIO 9          |
   +-----------------------+              +--------------------+
```

---

## 4. Pinleri ve Ekran Tipini Değiştirme

Farklı bir pin kombinasyonu veya SSD1306 sürücüsü kullanmak isterseniz, `firmware/include/config.h` dosyasındaki ilgili satırları düzenleyebilirsiniz:

```c
// I2C Pin Tanımları
#define OLED_SDA_PIN 8
#define OLED_SCL_PIN 9
#define OLED_I2C_FREQ 400000 // 400 kHz Fast I2C

// Ekran Sürücüsü Seçimi (Biri aktif olmalı)
#define USE_SH1106
// #define USE_SSD1306
```

---

## 5. Donanım İpuçları & Dikkat Edilmesi Gerekenler

1. **Dahili Pull-up Dirençleri:** Çoğu 1.3" I2C modülünde SDA ve SCL hatları üzerinde 4.7kΩ veya 10kΩ pull-up direnci zaten yer alır. Ekstra direnç bağlamanıza gerek yoktur.
2. **Kablo Uzunluğu:** Robot anahtarlık boyutunda tasarlandığı için kabloları mümkün olduğunca kısa (3-5 cm) tutmanız tavsiye edilir. Uzun kablolar yüksek frekanslı I2C (400kHz) sinyalinde parazite yol açabilir.
3. **Lehimleme:** ESP32 Super Mini'nin alt tarafındaki pinleri lehimlerken ekranın arkasına temas etmemesi için araya kapton bant veya ince bir izolasyon köpüğü yerleştiriniz.
