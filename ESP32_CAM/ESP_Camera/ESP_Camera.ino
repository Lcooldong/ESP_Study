

/**
 * This example shows how to send Email with inline image from ESP32 camera module.
 *
 * The ESP32 board used in this example is ESP32 PSRAM Timer Camera X (OV3660).
 *
 * The html and text version messages will be sent.
 *
 * Created by K. Suwatchai (Mobizt)
 *
 * Email: suwatchai@outlook.com
 *
 * Github: https://github.com/mobizt/ESP-Mail-Client
 *
 * Copyright (c) 2022 mobizt
 *
 */

#include <Arduino.h>
#if defined(ESP32)
#include <WiFi.h>
#endif

#include <ESP_Mail_Client.h>

#include "esp_camera.h"
#include "camera_pins.h"


#define WIFI_SSID "SK_WiFiGIGA9687"
#define WIFI_PASSWORD "1712042694"

/** For Gmail, the app password will be used for log in
 *  Check out https://github.com/mobizt/ESP-Mail-Client#gmail-smtp-and-imap-required-app-passwords-to-sign-in
 *
 * For Yahoo mail, log in to your yahoo mail in web browser and generate app password by go to
 * https://login.yahoo.com/account/security/app-passwords/add/confirm?src=noSrc
 *
 * To use Gmai and Yahoo's App Password to sign in, define the AUTHOR_PASSWORD with your App Password
 * and AUTHOR_EMAIL with your account email.
 */

/** The smtp host name e.g. smtp.gmail.com for GMail or smtp.office365.com for Outlook or smtp.mail.yahoo.com */
#define SMTP_HOST "smtp.gmail.com"

/** The smtp port e.g.
 * 25  or esp_mail_smtp_port_25
 * 465 or esp_mail_smtp_port_465
 * 587 or esp_mail_smtp_port_587
 */
#define SMTP_PORT esp_mail_smtp_port_587

/* The log in credentials */
#define AUTHOR_EMAIL "alarmrobotpolytech@gmail.com"
#define AUTHOR_PASSWORD "ebtogdwriajpmbhf"

#define TARGET_EMAIL "scooldong@gmail.com"

/* The SMTP Session object used for Email sending */
SMTPSession smtp;
camera_config_t config;

/* Callback function to get the Email sending status */
void smtpCallback(SMTP_Status status);

void setup()
{

    Serial.begin(115200);

    Serial.println();

    
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = Y2_GPIO_NUM;
    config.pin_d1 = Y3_GPIO_NUM;
    config.pin_d2 = Y4_GPIO_NUM;
    config.pin_d3 = Y5_GPIO_NUM;
    config.pin_d4 = Y6_GPIO_NUM;
    config.pin_d5 = Y7_GPIO_NUM;
    config.pin_d6 = Y8_GPIO_NUM;
    config.pin_d7 = Y9_GPIO_NUM;
    config.pin_xclk = XCLK_GPIO_NUM;
    config.pin_pclk = PCLK_GPIO_NUM;
    config.pin_vsync = VSYNC_GPIO_NUM;
    config.pin_href = HREF_GPIO_NUM;
    config.pin_sscb_sda = SIOD_GPIO_NUM;
    config.pin_sscb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;
    config.xclk_freq_hz = 20000000;
    config.pixel_format = PIXFORMAT_JPEG;
    config.frame_size = FRAMESIZE_QXGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;

    // camera init
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK)
    {
        Serial.printf("Camera init failed with error 0x%x", err);
        return;
    }

    Serial.print("Connecting to AP");

    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(200);
    }

    Serial.println("");
    Serial.println("WiFi connected.");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.println();





