# ESP32-C3 Super Mini — Example

PlatformIO/Arduino project for the ESP32-C3 Super Mini. Connects to a prioritised list of WiFi networks, syncs time via NTP, and exposes a small async HTTP API. The onboard LED blinks every second as a heartbeat.

## Features

- WiFi multi-network fallback — scans available networks and connects to the first known one in priority order
- NTP time sync (UTC+4, `pool.ntp.org`)
- Async HTTP server on port 80
- 1 Hz LED heartbeat on GPIO 8

## HTTP endpoints

| Method | Path | Response |
|--------|------|----------|
| GET | `/` | Plain-text usage hint |
| GET | `/health` | JSON — `ok`, `ip`, `mac`, `uptime_ms`, `time` |
| GET | `/info` | JSON — full chip info, WiFi details, heap stats, time |

## Requirements

- [PlatformIO](https://platformio.org/) (CLI or VS Code extension)
- ESP32-C3 Super Mini (or compatible DevKitM-1 board)

## Setup

1. Edit the `WIFI_LIST` array in [`src/main.cpp`](src/main.cpp) with your network credentials.
2. Adjust `GMT_OFFSET_SEC` if you are not in UTC+4.

## Build & flash

```bash
# Build
pio run

# Flash (auto-detects port)
pio run --target upload

# Monitor serial output
pio device monitor
```

## Dependencies

Managed via `platformio.ini`:

- [`ESP32Async/ESPAsyncWebServer`](https://github.com/ESP32Async/ESPAsyncWebServer)
- [`ESP32Async/AsyncTCP`](https://github.com/ESP32Async/AsyncTCP)
- [`bblanchon/ArduinoJson`](https://arduinojson.org/)
