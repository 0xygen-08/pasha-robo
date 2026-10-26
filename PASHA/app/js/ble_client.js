/**
 * Web Bluetooth API İstemcisi
 * Pasha BLE GATT servisiyle haberleşir, karakteristikleri yazar, okur ve bildirimleri dinler.
 */

const PASHA_UUIDS = {
  SERVICE:    "0000beef-7273-6861-6e64-657669636531",
  TIME:       "0000bee1-7273-6861-6e64-657669636531",
  WEATHER:    "0000bee2-7273-6861-6e64-657669636531",
  EXPRESSION: "0000bee3-7273-6861-6e64-657669636531",
  SETTINGS:   "0000bee4-7273-6861-6e64-657669636531",
  STATUS:     "0000bee5-7273-6861-6e64-657669636531"
};

class BleClient {
  constructor() {
    this.device = null;
    this.server = null;
    this.service = null;
    this.characteristics = {};
    this.isConnected = false;
    this.autoReconnect = true;
    this.reconnectAttempts = 0;

    this.onStatusCallback = null;
    this.onSettingsCallback = null;
    this.onConnectionChangeCallback = null;

    this.handleDisconnected = this.handleDisconnected.bind(this);
  }

  isSupported() {
    return !!(navigator.bluetooth && navigator.bluetooth.requestDevice);
  }

  async connect() {
    if (!this.isSupported()) {
      throw new Error("Bu tarayıcı Web Bluetooth desteklemiyor. Lütfen Chrome, Edge veya Android/PC kullanın.");
    }

    try {
      this.device = await navigator.bluetooth.requestDevice({
        filters: [{ name: "Pasha" }],
        optionalServices: [PASHA_UUIDS.SERVICE]
      });

      this.device.addEventListener('gattserverdisconnected', this.handleDisconnected);
      return await this.reconnectGatt();
    } catch (err) {
      console.error("[BLE] Bağlantı iptal edildi veya başarısız:", err);
      throw err;
    }
  }

  async reconnectGatt() {
    if (!this.device) return false;

    console.log("[BLE] GATT sunucusuna bağlanılıyor...");
    this.server = await this.device.gatt.connect();
    this.service = await this.server.getPrimaryService(PASHA_UUIDS.SERVICE);

    // Karakteristikleri al
    this.characteristics.time = await this.service.getCharacteristic(PASHA_UUIDS.TIME);
    this.characteristics.weather = await this.service.getCharacteristic(PASHA_UUIDS.WEATHER);
    this.characteristics.expression = await this.service.getCharacteristic(PASHA_UUIDS.EXPRESSION);
    this.characteristics.settings = await this.service.getCharacteristic(PASHA_UUIDS.SETTINGS);
    this.characteristics.status = await this.service.getCharacteristic(PASHA_UUIDS.STATUS);

    // Bildirimleri (Notification) başlat
    await this.characteristics.settings.startNotifications();
    this.characteristics.settings.addEventListener('characteristicvaluechanged', (e) => {
      this.handleSettingsChanged(e.target.value);
    });

    await this.characteristics.status.startNotifications();
    this.characteristics.status.addEventListener('characteristicvaluechanged', (e) => {
      this.handleStatusChanged(e.target.value);
    });

    this.isConnected = true;
    this.reconnectAttempts = 0;
    if (this.onConnectionChangeCallback) this.onConnectionChangeCallback(true);

    console.log("[BLE] Pasha'ya başarıyla bağlandı!");
    return true;
  }

  handleDisconnected() {
    this.isConnected = false;
    if (this.onConnectionChangeCallback) this.onConnectionChangeCallback(false);
    console.warn("[BLE] Bağlantı koptu.");

    // Otomatik yeniden bağlanma
    if (this.autoReconnect && this.device && this.reconnectAttempts < 5) {
      this.reconnectAttempts++;
      const delay = Math.min(2000 * this.reconnectAttempts, 8000);
      console.log(`[BLE] ${delay / 1000} sn içinde otomatik yeniden bağlanma denemesi (${this.reconnectAttempts}/5)...`);
      setTimeout(() => {
        if (!this.isConnected && this.device) {
          this.reconnectGatt().catch((e) => console.log("[BLE] Yeniden bağlanma denemesi başarısız:", e));
        }
      }, delay);
    }
  }

