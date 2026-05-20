#ifndef ANIMATION_H
#define ANIMATION_H

#include <Arduino.h>

extern uint8_t ledBuffer[192]; // 메인 코드의 버퍼 공유

void updateAnimation(int mode) {
    static uint32_t frameCount = 0;
    frameCount++;

    switch (mode) {
        case 1: // [애니메이션 1: 전체 레인보우 흐름 (단색 버전)]
            for (int i = 0; i < 64; i++) {
                uint8_t val = (sin((i + frameCount) * 0.2) + 1) * 127;
                ledBuffer[i * 3] = val;     // R
                ledBuffer[i * 3 + 1] = 0;   // G
                ledBuffer[i * 3 + 2] = 255 - val; // B
            }
            break;

        case 2: // [애니메이션 2: 물방울 번짐]
            for (int y = 0; y < 8; y++) {
                for (int x = 0; x < 8; x++) {
                    float dist = sqrt(pow(x - 3.5, 2) + pow(y - 3.5, 2));
                    uint8_t val = (sin(dist * 1.5 - frameCount * 0.15) + 1) * 127;
                    ledBuffer[(y * 8 + x) * 3] = val;
                    ledBuffer[(y * 8 + x) * 3 + 1] = val / 2;
                    ledBuffer[(y * 8 + x) * 3 + 2] = 255 - val;
                }
            }
            break;

        case 3: // [애니메이션 3: 스캔라인 (심장 박동)]
            uint8_t bright = (sin(frameCount * 0.1) + 1) * 127;
            memset(ledBuffer, 0, 192);
            for (int i = 0; i < 8; i++) {
                ledBuffer[(i * 8 + (frameCount / 5) % 8) * 3] = bright;
            }
            break;
    }
}

#endif