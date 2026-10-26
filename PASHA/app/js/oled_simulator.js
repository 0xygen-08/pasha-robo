/**
 * PASHA 128x64 OLED Canlı Ekran Simülatörü
 * Firmware'deki FaceEngine, ClockScreen ve WeatherScreen mantığını tarayıcıda birebir simüle eder.
 */

class OledSimulator {
  constructor(canvasId) {
    this.canvas = document.getElementById(canvasId);
    if (!this.canvas) return;
    this.ctx = this.canvas.getContext('2d');
    this.canvas.width = 128;
    this.canvas.height = 64;

    // Durum Değişkenleri
    this.currentScreen = 'face'; // 'face', 'clock', 'weather'
    this.currentExpr = 0; // EXPR_NORMAL
    this.faceStyle = 1;   // FACE_STYLE_ROUND_RECT
    this.clockStyle = 0;  // CLOCK_STYLE_BIG_DIGITAL
    this.weatherStyle = 0;// WEATHER_STYLE_BIG_ICON

    this.weatherData = {
      temp: 24,
      code: 0,
      min: 19,
      max: 28,
      humidity: 48,
      name: "Gunesli"
    };

    // Göz Parametreleri (Interpolasyon)
    this.currLeft = { x: 38, y: 32, w: 24, h: 28, r: 8, top: 0, bot: 0, browAngle: 0, browY: 0, isHeart: false, isWink: false };
    this.targetLeft = { ...this.currLeft };
    this.currRight = { x: 90, y: 32, w: 24, h: 28, r: 8, top: 0, bot: 0, browAngle: 0, browY: 0, isHeart: false, isWink: false };
    this.targetRight = { ...this.currRight };

    this.gazeX = 0;
    this.gazeY = 0;
    this.targetGazeX = 0;
    this.targetGazeY = 0;
    this.breathPhase = 0;

    this.isBlinking = false;
    this.blinkStart = 0;
    this.nextBlink = Date.now() + 3000;
    this.nextGaze = Date.now() + 4000;

    this.applyExpression(this.currentExpr);
    this.animate = this.animate.bind(this);
    requestAnimationFrame(this.animate);
  }

  setScreen(screen) {
    this.currentScreen = screen;
  }

  setExpression(exprId) {
    this.currentExpr = exprId;
    this.applyExpression(exprId);
  }

  setFaceStyle(style) {
    this.faceStyle = style;
    this.applyExpression(this.currentExpr);
  }

  setClockStyle(style) {
    this.clockStyle = style;
  }

  setWeatherStyle(style) {
    this.weatherStyle = style;
  }

  setWeather(data) {
    this.weatherData = { ...this.weatherData, ...data };
  }

