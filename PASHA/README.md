# PASHA – Anahtarlık Boyutunda Sevimli OLED Robot 🤖✨

Pasha, anahtarlık veya cep boyutunda tasarlanmış, minimalist ve son derece sevimli bir masaüstü robotudur. Tek arayüzü olan 1.3" OLED ekranında canlı, organik hareket eden ve duygu durumları olan bir yüz barındırır. Kendi kendine göz kırpar, merakla etrafına bakar, nefes alır, aşık olduğunda gözleri kalbe dönüşüp çarpar ve belirli aralıklarla telefon/bilgisayardan aldığı güncel saat ve hava durumu bilgilerini şık animasyonlarla gösterir.

Tüm ayarları, telefon veya bilgisayardan hiçbir uygulama yüklemeden doğrudan tarayıcı üzerinden çalışan **Web Bluetooth tabanlı PWA (Progressive Web App)** kontrol paneliyle yapılır.

---

## 📌 İçindekiler
1. [Özellikler](#-özellikler)
2. [Donanım ve Bağlantı Şeması](#-donanım-ve-bağlantı-şeması)
3. [Firmware Kurulumu ve Yükleme](#-firmware-kurulumu-ve-yükleme)
4. [Kontrol Uygulamasını Açma ve Yayınlama](#-kontrol-uygulamasını-açma-ve-yayınlama)
5. [BLE Protokolü ve Karakteristikler](#-ble-protokolü-ve-karakteristikler)
6. [Sorun Giderme](#-sorun-giderme)
7. [Varsayımlar](#-varsayımlar)

---

## ✨ Özellikler

- **Canlı ve Sevimli Yüz Motoru:**
  - **12 Duygu İfadesi:** Normal, Mutlu, Tatlı/Sevecen, Kızgın, Huysuz, Şaşırmış, Aşık (kalp gözler + nabız atışı animasyonu), Üzgün, Uykulu (uçuşan Zzz harfleri), Heyecanlı, Şüpheli ve Göz Kırpma.
  - **Canlılık Davranışları:** Doğal aralıklarla göz kırpma (blink), farklı yönlere bakma (gaze wandering), organik nefes alma salınımı, sürpriz göz devirme animasyonu.
  - **4 Yüz Stili:** Minimal Nokta Gözler, Klasik Robot (Oval/Kaşlı), Kawaii (Büyük göz, çift parlama, yanak allıkları, kedi ağzı), Retro 8-bit Piksel Gözler.
  - **Pürüzsüz İnterpolasyon:** Tüm ifadeler ve göz pozisyonları arasında 30 FPS akıcı ease-out geçişi.
  - **Otomatik / Manuel Mod:** 5-15 saniyede bir rastgele duygu değişimi veya uygulamadan seçilen sabit ifade.
- **Akıllı Zamanlayıcı (Scheduler):**
  - Ayarlanabilir döngü: Varsayılan her 30 saniyede bir 5 sn saat, ardından 5 sn hava durumu gösterilir ve yüze dönülür.
  - Ekranlar arasında yatay kaydırma (horizontal slide) geçiş animasyonu.
  - **4 Saat Stili:** Büyük Dijital, Mini Pasha Yüzlü Saat, Analog Kadran, Nostaljik 7-Segment.
  - **3 Hava Durumu Stili:** Büyük İkon + Derece, Hareketli İkon (dönen güneş ışınları, düşen yağmur damlaları, salınan bulutlar) + Derece, Kompakt Bilgi Satırı.
  - 3 saatten eski hava verilerinde sağ üstte bayat veri uyarı noktası.
- **Güç Yönetimi:**
  - U8g2 kontrastı ile parlaklık kontrolü.
  - Belirlenen süre (örn. 10 dk) boyunca bağlantı/hareket olmazsa ekran uyku moduna geçer.
  - BLE bağlı değilken CPU frekansı 80 MHz'e düşürülerek pil ömrü uzatılır.
- **Web Bluetooth PWA:**
  - HTML5 Canvas üzerinde 128x64 çözünürlükte firmware'in çizim mantığını birebir simüle eden canlı önizleme.
  - Open-Meteo API entegrasyonu (GPS Geolocation veya şehir arama ile API keysiz anlık hava durumu).
  - Bağlanınca anlık saat ve zaman dilimini otomatik senkronize etme.
  - Kalıcı ayarları (NVS) uzaktan okuma ve yazma.

---

## 🛠️ Donanım ve Bağlantı Şeması

Pasha'nın donanım listesi son derece sadedir:
- **MCU:** ESP32-C3 Super Mini (Dahili BLE 5.0)
- **Ekran:** 1.3" OLED, 128x64 Piksel, I2C Arayüzü (Varsayılan sürücü: SH1106)
- **Güç:** 3.7V LiPo Batarya (150-500 mAh) + TP4056 USB-C Şarj Modülü

### ASCII Bağlantı Şeması

```text
   +-----------------------+              +------------------------+
   | 1.3" OLED (128x64)    |              |   ESP32-C3 SUPER MINI  |
   |                       |              |                        |
   | [VCC] ----------------+--------------+--> 3V3                 |
   | [GND] ----------------+--------------+--> GND                 |
   | [SDA] ----------------+--------------+--> GPIO 8              |
   | [SCL] ----------------+--------------+--> GPIO 9              |
   +-----------------------+              +----+--------------+----+
                                               | 5V       GND |
                                               +--+---------+-+
                                                  |         |
                                                [OUT+]    [OUT-]
                                               +--+---------+-+
                                               | TP4056 MODÜL |
                                               |  [B+]   [B-] |
                                               +---+-------+--+
                                                   |       |
                                                [ + ]     [ - ]
                                               +---+-------+--+
                                               |  3.7V LiPo   |
                                               +--------------+
```

> Ayrıntılı pin şeması ve montaj ipuçları için [docs/WIRING.md](docs/WIRING.md) ve [docs/BATTERY.md](docs/BATTERY.md) dosyalarına göz atabilirsiniz.

---

## 🚀 Firmware Kurulumu ve Yükleme

Firmware, **PlatformIO** araç zinciri ve **Arduino** framework'ü ile geliştirilmiştir.

### Adım 1: Gereksinimler
- Bilgisayarınızda [VS Code](https://code.visualstudio.com/) ve **PlatformIO IDE** eklentisi kurulu olmalıdır (veya terminalden `pio` CLI).

### Adım 2: Projeyi Açma
1. VS Code'da `File -> Open Folder` diyerek `pasha/firmware` klasörünü açın.
2. PlatformIO gerekli kütüphaneleri (`U8g2`, `NimBLE-Arduino`) otomatik olarak indirecektir.

### Adım 3: Ekran Tipinizi Doğrulayın
`firmware/include/config.h` dosyasını açın. Eğer ekranınız 1.3" SH1106 ise hiçbir değişiklik yapmanıza gerek yoktur. Eğer 0.96" veya SSD1306 tabanlı bir ekran kullanıyorsanız:
```c
// config.h içinde:
// #define USE_SH1106
#define USE_SSD1306
```

### Adım 4: Kodu Yükleme
1. ESP32-C3 Super Mini kartınızı USB kablosu ile bilgisayara bağlayın.
2. Kart ilk defa programlanıyorsa veya boot döngüsündeyse, kart üzerindeki **BOOT** butonuna basılı tutarken **RESET** butonuna bir kez basıp bırakın, ardından BOOT butonunu bırakın (Download Bootloader modu).
3. VS Code'un alt barındaki **PlatformIO: Upload** (sağa ok simgesi `→`) butonuna basın veya terminalden şu komutu çalıştırın:
```bash
pio run --target upload
```
4. Yükleme tamamlandığında ekranınızda Pasha'nın sevimli gözleri belirecektir!

---

## 📱 Kontrol Uygulamasını Açma ve Yayınlama

Kontrol paneli, Web Bluetooth destekleyen herhangi bir tarayıcıda çalışan tek sayfalık modern bir PWA uygulamasıdır.

### 1. Yerel Olarak Çalıştırma (Geliştirici Modu)
Web Bluetooth güvenlik gereği **HTTPS** veya **localhost** üzerinden çalışmak zorundadır:
- VS Code **Live Server** eklentisiyle `pasha/app/index.html` dosyasına sağ tıklayıp "Open with Live Server" diyebilirsiniz.
- Veya Python ile yerel sunucu başlatabilirsiniz:
```bash
cd pasha/app
python3 -m http.server 8000
```
Tarayıcınızda `http://localhost:8000` adresine girin.

### 2. GitHub Pages ile Canlıya Yayınlama (Deploy)
1. Kodlarınızı bir GitHub deposuna yükleyin.
2. Deponuzun `Settings -> Pages` menüsüne gidin.
3. Branch olarak `main` (veya `master`), klasör olarak `/app` (veya kök dizin) seçin ve kaydedin.
4. GitHub size ücretsiz bir `https://<kullanici-adiniz>.github.io/<repo-adi>/` HTTPS bağlantısı sağlayacaktır. Bu bağlantıyı telefonunuzdan açıp doğrudan kullanabilirsiniz!

### ⚠️ Tarayıcı ve Platform Uyumluluğu
- **Android:** Google Chrome veya Microsoft Edge ile sorunsuz çalışır.
- **Windows / macOS / Linux:** Chrome, Edge veya Opera ile doğrudan çalışır.
- **iOS (iPhone/iPad):** Apple, iOS Safari üzerinde Web Bluetooth API'sini kısıtlamıştır. **iOS kullanıcıları App Store'dan ücretsiz "Bluefy – Web BLE Browser" tarayıcısını indirerek uygulamayı bu tarayıcı içinde açmalıdır.**
- **Ana Ekrana Ekleme (PWA):** Tarayıcı menüsünden "Uygulamayı Yükle" veya "Ana Ekrana Ekle" diyerek Pasha'yı bağımsız bir mobil uygulama gibi kullanabilirsiniz.

---

## 📡 BLE Protokolü ve Karakteristikler

Pasha, tek bir özel GATT servisi (`0000beef-7273-6861-6e64-657669636531`) kullanır:

| Karakteristik | UUID | Veri Formatı | Açıklama |
| :--- | :--- | :--- | :--- |
| **TIME** | `0000bee1-...` | `uint32 epoch + int16 tzOffset` | Saat ve zaman dilimi senkronizasyonu |
| **WEATHER** | `0000bee2-...` | `int8 temp, uint8 code, int8 min, int8 max, uint8 humidity` | Anlık hava durumu |
| **EXPRESSION** | `0000bee3-...` | `uint8 exprId, uint8 mode` | Duygu seçimi ve oto/manuel modu |
| **SETTINGS** | `0000bee4-...` | `11 byte packed struct` | Stiller, aralıklar, parlaklık, uyku |
| **STATUS** | `0000bee5-...` | `7 byte status struct` | Firmware sürümü, ekran ve BLE durumu |

> Detaylı paket bayt yapısı ve veri tabloları için [docs/PROTOCOL.md](docs/PROTOCOL.md) dosyasını inceleyebilirsiniz.

---

## 🔍 Sorun Giderme

### 1. Ekran Boş (Işık Gelmiyor veya Karıncalı)
- **Ekran Sürücüsü Uyuşmazlığı:** 1.3" OLED'lerin %90'ı **SH1106** sürücüsüne sahiptir. Ancak bazı satıcılar SSD1306 sürücülü 1.3" ekranlar satmaktadır. Karıncalı veya 2 piksel kaymış görüntü alıyorsanız `config.h` dosyasında `USE_SSD1306` seçeneğini aktif edip yeniden yükleyin.
- **I2C Pin Bağlantısı:** ESP32-C3 Super Mini kartlarında SDA pininin `GPIO8`, SCL pininin `GPIO9` pinine bağlı olduğundan emin olun.
- **Lehim Teması:** OLED pinlerinin ESP32 arkasındaki metal plakalara kısa devre yapmadığından emin olun.

### 2. Web Bluetooth Cihazı Görmüyor / Bağlanmıyor
- **Bluetooth İzinleri:** Bilgisayarınızın veya telefonunuzun Bluetooth'unun açık olduğundan emin olun.
- **HTTPS Gereksinimi:** Web Bluetooth HTTP üzerinden (localhost hariç) çalışmaz. Uygulamanın `https://` ile açıldığından emin olun.
- **Cihaz Meşgul mü?:** BLE tek noktadan bağlantıyı (1 central) destekler. Eğer Pasha başka bir telefona bağlıysa reklam yayını durur. Diğer bağlantıyı kesin.
- **Bağlantı Kopunca:** Cihaz bağlantı koptuğunda otomatik olarak yeniden reklam yayınına başlar.

### 3. Saat Sıfırlanıyor
- Pasha donanımında RTC (Gerçek Zamanlı Saat) pili veya entegresi bulunmaz (boyut ve maliyet optimizasyonu). Saat telefondan Bluetooth ile bağlanınca milisaniye hassasiyetiyle senkronize edilir ve `millis()` sayacıyla ilerletilir. Güç kesildiğinde saat kaybolur; telefonla tekrar bağlanıldığında otomatik güncellenir.

---

## 📝 Varsayımlar

Proje geliştirilirken yapılan makul teknik varsayımlar:
1. **Donanım Sınırları:** Pasha'da harici buton, RTC ve sensör bulunmadığından; tüm etkileşim, duygu seçimi, saat ve hava durumu verisinin Web Bluetooth üzerinden telefon/PC ile sağlandığı varsayılmıştır.
2. **Saat Hassasiyeti:** Dahili RTC olmaması sebebiyle saat `millis()` üzerinden hesaplanır. Tipik ESP32 kristal sapmasını kompanse etmek için Web Bluetooth uygulaması bağlı kaldığı sürece **her 15 dakikada bir** saati arka planda otomatik günceller.
3. **Ekran Sürücüsü:** Varsayılan 1.3" ekran sürücüsü SH1106 kabul edilmiş; `config.h` içindeki tek bir `#define USE_SSD1306` makrosuyla SSD1306'ya dönüştürülebilir hale getirilmiştir.
4. **Hava Durumu Servisi:** Kullanıcıdan herhangi bir API anahtarı veya kredi kartı talep etmemek için tamamen açık ve ücretsiz olan **Open-Meteo API** tercih edilmiştir.
5. **iOS Uyumluluğu:** Safari'nin Web Bluetooth desteği sunmaması sebebiyle iOS için App Store'daki ücretsiz "Bluefy" tarayıcısının kullanılması önerilmiştir.
6. **Güç Kaynağı:** TP4056'nın `OUT+` çıkışının 3.7V - 4.2V gerilim üretmesi sebebiyle, ESP32-C3 Super Mini'nin dahili 3.3V LDO regülatörüne sahip olan `5V/VIN` pinine bağlanması gerektiği varsayılmıştır.
