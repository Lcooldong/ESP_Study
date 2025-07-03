#include <Arduino.h>
#include <WiFi.h>
#include <LittleFS.h>


#include "AsyncTCP.h"
#include "ESPAsyncWebServer.h"
#include "ElegantOTA.h"

#define FILESYSTEM LittleFS
#define STM32_UART Serial1

const int RX_Pin = D7;
const int TX_Pin = D6;

const char* ssid = "ESP32_S3_XIAO";
const char* password = "1234567890";

unsigned long ota_progress_millis = 0;

AsyncWebServer server(80);

void onOTAStart();
void onOTAProgress(size_t current, size_t final);
void onOTAEnd(bool success);
void serverLoad();
void listFiles();
void printBinFile(const char* path);
void sendFileToSTM32(const char* path);
void SerialCommand();


const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>ESP Web Server</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="icon" href="data:,">
  <style>
    html {font-family: Arial; display: inline-block; text-align: center;}
    h2 {font-size: 3.0rem;}
    p {font-size: 3.0rem;}
    body {max-width: 600px; margin:0px auto; padding-bottom: 25px;}
    .switch {position: relative; display: inline-block; width: 120px; height: 68px} 
    .switch input {display: none}
    .slider {position: absolute; top: 0; left: 0; right: 0; bottom: 0; background-color: #ccc; border-radius: 6px}
    .slider:before {position: absolute; content: ""; height: 52px; width: 52px; left: 8px; bottom: 8px; background-color: #fff; -webkit-transition: .4s; transition: .4s; border-radius: 3px}
    input:checked+.slider {background-color: #b30000}
    input:checked+.slider:before {-webkit-transform: translateX(52px); -ms-transform: translateX(52px); transform: translateX(52px)}
  </style>
</head>
<body>
  <h2>ESP Web Server</h2>
  %BUTTONPLACEHOLDER%
<script>function toggleCheckbox(element) {
  var xhr = new XMLHttpRequest();
  if(element.checked){ xhr.open("GET", "/update?output="+element.id+"&state=1", true); }
  else { xhr.open("GET", "/update?output="+element.id+"&state=0", true); }
  xhr.send();
}
</script>
</body>
</html>
)rawliteral";

String outputState(int output){
  if(digitalRead(output)){
    return "checked";
  }
  else {
    return "";
  }
}


String processor(const String& var){
  //Serial.println(var);
  if(var == "BUTTONPLACEHOLDER"){
    String buttons = "";
    buttons += "<h4>Output - GPIO 2</h4><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this)\" id=\"2\" " + outputState(2) + "><span class=\"slider\"></span></label>";
    buttons += "<h4>Output - GPIO 4</h4><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this)\" id=\"4\" " + outputState(4) + "><span class=\"slider\"></span></label>";
    buttons += "<h4>Output - GPIO 33</h4><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this)\" id=\"33\" " + outputState(33) + "><span class=\"slider\"></span></label>";
    return buttons;
  }
  return String();
}



uint32_t lastMilis = 0;
uint32_t currentMillis = 0;
int counter = 0;
bool ledState = false;

void setup() {
  Serial.begin(115200);
  STM32_UART.begin(115200, SERIAL_8N1, RX_Pin, TX_Pin); // Initialize the internal UART with RX on GPIO 16 and TX on GPIO 17
  pinMode(LED_BUILTIN, OUTPUT); // Initialize the built-in LED pin
  Serial.println("Starting the ESP32S3...");
  
  delay(3000);

  if (!LittleFS.begin(true)) 
  {
    Serial.println("LittleFS Mount Failed");
  } 
  else 
  {
    Serial.println("LittleFS Mounted");
  }

  WiFi.mode(WIFI_AP); // Set WiFi mode to Access Point
  WiFi.softAP(ssid, password); // Start the WiFi access point
  delay(500);

  IPAddress myIP = WiFi.softAPIP(); // Get the IP address of the access point
  Serial.print("Access Point IP address: ");
  Serial.println(myIP); // Print the IP address

  
  serverLoad();
}

void loop() {
  SerialCommand(); // Handle serial commands

  currentMillis = millis();
  if (currentMillis - lastMilis >= 1000) { // Check if 1 second has passed
    lastMilis = currentMillis; // Update lastMilis to current time
    Serial.print("Counter: ");
    Serial.println(counter); // Print the counter value
    counter++; // Increment the counter
    ledState = !ledState; // Toggle the LED state
    digitalWrite(LED_BUILTIN, ledState ? HIGH : LOW); // Update the

    // STM32_UART.printf("Counter: [%d]\r\n", counter); // Print the counter value to the internal UART
  }

  if(STM32_UART.available()) {
    char input = STM32_UART.read(); // Read input from the internal UART
    Serial.printf("Received from UART: %c\r\n", input); // Print the received input to the Serial Monitor
  }

  ElegantOTA.loop();
}



void onOTAStart() {
  // Log when OTA has started
  Serial.println("OTA update started!");
  // <Add your own code here>
}

void onOTAProgress(size_t current, size_t final) {
  // Log every 1 second
  if (millis() - ota_progress_millis > 1000) {
    ota_progress_millis = millis();
    Serial.printf("OTA Progress Current: %u bytes, Final: %u bytes\n", current, final);
  }
}

void onOTAEnd(bool success) {
  // Log when OTA has finished
  if (success) {
    Serial.println("OTA update finished successfully!");
  } else {
    Serial.println("There was an error during OTA update!");
  }
  // <Add your own code here>
}

void listFiles() {
  File root = FILESYSTEM.open("/");
  File file = root.openNextFile();

  while (file) {
    Serial.print("FILE: ");
    Serial.println(file.name());
    file = root.openNextFile();
  }
}

void printBinFile(const char* path) {
  if (!LittleFS.exists(path)) {
    Serial.printf("[ERROR] File not found: %s\n", path);
    return;
  }

  File file = LittleFS.open(path, "r");
  if (!file) {
    Serial.printf("[ERROR] Failed to open file: %s\n", path);
    return;
  }

  Serial.printf("===== Begin Dump: %s =====\n", path);
  size_t addr = 0;
  const size_t bytesPerLine = 16;

  while (file.available()) {
    uint8_t buffer[bytesPerLine];
    size_t len = file.read(buffer, bytesPerLine);

    // Print address
    Serial.printf("%08X  ", (unsigned int)addr);

    // Print hex bytes
    for (size_t i = 0; i < bytesPerLine; ++i) {
      if (i < len) {
        Serial.printf("%02X ", buffer[i]);
      } else {
        Serial.print("   ");
      }
    }

    Serial.print(" ");

    // Print ASCII characters
    for (size_t i = 0; i < len; ++i) {
      if (buffer[i] >= 32 && buffer[i] <= 126) {
        Serial.printf("%c", buffer[i]);
      } else {
        Serial.print(".");
      }
    }

    Serial.println();
    addr += len;
  }

  Serial.printf("===== End Dump: %s =====\n", path);
  file.close();
}



void sendFileToSTM32(const char* path) {
  File file = LittleFS.open(path, "r");
  if (!file) {
    Serial.println("Failed to open file for reading");
    return;
  }

  Serial.printf("Sending %s to STM32...\n", path);
  while (file.available()) {
    STM32_UART.write(file.read()); // 또는 readBytes if needed
  }
  file.close();
}

void SerialCommand()
{
  if(Serial.available())
  {
    char cmd = Serial.read();
    switch(cmd)
    {
      case '1': // Turn on GPIO 2
        Serial.println("CMD[1]");
        listFiles();
        break;
      case '2': // Turn off GPIO 2
        Serial.println("CMD[2]");
        printBinFile("/250702_stm32.bin"); // Example file to print
        break;
      case '3': // Turn on GPIO 4
        
        Serial.println("CMD[3]");
        sendFileToSTM32("/250702_stm32.bin"); // Example file to send to STM32
        break;
      case '4': // Turn off GPIO 4
        
        Serial.println("CMD[4]");
        break;
      case '5': // Turn on GPIO 33
        
        Serial.println("CMD[5]");
        break;
      case '6': // Turn off GPIO 33
        
        Serial.println("CMD[6]");
        break;
      default:
        Serial.println("Unknown command");
    }

  }

}

void serverLoad()
{
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/html", index_html, processor);
  });

  server.on("/upload", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/stm32_upload.html", "text/html");

  });

  server.on("/ota/upload", HTTP_POST,
    [](AsyncWebServerRequest *request) { // 완료 후 응답
      request->send(200, "text/plain", "Upload complete");
    },
    [](AsyncWebServerRequest *request, String filename, size_t index,
       uint8_t *data, size_t len, bool final) {

      static File uploadFile;

      if (index == 0) {
        Serial.printf("UploadStart: %s\n", filename.c_str());
        filename = "/" + filename;
        uploadFile = LittleFS.open(filename, "w");
        if (!uploadFile) {
          Serial.println("Failed to open file for writing");
          return;
        }
      }

      if (uploadFile) {
        uploadFile.write(data, len);
      }

      if (final) {
        Serial.printf("UploadEnd: %s (%u bytes)\n", filename.c_str(), index + len);
        if (uploadFile) uploadFile.close();
      }
    }
  );

  server.on("/list", HTTP_GET, [](AsyncWebServerRequest *request){
    String json = "[";
    File root = LittleFS.open("/");
    File file = root.openNextFile();
    while (file) {
      if (json != "[") json += ",";
      json += "\"" + String(file.name()) + "\"";
      file = root.openNextFile();
    }
    json += "]";
    request->send(200, "application/json", json);
  });

  
 

  ElegantOTA.begin(&server);    // Start ElegantOTA
  // ElegantOTA callbacks
  ElegantOTA.onStart(onOTAStart);
  ElegantOTA.onProgress(onOTAProgress);
  ElegantOTA.onEnd(onOTAEnd);


  server.begin();

}