  applyExpression(expr) {
    let baseW = 24, baseH = 28, baseR = 8;
    if (this.faceStyle === 0) { baseW = 22; baseH = 22; baseR = 11; }
    else if (this.faceStyle === 2) { baseW = 28; baseH = 32; baseR = 14; }
    else if (this.faceStyle === 3) { baseW = 24; baseH = 24; baseR = 0; }

    this.targetLeft = {
      x: 38, y: 32, w: baseW, h: baseH, r: baseR,
      top: 0, bot: 0, browAngle: 0, browY: 0,
      isHeart: false, isWink: false
    };
    this.targetRight = { ...this.targetLeft, x: 90 };

    switch (Number(expr)) {
      case 0: // Normal
        break;
      case 1: // Mutlu
        this.targetLeft.bot = 0.65;
        this.targetRight.bot = 0.65;
        this.targetLeft.browY = -2;
        this.targetRight.browY = -2;
        break;
      case 2: // Tatlı
        this.targetLeft.w += 2; this.targetRight.w += 2;
        this.targetLeft.bot = 0.35; this.targetRight.bot = 0.35;
        this.targetLeft.browY = -3; this.targetRight.browY = -3;
        break;
      case 3: // Kızgın
        this.targetLeft.browAngle = 22; this.targetRight.browAngle = -22;
        this.targetLeft.browY = 4; this.targetRight.browY = 4;
        this.targetLeft.top = 0.45; this.targetRight.top = 0.45;
        break;
      case 4: // Huysuz
        this.targetLeft.h *= 0.55; this.targetRight.h *= 0.55;
        this.targetLeft.browAngle = 10; this.targetRight.browAngle = -10;
        this.targetLeft.top = 0.5; this.targetRight.top = 0.5;
        break;
      case 5: // Şaşırmış
        this.targetLeft.w += 6; this.targetLeft.h += 6;
        this.targetRight.w += 6; this.targetRight.h += 6;
        this.targetLeft.browY = -7; this.targetRight.browY = -7;
        break;
      case 6: // Aşık
        this.targetLeft.isHeart = true;
        this.targetRight.isHeart = true;
        this.targetLeft.browY = -4; this.targetRight.browY = -4;
        break;
      case 7: // Üzgün
        this.targetLeft.browAngle = -18; this.targetRight.browAngle = 18;
        this.targetLeft.top = 0.35; this.targetRight.top = 0.35;
        break;
      case 8: // Uykulu
        this.targetLeft.top = 0.70; this.targetRight.top = 0.70;
        this.targetLeft.browY = 3; this.targetRight.browY = 3;
        break;
      case 9: // Heyecanlı
        this.targetLeft.h += 3; this.targetRight.h += 3;
        this.targetLeft.bot = 0.40; this.targetRight.bot = 0.40;
        this.targetLeft.browY = -5; this.targetRight.browY = -5;
        break;
      case 10: // Şüpheli
        this.targetLeft.top = 0.55; this.targetLeft.bot = 0.20;
        this.targetLeft.browAngle = 15;
        this.targetRight.browAngle = -12; this.targetRight.browY = -4;
        break;
      case 11: // Göz Kırpma
        this.targetLeft.bot = 0.25;
        this.targetRight.top = 1.0;
        this.targetRight.isWink = true;
        break;
    }
  }

  update() {
    const now = Date.now();
    this.breathPhase += 0.05;

    // Göz kırpma
    if (!this.isBlinking && now > this.nextBlink) {
      this.isBlinking = true;
      this.blinkStart = now;
      this.nextBlink = now + 3000 + Math.random() * 3000;
    }
    if (this.isBlinking && (now - this.blinkStart > 160)) {
      this.isBlinking = false;
    }

    // Bakış yönü
    if (now > this.nextGaze) {
      this.nextGaze = now + 3500 + Math.random() * 4000;
      const gazes = [
        [0, 0], [0, 0], [-6, 0], [6, 0], [0, -5], [0, 5], [-4, -4], [4, -4]
      ];
      const g = gazes[Math.floor(Math.random() * gazes.length)];
      this.targetGazeX = g[0];
      this.targetGazeY = g[1];
    }

    const k = 0.22;
    this.gazeX += (this.targetGazeX - this.gazeX) * 0.25;
    this.gazeY += (this.targetGazeY - this.gazeY) * 0.25;

    let blinkFactor = 0;
    if (this.isBlinking) {
      const el = now - this.blinkStart;
      blinkFactor = el < 80 ? (el / 80) : (1.0 - ((el - 80) / 80));
    }

    const interp = (curr, target) => {
      curr.x += (target.x - curr.x) * k;
      curr.y += (target.y - curr.y) * k;
      curr.w += (target.w - curr.w) * k;
      curr.h += (target.h - curr.h) * k;
      curr.r += (target.r - curr.r) * k;
      curr.top += (Math.max(target.top, blinkFactor) - curr.top) * 0.35;
      curr.bot += (target.bot - curr.bot) * k;
      curr.browAngle += (target.browAngle - curr.browAngle) * k;
      curr.browY += (target.browY - curr.browY) * k;
      curr.isHeart = target.isHeart;
      curr.isWink = target.isWink;
    };

    interp(this.currLeft, this.targetLeft);
    interp(this.currRight, this.targetRight);
  }