//    /*  Set the network reconnection option */
//    MailClient.networkReconnect(true);
//
//    /** Enable the debug via Serial port
//     * 0 for no debugging
//     * 1 for basic level debugging
//     *
//     * Debug port can be changed via ESP_MAIL_DEFAULT_DEBUG_PORT in ESP_Mail_FS.h
//     */
//    smtp.debug(1);
//
//    /* Set the callback function to get the sending results */
//    smtp.callback(smtpCallback);
//
//    /* Declare the session config data */
//    ESP_Mail_Session session;
//
//    /* Set the session config */
//    session.server.host_name = SMTP_HOST;
//    session.server.port = SMTP_PORT;
//    session.login.email = AUTHOR_EMAIL;
//    session.login.password = AUTHOR_PASSWORD;
//    session.login.user_domain = F("mydomain.net");
//
//    /* Set the NTP config time */
//    session.time.ntp_server = F("pool.ntp.org,time.nist.gov");
//    session.time.gmt_offset = 3;
//    session.time.day_light_offset = 0;
//
//    /* Declare the message class */
//    SMTP_Message message;
//
//    /* Enable the chunked data transfer with pipelining for large message if server supported */
//    message.enable.chunking = true;
//
//    /* Set the message headers */
//    message.sender.name = F("ESP Mail");
//    message.sender.email = AUTHOR_EMAIL;
//
//    message.subject = F("Test sending camera image");
//    message.addRecipient(F("user1"), F(TARGET_EMAIL));
//
//    message.html.content = F("<span style=\"color:#ff0000;\">The camera image.</span><br/><br/><img src=\"cid:image-001\" alt=\"esp32 cam image\"  width=\"2048\" height=\"1536\">");
//
//    /** The content transfer encoding e.g.
//     * enc_7bit or "7bit" (not encoded)
//     * enc_qp or "quoted-printable" (encoded) <- not supported for message from blob and file
//     * enc_base64 or "base64" (encoded)
//     * enc_binary or "binary" (not encoded)
//     * enc_8bit or "8bit" (not encoded)
//     * The default value is "7bit"
//     */
//    message.html.transfer_encoding = Content_Transfer_Encoding::enc_7bit;
//
//    /** The HTML text message character set e.g.
//     * us-ascii
//     * utf-8
//     * utf-7
//     * The default value is utf-8
//     */
//    message.html.charSet = F("utf-8");
//
//    camera_fb_t *fb = esp_camera_fb_get();
//
//    SMTP_Attachment att;
//
//    /** Set the inline image info e.g.
//     * file name, MIME type, file path, file storage type,
//     * transfer encoding and content encoding
//     */
//    att.descr.filename = F("camera.jpg");
//    att.descr.mime = F("image/jpg");
//
//    att.blob.data = fb->buf;
//    att.blob.size = fb->len;
//
//    att.descr.content_id = F("image-001"); // The content id (cid) of camera.jpg image in the src tag
//
//    /* Need to be base64 transfer encoding for inline image */
//    att.descr.transfer_encoding = Content_Transfer_Encoding::enc_base64;
//
//    /* Add inline image to the message */
//    message.addInlineImage(att);
//
//    /* Connect to server with the session config */
//    if (!smtp.connect(&session))
//        return;
//
//    /* Start sending the Email and close the session */
//    if (!MailClient.sendMail(&smtp, &message, true))
//        Serial.println("Error sending Email, " + smtp.errorReason());
//
//    // to clear sending result log
//    // smtp.sendingResult.clear();
//
//    ESP_MAIL_PRINTF("Free Heap: %d\n", MailClient.getFreeHeap());
}

void loop()
{
  if(Serial.available() > 0)
  {
      char letter = Serial.read();
      if(letter == 's'){
        Serial.println("------s pressed------");
        send_email();
      
      
      }
    
  }
  
}

/* Callback function to get the Email sending status */
void smtpCallback(SMTP_Status status)
{
    /* Print the current status */
    Serial.println(status.info());

    /* Print the sending result */
    if (status.success())
    {
        // ESP_MAIL_PRINTF used in the examples is for format printing via debug Serial port
        // that works for all supported Arduino platform SDKs e.g. AVR, SAMD, ESP32 and ESP8266.
        // In ESP32 and ESP32, you can use Serial.printf directly.

        Serial.println("----------------");
        ESP_MAIL_PRINTF("Message sent success: %d\n", status.completedCount());
        ESP_MAIL_PRINTF("Message sent failed: %d\n", status.failedCount());
        Serial.println("----------------\n");

        for (size_t i = 0; i < smtp.sendingResult.size(); i++)
        {
            /* Get the result item */
            SMTP_Result result = smtp.sendingResult.getItem(i);

            // In case, ESP32, ESP8266 and SAMD device, the timestamp get from result.timestamp should be valid if
            // your device time was synched with NTP server.
            // Other devices may show invalid timestamp as the device time was not set i.e. it will show Jan 1, 1970.
            // You can call smtp.setSystemTime(xxx) to set device time manually. Where xxx is timestamp (seconds since Jan 1, 1970)
            time_t ts = (time_t)result.timestamp;

            ESP_MAIL_PRINTF("Message No: %d\n", i + 1);
            ESP_MAIL_PRINTF("Status: %s\n", result.completed ? "success" : "failed");
            ESP_MAIL_PRINTF("Date/Time: %s\n", asctime(localtime(&ts)));
            ESP_MAIL_PRINTF("Recipient: %s\n", result.recipients.c_str());
            ESP_MAIL_PRINTF("Subject: %s\n", result.subject.c_str());
        }
        Serial.println("----------------\n");

        // You need to clear sending result as the memory usage will grow up.
        smtp.sendingResult.clear();
    }
}

