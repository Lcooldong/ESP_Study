#include <Arduino.h>
#include <TinyPICO.h>

volatile uint32_t cnt = 0;
// Interval between internal temperature reads
unsigned long next_temp_read = 0;   // Next time step in milliseconds
uint16_t temp_read_interval = 1000;  // This is in milliseconds

// Initialise the TinyPICO library
TinyPICO tp = TinyPICO();

void setup() {
  Serial.begin(115200);

  tp.DotStar_SetPixelColor(0xFF00FF);
  tp.DotStar_Show();
}

void loop() {
  Serial.printf("COUNT : %d\r\n", cnt++);
  delay(1000);
  //tp.DotStar_CycleColor(10);

}