  animate() {
    this.update();
    this.draw();
    requestAnimationFrame(this.animate);
  }

  draw() {
    const ctx = this.ctx;
    ctx.fillStyle = "#030508";
    ctx.fillRect(0, 0, 128, 64);
    ctx.fillStyle = "#00f2fe";
    ctx.strokeStyle = "#00f2fe";

    if (this.currentScreen === 'face') {
      this.drawFace();
    } else if (this.currentScreen === 'clock') {
      this.drawClock();
    } else if (this.currentScreen === 'weather') {
      this.drawWeather();
    }
  }

  // ==========================================
  // YÜZ ÇİZİMİ
  // ==========================================
  drawFace() {
    this.drawEye(this.currLeft, false);
    this.drawEye(this.currRight, true);
    this.drawMouth();

    if (Number(this.currentExpr) === 8) {
      this.drawZzz();
    }
  }

  drawEye(eye, isRight) {
    const ctx = this.ctx;
    const breathY = Math.sin(this.breathPhase) * 1.5;
    const cx = eye.x + this.gazeX;
    const cy = eye.y + this.gazeY + breathY;

    if (eye.isHeart) {
      const scale = 1.0 + 0.2 * Math.abs(Math.sin(Date.now() * 0.007));
      this.drawHeart(cx, cy, scale);
      return;
    }

    if (this.faceStyle === 0) {
      // Dots
      if (eye.top >= 0.85 || (isRight && eye.isWink)) {
        ctx.fillRect(cx - eye.w / 2, cy - 1, eye.w, 3);
        return;
      }
      ctx.beginPath();
      ctx.arc(cx, cy, eye.w / 2, 0, Math.PI * 2);
      ctx.fill();

      // Kapaklar
      if (eye.top > 0.05) {
        ctx.fillStyle = "#030508";
        ctx.fillRect(cx - eye.w / 2 - 2, cy - eye.h / 2 - 2, eye.w + 4, eye.h * eye.top);
        ctx.fillStyle = "#00f2fe";
      }
      if (eye.bot > 0.05) {
        ctx.fillStyle = "#030508";
        ctx.fillRect(cx - eye.w / 2 - 2, cy + eye.h / 2 - (eye.h * eye.bot), eye.w + 4, eye.h * eye.bot + 2);
        ctx.fillStyle = "#00f2fe";
      }
    } else if (this.faceStyle === 1) {
      // RoundRect
      const x0 = cx - eye.w / 2;
      const y0 = cy - eye.h / 2;

      if (eye.top >= 0.88 || (isRight && eye.isWink)) {
        this.fillRoundRect(x0, cy - 2, eye.w, 4, 2);
        return;
      }

      this.fillRoundRect(x0, y0, eye.w, eye.h, Math.min(eye.r, eye.w / 2));

      if (eye.top > 0.05) {
        ctx.fillStyle = "#030508";
        ctx.fillRect(x0 - 2, y0 - 2, eye.w + 4, eye.h * eye.top);
        ctx.fillStyle = "#00f2fe";
      }
      if (eye.bot > 0.05) {
        ctx.fillStyle = "#030508";
        ctx.fillRect(x0 - 2, y0 + eye.h - (eye.h * eye.bot), eye.w + 4, eye.h * eye.bot + 2);
        ctx.fillStyle = "#00f2fe";
      }

      // Kaşlar
      if (Math.abs(eye.browAngle) > 2 || Math.abs(eye.browY) > 1) {
        ctx.lineWidth = 2;
        const browY = y0 - 5 + eye.browY;
        const dy = Math.tan(eye.browAngle * Math.PI / 180) * (eye.w / 2);
        ctx.beginPath();
        ctx.moveTo(x0, browY - dy);
        ctx.lineTo(x0 + eye.w, browY + dy);
        ctx.stroke();
      }
    } else if (this.faceStyle === 2) {
      // Kawaii
      const r = eye.w / 2;
      if (eye.top >= 0.85 || (isRight && eye.isWink)) {
        ctx.lineWidth = 2.5;
        ctx.beginPath();
        ctx.arc(cx, cy, r, Math.PI, 0);
        ctx.stroke();
        return;
      }

      ctx.beginPath();
      ctx.arc(cx, cy, r, 0, Math.PI * 2);
      ctx.fill();

      if (eye.bot > 0.05) {
        ctx.fillStyle = "#030508";
        ctx.fillRect(cx - r - 2, cy + r - (eye.h * eye.bot), r * 2 + 4, eye.h * eye.bot + 2);
        ctx.fillStyle = "#00f2fe";
      }
      if (eye.top > 0.05) {
        ctx.fillStyle = "#030508";
        ctx.fillRect(cx - r - 2, cy - r - 2, r * 2 + 4, eye.h * eye.top);
        ctx.fillStyle = "#00f2fe";
      }

      // Parıltılar
      if (eye.top < 0.4) {
        ctx.fillStyle = "#030508";
        ctx.beginPath();
        ctx.arc(cx - 3 + this.gazeX * 0.4, cy - 4 + this.gazeY * 0.4, 3.5, 0, Math.PI * 2);
        ctx.fill();
        ctx.beginPath();
        ctx.arc(cx + 4 + this.gazeX * 0.4, cy + 4 + this.gazeY * 0.4, 2, 0, Math.PI * 2);
        ctx.fill();
        ctx.fillStyle = "#00f2fe";
      }
    } else if (this.faceStyle === 3) {
      // Pixel Robot
      const sx = cx - 12;
      const sy = cy - 12;

      if (eye.top >= 0.85 || (isRight && eye.isWink)) {
        ctx.fillRect(sx, cy - 2, 24, 4);
        return;
      }

      ctx.fillRect(sx + 4, sy, 16, 24);
      ctx.fillRect(sx, sy + 4, 24, 16);

      ctx.fillStyle = "#030508";
      ctx.fillRect(sx + 8 + this.gazeX * 0.5, sy + 8 + this.gazeY * 0.5, 6, 6);
      ctx.fillStyle = "#00f2fe";

      if (eye.top > 0.05) {
        ctx.fillStyle = "#030508";
        ctx.fillRect(sx - 2, sy - 2, 28, 24 * eye.top);
        ctx.fillStyle = "#00f2fe";
      }
    }
  }

