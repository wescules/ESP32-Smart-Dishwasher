#include <Arduino.h>
#include <dishwasher.h>

unsigned long stepStart = 0;
int currentCycleIdx = 1;
int currentStepIdx = 0;
bool waterLow = false, waterHigh = false, hasError = false;
DishWasherState currentState = IDLE;
unsigned long lastSensorCheck = 0;

void initDishwasher() {
  pinMode(PIN_INPUT_SOLENOID, OUTPUT); pinMode(PIN_DRAIN_SOLENOID, OUTPUT);
  pinMode(PIN_PUMP, OUTPUT); pinMode(PIN_HEATER, OUTPUT);
  pinMode(PIN_WATER_LOW, INPUT); pinMode(PIN_WATER_HIGH, INPUT);
  digitalWrite(PIN_INPUT_SOLENOID, LOW); digitalWrite(PIN_DRAIN_SOLENOID, LOW);
  digitalWrite(PIN_PUMP, LOW); digitalWrite(PIN_HEATER, LOW);
}

void updateWaterLevels() {
  // MOCK MODE: Override real sensors
  if (mockSensors) {
    waterLow = mockWaterLow;
    waterHigh = mockWaterHigh;
    return;
  }

  if (millis() - lastSensorCheck < SENSOR_INTERVAL_MS) return;
  lastSensorCheck = millis();

  pinMode(PIN_WATER_LOW, INPUT_PULLUP); delayMicroseconds(100);
  bool rawLow = (digitalRead(PIN_WATER_LOW) == LOW);
  pinMode(PIN_WATER_LOW, INPUT);

  pinMode(PIN_WATER_HIGH, INPUT_PULLUP); delayMicroseconds(100);
  bool rawHigh = (digitalRead(PIN_WATER_HIGH) == LOW);
  pinMode(PIN_WATER_HIGH, INPUT);

  static bool lowStable = false, highStable = false;
  static unsigned long lowTime = 0, highTime = 0;
  if (rawLow != lowStable) lowTime = millis();
  if (rawHigh != highStable) highTime = millis();
  if (millis() - lowTime > 500) lowStable = rawLow;
  if (millis() - highTime > 500) highStable = rawHigh;

  waterLow = lowStable; waterHigh = highStable;
}

void runStep() {
  if (currentState == ERROR) {
    digitalWrite(PIN_INPUT_SOLENOID, LOW); digitalWrite(PIN_DRAIN_SOLENOID, LOW);
    digitalWrite(PIN_PUMP, LOW); digitalWrite(PIN_HEATER, LOW);
    return;
  }
  if (currentState == IDLE) { currentStepIdx = 0; return; }

  const DishCycle& cycle = AVAILABLE_CYCLES[currentCycleIdx];
  if (currentStepIdx >= cycle.stepCount) { 
    currentState = IDLE; currentStepIdx = 0; 
    Serial.println("Cycle Complete!"); 
    return; 
  }

  const CycleStep& step = cycle.steps[currentStepIdx];
  unsigned long elapsed = millis() - stepStart;
  bool stepDone = false;

  // SAFETY: Kill heater instantly if water drops
  if (step.type == STEP_HEAT && !waterLow) {
    activeError = ERROR_LOW_WATER; currentState = ERROR;
    digitalWrite(PIN_HEATER, LOW);
    Serial.println("ERROR: Heater cut - water level low!"); return;
  }

  switch(step.type) {
    case STEP_FILL:
      digitalWrite(PIN_INPUT_SOLENOID, HIGH);
      if (waterLow || waterHigh || (step.durationMs > 0 && elapsed >= step.durationMs)) { stepDone = true; currentState = FILLING; }
      break;
    case STEP_HEAT:
      digitalWrite(PIN_HEATER, HIGH);
      if (step.durationMs > 0 && elapsed >= step.durationMs) { stepDone = true; currentState = HEATING; }
      break;
    case STEP_WASH:
      digitalWrite(PIN_PUMP, HIGH);
      // FIX: Only completes on timeout. If durationMs == 0, it runs forever until next step.
      if (step.durationMs > 0 && elapsed >= step.durationMs) { stepDone = true; currentState = WASHING; }
      break;
    case STEP_DRAIN:
      digitalWrite(PIN_DRAIN_SOLENOID, HIGH); digitalWrite(PIN_PUMP, HIGH);
      if (!waterLow || (step.durationMs > 0 && elapsed >= step.durationMs)) { stepDone = true; currentState = DRAINING; }
      break;
    case STEP_WAIT:
      if (step.durationMs > 0 && elapsed >= step.durationMs) { stepDone = true; currentState = IDLE; }
      break;
    case STEP_END:
      stepDone = true;
      break;
  }

  if (stepDone) {
    digitalWrite(PIN_INPUT_SOLENOID, LOW); digitalWrite(PIN_DRAIN_SOLENOID, LOW);
    digitalWrite(PIN_PUMP, LOW); digitalWrite(PIN_HEATER, LOW);
    
    currentStepIdx++;
    stepStart = millis();
    
    Serial.printf("⏭️ Step %d [%s] finished (Dur: %lums, Elapsed: %lums)\n", 
      currentStepIdx-1, stepToString(step.type), step.durationMs, elapsed);
    if (currentStepIdx < cycle.stepCount) {
      Serial.printf("➡️  Next: Step %d [%s] (Dur: %lums)\n", 
        currentStepIdx, stepToString(cycle.steps[currentStepIdx].type), cycle.steps[currentStepIdx].durationMs);
    }
  }
}

// SAFE ERROR CLEARING FUNCTION
void clearError() {
  if (currentState != ERROR) return;

  // Prevent clearing if hardware fault still exists
  if (activeError == ERROR_LOW_WATER && !waterLow) {
    Serial.println("⛔ Cannot clear: Water still low. Fix hardware first.");
    return;
  }
  if (activeError == ERROR_FILL_TIMEOUT && !waterLow) {
    Serial.println("⛔ Cannot clear: Water supply still blocked.");
    return;
  }

  activeError = NO_ERROR;
  currentState = IDLE;
  currentStepIdx = 0;
  Serial.println("✅ Error cleared. System ready.");
}
void resetDishwasher() {
  currentState = IDLE; currentStepIdx = 0; hasError = false;
  digitalWrite(PIN_INPUT_SOLENOID, LOW); digitalWrite(PIN_DRAIN_SOLENOID, LOW);
  digitalWrite(PIN_PUMP, LOW); digitalWrite(PIN_HEATER, LOW);
}