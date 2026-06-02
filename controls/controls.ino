const int IN1 = 2;  // L298N-F input 1
const int IN2 = 3;  // L298N-F input 2
const int IN3 = 4;  // L298N-F input 3
const int IN4 = 5;  // L298N-F input 4

const int INA = 8;  // L298N-R input A
const int INB = 9;  // L298N-R input B
const int INC = 10; // L298N-R input C
const int IND = 11; // L298N-R input D

enum Direction { DIR_STOP, DIR_FWD, DIR_REV }; // Current state of droid
Direction currentDir = DIR_STOP;               // Begins motionless

void setup() {
  Serial.begin(9600);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(INA, OUTPUT);
  pinMode(INB, OUTPUT);
  pinMode(INC, OUTPUT);
  pinMode(IND, OUTPUT);

  // Start with motor off
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
  digitalWrite(INA, LOW);
  digitalWrite(INB, LOW);
  digitalWrite(INC, LOW);
  digitalWrite(IND, LOW);
  Serial.println("Ready. Commands: '2 HIGH', '2 LOW', '4 HIGH', '4 LOW', ... 'STOP'");
}
void loop() {
  if (Serial.available() > 0) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    cmd.toUpperCase();
    // Manual pin commands sidestep state setup, BE AWARE
    if      (cmd == "2 HIGH") { digitalWrite(IN1, HIGH); Serial.println("Pin 2 HIGH");  }
    else if (cmd == "2 LOW")  { digitalWrite(IN1, LOW);  Serial.println("Pin 2 LOW");   }
    else if (cmd == "3 HIGH") { digitalWrite(IN2, HIGH); Serial.println("Pin 3 HIGH");  }
    else if (cmd == "3 LOW")  { digitalWrite(IN2, LOW);  Serial.println("Pin 3 LOW");   }
    else if (cmd == "4 HIGH") { digitalWrite(IN3, HIGH);  Serial.println("Pin 4 HIGH"); }
    else if (cmd == "4 LOW")  { digitalWrite(IN3, LOW);  Serial.println("Pin 4 LOW");   }
    else if (cmd == "5 HIGH") { digitalWrite(IN4, HIGH);  Serial.println("Pin 5 HIGH"); }
    else if (cmd == "5 LOW")  { digitalWrite(IN4, LOW);  Serial.println("Pin 5 LOW");   }
    else if (cmd == "8 HIGH") { digitalWrite(INA, HIGH);  Serial.println("Pin 8 HIGH"); }
    else if (cmd == "8 LOW")  { digitalWrite(INA, LOW);  Serial.println("Pin 8 LOW");   }
    else if (cmd == "9 HIGH") { digitalWrite(INB, HIGH);  Serial.println("Pin 9 HIGH"); }
    else if (cmd == "9 LOW")  { digitalWrite(INB, LOW);  Serial.println("Pin 9 LOW");   }
    else if (cmd == "10 HIGH"){ digitalWrite(INC, HIGH);  Serial.println("Pin 10 HIGH");}
    else if (cmd == "10 LOW") { digitalWrite(INC, LOW);  Serial.println("Pin 10 LOW");  }
    else if (cmd == "11 HIGH"){ digitalWrite(IND, HIGH);  Serial.println("Pin 11 HIGH");}
    else if (cmd == "11 LOW") { digitalWrite(IND, LOW);  Serial.println("Pin 11 LOW");  }
    
    else if (cmd == "FWD")    { FWD(); Serial.println("Pin 2,4,8,10 HIGH"); }
    else if (cmd == "REV")    { REV(); Serial.println("Pin 3,5,9,11 HIGH"); }
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
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, LOW);
    digitalWrite(INA, LOW);
    digitalWrite(INB, LOW);
    digitalWrite(INC, LOW);
    digitalWrite(IND, LOW);
  }
  

void FWD(){
  if (currentDir == DIR_FWD) {
    Serial.println("Already FWD");
    return;
  }
  if (currentDir == DIR_REV) {
    STOP();
    delay(200);
    // Set wheels to FWD
    digitalWrite(IN1, HIGH);
    //delay(50);// TEMPORARY: REMOVE AFTER TESTING
    digitalWrite(IN3, HIGH);
    //delay(50);// TEMPORARY: REMOVE AFTER TESTING
    digitalWrite(INA, HIGH);
    //delay(50);// TEMPORARY: REMOVE AFTER TESTING
    digitalWrite(INC, HIGH);
    currentDir = DIR_FWD;
  }
  // Set wheels to FWD
  digitalWrite(IN1, HIGH);
  //delay(50);// TEMPORARY: REMOVE AFTER TESTING
  digitalWrite(IN3, HIGH);
  //delay(50);// TEMPORARY: REMOVE AFTER TESTING
  digitalWrite(INA, HIGH);
  //delay(50);// TEMPORARY: REMOVE AFTER TESTING
  digitalWrite(INC, HIGH);
  currentDir = DIR_FWD;
}

void REV(){
  if (currentDir == DIR_REV) {
    Serial.println("Already REV");
    return;
  }
  if (currentDir == DIR_FWD) {
    STOP();
    delay(200);
    // Set wheels to REV
    digitalWrite(IN2, HIGH);
    //delay(50); // TEMPORARY: REMOVE AFTER TESTING
    digitalWrite(IN4, HIGH);
    //delay(50); // TEMPORARY: REMOVE AFTER TESTING
    digitalWrite(INB, HIGH);
    //delay(50);// TEMPORARY: REMOVE AFTER TESTING
    digitalWrite(IND, HIGH);
    currentDir = DIR_REV;
  }
  // Set wheels to REV
  digitalWrite(IN2, HIGH);
  //delay(50); // TEMPORARY: REMOVE AFTER TESTING
  digitalWrite(IN4, HIGH);
  //delay(50); // TEMPORARY: REMOVE AFTER TESTING
  digitalWrite(INB, HIGH);
  //delay(50);// TEMPORARY: REMOVE AFTER TESTING
  digitalWrite(IND, HIGH);
  currentDir = DIR_REV;
}
