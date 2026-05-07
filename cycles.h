#pragma once
#include <config.h>

// ⚙️ Step System
enum StepType { STEP_FILL, STEP_HEAT, STEP_WASH, STEP_DRAIN, STEP_WAIT, STEP_END };

struct CycleStep {
  StepType type;
  unsigned long durationMs; // 0 = run until condition met
};

struct DishCycle {
  const char* name;
  const CycleStep* steps;
  int stepCount;
};

// Macro helpers
#define S(type, dur) {type, dur}
#define END {STEP_END, 0}

// 🔄 Predefined Cycles (extern to avoid multiple definition)
extern const DishCycle AVAILABLE_CYCLES[];
extern const int CYCLE_COUNT;

// 🧰 Helper function declarations
const char* stateToString(DishWasherState s);
const char* stepToString(StepType t);
unsigned long getStepTimeRemaining(const CycleStep& step, unsigned long stepStart);