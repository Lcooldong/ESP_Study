const int ledPin = 13;

const int freg = 10;
const int ledChannel = 0;
const int resolution = 8;

void setup() {
  Serial.begin(115200);
  ledcSetup(ledChannel, freg, resolution);

  ledcAttachPin(ledPin, ledChannel);

}

void loop() {
  for(int dutyCycle = 0; dutyCycle <= 255; dutyCycle++){
    ledcWrite(ledChannel, dutyCycle);
    delay(15);  
  }

  for(int dutyCycle = 255; dutyCycle >= 0; dutyCycle--){
    ledcWrite(ledChannel, dutyCycle);
    delay(15);  
  }

}
