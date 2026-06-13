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
  
  Serial.println("Ready. Commands: '2 HIGH', '2 LOW', '4 HIGH', '4 LOW', ... 'STOP'");
}
void loop() {
  if (Serial.available() > 0) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    cmd.toUpperCase();
    // Manual pin commands sidestep state setup, BE AWARE
    if      (cmd == "2 HIGH") { digitalWrite(F_IN1, HIGH); Serial.println("Pin 2 HIGH");  }
    else if (cmd == "2 LOW")  { digitalWrite(F_IN1, LOW);  Serial.println("Pin 2 LOW");   }
    else if (cmd == "3 HIGH") { digitalWrite(F_IN2, HIGH); Serial.println("Pin 3 HIGH");  }
    else if (cmd == "3 LOW")  { digitalWrite(F_IN2, LOW);  Serial.println("Pin 3 LOW");   }
    else if (cmd == "4 HIGH") { digitalWrite(F_IN3, HIGH);  Serial.println("Pin 4 HIGH"); }
    else if (cmd == "4 LOW")  { digitalWrite(F_IN3, LOW);  Serial.println("Pin 4 LOW");   }
    else if (cmd == "5 HIGH") { digitalWrite(F_IN4, HIGH);  Serial.println("Pin 5 HIGH"); }
    else if (cmd == "5 LOW")  { digitalWrite(F_IN4, LOW);  Serial.println("Pin 5 LOW");   }
    else if (cmd == "8 HIGH") { digitalWrite(R_IN1, HIGH);  Serial.println("Pin 8 HIGH"); }
    else if (cmd == "8 LOW")  { digitalWrite(R_IN1, LOW);  Serial.println("Pin 8 LOW");   }
    else if (cmd == "9 HIGH") { digitalWrite(R_IN2, HIGH);  Serial.println("Pin 9 HIGH"); }
    else if (cmd == "9 LOW")  { digitalWrite(R_IN2, LOW);  Serial.println("Pin 9 LOW");   }
    else if (cmd == "10 HIGH"){ digitalWrite(R_IN3, HIGH);  Serial.println("Pin 10 HIGH");}
    else if (cmd == "10 LOW") { digitalWrite(R_IN3, LOW);  Serial.println("Pin 10 LOW");  }
    else if (cmd == "11 HIGH"){ digitalWrite(R_IN4, HIGH);  Serial.println("Pin 11 HIGH");}
    else if (cmd == "11 LOW") { digitalWrite(R_IN4, LOW);  Serial.println("Pin 11 LOW");  }

    // Regular commands begin
    else if (cmd == "FWD")    { FWD(); Serial.println("Pin 2,4,7,8 HIGH"); }
    else if (cmd == "REV")    { REV(); Serial.println("Pin 12,13,A0,A1 HIGH"); }
    else if (cmd.startsWith("SPEED ")) {
      int newSpeed = cmd.substring(6).toInt();
      /* 
      Motors don't actually work between [0,255], it's more like [150,255].
      That's when all four motors move, although motor 4 moves slightly lower.
      Can implement "kick" to get all motors started at sufficient speed then roll them down. 
      Could also play with range so that it is actually within barrier of all motors moving and then take a percentage. 
      */
      newSpeed = constrain(newSpeed, 0, 255); 
      currentSpeed = newSpeed;
      Serial.print("Speed set to "); Serial.println(currentSpeed);
      // Apply newSpeed
      applySpeed(); 
      }
    else if (cmd == "STOP")   {
      STOP();
      Serial.println("ALL LOW (motor stopped)");
    }
    else Serial.println("Unknown. Use: '2 HIGH', '2 LOW', '4 HIGH', '4 LOW', 'STOP'...");
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
  return;
  }
  
void FWD(){
  if (currentDir == DIR_FWD) {
    Serial.println("Already FWD");
    return;
  }
  else if (currentDir == DIR_REV) {
    STOP();
    delay(200);
    currentDir = DIR_FWD;
    // Set wheels to FWD
    digitalWrite(F_IN1, HIGH);
    digitalWrite(F_IN3, HIGH);
    digitalWrite(R_IN1, HIGH);
    digitalWrite(R_IN3, HIGH);
    return;
  }
  currentDir = DIR_FWD;
  // Set wheels to FWD
  digitalWrite(F_IN1, HIGH);
  digitalWrite(F_IN3, HIGH);
  digitalWrite(R_IN1, HIGH);
  digitalWrite(R_IN3, HIGH);
  return;
}

void REV(){
  if (currentDir == DIR_REV) {
    Serial.println("Already REV");
    return;
  }
  else if (currentDir == DIR_FWD) {
    STOP();
    delay(200);
    currentDir = DIR_REV;
    // Set wheels to REV
    digitalWrite(F_IN2, HIGH);
    digitalWrite(F_IN4, HIGH);
    digitalWrite(R_IN2, HIGH);
    digitalWrite(R_IN4, HIGH);
    return;
  }
  currentDir = DIR_REV;
  // Set wheels to REV
  digitalWrite(F_IN2, HIGH);
  digitalWrite(F_IN4, HIGH);
  digitalWrite(R_IN2, HIGH);
  digitalWrite(R_IN4, HIGH);
  return;
}

void applySpeed(){
  analogWrite(F_ENA, currentSpeed);
  analogWrite(F_ENB, currentSpeed);
  analogWrite(R_ENA, currentSpeed);
  analogWrite(R_ENB, currentSpeed);
  return;
}
