#include <Arduino.h>
#include "esp_camera.h"
#include <SD_MMC.h>
#include <EEPROM.h>
#include "FS.h"
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "driver/rtc_io.h"
#include <WiFi.h>
#include "time.h"

#include "ESP_Mail_Client.h"

#define USE_INCREMENTAL_FILE_NUMBERING


// #define USE_TIMESTAMP
#define SEND_EMAIL

// #define TRIGGER_MODE
#define TIMED_MODE

#define WIFI_SSID "SK_WiFiGIGA9687"
#define WIFI_PASSWORD "1712042694"
#define SMTP_HOST "smtp.gmail.com"
#define SMTP_PORT 587
#define AUTHOR_EMAIL "alarmrobotpolytech@gmail.com"
#define AUTHOR_PASSWORD "ebtogdwriajpmbhf"
#define TARGET_EMAIL "scooldong@gmail.com"

const byte ledPin = GPIO_NUM_33;
const byte flashPin = GPIO_NUM_4;
const byte triggerPin = GPIO_NUM_13;
const byte flashPower = 1;
#ifdef TIMED_MODE
  const int timeLapseInterval = 30;
#endif
const int startupDelayMillis = 3000;


int pictureNumber = 0;
String path;
#ifdef SEND_EMAIL
  SMTPSession smtp;
  void smtpCallback(SMTP_Status status);

void smtpCallback(SMTP_Status status){
  Serial.println(status.info());
  if(status.success()){
    Serial.println("----------------");
    Serial.printf("Message sent success: %d\n", status.completedCount());
    Serial.printf("Message sent failed: %d\n", status.failedCount());
    Serial.println("----------------\n");
    struct tm dt;
    for (size_t i = 0; i < smtp.sendingResult.size(); i++)
    {
      SMTP_Result result = smtp.sendingResult.getItem(i);
      localtime_r((const time_t*)result.timestamp, &dt);
      Serial.printf("Message No: %d\n", i + 1);
      Serial.printf("Status: %s\n", result.completed ? "success" : "failed");
      Serial.printf("Data/Time: %d/%d/%d %d:%d:%d\n", dt.tm_year + 1900, 
                                                      dt.tm_mon + 1,
                                                      dt.tm_mday,
                                                      dt.tm_hour,
                                                      dt.tm_min,
                                                      dt.tm_sec);
      Serial.printf("Recipient: %s\n", result.recipients);
      Serial.printf("Subject: %s\n", result.subject);
    }
    Serial.println("----------------");
    
  }
}
#endif

void sleep(){
  pinMode(triggerPin, INPUT_PULLDOWN);
  rtc_gpio_hold_en(GPIO_NUM_4);
  digitalWrite(ledPin, HIGH);
  delay(1000);
  #ifdef TRIGGER_MODE
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_13, 1);
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_13, 0);
  #elif defined(TIMED_MODE)
    esp_sleep_enable_timer_wakeup(timeLapseInterval * 1000000);
  #endif
  Serial.println("< Going to sleep now >");
  esp_deep_sleep_start();
  Serial.println("This will never be printed");
}



