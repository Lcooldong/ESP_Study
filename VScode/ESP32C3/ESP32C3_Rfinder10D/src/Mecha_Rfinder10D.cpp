#include <Arduino.h>

#include "Mecha_Rfinder10D.h"

#define BUFFER_FRAME_1_SIZE_PTR 11
#define BUFFER_FRAME_SIZE_PTR 1
// #define BUFFER_END_PTR (BUFFER_FRAME_1_SIZE_PTR - 1)

#define START_CHR 0x0C
#define END_CHR 0x0E

bool Mecha_Rfinder10D::init(Stream &port) {
  this->port = &port;

  this->buffer_ptr = 0;
  this->is_start_chr_read = false;

  return true;
}

void Mecha_Rfinder10D::parseBuffer(uint8_t number_of_targets) {
  for (uint8_t i = 0; i < number_of_targets; i++) {
    memcpy(target_list + i, this->buffer + (i * 8 + 2), sizeof(target_data_struct_t));
  }
}

/**
 * @brief 새로운 데이터 Fetching
 *
 * @note 비 AVR 보드의 Serial Event 미지원으로 Polling 방식으로 새로운 데이터를 읽음. 새로운 데이터를 읽기 위해서는 .fetchNewData() 메소드가 빠르게 실행되어야 함.
 *
 */
void Mecha_Rfinder10D::fetchNewData(void) {
  if (!this->port->available()) {
    return;
  }

  while (this->port->available()) {
    uint8_t c = this->port->read();

    if (this->is_start_chr_read) {
      // start chr received (finding end chr)
      this->buffer_ptr++;
      this->buffer[this->buffer_ptr] = c;

      if (this->buffer[BUFFER_FRAME_SIZE_PTR] * 8 + 2 == this->buffer_ptr) {
        if (END_CHR == c) { // end chr received
          this->target_cnt = this->buffer[BUFFER_FRAME_SIZE_PTR];
          this->is_new_data = true;

          parseBuffer(this->target_cnt);
        } else {
          this->target_cnt = 0;
          // read error
        }
        // reset buffer
        this->buffer_ptr = 0;
        this->is_start_chr_read = false;
      }
    } else {
      // start chr not received (finding start chr)
      if (START_CHR == c) {
        this->is_start_chr_read = true;
        this->buffer[this->buffer_ptr] = c;
      }
    }
  }

  return;
}

/**
 * @brief 직전 데이터 읽기 메소드 호출 후 새로운 데이터의 도착 여부 반환
 * @note 인식된 타겟 수가 없더라도 업데이트됨
 *
 * @return true 새로운 데이터 도착함
 * @return false 새로운 데이터 없음
 */
bool Mecha_Rfinder10D::isNewData(void) {
  if (this->is_new_data) {
    this->is_new_data = false;

    return true;
  }

  return false;
}

/**
 * @brief 인식된 타겟 수 출력
 *
 * @return int 인식된 타겟 수
 */
int Mecha_Rfinder10D::getTargetCnt(void) {
  return (int)this->target_cnt;
}

/**
 * @brief 신호 강도 반환
 *
 * @param target 타겟 번호 입력. 기본값 = 0
 * @return float 신호 강도 (단위: dB)
 */
float Mecha_Rfinder10D::getStrength(int target) {
  if (target > this->target_cnt) {
    return 0;
  }

  return (float)this->target_list[target].strength / 100;
}

/**
 * @brief 거리 반환
 *
 * @param target 타겟 번호 입력. 기본값 = 0
 * @return int 거리 (단위: mm)
 */
int Mecha_Rfinder10D::getRange(int target) {
  if (target > this->target_cnt) {
    return 0;
  }

  return (int)this->target_list[target].range;
}
