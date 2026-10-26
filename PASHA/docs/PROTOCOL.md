# PASHA BLE GATT Protokol Dokümantasyonu

Pasha robotu, kompakt Bluetooth Low Energy (BLE) GATT protokolü ile haberleşir. Paket boyutları standart BLE MTU sınırını (23 bayt) aşmayacak şekilde optimize edilmiş ikili (binary) biçimdedir.

## Servis ve Karakteristik UUID'leri

| Tanım | UUID | Erişim | Boyut | Açıklama |
| :--- | :--- | :--- | :--- | :--- |
| **Pasha Service** | `0000beef-7273-6861-6e64-657669636531` | - | - | Özel Pasha GATT Servisi |
| **TIME** | `0000bee1-7273-6861-6e64-657669636531` | WRITE, WRITE_NR | 6 Bayt | Unix Epoch & Zaman Dilimi |
| **WEATHER** | `0000bee2-7273-6861-6e64-657669636531` | WRITE, WRITE_NR | 5 Bayt | Hava durumu ve sıcaklık |
| **EXPRESSION** | `0000bee3-7273-6861-6e64-657669636531` | WRITE, WRITE_NR | 2 Bayt | Duygu ifadesi & Otomatik Mod |
| **SETTINGS** | `0000bee4-7273-6861-6e64-657669636531` | READ, WRITE, NOTIFY | 11 Bayt | Kalıcı ayarlar ve stiller |
| **STATUS** | `0000bee5-7273-6861-6e64-657669636531` | READ, NOTIFY | 7 Bayt | Firmware ve bağlantı durumu |

---

## Karakteristik Paket Yapıları

Tüm çok baytlı tamsayılar **Little-Endian** biçimindedir.

### 1. TIME Karakteristiği (6 Bayt)
Telefon veya bilgisayarın yerel saati bağlandığında veya periyodik olarak iletilir.

| Bayt Ofseti | Alan | Veri Tipi | Aralık | Açıklama |
| :--- | :--- | :--- | :--- | :--- |
| `0 - 3` | `epoch` | `uint32_t` (Little-Endian) | `0 - 4294967295` | Unix Epoch Saniyesi (UTC) |
| `4 - 5` | `tzOffsetMinutes` | `int16_t` (Little-Endian) | `-720 - +840` | UTC'ye göre dakika farkı (Örn: UTC+3 için `+180`) |

---

### 2. WEATHER Karakteristiği (5 Bayt)
Open-Meteo servisinden çekilen ve WMO standart kodlarından dönüştürülen hava verisi.

| Bayt Ofseti | Alan | Veri Tipi | Aralık | Açıklama |
| :--- | :--- | :--- | :--- | :--- |
| `0` | `temperature` | `int8_t` | `-50 .. +60` | Anlık Sıcaklık (°C) |
| `1` | `weatherCode` | `uint8_t` | `0 .. 6` | Hava Durumu Kodu (Aşağıdaki tabloya bakınız) |
| `2` | `tempMin` | `int8_t` | `-50 .. +60` | Günün En Düşük Sıcaklığı (°C) |
| `3` | `tempMax` | `int8_t` | `-50 .. +60` | Günün En Yüksek Sıcaklığı (°C) |
| `4` | `humidity` | `uint8_t` | `0 .. 100` | Bağıl Nem Yüzdesi (%) |

#### Hava Durumu Kodları (`WeatherCode`)
- `0`: Güneşli / Açık
- `1`: Parçalı Bulutlu
- `2`: Bulutlu / Kapalı
- `3`: Yağmurlu / Çisenti
- `4`: Karlı
- `5`: Fırtınalı / Şimşekli
- `6`: Sisli / Puslu
- `255`: Veri Yok

---

### 3. EXPRESSION Karakteristiği (2 Bayt)
Robotun yüz ifadesini ve otomatik duygu modunu yönetir.

| Bayt Ofseti | Alan | Veri Tipi | Değerler |
| :--- | :--- | :--- | :--- |
| `0` | `expressionId` | `uint8_t` | `0 .. 11` (Duygu tablosu aşağıda) |
| `1` | `mode` | `uint8_t` | `0`: Manuel (Sabit ifade), `1`: Otomatik (Rastgele döngü) |

