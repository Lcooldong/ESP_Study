#define LED1 2
#define LED2 4

TaskHandle_t Task1;
TaskHandle_t Task2;

void setup()
{
  Serial.begin(115200);
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);

  xTaskCreatePinnedToCore(
    blink5000,         // 태스크 함수
    "Task1",           // 테스크 이름
    10000,             // 스택 크기(워드단위)
    NULL,              // 태스크 파라미터
    1,                 // 태스크 우선순위
    &Task1,            // 태스크 핸들
    0);                // 실행될 코어

  xTaskCreatePinnedToCore(
    blink1000,          // 태스크 함수
    "Task2",           // 테스크 이름
    10000,             // 스택 크기(워드단위)
    NULL,              // 태스크 파라미터
    1,                 // 태스크 우선순위
    &Task2,            // 태스크 핸들
    1);                // 실행될 코어
}

void blink5000 ( void *param )
{
  Serial.print("# Task 1 running on core ");
  Serial.println(xPortGetCoreID());

  while (1) {
    Serial.println("LED1 ON");
    digitalWrite(LED1, HIGH);
    delay(5000);
    Serial.println("LED1 OFF");
    digitalWrite(LED1, LOW);
    delay(5000);
  }
}

void blink1000 ( void *param )
{
  Serial.print("# Task 2 running on core ");
  Serial.println(xPortGetCoreID());

  while (1) {
    Serial.println("LED2 ON");
    digitalWrite(LED2, HIGH);
    delay(1000);
    Serial.println("LED2 OFF");
    digitalWrite(LED2, LOW);
    delay(1000);
  }
}

void loop()   // 기본적으론 core 1
{
}
