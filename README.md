# ESP32 Dishwasher Controller
WiFi-connected dishwasher controller with customizable cycles, live web UI, and safety interlocks.

## Features
- Step-based cycle execution (Fill, Heat, Wash, Drain, Dry)
- Real-time web dashboard with progress bar & sensor status
- Mock sensor test mode for software-only debugging
- Hardware safety: heater cutoff on low water, timeout fallbacks

## Hardware Wiring
| Component | ESP32 Pin |
|-----------|-----------|
| Inlet Solenoid | GPIO 25 |
| Drain Solenoid | GPIO 26 |
| Circulation Pump | GPIO 27 |
| Heating Element (SSR) | GPIO 14 |
| Water Level LOW | GPIO 32 |
| Water Level HIGH | GPIO 33 |

## Setup
1. Install ESP32 board package in Arduino IDE
2. Edit `config.h` with your WiFi credentials
3. Upload to ESP32
4. Open Serial Monitor to get IP address
5. Visit `http://<IP>` in browser

  
 <img width="2096" height="1258" alt="wifidishwasher diagram" src="https://github.com/user-attachments/assets/15c4e38c-861e-4c88-8c4c-e412cd9a298d" />
