#include "main.h"


uint32_t currentMillis = 0;
uint32_t previousMillis[4] = {0,};
int counter = 0;
uint8_t ledValue = 0;
bool can_switch = false;

HardwareSerial Serail1(1);  // UART1
static RotaryEncoder rotaryEncoder(ROTARY_CLK_PIN, ROTARY_DT_PIN, RotaryEncoder::LatchMode::TWO03);; // Rotary encoder object

void serial_command();

void setup() {
  Serial.begin(115200);
  Serial1.begin(115200, SERIAL_8N1, GPIO_NUM_13, GPIO_NUM_15); // UART1 with RX on GPIO 16 and TX on GPIO 17
  ledcAttachPin(LED_PIN, LED_CHANNEL); // Attach LED pin to channel (not used in this example)
  ledcSetup(LED_CHANNEL, LED_FREQUENCY, LED_RESOLUTION); // Setup LED channel (not used in this example)
  ledcWrite(LED_CHANNEL, 0); // Set LED brightness to 50% (not used in this example)

  button_init();

  can_init(CAN_RX_PIN, CAN_TX_PIN);
  Serial.printf("FLASH: %d MB\r\n", ESP.getFlashChipSize()/(1024*1024)); // Should print 16777216 for 16MB
  delay(100); 

  littleFS_init();
  
  tft_init();
  tft_printf(0, 25, "%s  ", menu_items[item_selected]);
  tft_printf(0, 60, "Click  ");
  // tft_clear(TFT_BLACK);
}

void loop() {
  currentMillis = millis();

  if (currentMillis - previousMillis[0] >= 1000) { // Update every second
    previousMillis[0] = currentMillis;
    counter++;
    Serial.printf("Counter: %d %d|\r\n", counter, digitalRead(ROTARY_SW_PIN)); // Print counter and button state to serial
    Serial1.printf("Serial1: %d\r\n", counter); // Send counter value over UART1

    // tft.fillScreen(TFT_BLACK); // Clear screen
    
    tft_printf(0, 5, "[%d]\r\n", counter);

    if(can_switch)
    {
      uint8_t test[8] = {0,};
      for (int i = 0; i < 8; i++)
      {
        test[i] = i + counter; // Fill test array with some data
      }
      can_send(0x123, test, sizeof(test)/sizeof(test[0]));
    }
  }
  else if (currentMillis - previousMillis[1] >= 10) { // Update every 500ms
    previousMillis[1] = currentMillis;
    button_tick();
  }
  else if (currentMillis - previousMillis[2] >= 3) { // Update every 5ms
    previousMillis[2] = currentMillis;
    encoder_update();
     
  }
  else
  {
    can_receive(); // Check for received CAN messages
    serial_command(); // Handle serial commands
  }
}


void encoder_update()
{
  static int pos = 0;

  rotaryEncoder.tick();
  
  int newPos = (int)(rotaryEncoder.getPosition()/2); // Two steps per click
  if (pos != newPos) {          // changed in pos
    Serial.print("pos:");
    Serial.print(newPos);
    int dir = (int)(rotaryEncoder.getDirection());
    Serial.print(" dir:");
    Serial.println(dir);

    item_sel_previous = item_selected; // Store previous item index
    if(dir == 1){
      item_selected++;
      if(item_selected >= sizeof(menu_items)/sizeof(menu_items[0])) {
        item_selected = 0; // Wrap around to the first menu
      }
    }
    else if(dir == -1){
      item_selected--;
      if(item_selected < 0) {
        item_selected = sizeof(menu_items)/sizeof(menu_items[0]) - 1; // Wrap around to the last menu
      }
    }
    pos = newPos;
    Serial.printf("Item selected: %s\r\n", menu_items[item_selected]); // Print selected item index to serial
    
    tft_printf(0, 25, "%s  ", menu_items[item_selected]);
  }
}




void serial_command()
{
  if(Serial.available()) {
    char c = Serial.read();
    switch (c)
    {
    case '1': 
      ledValue++;
      break;
    case '2': 
      ledValue--;
      break;
    case '3':
      can_switch = !can_switch;
      break;
    case '4':
      Serial.printf("-------------------------Restart-----------------------\r\n");
      ESP.restart();
      break;
    case '5':


      break;
    case '6':

      break;
    case '7':

      break;
    case '8':
      drawJpg(0, 0, "/panda240.jpg");
      break;
    case '9':
      drawJpg(0, 0, "/panda135_180.jpg");
      break;
    case '0':
      listDir(LittleFS, "/", 0);
      break;
    }
    ledcWrite(LED_CHANNEL, ledValue); // Set LED brightness to 50% (not used in this example)
    Serial.printf("LED Value: %d\r\n", ledValue); // Print LED value to serial
  }
}







