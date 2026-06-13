const int F_IN1 = 2;  // L298N-F input 1
const int F_IN2 = 4;  // L298N-F input 2
const int F_IN3 = 7;  // L298N-F input 3
const int F_IN4 = 8;  // L298N-F input 4

const int F_ENA = 3;  // L298N-F PWM (F1,F2)
const int F_ENB = 5;  // L298N-F PWM (F3,F4)

const int R_IN1 = 12;  // L298N-R input A
const int R_IN2 = 13;  // L298N-R input B
const int R_IN3 = A0;  // L298N-R input C
const int R_IN4 = A1;  // L298N-R input D

const int R_ENA = 9;   // L298N-R PWM (R1,R2)
const int R_ENB = 10;  // L298N-R PWM (R3,R4)

enum Direction { DIR_STOP, DIR_FWD, DIR_REV }; // Current state of droid
// Begins motionless
Direction currentDir = DIR_STOP;
int currentSpeed = 0;

void setup() {
  Serial.begin(9600);

  // Initialize motor pins
  pinMode(F_IN1, OUTPUT);
  pinMode(F_IN2, OUTPUT);
  pinMode(F_IN3, OUTPUT);
  pinMode(F_IN4, OUTPUT);
  pinMode(R_IN1, OUTPUT);
  pinMode(R_IN2, OUTPUT);
  pinMode(R_IN3, OUTPUT);
  pinMode(R_IN4, OUTPUT);

  // Initialize PWM pins
  pinMode(F_ENA, OUTPUT);
  pinMode(F_ENB, OUTPUT);
  pinMode(R_ENA, OUTPUT);
  pinMode(R_ENB, OUTPUT);

  // Start with motor off
  digitalWrite(F_IN1, LOW);
  digitalWrite(F_IN2, LOW);
  digitalWrite(F_IN3, LOW);
  digitalWrite(F_IN4, LOW);
  digitalWrite(R_IN1, LOW);
  digitalWrite(R_IN2, LOW);
  digitalWrite(R_IN3, LOW);
  digitalWrite(R_IN4, LOW);

  // Start with PWM pins at default speed
  analogWrite(F_ENA, currentSpeed);
  analogWrite(F_ENB, currentSpeed);
  analogWrite(R_ENA, currentSpeed);
  analogWrite(R_ENB, currentSpeed);

  Serial.println("Ready. Commands: 'FWD', 'REV', 'STOP', 'SPEED <0-255>', or pin: 'F_IN1 HIGH', 'R_IN3 LOW', ...");
}
void loop() {
  if (Serial.available() > 0) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    cmd.toUpperCase();
    // Manual pin commands sidestep state setup, BE AWARE
    if      (cmd == "F_IN1 HIGH") { digitalWrite(F_IN1, HIGH); Serial.println("F_IN1 HIGH (pin 2)");  }
    else if (cmd == "F_IN1 LOW")  { digitalWrite(F_IN1, LOW);  Serial.println("F_IN1 LOW (pin 2)");   }
    else if (cmd == "F_IN2 HIGH") { digitalWrite(F_IN2, HIGH); Serial.println("F_IN2 HIGH (pin 4)");  }
    else if (cmd == "F_IN2 LOW")  { digitalWrite(F_IN2, LOW);  Serial.println("F_IN2 LOW (pin 4)");   }
    else if (cmd == "F_IN3 HIGH") { digitalWrite(F_IN3, HIGH); Serial.println("F_IN3 HIGH (pin 7)");  }
    else if (cmd == "F_IN3 LOW")  { digitalWrite(F_IN3, LOW);  Serial.println("F_IN3 LOW (pin 7)");   }
    else if (cmd == "F_IN4 HIGH") { digitalWrite(F_IN4, HIGH); Serial.println("F_IN4 HIGH (pin 8)");  }
    else if (cmd == "F_IN4 LOW")  { digitalWrite(F_IN4, LOW);  Serial.println("F_IN4 LOW (pin 8)");   }
    else if (cmd == "R_IN1 HIGH") { digitalWrite(R_IN1, HIGH); Serial.println("R_IN1 HIGH (pin 12)"); }
    else if (cmd == "R_IN1 LOW")  { digitalWrite(R_IN1, LOW);  Serial.println("R_IN1 LOW (pin 12)");  }
    else if (cmd == "R_IN2 HIGH") { digitalWrite(R_IN2, HIGH); Serial.println("R_IN2 HIGH (pin 13)"); }
    else if (cmd == "R_IN2 LOW")  { digitalWrite(R_IN2, LOW);  Serial.println("R_IN2 LOW (pin 13)");  }
    else if (cmd == "R_IN3 HIGH") { digitalWrite(R_IN3, HIGH); Serial.println("R_IN3 HIGH (pin A0)"); }
    else if (cmd == "R_IN3 LOW")  { digitalWrite(R_IN3, LOW);  Serial.println("R_IN3 LOW (pin A0)");  }
    else if (cmd == "R_IN4 HIGH") { digitalWrite(R_IN4, HIGH); Serial.println("R_IN4 HIGH (pin A1)"); }
    else if (cmd == "R_IN4 LOW")  { digitalWrite(R_IN4, LOW);  Serial.println("R_IN4 LOW (pin A1)");  }

    else if (cmd == "FWD")    { FWD(); Serial.println("F_IN1,F_IN4,R_IN1,R_IN3 HIGH"); }
    else if (cmd == "REV")    { REV(); Serial.println("F_IN2,F_IN3,R_IN2,R_IN4 HIGH"); }
    else if (cmd.startsWith("SPEED ")) {
      int newSpeed = cmd.substring(6).toInt();
      newSpeed = constrain(newSpeed, 0, 255); // Motors don't actually work between [0,255], it's more like [100,255]. This is due to the motors not being able to interpret switches in voltage above a certain frequency
      currentSpeed = newSpeed;
      Serial.print("Speed set to "); Serial.println(currentSpeed);
      applySpeed(); // Apply newSpeed
      }
    else if (cmd == "STOP")   {
      STOP();
      Serial.println("ALL LOW (motor stopped)");
    }
    else Serial.println("Unknown. Use: 'FWD', 'REV', 'STOP', 'SPEED <0-255>', or 'F_IN1 HIGH', 'R_IN3 LOW', ...");
  }
}

