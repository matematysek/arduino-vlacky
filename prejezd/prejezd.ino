#include <Servo.h>
#define STATE_INIT         0
#define STATE_OPEN_FREE    1
#define STATE_OPEN_ACTIVE  2
#define STATE_PREWARN      3
#define STATE_CLOSING      4
#define STATE_CLOSED       5
#define STATE_OPENING      6
#define STATE_OPENING_II   7

Servo sranky;

int state = STATE_INIT;
int servoState = 0;
bool zabzar = true;

unsigned long prewarnStart = 0;
unsigned long closeStart = 0;
unsigned long openStart = 0;

void setup() {
  pinMode(2, OUTPUT); // LED 1 červená
  pinMode(3, OUTPUT); // LED 2 červená
  pinMode(4, OUTPUT); // LED lunobílá
  pinMode(5, OUTPUT); // LED místo repro
  pinMode(10, INPUT_PULLUP); // ruční spouštění

  sranky.attach(12);
  Serial.begin(115200);
  Serial.println("prejezd.ino");
  Serial.println("Prejezd inicializovan");
}

void loop() {
  if (digitalRead(10) == LOW && state == STATE_CLOSING || digitalRead(10) == LOW && state == STATE_PREWARN) {
    Serial.println("RESET -> STATE_OPEN_ACTIVE");
    sranky.write(0);       // Zavory nahoru
    servoState = 0;
    digitalWrite(2, LOW);
    digitalWrite(3, LOW);
    digitalWrite(5, LOW);
    digitalWrite(4, HIGH); // Lunobila on
    state = STATE_OPEN_ACTIVE;
    return;  // Skip current loop cycle, start fresh
  }

  switch (state) {
    case STATE_INIT:
      sranky.write(90); // zavory dole
      servoState = 90;
      state = zabzar ? STATE_OPEN_ACTIVE : STATE_OPEN_FREE;
      break;

    case STATE_OPEN_FREE:
      sranky.write(0);
      servoState = 0;
      digitalWrite(2, LOW);
      digitalWrite(3, LOW);
      break;

    case STATE_OPEN_ACTIVE:
      sranky.write(0);
      servoState = 0;
      blinkLunobila();
      if (digitalRead(10) == HIGH) {
        Serial.println("Rucni spusteni -> predzvaneni");   
        digitalWrite(2, LOW);
        digitalWrite(3, LOW);
        digitalWrite(5, HIGH); // indikace
        digitalWrite(4, LOW);  // vypnout lunobílou
        prewarnStart = millis();
        state = STATE_PREWARN;
      }
      break;

    case STATE_PREWARN:
      blinkCervene();
      if (millis() - prewarnStart >= 15000) {
        Serial.println("Predzvaneni skoncilo -> zavory dolu");
        closeStart = millis();
        state = STATE_CLOSING;
      }
      break;

    case STATE_CLOSING:
      blinkCervene();
      if (millis() - closeStart <= 1000) {
        int angle = map(millis() - closeStart, 0, 1000, servoState, 90);
        sranky.write(angle);
      } else {
        sranky.write(90);
        servoState = 90;
        state = STATE_CLOSED;
        Serial.println("Zavory dole");
      }
      break;

    case STATE_CLOSED:
      blinkCervene();
      if (digitalRead(10) == LOW) {
        Serial.println("Rucni odblokovani -> zavory nahoru");
        digitalWrite(5, HIGH);
        digitalWrite(4, LOW);
        openStart = millis();
        state = STATE_OPENING;
      }
      break;

    case STATE_OPENING:        
    digitalWrite(5, LOW);
      blinkCervene();
      if (millis() - openStart <= 1000) {
        int angle = map(millis() - openStart, 0, 1000, servoState, 0);
        sranky.write(angle);
      } else {
        sranky.write(0);
        servoState = 0;
        state = STATE_OPENING_II;
        Serial.println("Zavory nahore");
    
        digitalWrite(2, LOW);
        digitalWrite(3, LOW);
      }
      break;
  
  case STATE_OPENING_II:
    servoState = 0;
    state = STATE_OPEN_ACTIVE;
    break;
  }
}

void blinkCervene() {
  if ((millis() / 1000) % 2 == 0) {
    digitalWrite(2, HIGH);
    digitalWrite(3, LOW);
  } else {
    digitalWrite(2, LOW);
    digitalWrite(3, HIGH);
  }
}

void blinkLunobila() {
  if ((millis() / 1500) % 2 == 0) {
    digitalWrite(4, HIGH);
  } else {
    digitalWrite(4, LOW);
  }
}
