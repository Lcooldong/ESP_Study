#ifndef ANIMATION_H
#define ANIMATION_H

#include <Arduino.h>

#define MAX_ANIMATION_MODE 5
extern uint8_t ledBuffer[192]; // 메인 코드의 버퍼 공유

void updateAnimation(int mode) {
    static uint32_t frameCount = 0;
    frameCount++;

    switch (mode) {
        case 1: // [애니메이션 1: 전체 레인보우 흐름 (명암비 극대화)]
            for (int i = 0; i < 64; i++) {
                // sin 값을 0~1 사이로 만들고, 3제곱을 하여 대비를 엄청나게 키움
                float sineValue = (sin((i + frameCount) * 0.3) + 1.0) / 2.0; 
                float sharpValue = pow(sineValue, 3.0); // 3제곱 (완전히 꺼지는 구간이 길어짐)
                
                uint8_t val = sharpValue * 255; 

                ledBuffer[i * 3] = val;           // R
                ledBuffer[i * 3 + 1] = 0;         // G
                ledBuffer[i * 3 + 2] = 255 - val; // B (적색과 청색 교차)
            }
            break;

        case 2: // [애니메이션 2: 물방울 번짐 파문 (Ripple)]
            for (int y = 0; y < 8; y++) {
                for (int x = 0; x < 8; x++) {
                    // 중심점(3.5, 3.5)으로부터의 거리
                    float dist = sqrt(pow(x - 3.5, 2) + pow(y - 3.5, 2));
                    
                    // 파동 생성: 파장이 퍼져나가는 느낌
                    // 거리(dist)와 시간(frameCount)을 결합하여 밖으로 퍼지게 함
                    float sineValue = (sin(dist * 1.5 - frameCount * 0.2) + 1.0) / 2.0;
                    
                    // 명암비를 위해 4제곱 적용 (파문이 선명한 선처럼 보임)
                    uint8_t val = pow(sineValue, 4.0) * 255;
                    
                    // 거리에 따라 기본 밝기를 조금 줄여서 가장자리가 부드럽게 사라지게 함 (선택사항)
                    if(dist > 4.5) val = val / 2;

                    ledBuffer[(y * 8 + x) * 3] = val;
                    ledBuffer[(y * 8 + x) * 3 + 1] = val / 3;
                    ledBuffer[(y * 8 + x) * 3 + 2] = val; // 전체적으로 보라색/핑크색 물방울
                }
            }
            break;

        case 3: // [애니메이션 3: 나이트 라이더 (Knight Rider) 스캔라인]
            memset(ledBuffer, 0, 192); // 일단 다 끄기
            {
                // 0~7 사이를 왕복하는 핑퐁 로직
                int pos = (frameCount / 3) % 14; 
                if (pos > 7) pos = 14 - pos; // 8~13을 6~1로 반전 (왕복)

                for (int y = 0; y < 8; y++) {
                    // 메인 라인 (가장 밝음)
                    ledBuffer[(y * 8 + pos) * 3] = 255; 
                    
                    // 꼬리 (서서히 어두워짐)
                    if (pos > 0) ledBuffer[(y * 8 + pos - 1) * 3] = 60;
                    if (pos < 7) ledBuffer[(y * 8 + pos + 1) * 3] = 60;
                }
            }
            break;

        case 4: // [추가] 반짝이는 별빛 (Sparkle)
            // 매 프레임마다 무작위로 화면 전체를 조금씩 어둡게(Fade) 만듦
            for (int i = 0; i < 192; i++) {
                if (ledBuffer[i] > 10) ledBuffer[i] -= 10; // 잔상 효과
                else ledBuffer[i] = 0;
            }
            
            // 가끔씩 랜덤한 픽셀 하나를 최대 밝기로 켬
            if (frameCount % 2 == 0) {
                int randomPixel = random(0, 64);
                // 흰색/푸른색 별빛 느낌
                ledBuffer[randomPixel * 3] = 200;
                ledBuffer[randomPixel * 3 + 1] = 200;
                ledBuffer[randomPixel * 3 + 2] = 255;
            }
            break;

        case 5: // [추가] 디지털 비 (Matrix Rain)
            // 1. 전체 화면을 조금씩 아래로 어둡게 만들기 (Fade out)
            for (int i = 0; i < 192; i++) {
                if (ledBuffer[i] > 20) ledBuffer[i] -= 20; 
                else ledBuffer[i] = 0;
            }
            
            // 2. 가끔씩 맨 윗줄(y=0) 랜덤한 열(x)에 새로운 빗방울 생성
            if (random(0, 10) > 7) {
                int randomCol = random(0, 8);
                ledBuffer[randomCol * 3] = 0;       // R
                ledBuffer[randomCol * 3 + 1] = 255; // G (매트릭스 초록색)
                ledBuffer[randomCol * 3 + 2] = 0;   // B
            }

            // 3. 빗방울을 아래로 이동 (일정 프레임마다)
            if (frameCount % 3 == 0) {
                for (int y = 7; y > 0; y--) { // 밑에서부터 위로 끌어내림
                    for (int x = 0; x < 8; x++) {
                        // 윗줄의 값을 아랫줄로 복사 (초록색 채널만 복사)
                        ledBuffer[(y * 8 + x) * 3 + 1] = ledBuffer[((y - 1) * 8 + x) * 3 + 1];
                    }
                }
                // 맨 윗줄은 비워줌
                for (int x = 0; x < 8; x++) {
                    ledBuffer[x * 3 + 1] = 0;
                }
            }
            break;
    }
}

#endif