#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME680.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define BME_SCK 13
#define BME_MISO 12
#define BME_MOSI 11
#define BME_CS 10

#define BME680_ADDRESS 0x77
#define TEMPERATURE_COMPENSATION 6
#define SEALEVELPRESSURE_HPA (1013.25)

Adafruit_BME680 bme; // I2C
//Adafruit_BME680 bme(BME_CS); // hardware SPI
//Adafruit_BME680 bme(BME_CS, BME_MOSI, BME_MISO, BME_SCK);

uint64_t bmeLastTime = 0;
uint64_t bmePrintTime = 0;

uint64_t lastTime = 0;
bool oledState = false;
bool ledState = false;
uint64_t count = 0;

void readBME680();
void connectOLED(uint8_t address);
void checkI2C(int _delay);


void setup() {
  Serial.begin(115200);
  // delay(1000);
  // pinMode(LED_BUILTIN, OUTPUT);
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  display.display();
  delay(500); 

  // Clear the buffer
  display.clearDisplay();

  if (!bme.begin()) {
  Serial.println(F("Could not find a valid BME680 sensor, check wiring!"));
    while (1);
  }
  else
  {
    Serial.println("Setup BME680 Done");
  }
  
  // Set up oversampling and filter initialization
  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);
  bme.setPressureOversampling(BME680_OS_4X);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
  bme.setGasHeater(320, 150); // 320*C for 150 ms

  // Draw a single pixel in white
  display.drawPixel(10, 10, SSD1306_WHITE);

  delay(1000);
  display.clearDisplay();
  display.setTextSize(1);      // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE); // Draw white text
}

void loop() {
  // digitalWrite(LED_BUILTIN, ledState);
  
  if(millis() - lastTime > 1000)
  {
    count++;

    lastTime = millis();
  }

  

 
  readBME680();
  // Serial.print(F("Reading completed at "));
  // Serial.println(millis());

  if(millis() - bmePrintTime > 1000)
  {
    connectOLED(SCREEN_ADDRESS);

    Serial.printf("[%d]\r\n", count);


    Serial.print(F("Temperature = "));
    float degree = bme.temperature - TEMPERATURE_COMPENSATION;
    Serial.print(degree);
    Serial.println(F(" *C"));

    // display.setTextColor(SSD1306_WHITE); // Draw white text
    // display.setTextSize(1);
    display.clearDisplay();
    display.setCursor(0, 16);
    display.printf("[%d]", count);
    display.setCursor(0, 32);
    display.printf("T : %.2f 'c", degree);
    display.setCursor(0, 48);
    display.printf("H : %.2f %%", bme.humidity);
    display.display();

    Serial.print(F("Pressure = "));
    Serial.print(bme.pressure / 100.0);
    Serial.println(F(" hPa"));

    Serial.print(F("Humidity = "));
    Serial.print(bme.humidity);
    Serial.println(F(" %"));

    Serial.print(F("Gas = "));
    Serial.print(bme.gas_resistance / 1000.0);
    Serial.println(F(" KOhms"));

    Serial.print(F("Approx. Altitude = "));
    Serial.print(bme.readAltitude(SEALEVELPRESSURE_HPA));
    Serial.println(F(" m"));

    Serial.println();
    bmePrintTime = millis();
  }
  
}
void readBME680()
{
  unsigned long endTime = bme.beginReading();
  if(millis() - bmeLastTime > 50)
  {
    if (endTime == 0) {
      Serial.println(F("Failed to begin reading :("));
      return;
    }
    // Serial.print(F("Reading started at "));
    // Serial.print(millis());
    // Serial.print(F(" and will finish at "));
    // Serial.println(endTime);

    // Serial.println(F("You can do other work during BME680 measurement."));
    // delay(50); // This represents parallel work.
    // There's no need to delay() until millis() >= endTime: bme.endReading()
    // takes care of that. It's okay for parallel work to take longer than
    // BME680's measurement time.

    // Obtain measurement results from BME680. Note that this operation isn't
    // instantaneous even if milli() >= endTime due to I2C/SPI latency.
    if (!bme.endReading()) {
      Serial.println(F("Failed to complete reading :("));
      return;
    }
    bmeLastTime = millis();
  } 
}

void connectOLED(uint8_t address){
  // Wire.begin();
  Wire.beginTransmission(address);
  byte error = Wire.endTransmission();
  
  if (error == 0) 
  {
    // Serial.println("OLED Connected");
    if(!oledState)
    {
      // u8g2.begin();
      display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
      display.clearDisplay();
      oledState = true;
    }
  }
  else
  {
    Serial.printf("OLED Disconnected [Code : 0x%02X]\r\n", error);
    oledState = false;
  }
  // Wire.end();
}

void checkI2C(int _delay){
  // delay(1000);
  Wire.begin();
  byte error, address;
  int nDevices;
  Serial.println("Scanning...");
  nDevices = 0;
  for(address = 1; address < 127; address++ ) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    if (error == 0) {
      Serial.print("I2C device found at address 0x");
      if (address<16) {
        Serial.print("0");
      }
      Serial.println(address,HEX);
      nDevices++;
    }
    else if (error==4) {
      Serial.print("Unknow error at address 0x");
      if (address<16) {
        Serial.print("0");
      }
      Serial.println(address,HEX);
    }    
  }
  if (nDevices == 0) {
    Serial.println("No I2C devices found\n");
  }
  else {
    Serial.println("done\n");
  }
  Wire.end();
  delay(_delay);
}