#### Duygu İfadeleri Listesi (`ExpressionId`)
- `0`: `EXPR_NORMAL` (Normal, nötr sevimli)
- `1`: `EXPR_HAPPY` (Mutlu hilal gözler)
- `2`: `EXPR_SWEET` (Sevecen / tatlı, mahcup yanaklar)
- `3`: `EXPR_ANGRY` (Kızgın çatık kaşlar)
- `4`: `EXPR_GRUMPY` (Sinirli / huysuz, dar gözler)
- `5`: `EXPR_SURPRISED` (Şaşırmış, koca açık gözler)
- `6`: `EXPR_IN_LOVE` (Aşık, kalp gözler + hafif kalp atışı)
- `7`: `EXPR_SAD` (Üzgün, aşağı süzülen göz kapakları)
- `8`: `EXPR_SLEEPY` (Uykulu, Zzz balonu)
- `9`: `EXPR_EXCITED` (Sevinçli / heyecanlı)
- `10`: `EXPR_SUSPICIOUS` (Şüpheli, tek göz kısık)
- `11`: `EXPR_WINK` (Göz kırpma)

---

### 4. SETTINGS Karakteristiği (12 Bayt)
Görsel stiller, süreler, ekran parlaklığı ve uyku ayarları.

| Bayt Ofseti | Alan | Veri Tipi | Varsayılan | Açıklama |
| :--- | :--- | :--- | :--- | :--- |
| `0` | `faceStyle` | `uint8_t` | `1` | `0`: Nokta, `1`: Klasik Robot, `2`: Kawaii, `3`: Retro Piksel |
| `1` | `clockStyle` | `uint8_t` | `0` | `0`: Büyük Dijital, `1`: Mini Yüz, `2`: Analog, `3`: 7-Segment |
| `2` | `weatherStyle` | `uint8_t` | `0` | `0`: Büyük İkon, `1`: Animasyonlu İkon, `2`: Kompakt Metin |
| `3 - 4` | `cycleIntervalSec` | `uint16_t` | `30` | Ekranlar arası döngü aralığı (saniye) |
| `5` | `clockDurationSec` | `uint8_t` | `5` | Saatin ekranda kalma süresi (saniye) |
| `6` | `weatherDurSec` | `uint8_t` | `5` | Hava durumunun ekranda kalma süresi (saniye) |
| `7` | `clockEnabled` | `uint8_t` | `1` | `1`: Saat açık, `0`: Saat kapalı |
| `8` | `weatherEnabled` | `uint8_t` | `1` | `1`: Hava açık, `0`: Hava kapalı |
| `9` | `brightness` | `uint8_t` | `255` | Ekran kontrast seviyesi (`0 - 255`) |
| `10` | `sleepTimeoutMin` | `uint8_t` | `10` | Otomatik uyku süresi (`0`: Kapalı, `1 - 60` dakika) |
| `11` | `autoExprMode` | `uint8_t` | `1` | `1`: Otomatik rastgele duygu modu, `0`: Manuel |

---

### 5. STATUS Karakteristiği (7 Bayt)
Cihaz tarafından 2 saniyede bir veya durum değiştikçe yayınlanır (`NOTIFY`).

| Bayt Ofseti | Alan | Veri Tipi | Açıklama |
| :--- | :--- | :--- | :--- |
| `0` | `fwMajor` | `uint8_t` | Firmware ana sürüm (`1`) |
| `1` | `fwMinor` | `uint8_t` | Firmware alt sürüm (`0`) |
| `2` | `isConnected` | `uint8_t` | `1`: BLE aktif bağlı, `0`: Boşta |
| `3` | `hasTime` | `uint8_t` | `1`: Saat senkronize edildi, `0`: Bekleniyor |
| `4` | `hasWeather` | `uint8_t` | `1`: Geçerli hava verisi var, `0`: Yok |
| `5` | `currentExpression` | `uint8_t` | Ekranda anlık oynatılan duygu ID |
| `6` | `currentScreen` | `uint8_t` | `0`: Yüz, `1`: Saat, `2`: Hava, `3`: Geçiş, `4`: Uyku |
