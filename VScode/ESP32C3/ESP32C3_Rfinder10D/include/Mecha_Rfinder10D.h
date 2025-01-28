#ifndef SRC_MECHA_VOICERECOGNITION
#define SRC_MECHA_VOICERECOGNITION

#include <stdbool.h>
#include <stdint.h>

#include <Arduino.h>

class Mecha_Rfinder10D {
public:
  // Mecha_Rfinder10D(Stream &_port);
  bool init(Stream &port);

  void fetchNewData(void);

  bool isNewData(void);
  int getTargetCnt(void);
  float getStrength(int target = 0);
  int getRange(int target = 0);

private:
  typedef struct {
    uint8_t header : 8;    // 1 byte
    uint8_t reserved : 8;  // 1 byte
    int16_t strength : 16; // 2 byte
    uint32_t range : 32;   // 4 byte
  } __attribute__((packed)) target_data_struct_t;

  void parseBuffer(uint8_t number_of_targets);

  Stream *port;
  uint8_t buffer[100];

  target_data_struct_t target_list[10];
  uint8_t target_cnt;

  uint8_t buffer_ptr;
  bool is_start_chr_read;

  bool is_new_data;
};

#endif /* SRC_MECHA_VOICERECOGNITION */
