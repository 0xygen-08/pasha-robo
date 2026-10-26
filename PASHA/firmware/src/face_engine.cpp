#include "face_engine.h"
#include <math.h>

static float pashaLerp(float a, float b, float t) {
    return a + (b - a) * t;
}

FaceEngine& FaceEngine::getInstance() {
    static FaceEngine instance;
    return instance;
}

FaceEngine::FaceEngine() :
    currentExpr(EXPR_NORMAL),
    targetExpr(EXPR_NORMAL),
    faceStyle(static_cast<FaceStyle>(DEFAULT_FACE_STYLE)),
    isAutoExprMode(DEFAULT_EXPR_MODE == 1),
    lastExprChangeMs(0),
    nextExprIntervalMs(8000),
    lastBlinkMs(0),
    nextBlinkIntervalMs(3500),
    isBlinking(false),
    blinkStartMs(0),
    lastGazeMs(0),
    nextGazeIntervalMs(4000),
    currentGazeX(0.0f),
    currentGazeY(0.0f),
    targetGazeX(0.0f),
    targetGazeY(0.0f),
    breathPhase(0.0f),
    heartBeatScale(1.0f),
    isEyeRolling(false),
    eyeRollAngle(0.0f)
{
    // Varsayılan göz başlangıç parametreleri
    currLeft = {38.0f, 32.0f, 24.0f, 28.0f, 8.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, false, false, false};
    currRight = {90.0f, 32.0f, 24.0f, 28.0f, 8.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, false, false, false};
    targetLeft = currLeft;
    targetRight = currRight;
}

void FaceEngine::begin() {
    applyExpressionTargets(currentExpr);
    currLeft = targetLeft;
    currRight = targetRight;
    lastExprChangeMs = millis();
    lastBlinkMs = millis();
    lastGazeMs = millis();
}

void FaceEngine::setExpression(ExpressionId expr, bool manual) {
    if (expr >= EXPR_COUNT) expr = EXPR_NORMAL;
    targetExpr = expr;
    currentExpr = expr;
    if (manual) {
        isAutoExprMode = false;
    }
    applyExpressionTargets(expr);
    lastExprChangeMs = millis();
}

void FaceEngine::setFaceStyle(FaceStyle style) {
    if (style >= FACE_STYLE_COUNT) style = FACE_STYLE_ROUND_RECT;
    faceStyle = style;
    applyExpressionTargets(currentExpr);
}

void FaceEngine::setAutoMode(bool autoMode) {
    isAutoExprMode = autoMode;
    lastExprChangeMs = millis();
}

void FaceEngine::setGaze(GazeDirection gaze) {
    switch (gaze) {
        case GAZE_LEFT:       targetGazeX = -7.0f; targetGazeY = 0.0f; break;
        case GAZE_RIGHT:      targetGazeX = 7.0f;  targetGazeY = 0.0f; break;
        case GAZE_UP:         targetGazeX = 0.0f;  targetGazeY = -6.0f; break;
        case GAZE_DOWN:       targetGazeX = 0.0f;  targetGazeY = 6.0f; break;
        case GAZE_UP_LEFT:    targetGazeX = -5.0f; targetGazeY = -5.0f; break;
        case GAZE_UP_RIGHT:   targetGazeX = 5.0f;  targetGazeY = -5.0f; break;
        case GAZE_DOWN_LEFT:  targetGazeX = -5.0f; targetGazeY = 5.0f; break;
        case GAZE_DOWN_RIGHT: targetGazeX = 5.0f;  targetGazeY = 5.0f; break;
        case GAZE_CENTER:
        default:              targetGazeX = 0.0f;  targetGazeY = 0.0f; break;
    }
}

void FaceEngine::triggerBlink() {
    isBlinking = true;
    blinkStartMs = millis();
}

void FaceEngine::triggerEyeRoll() {
    isEyeRolling = true;
    eyeRollAngle = 0.0f;
}

