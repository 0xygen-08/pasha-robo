/**
 * Pasha Web Bluetooth PWA Ana Uygulama Mantığı
 */

document.addEventListener('DOMContentLoaded', () => {
  // Service Worker Kaydı
  if ('serviceWorker' in navigator) {
    navigator.serviceWorker.register('./sw.js').catch((err) => {
      console.log("[PWA] Service Worker kaydedilemedi:", err);
    });
  }

  // Modülleri Başlat
  const simulator = new OledSimulator('oledCanvas');
  const weatherProvider = new WeatherProvider();
  const ble = new BleClient();

  // Mevcut Ayarlar Önbelleği
  let currentSettings = {
    faceStyle: 1,
    clockStyle: 0,
    weatherStyle: 0,
    cycleIntervalSec: 30,
    clockDurationSec: 5,
    weatherDurSec: 5,
    clockEnabled: true,
    weatherEnabled: true,
    brightness: 255,
    sleepTimeoutMin: 10
  };

  let isAutoExprMode = true;
  let selectedExprId = 0;

  // DOM Elementleri
  const btnConnect = document.getElementById('btnConnect');
  const connectionBadge = document.getElementById('connectionBadge');
  const badgeText = document.getElementById('badgeText');
  const fwVersionText = document.getElementById('fwVersionText');

  const exprGrid = document.getElementById('exprGrid');
  const autoModeToggle = document.getElementById('autoModeToggle');

  const faceStyleOptions = document.getElementById('faceStyleOptions');
  const clockStyleOptions = document.getElementById('clockStyleOptions');
  const weatherStyleOptions = document.getElementById('weatherStyleOptions');

  const sliderCycleInterval = document.getElementById('sliderCycleInterval');
  const valCycleInterval = document.getElementById('valCycleInterval');
  const sliderClockDuration = document.getElementById('sliderClockDuration');
  const valClockDuration = document.getElementById('valClockDuration');
  const sliderWeatherDuration = document.getElementById('sliderWeatherDuration');
  const valWeatherDuration = document.getElementById('valWeatherDuration');
  const sliderBrightness = document.getElementById('sliderBrightness');
  const valBrightness = document.getElementById('valBrightness');
  const sliderSleep = document.getElementById('sliderSleep');
  const valSleep = document.getElementById('valSleep');

  const toggleClockEn = document.getElementById('toggleClockEn');
  const toggleWeatherEn = document.getElementById('toggleWeatherEn');

  const btnSyncTime = document.getElementById('btnSyncTime');
  const btnGetLocationWeather = document.getElementById('btnGetLocationWeather');
  const btnSearchCity = document.getElementById('btnSearchCity');
  const inputCity = document.getElementById('inputCity');
  const btnResetDefaults = document.getElementById('btnResetDefaults');

  const weatherCity = document.getElementById('weatherCity');
  const weatherTemp = document.getElementById('weatherTemp');
  const weatherDesc = document.getElementById('weatherDesc');

  const toast = document.getElementById('toast');

  // ==========================================
  // YARDIMCI BİLDİRİM (TOAST)
  // ==========================================
  function showToast(message, duration = 3000) {
    toast.textContent = message;
    toast.classList.add('show');
    setTimeout(() => {
      toast.classList.remove('show');
    }, duration);
  }

  // ==========================================
  // BLE BAĞLANTI DURUMU YÖNETİMİ
  // ==========================================
  btnConnect.addEventListener('click', async () => {
    if (ble.isConnected) {
      ble.disconnect();
      return;
    }

    try {
      connectionBadge.className = 'status-badge connecting';
      badgeText.textContent = 'Aranıyor...';
      btnConnect.disabled = true;

      await ble.connect();
    } catch (err) {
      connectionBadge.className = 'status-badge';
      badgeText.textContent = 'Bağlantı Yok';
      btnConnect.disabled = false;
      showToast(err.message || 'Bluetooth bağlantısı kurulamadı');
    }
  });

  ble.onConnectionChangeCallback = (connected) => {
    btnConnect.disabled = false;
    if (connected) {
      connectionBadge.className = 'status-badge connected';
      badgeText.textContent = 'Bağlandı';
      btnConnect.textContent = 'Bağlantıyı Kes';
      btnConnect.classList.add('disconnect');
      showToast("Pasha robotuna başarıyla bağlandı! 🤖");

      // Bağlanınca hemen saati ve mevcut havayı senkronize et
      syncCurrentTime();
      if (weatherProvider.currentData) {
        ble.sendWeather(weatherProvider.getBinaryPacket());
      } else {
        // Otomatik konumdan hava durumunu çekmeye çalış
        weatherProvider.fetchByGeolocation()
          .then((data) => {
            updateWeatherUI(data);
            ble.sendWeather(weatherProvider.getBinaryPacket());
          })
          .catch(() => {
            // Varsayılan olarak İstanbul'u al
            weatherProvider.searchCityAndFetch('Istanbul')
              .then((data) => {
                updateWeatherUI(data);
                ble.sendWeather(weatherProvider.getBinaryPacket());
              }).catch(()=>{});
          });
      }
    } else {
      connectionBadge.className = 'status-badge';
      badgeText.textContent = 'Bağlantı Yok';
      btnConnect.textContent = "Pasha'ya Bağlan";
      btnConnect.classList.remove('disconnect');
      showToast("Bağlantı kesildi.");
    }
  };

  ble.onStatusCallback = (status) => {
    if (fwVersionText) {
      fwVersionText.textContent = `FW: v${status.fwMajor}.${status.fwMinor}`;
    }
  };

  ble.onSettingsCallback = (settings) => {
    currentSettings = { ...currentSettings, ...settings };
    syncSettingsToUI();
  };

  // ==========================================
  // SAAT SENKRONİZASYONU
  // ==========================================
  function syncCurrentTime() {
    const now = new Date();
    const epochSeconds = Math.floor(now.getTime() / 1000);
    // getTimezoneOffset() UTC'ye göre dakika farkını ters verir (-180 = UTC+3)
    const tzOffsetMinutes = now.getTimezoneOffset() * -1;

    if (ble.isConnected) {
      ble.sendTime(epochSeconds, tzOffsetMinutes);
      showToast("Saat Pasha ile senkronize edildi ⏰");
    }
  }

  btnSyncTime.addEventListener('click', () => {
    syncCurrentTime();
  });

  // Her 15 dakikada bir otomatik zaman senkronizasyonu
  setInterval(() => {
    if (ble.isConnected) {
      syncCurrentTime();
    }
  }, 15 * 60 * 1000);

  // ==========================================
  // HAVA DURUMU İŞLEMLERİ
  // ==========================================
  function updateWeatherUI(data) {
    weatherCity.textContent = data.city;
    weatherTemp.textContent = `${data.temp}°C`;
    weatherDesc.textContent = `${data.name} | Nem: %${data.humidity} | ${data.min}° / ${data.max}°`;

    simulator.setWeather(data);
  }

  btnGetLocationWeather.addEventListener('click', async () => {
    try {
      showToast("Konum alınıyor...");
      const data = await weatherProvider.fetchByGeolocation();
      updateWeatherUI(data);

      if (ble.isConnected) {
        await ble.sendWeather(weatherProvider.getBinaryPacket());
        showToast("Hava durumu cihaza gönderildi! ☀️");
      } else {
        showToast("Hava verisi alındı (Simülatör güncellendi).");
      }
    } catch (err) {
      showToast(err.message);
    }
  });

  btnSearchCity.addEventListener('click', async () => {
    const city = inputCity.value.trim();
    if (!city) {
      showToast("Lütfen bir şehir adı girin.");
      return;
    }

    try {
      showToast(`'${city}' aranıyor...`);
      const data = await weatherProvider.searchCityAndFetch(city);
      updateWeatherUI(data);

      if (ble.isConnected) {
        await ble.sendWeather(weatherProvider.getBinaryPacket());
        showToast("Hava durumu cihaza gönderildi! 🌦️");
      } else {
        showToast("Şehir bulundu (Simülatör güncellendi).");
      }
    } catch (err) {
      showToast(err.message);
    }
  });

  // Başlangıçta varsayılan hava durumunu yükle
  weatherProvider.searchCityAndFetch("Istanbul")
    .then((data) => updateWeatherUI(data))
    .catch(() => {});

  // ==========================================
  // İFADE SEÇİMİ
  // ==========================================
  const expressions = [
    { id: 0, icon: "😊", name: "Normal" },
    { id: 1, icon: "😄", name: "Mutlu" },
    { id: 2, icon: "🥰", name: "Tatlı" },
    { id: 3, icon: "😠", name: "Kızgın" },
    { id: 4, icon: "😒", name: "Huysuz" },
    { id: 5, icon: "😲", name: "Şaşırmış" },
    { id: 6, icon: "😍", name: "Aşık" },
    { id: 7, icon: "😢", name: "Üzgün" },
    { id: 8, icon: "😴", name: "Uykulu" },
    { id: 9, icon: "🤩", name: "Heyecanlı" },
    { id: 10, icon: "🤨", name: "Şüpheli" },
    { id: 11, icon: "😉", name: "Göz Kırp" }
  ];

  expressions.forEach((expr) => {
    const btn = document.createElement('button');
    btn.className = `expr-btn ${expr.id === 0 ? 'active' : ''}`;
    btn.innerHTML = `
      <span class="expr-icon">${expr.icon}</span>
      <span class="expr-name">${expr.name}</span>
    `;

    btn.addEventListener('click', () => {
      document.querySelectorAll('.expr-btn').forEach(b => b.classList.remove('active'));
      btn.classList.add('active');

      selectedExprId = expr.id;
      simulator.setExpression(selectedExprId);

      // Manuel olarak ifade seçildiğinde otomatik modu kapat
      isAutoExprMode = false;
      autoModeToggle.checked = false;

      if (ble.isConnected) {
        ble.sendExpression(selectedExprId, 0);
      }
    });

    exprGrid.appendChild(btn);
  });

  autoModeToggle.addEventListener('change', (e) => {
    isAutoExprMode = e.target.checked;
    if (ble.isConnected) {
      ble.sendExpression(selectedExprId, isAutoExprMode ? 1 : 0);
    }
    showToast(isAutoExprMode ? "Otomatik duygu modu açıldı" : "Sabit manuel duygu modu");
  });

  // ==========================================
  // GÖRSEL STİL SEÇİCİLER
  // ==========================================
  function setupStyleChips(container, activeIndex, onChange) {
    const chips = container.querySelectorAll('.style-chip');
    chips.forEach((chip) => {
      chip.addEventListener('click', () => {
        chips.forEach(c => c.classList.remove('active'));
        chip.classList.add('active');
        const val = Number(chip.dataset.val);
        onChange(val);
      });
    });
  }

  setupStyleChips(faceStyleOptions, currentSettings.faceStyle, (val) => {
    currentSettings.faceStyle = val;
    simulator.setFaceStyle(val);
    saveSettingsToDevice();
  });

  setupStyleChips(clockStyleOptions, currentSettings.clockStyle, (val) => {
    currentSettings.clockStyle = val;
    simulator.setClockStyle(val);
    saveSettingsToDevice();
  });

  setupStyleChips(weatherStyleOptions, currentSettings.weatherStyle, (val) => {
    currentSettings.weatherStyle = val;
    simulator.setWeatherStyle(val);
    saveSettingsToDevice();
  });

  // ==========================================
  // SİMÜLATÖR EKRAN SEKMELERİ
  // ==========================================
  document.querySelectorAll('.screen-tab-btn').forEach(tab => {
    tab.addEventListener('click', () => {
      document.querySelectorAll('.screen-tab-btn').forEach(t => t.classList.remove('active'));
      tab.classList.add('active');
      const scr = tab.dataset.screen;
      simulator.setScreen(scr);
    });
  });

  // ==========================================
  // AYARLAR & SLIDER'LAR
  // ==========================================
  sliderCycleInterval.addEventListener('input', (e) => {
    valCycleInterval.textContent = `${e.target.value} sn`;
    currentSettings.cycleIntervalSec = Number(e.target.value);
  });
  sliderCycleInterval.addEventListener('change', saveSettingsToDevice);

  sliderClockDuration.addEventListener('input', (e) => {
    valClockDuration.textContent = `${e.target.value} sn`;
    currentSettings.clockDurationSec = Number(e.target.value);
  });
  sliderClockDuration.addEventListener('change', saveSettingsToDevice);

  sliderWeatherDuration.addEventListener('input', (e) => {
    valWeatherDuration.textContent = `${e.target.value} sn`;
    currentSettings.weatherDurSec = Number(e.target.value);
  });
  sliderWeatherDuration.addEventListener('change', saveSettingsToDevice);

  sliderBrightness.addEventListener('input', (e) => {
    valBrightness.textContent = e.target.value;
    currentSettings.brightness = Number(e.target.value);
  });
  sliderBrightness.addEventListener('change', saveSettingsToDevice);

  sliderSleep.addEventListener('input', (e) => {
    valSleep.textContent = (Number(e.target.value) === 0) ? "Kapalı" : `${e.target.value} dk`;
    currentSettings.sleepTimeoutMin = Number(e.target.value);
  });
  sliderSleep.addEventListener('change', saveSettingsToDevice);

  toggleClockEn.addEventListener('change', (e) => {
    currentSettings.clockEnabled = e.target.checked;
    saveSettingsToDevice();
  });

  toggleWeatherEn.addEventListener('change', (e) => {
    currentSettings.weatherEnabled = e.target.checked;
    saveSettingsToDevice();
  });

  function saveSettingsToDevice() {
    if (ble.isConnected) {
      ble.sendSettings(currentSettings);
    }
  }

  function syncSettingsToUI() {
    sliderCycleInterval.value = currentSettings.cycleIntervalSec;
    valCycleInterval.textContent = `${currentSettings.cycleIntervalSec} sn`;

    sliderClockDuration.value = currentSettings.clockDurationSec;
    valClockDuration.textContent = `${currentSettings.clockDurationSec} sn`;

    sliderWeatherDuration.value = currentSettings.weatherDurSec;
    valWeatherDuration.textContent = `${currentSettings.weatherDurSec} sn`;

    sliderBrightness.value = currentSettings.brightness;
    valBrightness.textContent = currentSettings.brightness;

    sliderSleep.value = currentSettings.sleepTimeoutMin;
    valSleep.textContent = (currentSettings.sleepTimeoutMin === 0) ? "Kapalı" : `${currentSettings.sleepTimeoutMin} dk`;

    toggleClockEn.checked = currentSettings.clockEnabled;
    toggleWeatherEn.checked = currentSettings.weatherEnabled;

    simulator.setFaceStyle(currentSettings.faceStyle);
    simulator.setClockStyle(currentSettings.clockStyle);
    simulator.setWeatherStyle(currentSettings.weatherStyle);

    updateChipsActive(faceStyleOptions, currentSettings.faceStyle);
    updateChipsActive(clockStyleOptions, currentSettings.clockStyle);
    updateChipsActive(weatherStyleOptions, currentSettings.weatherStyle);
  }

  function updateChipsActive(container, val) {
    container.querySelectorAll('.style-chip').forEach(c => {
      c.classList.toggle('active', Number(c.dataset.val) === val);
    });
  }

  btnResetDefaults.addEventListener('click', () => {
    currentSettings = {
      faceStyle: 1,
      clockStyle: 0,
      weatherStyle: 0,
      cycleIntervalSec: 30,
      clockDurationSec: 5,
      weatherDurSec: 5,
      clockEnabled: true,
      weatherEnabled: true,
      brightness: 255,
      sleepTimeoutMin: 10
    };
    syncSettingsToUI();
    saveSettingsToDevice();
    showToast("Ayarlar varsayılana sıfırlandı!");
  });
});
