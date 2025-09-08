#include <Servo.h>

Servo doorServo;

//pin setup
const int unlockPin = 3;
const int lockPin = 4;
const int redPin = 5;
const int greenPin = 6;
const int servoPin = 9;
const int irSensorPin = 10;
const int potPin = A0;

//admin functions setup
int timeOpening = 3000; // open door in 3secs
int lockPos = 180;
int unlockPos = 90;
int doorSpeed = 10; //speed for the door

//variables setup
bool isDoorLocked = true;
bool emergencyLock = false;
bool humanDetected = false;
bool adminLoggedIn = false;

unsigned long lastLockPress = 0;//time for last lock button to press
unsigned long lastDebounce = 0;
const unsigned long blinkInterval = 500; //blink in 2Hz
const unsigned long lockPressInterval = 2000; //time to count press in millisecs
const unsigned long buttonDelay = 50;// button press within 500ms
int lockButtonPressCount = 0;

int state = 1;// starts in state 1

const String adminPassword = "24670491";

//helper functions for color settings
void setColor(int red, int green){
  analogWrite(redPin, red);
  analogWrite(greenPin, green);
}

void adjustBrightness(int r, int g){
  int potValue = analogRead(potPin);
  int brightness = map(potValue, 0, 1023, 0, 255);

  analogWrite(redPin, (r * brightness) / 255);
  analogWrite(greenPin, (g * brightness) / 255);
}

void displayYellow(){
  setColor(255,255);//maximum of red and green brightness to form yellow
  adjustBrightness(255,255);
}

void greenSolid(){
  digitalWrite(greenPin,HIGH);
  digitalWrite(redPin,LOW);
}

void redSolid(){
  digitalWrite(greenPin,LOW);
  digitalWrite(redPin,HIGH);
}

void blinkRed(){
  setColor(255,0);
  adjustBrightness(255,0);

  for (int i = 0; i < 3; i++){
    digitalWrite(redPin,HIGH);
    delay(500);
    digitalWrite(redPin,LOW);
    delay(500);
  }
}

void flashYellow(){
  displayYellow();
  adjustBrightness(255,255);

  for (int i = 0; i < 3; i++){
    digitalWrite(redPin,HIGH);
    digitalWrite(greenPin,HIGH);
    delay(500);
    digitalWrite(redPin,LOW);
    digitalWrite(greenPin,LOW);
    delay(500);

  Serial.println("Yellow LED flashing");   
  } 
}

void checkButtons(){
  bool unlockButtonState = digitalRead(unlockPin) == LOW;
  bool lockButtonState = digitalRead(lockPin) ==LOW;
 
  if (unlockButtonState){
    isDoorLocked = false;
     state = 1;//unlocked with no detection
    Serial.println("THE SYSTEM IS UNLOCKED");
  }
  if (lockButtonState){
    isDoorLocked = true;
    state = 2;
    Serial.println("THE SYSTEM IS LOCKED");
  }
  // if (lockButtonState == HIGH){
  //   unsigned long currentTime = millis();

  //   if (currentTime - lastLockPress < buttonDelay){
  //     lockButtonPressCount++;
  //   }else{
  //     lockButtonPressCount = 1; //reset count if time of pressing has excced 500ms
  //   }
  //   lastLockPress = currentTime;

  //   if (lockButtonPressCount = 3){
  //     emergencyLock = true;
  //     Serial.println("Emergency lock has been activated");
  //     doorServo.write(lockPos);
  //     lockButtonPressCount = 0;
  //   }else{
  //     if (!isDoorLocked){
  //       isDoorLocked = true;
  //       Serial.println("THE SYSTEM IS LOCKED");
  //       doorServo.write(lockPos);
  //     }
  //   }
  // }else{
  //   lockButtonPressCount = 0;// reset the lock button if it hasn't been pressed
  //}
}

void setup(){
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(unlockPin, INPUT_PULLUP);
  pinMode(lockPin, INPUT_PULLUP);
  pinMode(irSensorPin, INPUT);

  doorServo.attach(servoPin);
  doorServo.write(lockPos);

  Serial.begin(9600);
}