// 12 İfadenin hedef geometrik parametreleri
void FaceEngine::applyExpressionTargets(ExpressionId expr) {
    // Temel göz boyutları
    float baseW = 24.0f;
    float baseH = 28.0f;
    float baseR = 8.0f;

    if (faceStyle == FACE_STYLE_DOTS) {
        baseW = 22.0f;
        baseH = 22.0f;
        baseR = 11.0f;
    } else if (faceStyle == FACE_STYLE_KAWAII) {
        baseW = 28.0f;
        baseH = 32.0f;
        baseR = 14.0f;
    } else if (faceStyle == FACE_STYLE_PIXEL) {
        baseW = 24.0f;
        baseH = 24.0f;
        baseR = 0.0f;
    }

    targetLeft.x = 38.0f;
    targetLeft.y = 32.0f;
    targetLeft.width = baseW;
    targetLeft.height = baseH;
    targetLeft.radius = baseR;
    targetLeft.eyelidTop = 0.0f;
    targetLeft.eyelidBottom = 0.0f;
    targetLeft.browAngle = 0.0f;
    targetLeft.browOffsetY = 0.0f;
    targetLeft.isHeart = false;
    targetLeft.isSquint = false;
    targetLeft.isWink = false;

    targetRight = targetLeft;
    targetRight.x = 90.0f;

    switch (expr) {
        case EXPR_NORMAL:
            // Nötr sevimli
            break;

        case EXPR_HAPPY:
            // Hilal şeklinde mutlu gözler
            targetLeft.eyelidBottom = 0.65f;
            targetRight.eyelidBottom = 0.65f;
            targetLeft.browOffsetY = -2.0f;
            targetRight.browOffsetY = -2.0f;
            break;

        case EXPR_SWEET:
            // Tatlı ve mahcup
            targetLeft.width = baseW + 2.0f;
            targetRight.width = baseW + 2.0f;
            targetLeft.eyelidBottom = 0.35f;
            targetRight.eyelidBottom = 0.35f;
            targetLeft.browOffsetY = -3.0f;
            targetRight.browOffsetY = -3.0f;
            break;

        case EXPR_ANGRY:
            // Çatık kaşlar ve basık üst kapaklar
            targetLeft.browAngle = 22.0f;
            targetRight.browAngle = -22.0f;
            targetLeft.browOffsetY = 4.0f;
            targetRight.browOffsetY = 4.0f;
            targetLeft.eyelidTop = 0.45f;
            targetRight.eyelidTop = 0.45f;
            break;

        case EXPR_GRUMPY:
            // Huysuz ve kısık
            targetLeft.height = baseH * 0.55f;
            targetRight.height = baseH * 0.55f;
            targetLeft.browAngle = 10.0f;
            targetRight.browAngle = -10.0f;
            targetLeft.browOffsetY = 2.0f;
            targetRight.browOffsetY = 2.0f;
            break;

        case EXPR_SURPRISED:
            // Kocaman yuvarlak ve kalkık kaşlar
            targetLeft.width = baseW + 6.0f;
            targetLeft.height = baseH + 6.0f;
            targetRight.width = baseW + 6.0f;
            targetRight.height = baseH + 6.0f;
            targetLeft.browOffsetY = -7.0f;
            targetRight.browOffsetY = -7.0f;
            break;

        case EXPR_IN_LOVE:
            // Kalp gözler
            targetLeft.isHeart = true;
            targetRight.isHeart = true;
            targetLeft.browOffsetY = -4.0f;
            targetRight.browOffsetY = -4.0f;
            break;

        case EXPR_SAD:
            // Üzgün aşağı eğimli kaşlar ve gözler
            targetLeft.browAngle = -18.0f;
            targetRight.browAngle = 18.0f;
            targetLeft.browOffsetY = -2.0f;
            targetRight.browOffsetY = -2.0f;
            targetLeft.eyelidTop = 0.35f;
            targetRight.eyelidTop = 0.35f;
            break;

        case EXPR_SLEEPY:
            // Yarı kapalı uykulu gözler
            targetLeft.eyelidTop = 0.70f;
            targetRight.eyelidTop = 0.70f;
            targetLeft.browOffsetY = 3.0f;
            targetRight.browOffsetY = 3.0f;
            break;

        case EXPR_EXCITED:
            // Heyecanlı parıldayan gözler
            targetLeft.height = baseH + 3.0f;
            targetRight.height = baseH + 3.0f;
            targetLeft.eyelidBottom = 0.40f;
            targetRight.eyelidBottom = 0.40f;
            targetLeft.browOffsetY = -5.0f;
            targetRight.browOffsetY = -5.0f;
            break;

        case EXPR_SUSPICIOUS:
            // Sol göz kısık ve şüpheli, sağ göz açık
            targetLeft.eyelidTop = 0.55f;
            targetLeft.eyelidBottom = 0.20f;
            targetLeft.browAngle = 15.0f;
            targetRight.browAngle = -12.0f;
            targetRight.browOffsetY = -4.0f;
            break;

        case EXPR_WINK:
            // Sol açık, sağ tam kırpılmış
            targetLeft.eyelidBottom = 0.25f;
            targetRight.eyelidTop = 1.0f;
            targetRight.isWink = true;
            break;

        default:
            break;
    }
}

