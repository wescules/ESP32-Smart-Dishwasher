#pragma once
#include <config.h>
#include <cycles.h>

// 🔄 Runtime Variables (extern for cross-file access)
extern unsigned long stepStart;
extern int currentCycleIdx;
extern int currentStepIdx;
extern bool waterLow, waterHigh, hasError;
extern DishWasherState currentState;

// 🧪 MOCK SENSOR CONTROLS
inline bool mockSensors = false;
inline bool mockWaterLow = false;
inline bool mockWaterHigh = false;

// 🧰 Function Declarations
void initDishwasher();
void updateWaterLevels();
void runStep();
void resetDishwasher();
void clearError();