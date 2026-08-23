/*
  Camera_Web_Stream.ino

  Simple MJPEG camera web stream for:
  Seeed Studio XIAO ESP32S3 Sense

  Copyright (c) 2026 Dogi Lab

  This software is provided for educational and experimental purposes.
  Hardware validated on physical XIAO ESP32S3 Sense hardware.
*/

#include <WiFi.h>
#include "esp_camera.h"
#include "esp_http_server.h"


// ==================================================
// WIFI
// ==================================================

const char* WIFI_SSID = "YOUR_WIFI_NAME";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

// ==================================================
// XIAO ESP32S3 SENSE CAMERA PINS
// ==================================================

#define CAM_PIN_PWDN    -1
#define CAM_PIN_RESET   -1

#define CAM_PIN_XCLK    10
#define CAM_PIN_SIOD    40
#define CAM_PIN_SIOC    39

#define CAM_PIN_D7      48
#define CAM_PIN_D6      11
#define CAM_PIN_D5      12
#define CAM_PIN_D4      14
#define CAM_PIN_D3      16
#define CAM_PIN_D2      18
#define CAM_PIN_D1      17
#define CAM_PIN_D0      15

#define CAM_PIN_VSYNC   38
#define CAM_PIN_HREF    47
#define CAM_PIN_PCLK    13


// ==================================================
// HTTP SERVER
// ==================================================

httpd_handle_t cameraServer = NULL;

static const char* STREAM_CONTENT_TYPE =
  "multipart/x-mixed-replace;boundary=frame";

static const char* STREAM_BOUNDARY =
  "\r\n--frame\r\n";


// ==================================================
// SIMPLE WEB PAGE
// ==================================================

static const char WEB_PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <meta name="viewport"
        content="width=device-width, initial-scale=1.0">

  <title>XIAO ESP32S3 Sense Camera</title>

  <style>
    body {
      background: #111;
      color: white;
      font-family: Arial, sans-serif;
      text-align: center;
      margin: 0;
      padding: 20px;
    }

    img {
      width: 100%;
      max-width: 800px;
      height: auto;
      border-radius: 8px;
    }
  </style>
</head>

<body>

  <h2>XIAO ESP32S3 Sense</h2>
  <p>Camera Web Stream | DogiLab </p>

  <img src="/stream">

</body>
</html>
)rawliteral";


// ==================================================
// ROOT PAGE
// ==================================================

esp_err_t rootHandler(httpd_req_t* request)
{
  httpd_resp_set_type(request, "text/html");

  return httpd_resp_send(
    request,
    WEB_PAGE,
    HTTPD_RESP_USE_STRLEN
  );
}


// ==================================================
// MJPEG STREAM
// ==================================================

esp_err_t streamHandler(httpd_req_t* request)
{
  esp_err_t result;

  result = httpd_resp_set_type(
    request,
    STREAM_CONTENT_TYPE
  );

  if (result != ESP_OK)
  {
    return result;
  }


  while (true)
  {
    camera_fb_t* frame = esp_camera_fb_get();

    if (frame == NULL)
    {
      Serial.println("ERROR: Camera frame capture failed.");
      return ESP_FAIL;
    }


    if (frame->format != PIXFORMAT_JPEG)
    {
      Serial.println("ERROR: Camera frame is not JPEG.");
      esp_camera_fb_return(frame);
      return ESP_FAIL;
    }


    char header[64];

    int headerLength = snprintf(
      header,
      sizeof(header),
      "Content-Type: image/jpeg\r\n"
      "Content-Length: %u\r\n\r\n",
      (unsigned int)frame->len
    );


    result = httpd_resp_send_chunk(
      request,
      STREAM_BOUNDARY,
      strlen(STREAM_BOUNDARY)
    );


    if (result == ESP_OK)
    {
      result = httpd_resp_send_chunk(
        request,
        header,
        headerLength
      );
    }


    if (result == ESP_OK)
    {
      result = httpd_resp_send_chunk(
        request,
        (const char*)frame->buf,
        frame->len
      );
    }


    esp_camera_fb_return(frame);


    if (result != ESP_OK)
    {
      Serial.println("Stream client disconnected.");
      break;
    }


    delay(1);
  }


  return result;
}


// ==================================================
// START WEB SERVER
// ==================================================

