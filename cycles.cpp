#include "cycles.h"

// const CycleStep quickRinse[] = {
//   S(STEP_FILL, 30000),   // Max 30s fill
//   S(STEP_WASH, 30000),   // Pump 30s
//   S(STEP_DRAIN, 30000),  // Max 30s drain
//   END
// };

// TEST CODE: Shortened durations for testing purposes
const CycleStep quickRinse[] = {
  S(STEP_FILL, 5000),   // Max 30s fill
  S(STEP_HEAT, 5000),
  S(STEP_WASH, 5000),   // Pump 30s
  S(STEP_DRAIN, 5000),  // Max 30s drain
  END
};

const CycleStep normalClean[] = {
  S(STEP_FILL, 60000),   // Max 60s fill (advances if waterLow triggers)
  S(STEP_HEAT, 180000),  // Heat 3 min
  S(STEP_WASH, 300000),  // Wash 5 min
  S(STEP_DRAIN, 60000),  // Max 60s drain (advances if tank empty)
  S(STEP_WAIT, 10000),   // Wait 10s
  END
};

const CycleStep deepClean[] = {
  S(STEP_FILL, 60000),
  S(STEP_HEAT, 300000),
  S(STEP_WASH, 600000),
  S(STEP_DRAIN, 60000),
  S(STEP_FILL, 30000),   // Rinse fill max 30s
  S(STEP_WASH, 120000),
  S(STEP_DRAIN, 60000),
  S(STEP_HEAT, 120000),
  END
};

const CycleStep steamMode[] = {
  S(STEP_FILL, 60000),
  S(STEP_HEAT, 240000),
  S(STEP_WAIT, 30000),
  S(STEP_DRAIN, 60000),
  S(STEP_DRAIN, 60000),
  END
};

const DishCycle AVAILABLE_CYCLES[] = {
  {"Quick Rinse", quickRinse, sizeof(quickRinse)/sizeof(CycleStep)},
  {"Normal Clean", normalClean, sizeof(normalClean)/sizeof(CycleStep)},
  {"Deep Clean", deepClean, sizeof(deepClean)/sizeof(CycleStep)},
  {"Steam Mode", steamMode, sizeof(steamMode)/sizeof(CycleStep)}
};
const int CYCLE_COUNT = sizeof(AVAILABLE_CYCLES) / sizeof(AVAILABLE_CYCLES[0]);

// ─────────────────────────────────────────────────────────────
// Helper Functions
// ─────────────────────────────────────────────────────────────

const char* stateToString(DishWasherState s) {
  switch(s) {
    case IDLE: return "IDLE"; case FILLING: return "FILLING";
    case HEATING: return "HEATING"; case WASHING: return "WASHING";
    case DRAINING: return "DRAINING"; case ERROR: return "ERROR";
    default: return "UNKNOWN";
  }
}

const char* stepToString(StepType t) {
  switch(t) {
    case STEP_FILL: return "Filling"; case STEP_HEAT: return "Heating";
    case STEP_WASH: return "Washing"; case STEP_DRAIN: return "Draining";
    case STEP_WAIT: return "Waiting"; case STEP_END: return "Done";
    default: return "Unknown";
  }
}

unsigned long getStepTimeRemaining(const CycleStep& step, unsigned long stepStart) {
  if (step.durationMs == 0) return 0;
  unsigned long elapsed = millis() - stepStart;
  return (elapsed >= step.durationMs) ? 0 : ((step.durationMs - elapsed) / 1000);
}