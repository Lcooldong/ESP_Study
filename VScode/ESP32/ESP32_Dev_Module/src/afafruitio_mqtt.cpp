#include "adafruitio_mqtt.h"




void MQTT::MQTT_connect(Adafruit_MQTT_Client _client) {
  int8_t ret;

  // Stop if already connected.
  if (_client.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 3;
  while ((ret = _client.connect()) != 0) { // connect will return 0 for connected
       Serial.println(_client.connectErrorString(ret));
       Serial.println("Retrying MQTT connection in 5 seconds...");
       _client.disconnect();
       delay(5000);  // wait 5 seconds
       retries--;
       if (retries == 0) {
         // basically die and wait for WDT to reset me
         while (1);
       }
  }

  Serial.println("MQTT Connected!");
}


void MQTT::MQTT_reconnect(Adafruit_MQTT_Client _client)
{
    if(! _client.ping(100))
    {
      Serial.println("ping lost");
      _client.disconnect();
      ESP.restart();
    }
    // else
    // {
    //   _client.processPackets(10000); // blocking
    // }

}

