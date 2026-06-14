#include <IRremote.hpp>

const int F_IN1 = 2;   // L298N-F input 1
const int F_IN2 = 4;   // L298N-F input 2
const int F_IN3 = 7;   // L298N-F input 3
const int F_IN4 = 8;   // L298N-F input 4
const int F_ENA = 6;   // L298N-F PWM (F1,F2)
const int F_ENB = 5;   // L298N-F PWM (F3,F4)

const int R_IN1 = 12;  // L298N-R input A
const int R_IN2 = 13;  // L298N-R input B
const int R_IN3 = A5;  // L298N-R input C
const int R_IN4 = A4;  // L298N-R input D
const int R_ENA = 9;   // L298N-R PWM (R1,R2)
const int R_ENB = 10;  // L298N-R PWM (R3,R4)

const int IR_PIN = A0; // KY-022 signal pin

// IR command codes (NEC protocol, command byte)
const uint8_t IR_FWD   = 0x18;
const uint8_t IR_REV   = 0x52;
const uint8_t IR_LEFT  = 0x08;
const uint8_t IR_RIGHT = 0x5A;
const uint8_t IR_STOP  = 0x1C;

// --- Tuning ---
// BRAKE_MS:  hold active brake this long before switching direction.
//            Increase if the droid lurches on reversal; decrease if it hesitates.
// RAMP_STEP / RAMP_MS: PWM units changed every RAMP_MS ms.
//            Lower RAMP_STEP or higher RAMP_MS = smoother but slower transitions.
// DEFAULT_SPEED: starting speed on first move command (motors need ~100+ to overcome friction).
const unsigned long BRAKE_MS      = 80;
const int           RAMP_STEP     = 8;
const unsigned long RAMP_MS       = 15;
const int           DEFAULT_SPEED = 150;

enum DriveState {
  STATE_STOPPED,
  STATE_FWD,
  STATE_REV,
  STATE_LEFT,
  STATE_RIGHT,
  STATE_BRAKING_TO_FWD,
  STATE_BRAKING_TO_REV,
  STATE_BRAKING_TO_LEFT,
  STATE_BRAKING_TO_RIGHT,
  STATE_BRAKING_TO_STOP
};

DriveState driveState = STATE_STOPPED;
int currentSpeed = 0;
int targetSpeed  = DEFAULT_SPEED;
unsigned long brakeStartMs = 0;
unsigned long lastRampMs   = 0;

void activateFWD();
void activateREV();
void activateLeft();
void activateRight();
void startBraking(DriveState next);
void applyBrake();
void allLow();
void applySpeed();

void setup() {
  Serial.begin(9600);

  int motorPins[] = {F_IN1, F_IN2, F_IN3, F_IN4, R_IN1, R_IN2, R_IN3, R_IN4};
  int pwmPins[]   = {F_ENA, F_ENB, R_ENA, R_ENB};
  for (int i = 0; i < 8; i++) pinMode(motorPins[i], OUTPUT);
  for (int i = 0; i < 4; i++) pinMode(pwmPins[i],   OUTPUT);

  allLow();
  applySpeed();

  // DISABLE_LED_FEEDBACK: pin 13 is R_IN2, so we cannot let IRremote toggle it
  IrReceiver.begin(IR_PIN, DISABLE_LED_FEEDBACK);

  Serial.println("Ready. IR active. Serial: FWD | REV | LEFT | RIGHT | STOP | SPEED <0-255>");
}

void loop() {
  handleIR();
  handleSerial();
  updateDrive();
}

// ---- IR input ----

void handleIR() {
  if (!IrReceiver.decode()) return;

  // Ignore repeat signals — speed control via IR to be added later
  if (!(IrReceiver.decodedIRData.flags & IRDATA_FLAGS_IS_REPEAT)) {
    switch (IrReceiver.decodedIRData.command) {
      case IR_FWD:   commandFWD();   break;
      case IR_REV:   commandREV();   break;
      case IR_LEFT:  commandLeft();  break;
      case IR_RIGHT: commandRight(); break;
      case IR_STOP:  commandSTOP();  break;
      default: break;
    }
  }

  IrReceiver.resume();
}

