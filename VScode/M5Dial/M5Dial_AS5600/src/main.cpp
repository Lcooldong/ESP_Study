#include <Arduino.h>
#include <AS5600.h>

#define I2C_SDA_PIN GPIO_NUM_13 // Yellow
#define I2C_SCL_PIN GPIO_NUM_15 // White

#define UART_RX_PIN GPIO_NUM_1 // White
#define UART_TX_PIN GPIO_NUM_2 // Yellow

AS5600 as5600;

int counter = 0;  

void i2c_scan()
{
  static uint8_t i2c_counter = 0;

  for (byte i = 1; i < 120; i++)
  {
    Wire.beginTransmission (i);
    if (Wire.endTransmission () == 0)
      {
      Serial1.print ("Found address: ");
      Serial1.print (i, DEC);
      Serial1.print (" (0x");
      Serial1.print (i, HEX);
      Serial1.println (")");
      i2c_counter++;
      delay (10);
      } // end of good response
  } // end of for loop
  Serial1.println ("Done.");
  Serial1.print ("Found ");
  Serial1.print (i2c_counter, DEC);
  Serial1.println (" device(s).");
}


void setup() {
  Serial.begin(115200);
  Serial1.begin(115200, SERIAL_8N1, UART_RX_PIN, UART_TX_PIN);

  Wire.setPins(I2C_SDA_PIN, I2C_SCL_PIN);
  Wire.begin();
  // i2c_scan();
  Serial.println(__FILE__);
  Serial.print("AS5600_LIB_VERSION: ");
  Serial.println(AS5600_LIB_VERSION);


  i2c_scan();
  delay(500);
  
  if(as5600.begin())
  {
    Serial1.println("AS5600 connected!");
    as5600.setDirection(AS5600_CLOCK_WISE);
  }
  else
  {
    Serial1.println("AS5600 not connected!");
  }
  Serial1.printf("State: %d\r\n", as5600.isConnected());
}

uint32_t lastMillis_as5600 = 0;

void loop() {
  uint32_t now = millis();

  if((now - lastMillis_as5600 >= 10))
  {
    counter++; 
    Serial1.printf("[%d]State: %d %d\r\n", counter, as5600.isConnected(), as5600.rawAngle());
    lastMillis_as5600 = now;
  }
}

