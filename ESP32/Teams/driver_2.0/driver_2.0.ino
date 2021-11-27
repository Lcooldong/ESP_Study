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
const int driver2[4] = {14, 34, 33, 35};  // motor3 (14, 34), motor4 (33, 35)  
const int cnt = sizeof(enable)/sizeof(enable[0]);
String dir;

void motor(int number, char* dir, int rate){
  int selection = 0;
  const int* tempDriver = driver1;
  if(number >=3){
   tempDriver = driver2;
   selection = 2;
  }
  else{
    tempDriver = driver1;
    selection = 0;
  }     
  digitalWrite(enable[selection], LOW);
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
  
  pinMode(2, OUTPUT);
  pinMode(0, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(12, OUTPUT);
  pinMode(32, OUTPUT);
  pinMode(25, OUTPUT);
  pinMode(27, OUTPUT);
  pinMode(15, OUTPUT);
  pinMode(16, OUTPUT);
  pinMode(17, OUTPUT);
  pinMode(21, OUTPUT);
  pinMode(22, OUTPUT);
  pinMode(10, OUTPUT);
  pinMode(13, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(23, OUTPUT);
  pinMode(19, OUTPUT);
  pinMode(18, OUTPUT);
  pinMode(26, OUTPUT);
  pinMode(36, OUTPUT);
  pinMode(9, OUTPUT);
  pinMode(14, OUTPUT);
  pinMode(34, OUTPUT);
  pinMode(33, OUTPUT);
  pinMode(35, OUTPUT);
  pinMode(39, OUTPUT);

  
  digitalWrite(2, HIGH);  // BUILTIN_LED
  digitalWrite(0, HIGH);//
  digitalWrite(4, HIGH);//
  digitalWrite(12, HIGH);//
  digitalWrite(32, HIGH);//
  digitalWrite(25, HIGH);//
  digitalWrite(27, HIGH);//

  digitalWrite(15, HIGH);//
  digitalWrite(16, HIGH);//
  digitalWrite(17, HIGH);//
  digitalWrite(21, HIGH);//
  digitalWrite(22, HIGH);//

  digitalWrite(10, HIGH);
  digitalWrite(13, HIGH);//
  digitalWrite(5, HIGH);//
  digitalWrite(23, HIGH);//
  digitalWrite(19, HIGH);//
  digitalWrite(18, HIGH);//
  digitalWrite(26, HIGH);//
  digitalWrite(36, HIGH);

  digitalWrite(9, HIGH);//
  digitalWrite(14, HIGH);//
  digitalWrite(34, HIGH);
  digitalWrite(33, HIGH);//
  digitalWrite(35, HIGH);
  digitalWrite(39, HIGH);
  
  //motor(3, "CW", 255);
  //motor(2, "CCW", 150);
}

void loop() {
    
//    digitalWrite(driver1[0], HIGH);
//    digitalWrite(driver1[1], LOW);
//    digitalWrite(driver1[2], HIGH);
//    digitalWrite(driver1[3], LOW);
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
