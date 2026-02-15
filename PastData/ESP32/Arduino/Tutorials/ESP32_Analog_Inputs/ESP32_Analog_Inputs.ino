// 0~3.3V  -> 0 ~ 4095  Non-linear(ADC 3.2 ~ 3.3V 구분 못함)

const int potPin = 34;
int potValue = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);

}

void loop() {
  potValue = analogRead(potPin);
  Serial.println(potValue);
  delay(500);
}