  disconnect() {
    this.autoReconnect = false;
    if (this.device && this.device.gatt.connected) {
      this.device.gatt.disconnect();
    }
    this.isConnected = false;
  }

  // ==========================================
  // KARAKTERİSTİK YAZMA İŞLEMLERİ
  // ==========================================
  async sendTime(epochSeconds, tzOffsetMinutes) {
    if (!this.isConnected || !this.characteristics.time) return;

    const buf = new ArrayBuffer(6);
    const view = new DataView(buf);
    view.setUint32(0, epochSeconds, true); // Little-endian
    view.setInt16(4, tzOffsetMinutes, true);

    await this.characteristics.time.writeValue(new Uint8Array(buf));
    console.log(`[BLE] Saat senkronize edildi: Epoch=${epochSeconds}, TZ=${tzOffsetMinutes}dk`);
  }

  async sendWeather(binaryPacket) {
    if (!this.isConnected || !this.characteristics.weather) return;
    await this.characteristics.weather.writeValue(binaryPacket);
    console.log("[BLE] Hava durumu paketi iletildi.");
  }

  async sendExpression(expressionId, mode = 1) {
    if (!this.isConnected || !this.characteristics.expression) return;
    const buf = new Uint8Array([expressionId, mode]);
    await this.characteristics.expression.writeValue(buf);
    console.log(`[BLE] İfade güncellendi: ID=${expressionId}, Mod=${mode}`);
  }

  async sendSettings(s) {
    if (!this.isConnected || !this.characteristics.settings) return;

    const buf = new ArrayBuffer(12);
    const view = new DataView(buf);
    view.setUint8(0, s.faceStyle);
    view.setUint8(1, s.clockStyle);
    view.setUint8(2, s.weatherStyle);
    view.setUint16(3, s.cycleIntervalSec, true); // Little-endian
    view.setUint8(5, s.clockDurationSec);
    view.setUint8(6, s.weatherDurSec);
    view.setUint8(7, s.clockEnabled ? 1 : 0);
    view.setUint8(8, s.weatherEnabled ? 1 : 0);
    view.setUint8(9, s.brightness);
    view.setUint8(10, s.sleepTimeoutMin);
    view.setUint8(11, s.autoExprMode ? 1 : 0);

    await this.characteristics.settings.writeValue(new Uint8Array(buf));
    console.log("[BLE] Ayarlar cihaza yazıldı.");
  }

  async readSettings() {
    if (!this.isConnected || !this.characteristics.settings) return null;
    const value = await this.characteristics.settings.readValue();
    return this.parseSettings(value);
  }

  parseSettings(dataView) {
    if (dataView.byteLength < 11) return null;
    return {
      faceStyle: dataView.getUint8(0),
      clockStyle: dataView.getUint8(1),
      weatherStyle: dataView.getUint8(2),
      cycleIntervalSec: dataView.getUint16(3, true),
      clockDurationSec: dataView.getUint8(5),
      weatherDurSec: dataView.getUint8(6),
      clockEnabled: dataView.getUint8(7) === 1,
      weatherEnabled: dataView.getUint8(8) === 1,
      brightness: dataView.getUint8(9),
      sleepTimeoutMin: dataView.getUint8(10),
      autoExprMode: dataView.byteLength >= 12 ? (dataView.getUint8(11) === 1) : true
    };
  }

  handleSettingsChanged(dataView) {
    const settings = this.parseSettings(dataView);
    if (settings && this.onSettingsCallback) {
      this.onSettingsCallback(settings);
    }
  }

  handleStatusChanged(dataView) {
    if (dataView.byteLength < 7) return;
    const status = {
      fwMajor: dataView.getUint8(0),
      fwMinor: dataView.getUint8(1),
      isConnected: dataView.getUint8(2) === 1,
      hasTime: dataView.getUint8(3) === 1,
      hasWeather: dataView.getUint8(4) === 1,
      currentExpression: dataView.getUint8(5),
      currentScreen: dataView.getUint8(6)
    };

    if (this.onStatusCallback) {
      this.onStatusCallback(status);
    }
  }
}

window.BleClient = BleClient;