void loop(){
  checkButtons(); // check for unlock and lock button press
  adminMenu();    // check if admin menu can be used

  humanDetected = digitalRead(irSensorPin) == HIGH;

  switch (state){
    case 1://system unlock,no detection
      greenSolid();
      // setColor(0,255);
      // adjustBrightness(0,255);      
      moveServo(lockPos);//close the door 
      Serial.println("System unlocked,no human detected");

      if (humanDetected){
        state = 3; //transition to unlock with human detected
      }      
      break;
      
    case 2://system locked but no detection
      redSolid();
      // setColor(255,0);
      // adjustBrightness(255,0);
      moveServo(lockPos);
      Serial.println("System locked,no human detected");  

      if (humanDetected){
        state = 5;
      } 
      break;
    
    case 3://system unlock and human detected
      blinkRed();
      // setColor(0,255);
      //adjustBrightness(0,255);
      moveServo(unlockPos);
      delay(timeOpening);
      Serial.println("System unlocked,human detected. Door Opening & Closing in 3seconds");
      state = 1;//return back to unlock but no human detected      
      break;
       
    case 5://system locked but human detected
      flashYellow();
      moveServo(lockPos);
      Serial.println("System locked, human detected");      
      break;

    case 6://emergency lock
      blinkRed();
      moveServo(lockPos);
      Serial.println("Emergency lock activated");
      break;

    default://handling unknown state
      setColor(0,0);
      Serial.println("Unknown state has been encountered, resetting to lock state");
      moveServo(lockPos);
      state = 2;
      break;
  }
  adjustBrightness(255,255);
}
//   if (!emergencyLock) {  // STATE 6: emergency lock
//     doorServo.write(lockPos); // close door
//     blinkRed();
//     setColor(255, 0); // solid red LED
//     adjustBrightness(255,0);
//   } else if (isDoorLocked) {
//     if (humanDetected == LOW) {  // STATE 5: lock but human detected, yellow blinks
//       flashYellowState();
//       setColor(255, 255);
//       adjustBrightness(255, 255);
//     } else {  // STATE 2: system is locked, door closed, solid RED if human detected
//       setColor(255, 0);
//       adjustBrightness(255, 0);
//       doorServo.write(lockPos);
//     }
//   } else if (!isDoorLocked) {
//     if (humanDetected == LOW) {  // STATE 3: door open, red blinks when IR sensor detects human
//       setColor(0, 255);
//       adjustBrightness(0, 255);
//       doorServo.write(unlockPos);
//       openAndBlinkState();
//     } else {  // STATE 1: No detection, door closes with green Solid
//       setColor(0, 255);
//       adjustBrightness(0, 255);
//       doorServo.write(lockPos);  // close the door
//       delay(timeOpening);
//     }
//   }
// }

void adminMenu() {
  String password = "";
  if (Serial.available() > 0) {
    password = Serial.readString();
    password.trim();
    Serial.println(password);
  }
  if (password == adminPassword) {
    adminLoggedIn = true;
    Serial.println("Password correct");
    showMenu();
  } else {
    Serial.println("Incorrect password");
  }
}

void showMenu(){
  while (adminLoggedIn) {
    Serial.println("Admin Menu");
    Serial.println("1. Set lock Position");
    Serial.println("2. Set unlock Position");
    Serial.println("3. Set Door Speed");
    Serial.println("4. Set Door Speed Time(for open and close)");
    Serial.println("5. Exit");

    while (Serial.available() == 0) {}
    int option = Serial.parseInt();

    switch(option) {
      case 1:
        Serial.println("Enter Lock Position: ");
        lockPos = Serial.parseInt();
        Serial.print("Lock Position is now set to: ");
        Serial.println(lockPos);
        break;

      case 2:
        Serial.println("Enter Unlock Position:");
        unlockPos = Serial.parseInt();
        Serial.print("Unlock Position set to: ");
        Serial.println(unlockPos);
        break;

      case 3:
        Serial.println("Enter the Door Speed (0-100):");
        doorSpeed = Serial.parseInt();
        if (doorSpeed < 0 || doorSpeed > 100) {
          Serial.println("Invalid num, Door speed should be between 0 to 100");
        } else {
          Serial.print("Door Speed set to: ");
          Serial.println(doorSpeed);
        }
        break;

      case 4:
        Serial.println("Enter the door speed time in millisecs: ");
        timeOpening = Serial.parseInt();
        if (timeOpening < 0 || timeOpening > 10000) {
          Serial.println("Invalid num, Door speed should be between 0 to 5000");
        } else {
          Serial.print("Door speed time is set to: ");
          Serial.println(timeOpening);
        }
        break;

      case 5:
        adminLoggedIn = false;
        Serial.println("Exiting Menu...");
        break;

      default:
        Serial.println("Invalid Option. Please try again.");
        break;
    }
    delay(3000);
  }
}

void moveServo(int targetPos){
  int currentPos = doorServo.read();
  if (currentPos < targetPos){
    for (int pos = currentPos; pos <= targetPos; pos++){
      doorServo.write(pos);
      delay(doorSpeed);
    }
  }
  else{
    for (int pos = currentPos; pos >= targetPos; pos--){
      doorServo.write(pos);
      delay(doorSpeed);
    }
  }
}
