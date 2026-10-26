# PASHA Güç ve Pil Yönetim Rehberi (BATTERY)

Pasha, anahtarlık boyutunda taşınabilir bir masaüstü/cep robotu olduğu için harici bir **3.7V LiPo batarya** ve **TP4056 USB-C şarj modülü** ile çalışacak şekilde tasarlanmıştır.

---

## 1. Güç Bağlantı Şeması

```text
   +-------------------+
   | 3.7V LiPo Batarya |
   | [B+]         [B-] |
   +---+-----------+---+
       |           |
       |           |
   +---+-----------+---------------------------+
   |  [B+]        [B-]                         |
   |           TP4056 ŞARJ MODÜLÜ              |
   |  [OUT+]     [OUT-]                        |
   +----+-----------+--------------------------+
        |           |
     [AÇ/KAPA]      |
      SWITCH        |
        |           |
        v           v
   +----+-----------+--------------------------+
   |   5V          GND                         |
   |          ESP32-C3 SUPER MINI              |
   |   3V3         GND                         |
   +----+-----------+--------------------------+
        |           |
        v           v
   +----+-----------+--------------------------+
   |   VCC         GND                         |
   |         1.3" OLED EKRAN                   |
   +-------------------------------------------+
```

> [!IMPORTANT]
> **TP4056 OUT+ Pini Nereye Bağlanmalı?**  
> Dolu bir LiPo batarya 4.2V, normal çalışma aralığında 3.7V gerilim üretir. ESP32-C3 Super Mini kartının üzerinde dahili bir 3.3V Low-Dropout (LDO) regülatör bulunmaktadır. Bu nedenle TP4056'nın **OUT+** çıkışını ESP32'nin **5V / VIN** pinine bağlayınız. LDO gerilimi temiz 3.3V'a düşürerek MCU ve OLED'i güvenle besler.

---

## 2. Akım Tüketimi Analizi

Pasha'nın firmware'i pil ömrünü maksimize etmek için dinamik güç optimizasyonuna sahiptir:

| Durum | CPU Frekansı | Ekran | BLE Durumu | Ortalama Akım |
| :--- | :--- | :--- | :--- | :--- |
| **Tam Aktif Animasyon** | 160 MHz | Açık (30 FPS) | BLE Bağlı | ~42 - 50 mA |
| **Boşta / Otonom Mod** | 80 MHz | Açık (30 FPS) | Reklam (Adv) | ~32 - 38 mA |
| **Ekran Uyku Modu** | 80 MHz | Kapalı (PowerSave) | Reklam (Adv) | ~12 - 15 mA |

Firmware içindeki optimizasyonlar:
1. **Dinamik CPU Frekansı:** Telefon/PC bağlı değilken CPU frekansı 160 MHz'den 80 MHz'e düşürülür.
2. **U8g2 PowerSave:** Uygulamadan ayarlanan süre (varsayılan 10 dakika) boyunca etkileşim veya bağlantı olmazsa ekran kapatılır.
3. **BLE Reklam Aralıkları:** Pil tüketimini minimize etmek için reklam aralığı 100-200 ms arasında tutulur.

---

## 3. Tahmini Pil Ömrü Tablosu

| Batarya Kapasitesi | Boyut (Yaklaşık) | Sürekli Aktif Ekran | %50 Aktif / %50 Uyku | Tam Uyku Modu |
| :--- | :--- | :--- | :--- | :--- |
| **150 mAh** (Minik LiPo) | 20 x 15 x 5 mm | ~3.5 saat | ~6 saat | ~10 - 12 saat |
| **300 mAh** (İdeal Cep Boyu)| 30 x 20 x 6 mm | ~7 - 8 saat | ~12 - 14 saat | ~20 - 24 saat |
| **500 mAh** (Masaüstü Stand) | 40 x 25 x 6 mm | ~12 - 13 saat | ~20 - 22 saat | ~35 - 40 saat |

---

## 4. Güvenlik ve Donanım Tavsiyeleri

- **Korumalı TP4056 Seçimi:** Aşırı deşarj (over-discharge < 2.5V) ve aşırı şarj koruması olan çift entegreli (DW01A + FS8205A) TP4056 kartlarını tercih ediniz.
- **RPROG Şarj Direnci:** TP4056 varsayılan olarak pili 1000mA (1A) ile şarj eder. Küçük boyutlu piller (örn. 150-300 mAh) için 1A çok yüksektir. TP4056 üzerindeki `R3` (RPROG) direncini 2.4kΩ (500mA) veya 4.7kΩ (250mA) ile değiştirmeniz bataryanın sağlığı ve güvenliği için önerilir.