  drawHeart(cx, cy, scale) {
    const ctx = this.ctx;
    const r = 7 * scale;
    ctx.beginPath();
    ctx.arc(cx - r, cy - 3 * scale, r, 0, Math.PI * 2);
    ctx.arc(cx + r, cy - 3 * scale, r, 0, Math.PI * 2);
    ctx.fill();

    ctx.beginPath();
    ctx.moveTo(cx - r * 2, cy - 3 * scale);
    ctx.lineTo(cx + r * 2, cy - 3 * scale);
    ctx.lineTo(cx, cy + 10 * scale);
    ctx.closePath();
    ctx.fill();
  }

  drawMouth() {
    const ctx = this.ctx;
    const breathY = Math.sin(this.breathPhase) * 1.5;
    const mouthY = 52 + breathY;

    if (this.faceStyle === 2 || Number(this.currentExpr) === 2 || Number(this.currentExpr) === 6) {
      // Yanak Allıkları
      ctx.lineWidth = 1.5;
      const cheekY = 44 + breathY;
      ctx.beginPath();
      ctx.moveTo(16, cheekY + 2); ctx.lineTo(22, cheekY - 2);
      ctx.moveTo(20, cheekY + 2); ctx.lineTo(26, cheekY - 2);
      ctx.moveTo(102, cheekY - 2); ctx.lineTo(108, cheekY + 2);
      ctx.moveTo(106, cheekY - 2); ctx.lineTo(112, cheekY + 2);
      ctx.stroke();
    }

    if (this.faceStyle === 2 || Number(this.currentExpr) === 1 || Number(this.currentExpr) === 9) {
      // 'w' Kedi Ağzı
      ctx.lineWidth = 1.5;
      ctx.beginPath();
      ctx.arc(60, mouthY, 4, 0, Math.PI);
      ctx.arc(68, mouthY, 4, 0, Math.PI);
      ctx.stroke();
    } else if (Number(this.currentExpr) === 5) {
      // 'o' Ağız
      ctx.beginPath();
      ctx.arc(64, mouthY, 3, 0, Math.PI * 2);
      ctx.stroke();
    }
  }

