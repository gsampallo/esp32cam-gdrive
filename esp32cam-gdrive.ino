#include <WiFi.h>
#include <WiFiClientSecure.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "Base64.h"

#include "esp_camera.h"

const char* ssid     = "SSID";   //your network SSID
const char* password = "PASSWORD";   //your network password
const char* myDomain = "script.google.com";
String myScript = "/macros/s/XXXXXXXXXXXXXXXXXXXXXX/exec";    //Replace with your own url
String myFilename = "filename=ESP32-CAM.jpg";
String mimeType = "&mimetype=image/jpeg";
String myImage = "&data=";

int waitingTime = 30000; //Wait 30 seconds to google response.

#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

WiFiClientSecure client;
const char* test_root_ca = \
                           "-----BEGIN CERTIFICATE-----\n" \
                           "MIIETDCCAzSgAwIBAgILBAAAAAABL07hSVIwDQYJKoZIhvcNAQEFBQAwVzELMAkG\n" \
                           "A1UEBhMCQkUxGTAXBgNVBAoTEEdsb2JhbFNpZ24gbnYtc2ExEDAOBgNVBAsTB1Jv\n" \
                           "b3QgQ0ExGzAZBgNVBAMTEkdsb2JhbFNpZ24gUm9vdCBDQTAeFw0wNjEyMTUwODAw\n" \
                           "MDBaFw0yODAxMjgxMjAwMDBaMEwxIDAeBgNVBAsTF0dsb2JhbFNpZ24gUm9vdCBD\n" \
                           "QSAtIFIyMRMwEQYDVQQKEwpHbG9iYWxTaWduMRMwEQYDVQQDEwpHbG9iYWxTaWdu\n" \
                           "MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAps8kDr4ubyiZRULEqz4h\n" \
                           "VJsL03+EcPoSs8u/h1/Gf4bTsjBc1v2t8Xvc5fhglgmSEPXQU977e35ziKxSiHtK\n" \
                           "pspJpl6op4xaEbx6guu+jOmzrJYlB5dKmSoHL7Qed7+KD7UCfBuWuMW5Oiy81hK5\n" \
                           "61l94tAGhl9eSWq1OV6INOy8eAwImIRsqM1LtKB9DHlN8LgtyyHK1WxbfeGgKYSh\n" \
                           "+dOUScskYpEgvN0L1dnM+eonCitzkcadG6zIy+jgoPQvkItN+7A2G/YZeoXgbfJh\n" \
                           "E4hcn+CTClGXilrOr6vV96oJqmC93Nlf33KpYBNeAAHJSvo/pOoHAyECjoLKA8Kb\n" \
                           "jwIDAQABo4IBIjCCAR4wDgYDVR0PAQH/BAQDAgEGMA8GA1UdEwEB/wQFMAMBAf8w\n" \
                           "HQYDVR0OBBYEFJviB1dnHB7AagbeWbSaLd/cGYYuMEcGA1UdIARAMD4wPAYEVR0g\n" \
                           "ADA0MDIGCCsGAQUFBwIBFiZodHRwczovL3d3dy5nbG9iYWxzaWduLmNvbS9yZXBv\n" \
                           "c2l0b3J5LzAzBgNVHR8ELDAqMCigJqAkhiJodHRwOi8vY3JsLmdsb2JhbHNpZ24u\n" \
                           "bmV0L3Jvb3QuY3JsMD0GCCsGAQUFBwEBBDEwLzAtBggrBgEFBQcwAYYhaHR0cDov\n" \
                           "L29jc3AuZ2xvYmFsc2lnbi5jb20vcm9vdHIxMB8GA1UdIwQYMBaAFGB7ZhpFDZfK\n" \
                           "iVAvfQTNNKj//P1LMA0GCSqGSIb3DQEBBQUAA4IBAQCZIivuijLTDAd+3RsgK1Bq\n" \
                           "lpEG2r5u13KWrVM/fvWPQufQ62SlZfLz4z0/WzEMfHmEOpeMDx+uwbzy67ig70H9\n" \
                           "vDGp/MlC5kS+HlbKdYuySTGZ/urpcWSGeo/l1WERQ+hAuzEM4tsYi5l0OGGrJICM\n" \
                           "+ag710nWZooYc8y8BjmLEDIODdOx9+9mExBZSMjPAcqZzJBymNs67cunu+JscI6m\n" \
                           "nmhj7Y+3LQWJztlU9k6rHkbbMEk/9mrgAfC8zYTUOfdVjgMVcdOdNO2dxtHIqsWE\n" \
                           "OTsN/SknUh6Dq0gjhVhQs5XGC7Mm4xYtqDDcA1BtXNEMzSqhR5rPIBvbQ4gfwvzg\n" \
                           "-----END CERTIFICATE-----\n";