void setup() {
  Serial.begin(115200);
  Serial.println("------- ESP32-CAM Start -------");
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
  Serial.printf("File : %s [%s]\n",__FILE__, __DATE__);
  // Serial.println();

  camera_config_t config;

  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = 5;
  config.pin_d1 = 18;
  config.pin_d2 = 19;
  config.pin_d3 = 21;
  config.pin_d4 = 36;
  config.pin_d5 = 39;
  config.pin_d6 = 34;
  config.pin_d7 = 35;
  config.pin_xclk = 0;
  config.pin_pclk = 22;
  config.pin_vsync = 25;
  config.pin_href = 23;
  config.pin_sscb_sda = 26;
  config.pin_sscb_scl = 27;
  config.pin_pwdn = 32;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  if(psramFound()){
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  }else{
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

  ledcSetup(7, 5000, 8);
  ledcAttachPin(4, 7);
  ledcWrite(7, flashPower);

  delay(250);
  esp_err_t err = esp_camera_init(&config);
  if(err != ESP_OK){
    Serial.printf("camera init failed with error 0x%x", err);
    sleep();
  }

  sensor_t* s = esp_camera_sensor_get();
  s->set_gain_ctrl(s, 1);                   // Auto Gain Control 0 ~ 1
  s->set_agc_gain(s, 0);                    // 0 ~ 30  
  s->set_gainceiling(s, (gainceiling_t)0);  // 0 ~ 6
  s->set_exposure_ctrl(s, 1);               // Auto Exposure Control 0 ~ 1
  s->set_aec_value(s, 300);                 // 0 ~ 1200, auto exposure control

  s->set_aec2(s, 0);                        // Auto Exposure Correction 0 ~ 1
  s->set_ae_level(s, 0);                    // Manual Exposure Correction -2 ~ 2

  s->set_awb_gain(s, 1);                    // Auto White Balance 0 ~ 1
  s->set_wb_mode(s, 0);                     // White Balance Mode 0 ~ 4, AUto, Sunny, Cloudy, Office, HOME
  s->set_whitebal(s, 1);                    // Wite Balance 0 ~ 1
  s->set_bpc(s, 0);                         // Black Pixel Correction 0 ~ 1
  s->set_wpc(s, 1);                         // White Pixel Correction 0 ~ 1
  s->set_brightness(s, 0);                  // Brightness -2 ~ 2
  s->set_contrast(s, 0);                    // Contrast   -2 ~ 2
  s->set_saturation(s, 0);                  // Saturation -2 ~ 2
  s->set_special_effect(s, 0);              // 0 ~ 6, No Effect, Negative, Grayscale, Red Tint, Green Tint, Blue Tint, Sepia

  s->set_lenc(s, 1);                        // Lens correction 0 ~ 1
  s->set_hmirror(s, 0);                     // Horizontal flip image 0 ~ 1
  s->set_vflip(s, 0);                       // Vertical flip image 0 ~ 1
  s->set_colorbar(s, 0);                    // Colour Testbar 0 ~ 1
  s->set_raw_gma(s, 1);                     // 0 ~ 1
  s->set_dcw(s, 1);                         // 0 ~ 1

  camera_fb_t* fb = NULL;
  delay(startupDelayMillis);
  
  // solution for dart green image
  for (size_t i = 0; i < 5; i++)
  {
    Serial.printf("frame count : %d\n", i);
    fb = esp_camera_fb_get();
    esp_camera_fb_return(fb);
  }
  delay(200);

  fb = esp_camera_fb_get();
  if(!fb){
    Serial.println("Camera capture failed");
    sleep();
  }

  ledcWrite(7, 0);

  path = "/pic";

  #ifdef USE_INCREMENTAL_FILE_NUMBERING
    EEPROM.begin(4);
    EEPROM.get(0, pictureNumber);
    pictureNumber += 1;
    path += String(pictureNumber) + "_";
    EEPROM.put(0, pictureNumber);
    EEPROM.commit();
  #endif

  #if defined(SEND_EMAIL) || defined(USE_TIMESTAMP)
    WiFi.mode(WIFI_AP_STA);
    WiFi.setHostname("ESP32_CAM");
    int connAttempts = 0;
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while(WiFi.status() != WL_CONNECTED && connAttempts < 10){
      Serial.print(".");
      delay(500);
      connAttempts++;
    }
    if(WiFi.isConnected()){
      Serial.println("");
      Serial.println("WiFi connected.");
      Serial.println("IP address: ");
      Serial.println(WiFi.localIP());
      Serial.print(" Signal Level:");
      Serial.println(WiFi.RSSI());
      Serial.println();
    }else{
      Serial.println(F("Failed to connect to Wi-Fi"));
      sleep();
    }
  #endif

  #ifdef USE_TIMESTAMP
    const long gmtOffset_sec = 0;
    const int daylightOffset_sec = 0;
    configTime(gmtOffset_sec, daylightOffset_sec, "pool.ntp.org");
    struct tm timeinfo;
    if(!getLocalTime(&timeinfo)){
      Serial.println("Failed to obtain time");
      sleep();
    }else{
      Serial.print("Current time is ");
      Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
      char timeStringBuff[50];
      strftime(timeStringBuff, sizeof(timeStringBuff), "%Y%m%d_%H%M%S", &timeinfo);
      path += (String)timeStringBuff;
    }
  #endif

  path += ".jpg";

  Serial.println("Starting SD Card");
  // if(!SD_MMC.begin()){
  //   Serial.println("SD Card Mount Failed");
  //   sleep();
  // }

  if(!MailClient.sdMMCBegin()){
    Serial.println("SD Card Mount Failed");
    sleep();
  }

  // uint8_t cardType = SD_MMC.cardType();
  // if(cardType == CARD_NONE){
  //   Serial.println("No SD Card attached");
  //   sleep();
  //   return;
  // }

  fs::FS &fs = SD_MMC;
  File file = fs.open(path.c_str(), FILE_WRITE);
  if(!file){
    Serial.printf("Failed to save to path: %s\n", path.c_str());
    sleep();
  }else{
    file.write(fb->buf, fb->len);
    Serial.printf("Save file to path: %s\n", path.c_str());
  }
  file.close();

  esp_camera_fb_return(fb);
  delay(1000);

  #ifdef SEND_EMAIL
    smtp.debug(1);
    smtp.callback(smtpCallback);
    ESP_Mail_Session session;
    session.server.host_name = SMTP_HOST;
    session.server.port = SMTP_PORT;
    session.login.email = AUTHOR_EMAIL;
    session.login.password = AUTHOR_PASSWORD;
    session.login.user_domain = "mydomain.net";

    SMTP_Message message;
    message.sender.name = "ESP32-CAM";
    message.sender.email = AUTHOR_EMAIL;
    message.subject = "Motion Detected - ESP32-CAM";
    message.addRecipient("Me", TARGET_EMAIL);
    //message.addRecipient("name", "email");
    message.text.content = "This is email Text";
    message.text.transfer_encoding = Content_Transfer_Encoding::enc_base64;
    
    SMTP_Attachment att;
    // att.descr.filename = "photo.jpg";
    String image = "pic";
    image += String(pictureNumber) + "_";
    image += ".jpg";
    att.descr.filename = image;
    att.descr.mime = "application/octet-stream"; // binary data
    att.file.path = path.c_str();
    att.file.storage_type = esp_mail_file_storage_type_sd;
    att.descr.transfer_encoding = Content_Transfer_Encoding::enc_base64;

    message.addAttachment(att);

    Serial.println("Connecting to SMTP");
    if(!smtp.connect(&session)){
      Serial.println("Couldn't connect");
      sleep();
    }

    Serial.println("Sending Mail");
    if(!MailClient.sendMail(&smtp, &message)){
      Serial.print("Error Sending Email, " + smtp.errorReason());
      sleep();
    }
  #endif

  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);

  sleep();

}

void loop() {
  delay(1);
}