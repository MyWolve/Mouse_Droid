// KY-022 IR receiver test — requires IRremote library v4+
// Install: Arduino IDE → Sketch → Include Library → Manage Libraries → "IRremote"
//
// Wiring:
//   KY-022 S  → A0
//   KY-022 -  → GND
//   KY-022 VCC → 5V
//
// Open Serial Monitor at 9600 baud and point a remote at the sensor.
// The built-in LED (pin 13) will blink on each received burst.

#include <IRremote.hpp>

const int IR_PIN = A0;

void setup() {
  Serial.begin(9600);
  IrReceiver.begin(IR_PIN, ENABLE_LED_FEEDBACK);
  Serial.println("KY-022 ready. Point a remote at the sensor and press a button.");
  Serial.println("------------------------------------------------------------------");
}

void loop() {
  if (!IrReceiver.decode()) return;

  auto &data = IrReceiver.decodedIRData;

  if (data.flags & IRDATA_FLAGS_IS_REPEAT) {
    Serial.println("(repeat)");
  } else {
    Serial.print("Protocol : "); Serial.println(getProtocolString(data.protocol));
    Serial.print("Address  : 0x"); Serial.println(data.address, HEX);
    Serial.print("Command  : 0x"); Serial.println(data.command, HEX);
    Serial.print("Raw hex  : 0x"); Serial.println(data.decodedRawData, HEX);
    if (data.protocol == UNKNOWN) {
      Serial.println("(Protocol unrecognised — try IRreceiveDump example for raw timing)");
    }
    Serial.println();
  }

  IrReceiver.resume();
}