// Canlılık davranışları (Kırpma, bakış, otomatik mod, nefes)
void FaceEngine::updateBehaviors() {
    uint32_t now = millis();

    // 1. Nefes alma ritmi (~2.2 saniye döngü)
    breathPhase += 0.05f;
    if (breathPhase > 6.28318f) breathPhase -= 6.28318f;

    // 2. Kalp atışı ritmi (Aşık ifadesinde)
    if (currentExpr == EXPR_IN_LOVE) {
        heartBeatScale = 1.0f + 0.18f * fabsf(sinf(now * 0.007f));
    } else {
        heartBeatScale = 1.0f;
    }

    // 3. Göz kırpma (Blink)
    if (!isBlinking && (now - lastBlinkMs > nextBlinkIntervalMs)) {
        triggerBlink();
        lastBlinkMs = now;
        nextBlinkIntervalMs = random(BLINK_INTERVAL_MIN_MS, BLINK_INTERVAL_MAX_MS);
    }

    if (isBlinking) {
        uint32_t elapsed = now - blinkStartMs;
        if (elapsed > BLINK_DURATION_MS) {
            isBlinking = false;
        }
    }

    // 4. Rastgele Bakış (Gaze wandering)
    if (now - lastGazeMs > nextGazeIntervalMs) {
        lastGazeMs = now;
        nextGazeIntervalMs = random(GAZE_INTERVAL_MIN_MS, GAZE_INTERVAL_MAX_MS);

        uint8_t r = random(0, 10);
        if (r < 4) {
            setGaze(GAZE_CENTER);
        } else if (r == 4) {
            setGaze(GAZE_LEFT);
        } else if (r == 5) {
            setGaze(GAZE_RIGHT);
        } else if (r == 6) {
            setGaze(GAZE_UP);
        } else if (r == 7) {
            setGaze(GAZE_DOWN);
        } else if (r == 8) {
            setGaze(GAZE_UP_RIGHT);
        } else {
            setGaze(GAZE_UP_LEFT);
        }

        // Bazen sürpriz göz devirme
        if (random(0, 20) == 0) {
            triggerEyeRoll();
        }
    }

    // Göz devirme animasyonu
    if (isEyeRolling) {
        eyeRollAngle += 0.25f;
        targetGazeX = cosf(eyeRollAngle) * 6.0f;
        targetGazeY = sinf(eyeRollAngle) * 5.0f;
        if (eyeRollAngle >= 6.28318f) {
            isEyeRolling = false;
            targetGazeX = 0.0f;
            targetGazeY = 0.0f;
        }
    }

    // 5. Otomatik modda rastgele duygu değişimi
    if (isAutoExprMode && (now - lastExprChangeMs > nextExprIntervalMs)) {
        lastExprChangeMs = now;
        nextExprIntervalMs = random(AUTO_EXPR_MIN_MS, AUTO_EXPR_MAX_MS);

        // Sevimli bir sonraki ifade seç
        ExpressionId next = static_cast<ExpressionId>(random(0, EXPR_COUNT));
        setExpression(next, false);
    }
}