void send_email()
{
  // camera init
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK)
    {
        Serial.printf("Camera init failed with error 0x%x", err);
        return;
    }

    Serial.print("Connecting to AP");

    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(200);
    }

    Serial.println("");
    Serial.println("WiFi connected.");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.println();

    /*  Set the network reconnection option */
    MailClient.networkReconnect(true);

    /** Enable the debug via Serial port
     * 0 for no debugging
     * 1 for basic level debugging
     *
     * Debug port can be changed via ESP_MAIL_DEFAULT_DEBUG_PORT in ESP_Mail_FS.h
     */
    smtp.debug(1);

    /* Set the callback function to get the sending results */
    smtp.callback(smtpCallback);

    /* Declare the session config data */
    ESP_Mail_Session session;

    /* Set the session config */
    session.server.host_name = SMTP_HOST;
    session.server.port = SMTP_PORT;
    session.login.email = AUTHOR_EMAIL;
    session.login.password = AUTHOR_PASSWORD;
    session.login.user_domain = F("mydomain.net");

    /* Set the NTP config time */
    session.time.ntp_server = F("pool.ntp.org,time.nist.gov");
    session.time.gmt_offset = 3;
    session.time.day_light_offset = 0;

    /* Declare the message class */
    SMTP_Message message;

    /* Enable the chunked data transfer with pipelining for large message if server supported */
    message.enable.chunking = true;

    /* Set the message headers */
    message.sender.name = F("ESP Mail");
    message.sender.email = AUTHOR_EMAIL;

    message.subject = F("Test sending camera image");
    message.addRecipient(F("user1"), F(TARGET_EMAIL));

    message.html.content = F("<span style=\"color:#ff0000;\">The camera image.</span><br/><br/><img src=\"cid:image-001\" alt=\"esp32 cam image\"  width=\"2048\" height=\"1536\">");

    /** The content transfer encoding e.g.
     * enc_7bit or "7bit" (not encoded)
     * enc_qp or "quoted-printable" (encoded) <- not supported for message from blob and file
     * enc_base64 or "base64" (encoded)
     * enc_binary or "binary" (not encoded)
     * enc_8bit or "8bit" (not encoded)
     * The default value is "7bit"
     */
    message.html.transfer_encoding = Content_Transfer_Encoding::enc_7bit;

    /** The HTML text message character set e.g.
     * us-ascii
     * utf-8
     * utf-7
     * The default value is utf-8
     */
    message.html.charSet = F("utf-8");

    camera_fb_t *fb = esp_camera_fb_get();

    SMTP_Attachment att;

    /** Set the inline image info e.g.
     * file name, MIME type, file path, file storage type,
     * transfer encoding and content encoding
     */
    att.descr.filename = F("camera.jpg");
    att.descr.mime = F("image/jpg");

    att.blob.data = fb->buf;
    att.blob.size = fb->len;

    att.descr.content_id = F("image-001"); // The content id (cid) of camera.jpg image in the src tag

    /* Need to be base64 transfer encoding for inline image */
    att.descr.transfer_encoding = Content_Transfer_Encoding::enc_base64;

    /* Add inline image to the message */
    message.addInlineImage(att);

    /* Connect to server with the session config */
    if (!smtp.connect(&session))
        return;

    /* Start sending the Email and close the session */
    if (!MailClient.sendMail(&smtp, &message, true))
        Serial.println("Error sending Email, " + smtp.errorReason());

    // to clear sending result log
    // smtp.sendingResult.clear();

    ESP_MAIL_PRINTF("Free Heap: %d\n", MailClient.getFreeHeap());
  
}
