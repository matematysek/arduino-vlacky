#include <Bounce2.h>
#include <Servo.h>

Servo sranky; //sranky jsou zavory, toto je servo k nim
Bounce2::Button button = Bounce2::Button();

int servoState = 90; //po nabehnuti zavory jsou dole
bool active = false;         // true kdyz je aktivni rucni spusteni
bool timerStarted = false;   // true kdyz bezi 15 sekund predzvaneni
unsigned long timeStart = 0;
static unsigned long servoMoveStart = 0; //dolu
static unsigned long servoMoveStart2 = 0; //nahoru
static bool movingServo = false; //dolu
static bool movingServo2 = false; //nahoru
void setup() {
  pinMode(2, OUTPUT); //LED A
  pinMode(3, OUTPUT); //LED B
  button.attach(10, INPUT_PULLUP); //rucni uzavreni prejezdu
  pinMode(11, INPUT_PULLUP); //rezervni
  sranky.attach(12); //serva od zavor
  pinMode(13, OUTPUT); //debug dioda

  Serial.begin(115200); //baudy na serial monitor logovani
  Serial.println("prejezd.ino"); //co to sakra je v tom arduinu nahrane?

  digitalWrite(2, LOW); 
  digitalWrite(3, LOW);
  sranky.write(90); //zavory dolu
}

void loop() {
  button.update(); //detekce zmen na pinu 10
  if (button.fell()) { //pokud se zmenil stav na false
    Serial.println("10 (rucni spusteni) aktivni");
    active = true; //aktivni
    timerStarted = false;  // resetovani casovace
    timeStart = millis();  // poznaceni pocatku
  }

  if (button.rose()) { //pokud se zmenil srav na true
    Serial.println("10 (rucni spusteni) neaktivni");
    timeStart = 0; //vynulovat casovac
    servoMoveStart2 = millis(); //zavory nahoru
    movingServo2 = true; //hybe se to nahoru
     
  }
  if (movingServo2) {
  int duration = 1000; // bude se to zvedat 1000 milisekund
  int progress = millis() - servoMoveStart2;
  if (progress <= duration) {
    int angle = map(progress, 90, duration, servoState, 0);
    sranky.write(angle);
  } else {
    sranky.write(0); //drzi nahore
    servoState = 0;
    movingServo2 = false;
    active = false;

  }
    digitalWrite(2, LOW); //natvrdo vypnout LED
    digitalWrite(3, LOW);
  }

  if (active) {
    unsigned long elapsed = millis() - timeStart;

    // blikaci svetla kazdou sekundu jedno zhasne a druhe se rozsviti
    if ((elapsed / 1000) % 2 == 0) {
      digitalWrite(2, HIGH);
      digitalWrite(3, LOW);
    } else {
      digitalWrite(2, LOW);
      digitalWrite(3, HIGH);
    }

    // po 15 sekundach (60 deleno zrychleni 4, lze i jiny cas dle libosti) zavory dolu stejnou mechanikou jako nahoru
    if (elapsed >= 15000 && !timerStarted) {
      servoMoveStart = millis();
       movingServo = true;
     timerStarted = true;
      Serial.println("Zavory jdou dolu");
    }
    if (movingServo) {
  int duration = 1000;
  int progress = millis() - servoMoveStart;
  if (progress <= duration) {
    int angle = map(progress, 0, duration, servoState, 90);
    sranky.write(angle);
  } else {
    sranky.write(90);
    servoState = 90;
    movingServo = false;
  }
  }
  }
  digitalWrite(13, (movingServo == true || movingServo2 == true)); //debug led kdyz jede servo
  
}