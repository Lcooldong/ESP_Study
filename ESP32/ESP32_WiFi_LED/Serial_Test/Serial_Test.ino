char cmd;

void setup() {
  Serial.begin(115200);
  

}

void loop() {
  if(Serial.available()){
    cmd = Serial.read();
    if(cmd == '0'){
      //Serial.write("get 0");
      Serial.println("get 0");
      delay(10);
      
    }else if(cmd == '1'){
      //Serial.write("get 1");
      Serial.println("get 1");
      delay(10);
      
    }
  }

}