  drawZzz() {
    const ctx = this.ctx;
    const t = Math.floor(Date.now() / 250);
    const off = (t % 16);
    ctx.font = "8px monospace";
    ctx.fillText("z", 104 + off / 2, 22 - off);
    ctx.font = "12px monospace";
    ctx.fillText("Z", 112 + off / 2, 16 - off);
  }

  // ==========================================
  // SAAT ÇİZİMİ
  // ==========================================
  drawClock() {
    const ctx = this.ctx;
    const d = new Date();
    const h = String(d.getHours()).padStart(2, '0');
    const m = String(d.getMinutes()).padStart(2, '0');
    const s = String(d.getSeconds()).padStart(2, '0');

    if (this.clockStyle === 0) {
      // Big Digital
      ctx.font = "bold 26px monospace";
      ctx.textAlign = "center";
      const colon = (d.getSeconds() % 2 === 0) ? ":" : " ";
      ctx.fillText(`${h}${colon}${m}`, 56, 42);
      ctx.font = "bold 11px monospace";
      ctx.fillText(`:${s}`, 102, 42);
      ctx.fillRect(20, 10, 88, 1);
    } else if (this.clockStyle === 1) {
      // Mini Face
      ctx.font = "bold 13px monospace";
      ctx.textAlign = "center";
      ctx.fillText(`${h}:${m}:${s}`, 64, 20);
      ctx.fillRect(28, 26, 72, 1);

      // Mini gözler
      const blink = (d.getSeconds() % 8 === 0);
      if (blink) {
        ctx.fillRect(44, 46, 12, 2);
        ctx.fillRect(72, 46, 12, 2);
      } else {
        this.fillRoundRect(44, 38, 12, 16, 4);
        this.fillRoundRect(72, 38, 12, 16, 4);
      }
    } else if (this.clockStyle === 2) {
      // Analog
      const cx = 64, cy = 32, r = 27;
      ctx.lineWidth = 1;
      ctx.beginPath();
      ctx.arc(cx, cy, r, 0, Math.PI * 2);
      ctx.stroke();

      const hrAngle = ((d.getHours() % 12) + d.getMinutes() / 60) * (Math.PI / 6) - Math.PI / 2;
      const minAngle = (d.getMinutes() + d.getSeconds() / 60) * (Math.PI / 30) - Math.PI / 2;
      const secAngle = d.getSeconds() * (Math.PI / 30) - Math.PI / 2;

      ctx.lineWidth = 2.5;
      ctx.beginPath();
      ctx.moveTo(cx, cy);
      ctx.lineTo(cx + Math.cos(hrAngle) * 14, cy + Math.sin(hrAngle) * 14);
      ctx.stroke();

      ctx.lineWidth = 1.5;
      ctx.beginPath();
      ctx.moveTo(cx, cy);
      ctx.lineTo(cx + Math.cos(minAngle) * 20, cy + Math.sin(minAngle) * 20);
      ctx.stroke();

      ctx.lineWidth = 1;
      ctx.beginPath();
      ctx.moveTo(cx, cy);
      ctx.lineTo(cx + Math.cos(secAngle) * 23, cy + Math.sin(secAngle) * 23);
      ctx.stroke();
    } else if (this.clockStyle === 3) {
      // Retro 7-Seg
      ctx.font = "bold 22px 'JetBrains Mono', monospace";
      ctx.textAlign = "center";
      const colon = (d.getSeconds() % 2 === 0) ? ":" : " ";
      ctx.fillText(`${h}${colon}${m}`, 64, 42);
    }
  }

