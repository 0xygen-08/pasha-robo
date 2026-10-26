#ifndef PASHA_FACE_ENGINE_H
#define PASHA_FACE_ENGINE_H

#include <Arduino.h>
#include "types.h"
#include "config.h"
#include "display_hal.h"

class FaceEngine {
public:
    static FaceEngine& getInstance();

    void begin();
    void update();
    void draw();

    // Duygu & Stil Değişimi
    void setExpression(ExpressionId expr, bool manual = false);
    ExpressionId getExpression() const { return currentExpr; }

    void setFaceStyle(FaceStyle style);
    FaceStyle getFaceStyle() const { return faceStyle; }

    void setAutoMode(bool autoMode);
    bool isAutoMode() const { return isAutoExprMode; }

    void setGaze(GazeDirection gaze);

    // Canlılık tetikleyicileri
    void triggerBlink();
    void triggerEyeRoll();

private:
    FaceEngine();
    ~FaceEngine() = default;

    // Hedef parametreleri ifadeden yükleme
    void applyExpressionTargets(ExpressionId expr);
    void updateInterpolation();
    void updateBehaviors();

    // Stil çizim metodları
    void drawEye(const EyeParameters& eye, bool isRight);
    void drawStyleDots(const EyeParameters& eye, bool isRight);
    void drawStyleRoundRect(const EyeParameters& eye, bool isRight);
    void drawStyleKawaii(const EyeParameters& eye, bool isRight);
    void drawStylePixel(const EyeParameters& eye, bool isRight);

    void drawMouthAndCheeks();
    void drawSleepZzz();

    // Durum değişkenleri
    ExpressionId currentExpr;
    ExpressionId targetExpr;
    FaceStyle faceStyle;
    bool isAutoExprMode;

    EyeParameters currLeft;
    EyeParameters targetLeft;
    EyeParameters currRight;
    EyeParameters targetRight;

    // Canlılık & Zamanlayıcılar
    uint32_t lastExprChangeMs;
    uint32_t nextExprIntervalMs;

    uint32_t lastBlinkMs;
    uint32_t nextBlinkIntervalMs;
    bool isBlinking;
    uint32_t blinkStartMs;

    uint32_t lastGazeMs;
    uint32_t nextGazeIntervalMs;
    float currentGazeX;
    float currentGazeY;
    float targetGazeX;
    float targetGazeY;

    // Nefes ve nabız fazı
    float breathPhase;
    float heartBeatScale;

    // Özel animasyon durumları
    bool isEyeRolling;
    float eyeRollAngle;
};

#endif // PASHA_FACE_ENGINE_H
