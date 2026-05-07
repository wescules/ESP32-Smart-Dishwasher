#pragma once
#include <Arduino.h>

// WiFi Credentials
inline const char* WIFI_SSID = "客厅";
inline const char* WIFI_PASS = "06110107";

// Pin Definitions
inline const int PIN_INPUT_SOLENOID = 25;  // Water inlet valve
inline const int PIN_DRAIN_SOLENOID = 26;  // Drain valve
inline const int PIN_PUMP            = 27;  // Circulation pump
inline const int PIN_HEATER          = 14;  // Heating element (via SSR)
inline const int PIN_WATER_LOW       = 32;  // Low water level probe
inline const int PIN_WATER_HIGH      = 33;  // High water level probe (safety)

// Sensor Timing (pulsed to reduce corrosion)
inline const unsigned long SENSOR_PULSE_MS = 100;
inline const unsigned long SENSOR_INTERVAL_MS = 1000;

// State Machine States
enum DishWasherState { IDLE, FILLING, HEATING, WASHING, DRAINING, ERROR };
enum DishError { NO_ERROR, ERROR_LOW_WATER, ERROR_FILL_TIMEOUT, ERROR_DRAIN_TIMEOUT, ERROR_OVERTEMP };
inline DishError activeError = NO_ERROR;