// Parametrelerin interpolasyonu (Yumuşak geçiş)
void FaceEngine::updateInterpolation() {
    float k = 0.22f; // Ease-out katsayısı

    // Gaze yumuşak yaklaşımı
    currentGazeX = pashaLerp(currentGazeX, targetGazeX, 0.25f);
    currentGazeY = pashaLerp(currentGazeY, targetGazeY, 0.25f);

    // Kırpma durumunda kapak geçersiz kılması
    float blinkFactor = 0.0f;
    if (isBlinking) {
        uint32_t elapsed = millis() - blinkStartMs;
        float half = BLINK_DURATION_MS / 2.0f;
        if (elapsed < half) {
            blinkFactor = elapsed / half;
        } else {
            blinkFactor = 1.0f - ((elapsed - half) / half);
        }
    }

    // Sol göz interpolasyonu
    currLeft.x = pashaLerp(currLeft.x, targetLeft.x, k);
    currLeft.y = pashaLerp(currLeft.y, targetLeft.y, k);
    currLeft.width = pashaLerp(currLeft.width, targetLeft.width, k);
    currLeft.height = pashaLerp(currLeft.height, targetLeft.height, k);
    currLeft.radius = pashaLerp(currLeft.radius, targetLeft.radius, k);
    currLeft.eyelidTop = pashaLerp(currLeft.eyelidTop, fmaxf(targetLeft.eyelidTop, blinkFactor), 0.35f);
    currLeft.eyelidBottom = pashaLerp(currLeft.eyelidBottom, targetLeft.eyelidBottom, k);
    currLeft.browAngle = pashaLerp(currLeft.browAngle, targetLeft.browAngle, k);
    currLeft.browOffsetY = pashaLerp(currLeft.browOffsetY, targetLeft.browOffsetY, k);
    currLeft.isHeart = targetLeft.isHeart;
    currLeft.isWink = targetLeft.isWink;

    // Sağ göz interpolasyonu
    currRight.x = pashaLerp(currRight.x, targetRight.x, k);
    currRight.y = pashaLerp(currRight.y, targetRight.y, k);
    currRight.width = pashaLerp(currRight.width, targetRight.width, k);
    currRight.height = pashaLerp(currRight.height, targetRight.height, k);
    currRight.radius = pashaLerp(currRight.radius, targetRight.radius, k);
    currRight.eyelidTop = pashaLerp(currRight.eyelidTop, fmaxf(targetRight.eyelidTop, blinkFactor), 0.35f);
    currRight.eyelidBottom = pashaLerp(currRight.eyelidBottom, targetRight.eyelidBottom, k);
    currRight.browAngle = pashaLerp(currRight.browAngle, targetRight.browAngle, k);
    currRight.browOffsetY = pashaLerp(currRight.browOffsetY, targetRight.browOffsetY, k);
    currRight.isHeart = targetRight.isHeart;
    currRight.isWink = targetRight.isWink;
}

void FaceEngine::update() {
    updateBehaviors();
    updateInterpolation();
}

void FaceEngine::draw() {
    drawEye(currLeft, false);
    drawEye(currRight, true);
    drawMouthAndCheeks();

    if (currentExpr == EXPR_SLEEPY) {
        drawSleepZzz();
    }
}

