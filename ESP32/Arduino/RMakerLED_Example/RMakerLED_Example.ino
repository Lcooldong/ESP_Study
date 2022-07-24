//This example demonstrates the ESP RainMaker with a standard Switch device.
#include "RMaker.h"
#include "WiFi.h"
#include "WiFiProv.h"
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
 #include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif
//#define LED_PIN 2
#define LED_COUNT 60
#define DEFAULT_POWER_MODE true
const char *service_name = "PROV_1234";
const char *pop = "abcd1234";

//GPIO for push button
#if CONFIG_IDF_TARGET_ESP32C3
static int gpio_0 = 3;        // Button
static int gpio_switch = 7;   // Switch
static int led_pin = 2;

#else
//GPIO for virtual device
static int gpio_0 = 0;
static int gpio_switch = 16;
#endif

/* Variable for reading pin status*/
bool switch_state = true;

//The framework provides some standard device types like switch, lightbulb, fan, temperaturesensor.
static Switch my_switch("Switch", &gpio_switch);
static LightBulb my_led("LED_STRIP", &led_pin);

Adafruit_NeoPixel strip = Adafruit_NeoPixel(LED_COUNT, led_pin, NEO_GRB + NEO_KHZ800);

void sysProvEvent(arduino_event_t *sys_event)
{
    switch (sys_event->event_id) {      
        case ARDUINO_EVENT_PROV_START:
#if CONFIG_IDF_TARGET_ESP32S2
        Serial.printf("\nProvisioning Started with name \"%s\" and PoP \"%s\" on SoftAP\n", service_name, pop);
        printQR(service_name, pop, "softap");
#else
        Serial.printf("\nProvisioning Started with name \"%s\" and PoP \"%s\" on BLE\n", service_name, pop);
        printQR(service_name, pop, "ble");
#endif         
        break;
    }
}

void write_callback(Device *device, Param *param, const param_val_t val, void *priv_data, write_ctx_t *ctx)
{
    const char *device_name = device->getDeviceName();
    const char *param_name = param->getParamName();

    if(strcmp(param_name, "Power") == 0) {
        Serial.printf("Received value = %s for %s - %s\n", val.val.b? "true" : "false", device_name, param_name);
        switch_state = val.val.b;
        (switch_state == false) ? digitalWrite(gpio_switch, LOW) : digitalWrite(gpio_switch, HIGH);
        param->updateAndReport(val);
        if(switch_state)
          pickOneLED(0, strip.Color(255, 40, 100), 50, 50);
        else
          pickOneLED(0, strip.Color(255, 40, 100), 0, 50);
    }
}

void setup()
{
    Serial.begin(115200);
    pinMode(gpio_0, INPUT_PULLUP);
    pinMode(gpio_switch, OUTPUT);
    digitalWrite(gpio_switch, DEFAULT_POWER_MODE);
#if defined(__AVR_ATtiny85__) && (F_CPU == 16000000)
    clock_prescale_set(clock_div_1);
#endif
    strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
    strip.show();            // Turn OFF all pixels ASAP
    strip.setBrightness(50); // Set BRIGHTNESS to about 1/5 (max = 255)

    
    Node my_node;    
    my_node = RMaker.initNode("ESP RainMaker Node");

    //Standard switch device
    my_switch.addCb(write_callback);
    
    //Add switch device to the node   
    my_node.addDevice(my_switch);

    //This is optional 
    RMaker.enableOTA(OTA_USING_PARAMS);
    //If you want to enable scheduling, set time zone for your region using setTimeZone(). 
    //The list of available values are provided here https://rainmaker.espressif.com/docs/time-service.html
    // RMaker.setTimeZone("Asia/Shanghai");
    // Alternatively, enable the Timezone service and let the phone apps set the appropriate timezone
    RMaker.enableTZService();

    RMaker.enableSchedule();

    RMaker.start();

    WiFi.onEvent(sysProvEvent);
#if CONFIG_IDF_TARGET_ESP32S2
    WiFiProv.beginProvision(WIFI_PROV_SCHEME_SOFTAP, WIFI_PROV_SCHEME_HANDLER_NONE, WIFI_PROV_SECURITY_1, pop, service_name);
#else
    WiFiProv.beginProvision(WIFI_PROV_SCHEME_BLE, WIFI_PROV_SCHEME_HANDLER_FREE_BTDM, WIFI_PROV_SECURITY_1, pop, service_name);
#endif
}

void loop()
{
    if(digitalRead(gpio_0) == LOW) { //Push button pressed

        // Key debounce handling
        delay(100);
        int startTime = millis();
        while(digitalRead(gpio_0) == LOW) delay(50);
        int endTime = millis();

        if ((endTime - startTime) > 10000) {
          // If key pressed for more than 10secs, reset all
          Serial.printf("Reset to factory.\n");
          RMakerFactoryReset(2);
        } else if ((endTime - startTime) > 3000) {
          Serial.printf("Reset Wi-Fi.\n");
          // If key pressed for more than 3secs, but less than 10, reset Wi-Fi
          RMakerWiFiReset(2);
        } else {
          // Toggle device state
          switch_state = !switch_state;
          Serial.printf("Toggle State to %s.\n", switch_state ? "true" : "false");
          my_switch.updateAndReportParam(ESP_RMAKER_DEF_POWER_NAME, switch_state);
          (switch_state == false) ? digitalWrite(gpio_switch, LOW) : digitalWrite(gpio_switch, HIGH);
            
      }
    }
    delay(100);
}


void pickOneLED(uint8_t ledNum, uint32_t color, uint8_t brightness, int wait){
    strip.setBrightness(brightness);
    strip.setPixelColor(ledNum, color);  
    strip.show();                                               
    delay(wait);
}

void resetNeopixel(){
  for(int i=0; i < 256; i++){
    pickOneLED(i, strip.Color(0, 0, 0), 0, 0 );
  } 
}