  // ==========================================
  // HAVA DURUMU ÇİZİMİ
  // ==========================================
  drawWeather() {
    const ctx = this.ctx;
    const { temp, code, min, max, humidity, name } = this.weatherData;

    if (this.weatherStyle === 0) {
      // Big Icon
      this.drawWeatherIcon(code, 36, 32, false);
      ctx.font = "bold 24px monospace";
      ctx.textAlign = "left";
      ctx.fillText(`${temp}°C`, 68, 36);
      ctx.font = "10px monospace";
      ctx.fillText(`${min}° / ${max}°`, 70, 52);
    } else if (this.weatherStyle === 1) {
      // Animated Icon
      this.drawWeatherIcon(code, 36, 32, true);
      ctx.font = "bold 22px monospace";
      ctx.textAlign = "left";
      ctx.fillText(`${temp}°C`, 68, 34);
      ctx.font = "10px sans-serif";
      ctx.fillText(name, 70, 50);
    } else if (this.weatherStyle === 2) {
      // Compact Text
      ctx.textAlign = "center";
      ctx.font = "bold 11px sans-serif";
      ctx.fillText(name, 64, 16);
      ctx.font = "bold 22px monospace";
      ctx.fillText(`${temp}°C`, 64, 40);
      ctx.font = "9px monospace";
      ctx.fillText(`Min:${min}° Max:${max}° Nem:%${humidity}`, 64, 56);
    }
  }

  drawWeatherIcon(code, cx, cy, animated) {
    const ctx = this.ctx;
    ctx.lineWidth = 1.5;

    if (code === 0) {
      // Güneş
      ctx.beginPath();
      ctx.arc(cx, cy, 8, 0, Math.PI * 2);
      ctx.fill();

      const base = animated ? (Date.now() * 0.002) : 0;
      for (let i = 0; i < 8; i++) {
        const a = base + (i * Math.PI / 4);
        ctx.beginPath();
        ctx.moveTo(cx + Math.cos(a) * 11, cy + Math.sin(a) * 11);
        ctx.lineTo(cx + Math.cos(a) * 16, cy + Math.sin(a) * 16);
        ctx.stroke();
      }
    } else if (code === 1 || code === 2) {
      // Bulutlu
      const shiftX = animated ? Math.sin(Date.now() * 0.003) * 3 : 0;
      const x = cx + shiftX;
      ctx.beginPath();
      ctx.arc(x - 8, cy + 2, 7, 0, Math.PI * 2);
      ctx.arc(x + 2, cy - 3, 9, 0, Math.PI * 2);
      ctx.arc(x + 11, cy + 3, 6, 0, Math.PI * 2);
      ctx.fill();
    } else if (code === 3) {
      // Yağmur
      ctx.beginPath();
      ctx.arc(cx - 8, cy - 4, 7, 0, Math.PI * 2);
      ctx.arc(cx + 2, cy - 9, 9, 0, Math.PI * 2);
      ctx.arc(cx + 11, cy - 3, 6, 0, Math.PI * 2);
      ctx.fill();

      const anim = animated ? ((Date.now() / 50) % 8) : 0;
      for (let i = -1; i <= 1; i++) {
        const rx = cx + (i * 9);
        const ry = cy + 4 + anim;
        ctx.beginPath();
        ctx.moveTo(rx, ry);
        ctx.lineTo(rx - 2, ry + 4);
        ctx.stroke();
      }
    }
  }

  fillRoundRect(x, y, w, h, r) {
    const ctx = this.ctx;
    ctx.beginPath();
    ctx.roundRect(x, y, w, h, r);
    ctx.fill();
  }
}

window.OledSimulator = OledSimulator;