// Göz çizimi yönlendirici
void FaceEngine::drawEye(const EyeParameters& eye, bool isRight) {
    if (eye.isHeart) {
        float cx = eye.x + currentGazeX;
        float cy = eye.y + currentGazeY + sinf(breathPhase) * 1.2f;
        DisplayHAL::getInstance().drawSmoothHeart((int16_t)cx, (int16_t)cy, heartBeatScale);
        return;
    }

    switch (faceStyle) {
        case FACE_STYLE_DOTS:
            drawStyleDots(eye, isRight);
            break;
        case FACE_STYLE_ROUND_RECT:
            drawStyleRoundRect(eye, isRight);
            break;
        case FACE_STYLE_KAWAII:
            drawStyleKawaii(eye, isRight);
            break;
        case FACE_STYLE_PIXEL:
            drawStylePixel(eye, isRight);
            break;
        default:
            drawStyleRoundRect(eye, isRight);
            break;
    }
}

// 1. Stil: Minimal Dots (Yuvarlak Gözler)
void FaceEngine::drawStyleDots(const EyeParameters& eye, bool isRight) {
    U8G2& u8g2 = DisplayHAL::getInstance().getU8g2();

    float breathY = sinf(breathPhase) * 1.0f;
    int16_t cx = (int16_t)(eye.x + currentGazeX);
    int16_t cy = (int16_t)(eye.y + currentGazeY + breathY);
    int16_t r = (int16_t)(eye.width / 2.0f);

    // Tam göz kırpılmış / çizgi ise
    if (eye.eyelidTop >= 0.85f || (isRight && eye.isWink)) {
        u8g2.drawHLine(cx - r, cy, r * 2 + 1);
        u8g2.drawHLine(cx - r + 1, cy + 1, (r * 2) - 1);
        return;
    }

    // Ana daire
    u8g2.drawDisc(cx, cy, r, U8G2_DRAW_ALL);

    // Üst göz kapağı kesimi
    if (eye.eyelidTop > 0.05f) {
        int16_t cutH = (int16_t)(eye.height * eye.eyelidTop);
        u8g2.setDrawColor(0);
        u8g2.drawBox(cx - r - 2, cy - r - 2, (r * 2) + 4, cutH);
        u8g2.setDrawColor(1);
    }

    // Alt göz kapağı kesimi (Gülümseme hilali)
    if (eye.eyelidBottom > 0.05f) {
        int16_t cutH = (int16_t)(eye.height * eye.eyelidBottom);
        u8g2.setDrawColor(0);
        u8g2.drawBox(cx - r - 2, cy + r - cutH + 2, (r * 2) + 4, cutH + 2);
        u8g2.setDrawColor(1);
    }
}

// 2. Stil: Oval / Yuvarlatılmış Dikdörtgen (Klasik Robot) + Kaş
void FaceEngine::drawStyleRoundRect(const EyeParameters& eye, bool isRight) {
    U8G2& u8g2 = DisplayHAL::getInstance().getU8g2();

    float breathY = sinf(breathPhase) * 1.5f;
    int16_t cx = (int16_t)(eye.x + currentGazeX);
    int16_t cy = (int16_t)(eye.y + currentGazeY + breathY);
    int16_t w = (int16_t)eye.width;
    int16_t h = (int16_t)eye.height;
    int16_t r = (int16_t)eye.radius;

    int16_t x0 = cx - (w / 2);
    int16_t y0 = cy - (h / 2);

    // Göz kırpılmışsa sevimli çizgi
    if (eye.eyelidTop >= 0.88f || (isRight && eye.isWink)) {
        u8g2.drawRBox(x0, cy - 2, w, 4, 2);
        return;
    }

    // Yuvarlatılmış gövde
    u8g2.drawRBox(x0, y0, w, h, r);

    // Üst kapak kesimi
    if (eye.eyelidTop > 0.05f) {
        int16_t cutH = (int16_t)(h * eye.eyelidTop);
        u8g2.setDrawColor(0);
        u8g2.drawBox(x0 - 2, y0 - 2, w + 4, cutH + 2);
        u8g2.setDrawColor(1);
    }

    // Alt kapak kesimi
    if (eye.eyelidBottom > 0.05f) {
        int16_t cutH = (int16_t)(h * eye.eyelidBottom);
        u8g2.setDrawColor(0);
        u8g2.drawBox(x0 - 2, y0 + h - cutH, w + 4, cutH + 2);
        u8g2.setDrawColor(1);
    }

    // Kaş Çizimi
    if (fabsf(eye.browAngle) > 2.0f || fabsf(eye.browOffsetY) > 1.0f) {
        int16_t browY = y0 - 5 + (int16_t)eye.browOffsetY;
        int16_t dy = (int16_t)(tanf(eye.browAngle * 0.01745f) * (w / 2));
        int16_t bx1 = x0;
        int16_t by1 = browY - dy;
        int16_t bx2 = x0 + w;
        int16_t by2 = browY + dy;
        DisplayHAL::getInstance().drawEyeBrow(bx1, by1, bx2, by2, 2);
    }
}

