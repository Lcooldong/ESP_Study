int batteryValue = 0;
int batteryPin = 34;

void setup() {
  Serial.begin(115200);
  // put your setup code here, to run once:

}

void loop() {
  // put your main code here, to run repeatedly:

}


void readVolt(){
    batteryValue = analogRead(batteryPin);
    Serial.print("value : ");
    Serial.println(batteryValue);
    int volt = map(batteryValue,0,4095,0,4200);
    float result = (float)volt / 100;
    Serial.print("전압 : ");
    Serial.print(result);
    Serial.println("V");
}