// ---- Serial input ----

void handleSerial() {
  if (!Serial.available()) return;
  String cmd = Serial.readStringUntil('\n');
  cmd.trim();
  cmd.toUpperCase();

  if      (cmd == "FWD")             commandFWD();
  else if (cmd == "REV")             commandREV();
  else if (cmd == "LEFT")            commandLeft();
  else if (cmd == "RIGHT")           commandRight();
  else if (cmd == "STOP")            commandSTOP();
  else if (cmd.startsWith("SPEED ")) commandSpeed(cmd.substring(6).toInt());
  else Serial.println("Unknown. Use: FWD | REV | LEFT | RIGHT | STOP | SPEED <0-255>");
}

// ---- Command handlers ----

// Returns true if driveState is a mid-brake transition
static bool isBraking() {
  return driveState == STATE_BRAKING_TO_FWD  || driveState == STATE_BRAKING_TO_REV  ||
         driveState == STATE_BRAKING_TO_LEFT || driveState == STATE_BRAKING_TO_RIGHT ||
         driveState == STATE_BRAKING_TO_STOP;
}

void commandFWD() {
  if (driveState == STATE_FWD)     { Serial.println("Already FWD"); return; }
  if (driveState == STATE_STOPPED) { activateFWD(); }
  else if (isBraking())            { driveState = STATE_BRAKING_TO_FWD; }
  else                             { startBraking(STATE_BRAKING_TO_FWD); }
  Serial.println("CMD: FWD");
}

void commandREV() {
  if (driveState == STATE_REV)     { Serial.println("Already REV"); return; }
  if (driveState == STATE_STOPPED) { activateREV(); }
  else if (isBraking())            { driveState = STATE_BRAKING_TO_REV; }
  else                             { startBraking(STATE_BRAKING_TO_REV); }
  Serial.println("CMD: REV");
}

void commandLeft() {
  if (driveState == STATE_LEFT)    { Serial.println("Already LEFT"); return; }
  if (driveState == STATE_STOPPED) { activateLeft(); }
  else if (isBraking())            { driveState = STATE_BRAKING_TO_LEFT; }
  else                             { startBraking(STATE_BRAKING_TO_LEFT); }
  Serial.println("CMD: LEFT");
}

void commandRight() {
  if (driveState == STATE_RIGHT)   { Serial.println("Already RIGHT"); return; }
  if (driveState == STATE_STOPPED) { activateRight(); }
  else if (isBraking())            { driveState = STATE_BRAKING_TO_RIGHT; }
  else                             { startBraking(STATE_BRAKING_TO_RIGHT); }
  Serial.println("CMD: RIGHT");
}

void commandSTOP() {
  if (driveState == STATE_STOPPED) { Serial.println("Already stopped"); return; }
  if (isBraking())                 { driveState = STATE_BRAKING_TO_STOP; }
  else                             { startBraking(STATE_BRAKING_TO_STOP); }
  Serial.println("CMD: STOP");
}

void commandSpeed(int s) {
  // Motors may not respond below ~100 PWM due to gear friction
  targetSpeed = constrain(s, 0, 255);
  Serial.print("Target speed: "); Serial.println(targetSpeed);
}

// ---- State machine ----