bool startCameraServer()
{
  httpd_config_t serverConfig = HTTPD_DEFAULT_CONFIG();

  serverConfig.server_port = 80;


  httpd_uri_t rootURI = {};

  rootURI.uri = "/";
  rootURI.method = HTTP_GET;
  rootURI.handler = rootHandler;
  rootURI.user_ctx = NULL;


  httpd_uri_t streamURI = {};

  streamURI.uri = "/stream";
  streamURI.method = HTTP_GET;
  streamURI.handler = streamHandler;
  streamURI.user_ctx = NULL;


  esp_err_t result =
    httpd_start(&cameraServer, &serverConfig);


  if (result != ESP_OK)
  {
    Serial.printf(
      "ERROR: Web server failed to start: 0x%X\n",
      result
    );

    return false;
  }


  httpd_register_uri_handler(
    cameraServer,
    &rootURI
  );


  httpd_register_uri_handler(
    cameraServer,
    &streamURI
  );


  return true;
}


// ==================================================
// INITIALIZE CAMERA
// ==================================================

bool initializeCamera()
{
  camera_config_t config = {};


  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;


  config.pin_d0 = CAM_PIN_D0;
  config.pin_d1 = CAM_PIN_D1;
  config.pin_d2 = CAM_PIN_D2;
  config.pin_d3 = CAM_PIN_D3;

  config.pin_d4 = CAM_PIN_D4;
  config.pin_d5 = CAM_PIN_D5;
  config.pin_d6 = CAM_PIN_D6;
  config.pin_d7 = CAM_PIN_D7;


  config.pin_xclk = CAM_PIN_XCLK;
  config.pin_pclk = CAM_PIN_PCLK;
  config.pin_vsync = CAM_PIN_VSYNC;
  config.pin_href = CAM_PIN_HREF;


  config.pin_sccb_sda = CAM_PIN_SIOD;
  config.pin_sccb_scl = CAM_PIN_SIOC;


  config.pin_pwdn = CAM_PIN_PWDN;
  config.pin_reset = CAM_PIN_RESET;


  config.xclk_freq_hz = 20000000;

  config.pixel_format = PIXFORMAT_JPEG;


  // VGA = 640 x 480
  // Good starting point for stable streaming.

  config.frame_size = FRAMESIZE_VGA;

  config.jpeg_quality = 12;


  if (psramFound())
  {
    Serial.println("PSRAM detected.");

    config.fb_location = CAMERA_FB_IN_PSRAM;
    config.fb_count = 2;
    config.grab_mode = CAMERA_GRAB_LATEST;
  }
  else
  {
    Serial.println("WARNING: PSRAM not detected.");

    config.fb_location = CAMERA_FB_IN_DRAM;
    config.fb_count = 1;
    config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  }


  esp_err_t result = esp_camera_init(&config);


  if (result != ESP_OK)
  {
    Serial.printf(
      "ERROR: Camera initialization failed: 0x%X\n",
      result
    );

    return false;
  }

  sensor_t* sensor = esp_camera_sensor_get();
  sensor->set_vflip(sensor, 1);

  Serial.println("Camera initialized.");

  return true;
}


// ==================================================
// CONNECT WIFI
// ==================================================

bool connectWiFi()
{
  Serial.println();
  Serial.print("Connecting to Wi-Fi");


  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);

  WiFi.begin(
    WIFI_SSID,
    WIFI_PASSWORD
  );


  unsigned long startTime = millis();

  const unsigned long WIFI_TIMEOUT = 20000;


  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);

    Serial.print(".");


    if (millis() - startTime >= WIFI_TIMEOUT)
    {
      Serial.println();
      Serial.println("ERROR: Wi-Fi connection timeout.");

      return false;
    }
  }


  Serial.println();
  Serial.println("Wi-Fi connected.");

  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());


  return true;
}


// ==================================================
// SETUP
// ==================================================

void setup()
{
  Serial.begin(115200);

  delay(1500);


  Serial.println();
  Serial.println("==============================");
  Serial.println("XIAO ESP32S3 Sense");
  Serial.println("Camera Web Stream");
  Serial.println("Dogi Lab");
  Serial.println("==============================");
  Serial.println();


  if (!initializeCamera())
  {
    Serial.println("SYSTEM STOPPED: Camera error.");

    return;
  }


  if (!connectWiFi())
  {
    Serial.println("SYSTEM STOPPED: Wi-Fi error.");

    return;
  }


  if (!startCameraServer())
  {
    Serial.println("SYSTEM STOPPED: Web server error.");

    return;
  }


  Serial.println();
  Serial.println("Camera server started.");

  Serial.print("Open in browser: http://");
  Serial.println(WiFi.localIP());

  Serial.println();
}


// ==================================================
// LOOP
// ==================================================

void loop()
{
  delay(1000);
}
