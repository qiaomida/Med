# Copilot Instructions for ESP32 PlatformIO Project

## Project Overview
This is a PlatformIO project targeting the `freenove_esp32_s3_wroom` board using the Arduino framework. It integrates OLED display (SSD1306), servo motor control, and Blinker IoT library for remote device management via mobile app.

## Architecture
- **Entry Point**: `src/main.cpp` handles setup, FreeRTOS task creation, and main loop
- **IoT Module**: `src/bblinker.h` and `src/bblinker.cpp` manage Blinker components (buttons, numbers) and WiFi connectivity
- **Hardware Integration**: OLED display on I2C (SDA=10, SCL=9), servo on pin 11
- **Concurrency**: FreeRTOS tasks pinned to specific cores (TaskOne on core 0, TaskTwo on core 1)

## Key Patterns
- **FreeRTOS Tasks**: Use `xTaskCreatePinnedToCore()` for multi-core execution, e.g., `xTaskCreatePinnedToCore(xTaskOne, "TaskOne", 4096, NULL, 1, NULL, 0);`
- **OLED Display**: Animation functions like `drawLines()`, `drawRect()`, `fillRect()` in `main.cpp` for visual effects
- **Blinker Components**: Define components in header (e.g., `BlinkerButton Button1("btn-abc")`), attach callbacks in init
- **Serial Debugging**: Print core ID and counters in tasks: `Serial.println("xTaskOne:" + String(xPortGetCoreID()) + "-" + String(taskOne));`

## Developer Workflows
- **Build**: `pio run --environment freenove_esp32_s3_wroom`
- **Upload**: `pio run --target upload --environment freenove_esp32_s3_wroom`
- **Debugging**: Monitor serial output at 115200 baud for task execution and Blinker logs

## Conventions
- **Credentials**: Hardcode WiFi SSID/PSWD and Blinker auth in `bblinker.h` (update for production)
- **Comments**: Use Chinese for hardware setup notes
- **Library Dependencies**: Pull from GitHub repos in `platformio.ini` (e.g., `https://github.com/ThingPulse/esp8266-oled-ssd1306.git`)
- **Task Priorities**: Higher numbers for higher priority (TaskTwo priority 2 > TaskOne priority 1)

## Integration Points
- **Blinker App**: Remote control via button callbacks and number updates
- **Hardware Pins**: OLED I2C on pins 10/9, servo on pin 11, built-in LED for status
- **External Libs**: SSD1306Wire for display, Servo for motor, Blinker for IoT

Reference: `platformio.ini` for build config, `src/bblinker.h` for component definitions.</content>
<parameter name="filePath">d:\Project_file\Platformio\ttt\.github\copilot-instructions.md