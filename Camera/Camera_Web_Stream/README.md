# Camera Web Stream

Simple MJPEG web camera streaming example for the **Seeed Studio XIAO ESP32S3 Sense**.

## Status

**Validated on physical XIAO ESP32S3 Sense hardware.**

## Requirements

* Seeed Studio XIAO ESP32S3 Sense
* Camera module
* 2.4 GHz Wi-Fi network
* Arduino/ESP32 development environment

## Usage

1. Open `Camera_Web_Stream.ino`.
2. Replace:

```cpp
const char* WIFI_SSID = "YOUR_WIFI_NAME";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";
```

with your Wi-Fi credentials.

3. Upload the sketch to the XIAO ESP32S3 Sense.
4. Open the Serial Monitor at **115200 baud**.
5. Wait for the board to display its IP address.
6. Open that IP address in a web browser connected to the same network.

The browser will display the live MJPEG camera stream.

## Features

* MJPEG live streaming
* Local web server
* Automatic IP address display through Serial
* PSRAM detection
* Camera orientation correction
* Wi-Fi connection timeout
* Basic error reporting

## Security

This example does **not include authentication**.

Any device on the same local network that can reach the ESP32's IP address may be able to access the camera stream.

It is intended as a simple educational and hardware-testing example.

## Validation

Tested and validated by **DogiLab** on physical XIAO ESP32S3 Sense hardware.

## License

Licensing terms are defined at the repository level.