void STOP(){
    // Set currentDir to STOP
    currentDir = DIR_STOP;
    // Set wheels to LOW position
    digitalWrite(F_IN1, LOW);
    digitalWrite(F_IN2, LOW);
    digitalWrite(F_IN3, LOW);
    digitalWrite(F_IN4, LOW);
    digitalWrite(R_IN1, LOW);
    digitalWrite(R_IN2, LOW);
    digitalWrite(R_IN3, LOW);
    digitalWrite(R_IN4, LOW);
  }


void FWD(){
  if (currentDir == DIR_FWD) {
    Serial.println("Already FWD");
    return;
  }
  if (currentDir == DIR_REV) {
    STOP();
    delay(20);
  }
  // Set wheels to FWD
  digitalWrite(F_IN1, HIGH);
  digitalWrite(F_IN4, HIGH);
  digitalWrite(R_IN1, HIGH);
  digitalWrite(R_IN3, HIGH);
  currentDir = DIR_FWD;
}

void REV(){
  if (currentDir == DIR_REV) {
    Serial.println("Already REV");
    return;
  }
  if (currentDir == DIR_FWD) {
    STOP();
    delay(20);
  }
  // Set wheels to REV
  digitalWrite(F_IN2, HIGH);
  digitalWrite(F_IN3, HIGH);
  digitalWrite(R_IN2, HIGH);
  digitalWrite(R_IN4, HIGH);
  currentDir = DIR_REV;
}

void applySpeed(){
  analogWrite(F_ENA, currentSpeed);
  analogWrite(F_ENB, currentSpeed);
  analogWrite(R_ENA, currentSpeed);
  analogWrite(R_ENB, currentSpeed);
}
