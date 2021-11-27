const int enable[4] = {0, 4, 12, 32};
const int driver1[4] = {5, 23, 19, 18};
const int driver2[4] = {14, 34, 33, 35};
const int cnt = sizeof(enable)/sizeof(enable[0]);

void setup() {
  Serial.begin(115200);

  for(int i = 0; i < cnt; i++){
    ledcSetup(i, 5000, 8);
    ledcAttachPin(enable[i], i);
    pinMode(driver1[i], OUTPUT);
    pinMode(driver2[i], OUTPUT);
  }
}

void loop() {
    
  
    digitalWrite(driver1[0], HIGH);
    digitalWrite(driver1[1], LOW);
    digitalWrite(driver1[2], HIGH);
    digitalWrite(driver1[3], LOW);
    for(int i = 0; i < 255; i++){
        ledcWrite(3, i);
        delay(10);
    }


}