// 3. Stil: Kawaii (Büyük Gözler + Parıltı + Allık + Minik Ağız)
void FaceEngine::drawStyleKawaii(const EyeParameters& eye, bool isRight) {
    U8G2& u8g2 = DisplayHAL::getInstance().getU8g2();

    float breathY = sinf(breathPhase) * 1.5f;
    int16_t cx = (int16_t)(eye.x + currentGazeX);
    int16_t cy = (int16_t)(eye.y + currentGazeY + breathY);
    int16_t r = (int16_t)(eye.width / 2.0f);

    // Kırpık göz tebessüm yayı
    if (eye.eyelidTop >= 0.85f || (isRight && eye.isWink)) {
        u8g2.drawCircle(cx, cy, r, U8G2_DRAW_UPPER_RIGHT | U8G2_DRAW_UPPER_LEFT);
        u8g2.drawCircle(cx, cy + 1, r, U8G2_DRAW_UPPER_RIGHT | U8G2_DRAW_UPPER_LEFT);
        return;
    }

    // Ana göz diski
    u8g2.drawDisc(cx, cy, r, U8G2_DRAW_ALL);

    // Kapak kesimleri
    if (eye.eyelidBottom > 0.05f) {
        int16_t cutH = (int16_t)(eye.height * eye.eyelidBottom);
        u8g2.setDrawColor(0);
        u8g2.drawBox(cx - r - 2, cy + r - cutH + 1, (r * 2) + 4, cutH + 2);
        u8g2.setDrawColor(1);
    }

    if (eye.eyelidTop > 0.05f) {
        int16_t cutH = (int16_t)(eye.height * eye.eyelidTop);
        u8g2.setDrawColor(0);
        u8g2.drawBox(cx - r - 2, cy - r - 2, (r * 2) + 4, cutH + 2);
        u8g2.setDrawColor(1);
    }

    // Kawaii Göz Parıltıları (Highlights)
    if (eye.eyelidTop < 0.4f) {
        u8g2.setDrawColor(0); // Siyah delik / highlight
        int16_t hx1 = cx - 3 + (int16_t)(currentGazeX * 0.4f);
        int16_t hy1 = cy - 4 + (int16_t)(currentGazeY * 0.4f);
        u8g2.drawDisc(hx1, hy1, 4, U8G2_DRAW_ALL);

        int16_t hx2 = cx + 4 + (int16_t)(currentGazeX * 0.4f);
        int16_t hy2 = cy + 4 + (int16_t)(currentGazeY * 0.4f);
        u8g2.drawDisc(hx2, hy2, 2, U8G2_DRAW_ALL);
        u8g2.setDrawColor(1);
    }
}

