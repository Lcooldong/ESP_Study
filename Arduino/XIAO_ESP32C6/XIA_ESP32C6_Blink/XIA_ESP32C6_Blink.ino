bool led_state = false;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  digitalWrite(LED_BUILTIN, led_state);
  Serial.printf("LED:%s\r\n", led_state ? "ON":"OFF");
  delay(500);

  led_state = !led_state;
}