void setup()
{
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
  
  Serial.begin(115200);
  delay(10);
  
  WiFi.mode(WIFI_STA);

  Serial.println("");
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);  

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  client.setCACert(test_root_ca);
  Serial.println("");
  Serial.println("STAIP address: ");
  Serial.println(WiFi.localIP());
    
  Serial.println("");

  camera_config_t config;
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
  config.frame_size = FRAMESIZE_VGA;  // UXGA|SXGA|XGA|SVGA|VGA|CIF|QVGA|HQVGA|QQVGA
  config.jpeg_quality = 10;
  config.fb_count = 1;
  
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    delay(1000);
    ESP.restart();
  }
}

boolean enviar = true;

void loop() {
  //if(enviar) {
    saveCapturedImage();
    enviar = false;
    delay(60000);
  //}
}

void saveCapturedImage() {
  Serial.println("Connect to " + String(myDomain));
  client.stop();
  if (client.connect(myDomain, 443)) {
    Serial.println("Connection successful");
    
    camera_fb_t * fb = NULL;
    fb = esp_camera_fb_get();  
    if(!fb) {
      Serial.println("Camera capture failed");
      delay(1000);
      ESP.restart();
      return;
    }
  
    char *input = (char *)fb->buf;
    char output[base64_enc_len(3)];
    String imageFile = "";
    for (int i=0;i<fb->len;i++) {
      base64_encode(output, (input++), 3);
      if (i%3==0) imageFile += urlencode(String(output));
    }
    String Data = myFilename+mimeType+myImage;
    
    esp_camera_fb_return(fb);
    
    Serial.println("Send a captured image to Google Drive.");
    
    client.println("POST " + myScript + " HTTP/1.1");
    client.println("Host: " + String(myDomain));
    client.println("Content-Length: " + String(Data.length()+imageFile.length()));
    client.println("Content-Type: application/x-www-form-urlencoded");
    client.println();
    
    client.print(Data);
    int Index;
    for (Index = 0; Index < imageFile.length(); Index = Index+1000) {
      client.print(imageFile.substring(Index, Index+1000));
    }
    
    Serial.println("Waiting for response.");
    long int StartTime=millis();
    while (!client.available()) {
      Serial.print(".");
      delay(100);
      if ((StartTime+waitingTime) < millis()) {
        Serial.println();
        Serial.println("No response.");
        //If you have no response, maybe need a greater value of waitingTime
        break;
      }
    }
    Serial.println();   
    while (client.available()) {
      Serial.print(char(client.read()));
    }  
  } else {         
    Serial.println("Connected to " + String(myDomain) + " failed.");
  }
  client.stop();
}

//https://github.com/zenmanenergy/ESP8266-Arduino-Examples/
String urlencode(String str)
{
    String encodedString="";
    char c;
    char code0;
    char code1;
    char code2;
    for (int i =0; i < str.length(); i++){
      c=str.charAt(i);
      if (c == ' '){
        encodedString+= '+';
      } else if (isalnum(c)){
        encodedString+=c;
      } else{
        code1=(c & 0xf)+'0';
        if ((c & 0xf) >9){
            code1=(c & 0xf) - 10 + 'A';
        }
        c=(c>>4)&0xf;
        code0=c+'0';
        if (c > 9){
            code0=c - 10 + 'A';
        }
        code2='\0';
        encodedString+='%';
        encodedString+=code0;
        encodedString+=code1;
        //encodedString+=code2;
      }
      yield();
    }
    return encodedString;
}
