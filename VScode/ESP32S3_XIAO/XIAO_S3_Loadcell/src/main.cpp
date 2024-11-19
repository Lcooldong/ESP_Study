#include <Arduino.h>
#include <HX711.h>

const int LOADCELL_DOUT_PIN = D0;
const int LOADCELL_SCK_PIN = D1;
const float poundToKg = 0.453592;

uint32_t scaleLastMillis = 0;

HX711 scale;

void setup() {
  Serial.begin(115200);

  while (!Serial){}
  
  delay(2000);

  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);

  Serial.println("Before setting up the scale:");
  Serial.print("read: \t\t");
  Serial.println(scale.read());			// print a raw reading from the ADC

  Serial.print("read average: \t\t");
  Serial.println(scale.read_average(20));  	// print the average of 20 readings from the ADC

  Serial.print("get value: \t\t");
  Serial.println(scale.get_value(5));		// print the average of 5 readings from the ADC minus the tare weight (not set yet)

  Serial.print("get units: \t\t");
  Serial.println(scale.get_units(5), 1);	// print the average of 5 readings from the ADC minus tare weight (not set) divided
            // by the SCALE parameter (not set yet)

  scale.set_scale(2280.f);                      // this value is obtained by calibrating the scale with known weights; see the README for details 칼리브레이션
  scale.tare();				                          // reset the scale to 0 영점 조절

  Serial.println("After setting up the scale:");

  Serial.print("read: \t\t");
  Serial.println(scale.read());                 // print a raw reading from the ADC

  Serial.print("read average: \t\t");
  Serial.println(scale.read_average(20));       // print the average of 20 readings from the ADC

  Serial.print("get value: \t\t");
  Serial.println(scale.get_value(5));		// print the average of 5 readings from the ADC minus the tare weight, set with tare()

  Serial.print("get units: \t\t");
  Serial.println(scale.get_units(5), 1);        // print the average of 5 readings from the ADC minus tare weight, divided
            // by the SCALE parameter set with set_scale


}

void loop() {
  

  uint32_t currentMillis = millis();
  if(currentMillis - scaleLastMillis > 100)
  {


    
    scaleLastMillis = currentMillis;
  }
  
  // scale.power_down();			        // put the ADC in sleep mode
  // delay(50);
  // scale.power_up();
  if(scale.wait_ready_timeout(100))
  {
    Serial.print("one reading:\t");
    Serial.println(scale.get_units() * poundToKg, 1);   // 1개 읽기
    // Serial.print("\t| average:\t");
    // Serial.println(scale.get_units(10) * poundToKg, 1); // 10개 평균

  }
  else
  {
    Serial.println("HX711 not found.");
  }

}