void updateDrive() {
  unsigned long now = millis();

  switch (driveState) {

    case STATE_FWD:
    case STATE_REV:
    case STATE_LEFT:
    case STATE_RIGHT:
      if (now - lastRampMs >= RAMP_MS) {
        lastRampMs = now;
        if      (currentSpeed < targetSpeed) currentSpeed = min(currentSpeed + RAMP_STEP, targetSpeed);
        else if (currentSpeed > targetSpeed) currentSpeed = max(currentSpeed - RAMP_STEP, 0);
        applySpeed();
      }
      break;

    case STATE_BRAKING_TO_FWD:
    case STATE_BRAKING_TO_REV:
    case STATE_BRAKING_TO_LEFT:
    case STATE_BRAKING_TO_RIGHT:
    case STATE_BRAKING_TO_STOP:
      applyBrake();
      if (now - lastRampMs >= RAMP_MS && currentSpeed > 0) {
        lastRampMs = now;
        currentSpeed = max(currentSpeed - RAMP_STEP, 0);
        applySpeed();
      }
      if (now - brakeStartMs >= BRAKE_MS && currentSpeed == 0) {
        if      (driveState == STATE_BRAKING_TO_FWD)   activateFWD();
        else if (driveState == STATE_BRAKING_TO_REV)   activateREV();
        else if (driveState == STATE_BRAKING_TO_LEFT)  activateLeft();
        else if (driveState == STATE_BRAKING_TO_RIGHT) activateRight();
        else { allLow(); driveState = STATE_STOPPED; Serial.println("Stopped."); }
      }
      break;

    case STATE_STOPPED:
      break;
  }
}

// ---- Drive helpers ----

void startBraking(DriveState next) {
  driveState   = next;
  brakeStartMs = millis();
  lastRampMs   = millis();
  applyBrake();
}

void activateFWD() {
  allLow();
  digitalWrite(F_IN2, HIGH);
  digitalWrite(F_IN4, HIGH);
  digitalWrite(R_IN1, HIGH);
  digitalWrite(R_IN4, HIGH);
  driveState = STATE_FWD;
  lastRampMs = millis();
  Serial.println("Running FWD.");
}

void activateREV() {
  allLow();
  digitalWrite(F_IN1, HIGH);
  digitalWrite(F_IN3, HIGH);
  digitalWrite(R_IN2, HIGH);
  digitalWrite(R_IN3, HIGH);
  driveState = STATE_REV;
  lastRampMs = millis();
  Serial.println("Running REV.");
}

// LEFT/RIGHT assume Motor A (IN1/IN2) = left side, Motor B (IN3/IN4) = right side.
// If the droid turns the wrong way, swap activateLeft() and activateRight() pin sets.
void activateLeft() {
  allLow();
  digitalWrite(F_IN1, HIGH); // Front-left  REV
  digitalWrite(F_IN4, HIGH); // Front-right FWD
  digitalWrite(R_IN2, HIGH); // Rear-left   REV
  digitalWrite(R_IN4, HIGH); // Rear-right  FWD
  driveState = STATE_LEFT;
  lastRampMs = millis();
  Serial.println("Running LEFT.");
}

void activateRight() {
  allLow();
  digitalWrite(F_IN2, HIGH); // Front-left  FWD
  digitalWrite(F_IN3, HIGH); // Front-right REV
  digitalWrite(R_IN1, HIGH); // Rear-left   FWD
  digitalWrite(R_IN3, HIGH); // Rear-right  REV
  driveState = STATE_RIGHT;
  lastRampMs = millis();
  Serial.println("Running RIGHT.");
}

// Short-brake: both INs HIGH per motor pair short-circuits the terminals,
// using back-EMF to resist rotation faster than coasting.
void applyBrake() {
  digitalWrite(F_IN1, HIGH); digitalWrite(F_IN2, HIGH);
  digitalWrite(F_IN3, HIGH); digitalWrite(F_IN4, HIGH);
  digitalWrite(R_IN1, HIGH); digitalWrite(R_IN2, HIGH);
  digitalWrite(R_IN3, HIGH); digitalWrite(R_IN4, HIGH);
}

void allLow() {
  digitalWrite(F_IN1, LOW); digitalWrite(F_IN2, LOW);
  digitalWrite(F_IN3, LOW); digitalWrite(F_IN4, LOW);
  digitalWrite(R_IN1, LOW); digitalWrite(R_IN2, LOW);
  digitalWrite(R_IN3, LOW); digitalWrite(R_IN4, LOW);
}

void applySpeed() {
  analogWrite(F_ENA, currentSpeed);
  analogWrite(F_ENB, currentSpeed);
  analogWrite(R_ENA, currentSpeed);
  analogWrite(R_ENB, currentSpeed);
}
