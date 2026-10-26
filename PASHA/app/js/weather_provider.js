/**
 * Open-Meteo Hava Durumu Servisi Entegrasyonu
 * Geolocation veya şehir arama ile hava durumu çeker ve WMO kodlarını Pasha kodlarına çevirir.
 */

class WeatherProvider {
  constructor() {
    this.currentData = null;
    this.cityName = "Bilinmeyen";
  }

  // WMO Standart Kodlarını Pasha WeatherCode Enum'ına Çevirir
  mapWmoToPasha(wmoCode) {
    if (wmoCode === 0 || wmoCode === 1) {
      return { code: 0, name: "Güneşli / Açık" };
    } else if (wmoCode === 2) {
      return { code: 1, name: "Parçalı Bulutlu" };
    } else if (wmoCode === 3) {
      return { code: 2, name: "Bulutlu / Kapalı" };
    } else if ((wmoCode >= 51 && wmoCode <= 67) || (wmoCode >= 80 && wmoCode <= 82)) {
      return { code: 3, name: "Yağmurlu" };
    } else if ((wmoCode >= 71 && wmoCode <= 77) || (wmoCode >= 85 && wmoCode <= 86)) {
      return { code: 4, name: "Karlı" };
    } else if (wmoCode >= 95 && wmoCode <= 99) {
      return { code: 5, name: "Fırtınalı" };
    } else if (wmoCode === 45 || wmoCode === 48) {
      return { code: 6, name: "Sisli / Puslu" };
    }
    return { code: 0, name: "Açık" };
  }

  // Cihazın GPS Konumundan Hava Durumu Al
  async fetchByCoordinates(lat, lon, cityName = "Mevcut Konum") {
    try {
      const url = `https://api.open-meteo.com/v1/forecast?latitude=${lat}&longitude=${lon}&current=temperature_2m,relative_humidity_2m,weather_code&daily=temperature_2m_max,temperature_2m_min&timezone=auto`;
      const res = await fetch(url);
      if (!res.ok) throw new Error("Hava durumu servisi yanit vermedi.");
      const json = await res.json();

      const current = json.current;
      const daily = json.daily;
      const mapped = this.mapWmoToPasha(current.weather_code);

      this.currentData = {
        city: cityName,
        temp: Math.round(current.temperature_2m),
        code: mapped.code,
        name: mapped.name,
        humidity: Math.round(current.relative_humidity_2m),
        min: Math.round(daily.temperature_2m_min[0]),
        max: Math.round(daily.temperature_2m_max[0]),
        timestamp: Date.now()
      };

      this.cityName = cityName;
      return this.currentData;
    } catch (err) {
      console.error("[Weather] Veri alinamadi:", err);
      throw err;
    }
  }

  // Geolocation API ile Otomatik Konum
  async fetchByGeolocation() {
    return new Promise((resolve, reject) => {
      if (!navigator.geolocation) {
        reject(new Error("Tarayıcınız konum servisini desteklemiyor."));
        return;
      }

      navigator.geolocation.getCurrentPosition(
        async (position) => {
          try {
            const lat = position.coords.latitude;
            const lon = position.coords.longitude;
            const data = await this.fetchByCoordinates(lat, lon, "Mevcut Konum");
            resolve(data);
          } catch (err) {
            reject(err);
          }
        },
        (error) => {
          reject(new Error("Konum izni verilmedi veya alinamadi."));
        },
        { timeout: 10000 }
      );
    });
  }

  // Open-Meteo Geocoding ile Şehir Arama
  async searchCityAndFetch(cityName) {
    try {
      const geoUrl = `https://geocoding-api.open-meteo.com/v1/search?name=${encodeURIComponent(cityName)}&count=1&language=tr&format=json`;
      const res = await fetch(geoUrl);
      const json = await res.json();

      if (!json.results || json.results.length === 0) {
        throw new Error(`'${cityName}' sehir bulunamadi.`);
      }

      const place = json.results[0];
      const displayName = `${place.name}, ${place.country_code ? place.country_code.toUpperCase() : ''}`;
      return await this.fetchByCoordinates(place.latitude, place.longitude, displayName);
    } catch (err) {
      console.error("[Weather Geocoding] Hata:", err);
      throw err;
    }
  }

  // Pasha Firmware Binary Paket Yapısı (5 Byte)
  getBinaryPacket() {
    if (!this.currentData) return null;
    const buf = new ArrayBuffer(5);
    const view = new DataView(buf);

    view.setInt8(0, this.currentData.temp);
    view.setUint8(1, this.currentData.code);
    view.setInt8(2, this.currentData.min);
    view.setInt8(3, this.currentData.max);
    view.setUint8(4, this.currentData.humidity);

    return new Uint8Array(buf);
  }
}

window.WeatherProvider = WeatherProvider;
