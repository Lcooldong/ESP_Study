// WEMOS D1 mini ESP32 

////REAR-////
//4-------3//
//---------//
//---------//
//---------//
//1-------2//
////FRONT////

const int enable[4] = {0, 4, 12, 32};     // motor 1,2,3,4
const int driver1[4] = {5, 23, 19, 18};   // motor1 (5, 23),  motor2 (19, 18)
const int driver2[4] = {13, 14, 33, 26};  // motor3 (13, 14), motor4 (33, 26)  
const int cnt = sizeof(enable)/sizeof(enable[0]);
String dir;
const int echo = 16;
const int trig = 17;

void motor(int number, char* dir, int rate){
  
  int selection;
  const int* tempDriver;
  
  if(number >=3){
   tempDriver = driver2;
  }
  else{
    tempDriver = driver1;
  }
  
  if (number % 2 == 0)selection = 2;
  else selection = 0;
  
  digitalWrite(enable[number-1], LOW);
  if (dir == "CW"){
    Serial.print(" CW ");
    digitalWrite(tempDriver[selection], HIGH);
    digitalWrite(tempDriver[selection+1], LOW);
  }
  else if(dir == "CCW"){
    Serial.print(" CCW ");
    digitalWrite(tempDriver[selection], LOW);
    digitalWrite(tempDriver[selection+1], HIGH);
  }
  ledcWrite(number, rate);
  Serial.print(tempDriver[selection]);
  Serial.print("|");
  Serial.println(tempDriver[selection+1]);
}




void setup() {
  Serial.begin(115200);

  for(int i = 0; i < cnt; i++){
    ledcSetup(i, 5000, 8);
    ledcAttachPin(enable[i], i);
    pinMode(driver1[i], OUTPUT);
    pinMode(driver2[i], OUTPUT);
  }

  pinMode(trig, OUTPUT);
  pinMode(echo, INPUT);

  
  motor(3, "CCW", 255);

}

void loop() {

//    for(int i = 0; i < 255; i++){
//        ledcWrite(3, i);
//        delay(10);
//    }
}

void stop_motor(){
  
}

void forward(){
  
}

void turn(){
  
}