// 4. Stil: Retro Piksel Robot (8-bit bloklar)
void FaceEngine::drawStylePixel(const EyeParameters& eye, bool isRight) {
    U8G2& u8g2 = DisplayHAL::getInstance().getU8g2();

    float breathY = sinf(breathPhase) * 1.0f;
    int16_t cx = (int16_t)(eye.x + currentGazeX);
    int16_t cy = (int16_t)(eye.y + currentGazeY + breathY);

    // 6x6 piksel bloklarından oluşan 24x24 göz
    int16_t startX = cx - 12;
    int16_t startY = cy - 12;

    if (eye.eyelidTop >= 0.85f || (isRight && eye.isWink)) {
        // Tek sıra piksel
        u8g2.drawBox(startX, cy - 2, 24, 4);
        return;
    }

    // Piksel göz gövdesi (köşeleri kırpık blok)
    u8g2.drawBox(startX + 4, startY, 16, 24);
    u8g2.drawBox(startX, startY + 4, 24, 16);

    // Gözbebeği boşluğu (Piksel boşluk)
    int16_t pX = startX + 8 + (int16_t)(currentGazeX * 0.5f);
    int16_t pY = startY + 8 + (int16_t)(currentGazeY * 0.5f);
    u8g2.setDrawColor(0);
    u8g2.drawBox(pX, pY, 6, 6);
    u8g2.setDrawColor(1);

    // Kapak kesimi
    if (eye.eyelidTop > 0.05f) {
        int16_t cutH = (int16_t)(24 * eye.eyelidTop);
        u8g2.setDrawColor(0);
        u8g2.drawBox(startX - 2, startY - 2, 28, cutH + 2);
        u8g2.setDrawColor(1);
    }
}

// Sevimli ağız ve allık çizimi
void FaceEngine::drawMouthAndCheeks() {
    U8G2& u8g2 = DisplayHAL::getInstance().getU8g2();
    float breathY = sinf(breathPhase) * 1.5f;

    // Kawaii veya Sweet / InLove durumlarında yanak allıkları
    if (faceStyle == FACE_STYLE_KAWAII || currentExpr == EXPR_SWEET || currentExpr == EXPR_IN_LOVE) {
        int16_t cheekY = (int16_t)(44 + breathY);
        // Sol yanak çizgileri
        u8g2.drawLine(16, cheekY + 2, 22, cheekY - 2);
        u8g2.drawLine(20, cheekY + 2, 26, cheekY - 2);
        // Sağ yanak çizgileri
        u8g2.drawLine(102, cheekY - 2, 108, cheekY + 2);
        u8g2.drawLine(106, cheekY - 2, 112, cheekY + 2);
    }

    // Sevimli küçük ağız
    int16_t mouthY = (int16_t)(52 + breathY);
    if (faceStyle == FACE_STYLE_KAWAII || currentExpr == EXPR_HAPPY || currentExpr == EXPR_EXCITED) {
        // 'w' şeklinde minik kedi tebessümü
        u8g2.drawCircle(60, mouthY, 4, U8G2_DRAW_LOWER_RIGHT | U8G2_DRAW_LOWER_LEFT);
        u8g2.drawCircle(68, mouthY, 4, U8G2_DRAW_LOWER_RIGHT | U8G2_DRAW_LOWER_LEFT);
    } else if (currentExpr == EXPR_SURPRISED) {
        // Küçük 'o' ağız
        u8g2.drawCircle(64, mouthY, 3, U8G2_DRAW_ALL);
    }
}

// Uykulu modda uçuşan 'Z' harfleri
void FaceEngine::drawSleepZzz() {
    U8G2& u8g2 = DisplayHAL::getInstance().getU8g2();
    uint32_t t = millis() / 250;

    int16_t off1 = (t % 16);
    int16_t off2 = ((t + 6) % 16);

    u8g2.setFont(u8g2_font_5x7_tf);
    u8g2.drawStr(104 + off1 / 2, 22 - off1, "z");
    u8g2.setFont(u8g2_font_6x12_tf);
    u8g2.drawStr(112 + off2 / 2, 16 - off2, "Z");
}
