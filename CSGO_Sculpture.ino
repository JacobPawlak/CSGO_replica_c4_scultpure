/*
  CS:GO C4 Sculpture 

  This working sculpture was developed for the Hungtown Airsoft battle that took place on 9/12/2020
  Author: Jacob Pawlak (call: W.D.Bird)
  Date: 09/11/2020
  
  The circuit:

*/

/******* INCLUDES  *******/

#include <Keypad.h>;
#include <Wire.h>;
#include <LiquidCrystal_I2C.h>


/******* CONSTANTS  *******/
// pins used: 2 3 4 5 6 7 8 9 10 11 12 A4(sda) A5(scl) 5v GND

//setting the number of rows and cols for the keypad
const byte keyRows = 4;
const byte keyCols = 3;
const byte lcdRows = 4;
const byte lcdCols = 16;

//making a keypad array to pass to the Keypad generator
char hexaKeys[keyRows][keyCols] = {
  {'1', '2', '3'},
  {'4', '5', '6'},
  {'7', '8', '9'},
  {'*', '0', '#'}
};

//making a byte array for the Keypad generator
byte rowPins[keyRows] = {8, 7, 6, 5}; 
byte colPins[keyCols] = {4, 3, 2};

//making the keypad 
Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, keyRows, keyCols);

//making the LCD screen
LiquidCrystal_I2C lcdScreen(0x27, lcdCols, lcdRows);


//setting pinNums for the lights, buzzer, and input pin
const int redLED = 9;
const int greenLED = 10;
const int buttonSIG = 11;
const int buzzer = 13;

//some useful vars for the loop and setup functions to use
bool armed = false;
int defaultRed = LOW;
int defaultGreen = HIGH;
int defaultButton = LOW;
String password = "7355608"; //change to 7355608 for authenticity
String userInput = ""; // empty string to init for the keypad input

//setting up some vars for the button holding (without dbounce because it doesnt matter here if i keep the arm lenght long enough
byte currSwitch, lastSwitch;
unsigned long timeStart;
unsigned long armedAt = 0;
bool bCheckingSwitch;
//these times are in milliseconds, so 3000 is 3 seconds
int armTime = 3500;
unsigned long bombTimer = 40000;


/******* SETUP ()  *******/


void setup() {

  //setting pins and serial output (for debugging)
  pinMode(redLED, OUTPUT);
  pinMode(greenLED, OUTPUT);
  pinMode(buttonSIG, INPUT_PULLUP);
  pinMode(buzzer, OUTPUT);
  Serial.begin(9600);

  //for the serial LCD
  Wire.begin();
  //setting up the screen
  lcdScreen.init();
  lcdScreen.backlight();
  lcdScreen.clear();
  lcdScreen.setCursor(2,0);
  lcdScreen.print("Reset Mode");
  lcdScreen.setCursor(2,1);
  lcdScreen.print("Hold ARM 4 sec");
  
  // set initial LED states
  digitalWrite(redLED, defaultRed);
  digitalWrite(greenLED, defaultGreen);
  Serial.println(password);

  //get the state of the button (should be low!)
  bCheckingSwitch = false;
  lastSwitch = digitalRead(buttonSIG);

}


/******* LOOP ()  *******/


void loop() {

    //read the state of the input button
    currSwitch = digitalRead( buttonSIG );
    //if we have a difference in state then we need to start count for the arming
    if( currSwitch != lastSwitch )
    {
        if( currSwitch == HIGH )
        {
            //start the counter for the button hold
            timeStart = millis();
            Serial.println("ARMING");
            lcdScreen.setCursor(2,2);
            lcdScreen.print("ARMING");
            bCheckingSwitch = true;
        }
        else
        {
            //at this point the switch has gone from pressed to depressed
            bCheckingSwitch = false;
            lcdScreen.clear();
            lcdScreen.setCursor(2,0);
            lcdScreen.print("Reset Mode");
            lcdScreen.setCursor(2,1);
            lcdScreen.print("Hold ARM 4 sec");
        }

        lastSwitch = currSwitch;       
    }

    //if we are going to look for the counter
    if( bCheckingSwitch )
    {
        //if we have found the button to have been pressed for longer than the armTime, the bomb is armed
        if( (millis() - timeStart ) >= armTime ){
          //arm the bomb
          Serial.println("ARMED");
          lcdScreen.clear();
          lcdScreen.setCursor(2,0);
          lcdScreen.print("C4 ARMED");
          armed = true;
          //grab the time that the bomb was added
          armedAt = millis();
          //turn on the red light and turn off the green light
          digitalWrite(redLED, !defaultRed);
          digitalWrite(greenLED, !defaultGreen);
        }
    }

  //this is where the bomb will have gone over it's timer, and detonated. 
  if(armedAt > 0){

    lcdScreen.setCursor(2,1);
    lcdScreen.print("TTE:" );

    //if we have reached the timer limit, time for the big boom
    if( (millis() - armedAt) >= bombTimer){
      //sound the buzzer to alert the players the bomb has gone off for
      // 10 seconds, then turn the buzzer off, switched to unarmed mode, 
      // reset the password flip the lights back, and reset the armedAt time
      for(int i = 0; i < 15; i++){
        digitalWrite(redLED, !defaultRed);
        digitalWrite(greenLED, defaultGreen);
        delay(30);
        digitalWrite(redLED, defaultRed);
        digitalWrite(greenLED, !defaultGreen);
        delay(30);
        lcdScreen.clear();
        lcdScreen.setCursor(2,0);
        lcdScreen.print("Oh No...");
      }
      lcdScreen.clear();
      lcdScreen.setCursor(2,0);
      lcdScreen.print("Failed to defuse");
      lcdScreen.setCursor(2,1);
      lcdScreen.print("Bomb Detonated");
      tone(buzzer, 550);
      delay(10000); 
      noTone(buzzer);
      delay(100);
      Serial.println("DETONATED");
      userInput = "";
      armed = !armed;
      digitalWrite(redLED, defaultRed);
      digitalWrite(greenLED, defaultGreen);
      armedAt = 0;
    }
  }

  //this is the section we are going to look for the keypad entry to match the password
  if(armed){
    //grab the keypad signal if it's there
    char customKey = customKeypad.getKey();
    //if we found a keypress
    if (customKey){
      //if it is any button other than the enter button (*) add it to the user entry string
      if(customKey != '*'){
        userInput += customKey;
        Serial.println(userInput);
        lcdScreen.setCursor(2,2);
        lcdScreen.print(userInput);
      }
      //if it's the * button, then we check for the password match
      else{
        //if we can match the password, great, disarm the bomb
        if(userInput == password){
          Serial.println("SOLVED");
          lcdScreen.clear();
          lcdScreen.setCursor(2,0);
          lcdScreen.print("Success! You");
          lcdScreen.setCursor(2,1);
          lcdScreen.print("Defused the bomb!");
          userInput = "";
          digitalWrite(redLED, defaultRed);
          digitalWrite(greenLED, defaultGreen);
          //congratulatory buzzer
          for(int i = 0; i < 8; i++){
            tone(buzzer, 1000);
            delay(60);
            noTone(buzzer);
            delay(60);
          }
          //disarm the bomb
          armed = !armed;
          armedAt = 0;
          lcdScreen.clear();
          lcdScreen.setCursor(2,0);
          lcdScreen.print("Reset Mode");
          lcdScreen.setCursor(2,1);
          lcdScreen.print("Hold ARM 4 sec");
        }
        //if we didnt get the match, clear the user entry and make a bad sounding beep
        else{
          Serial.println("INCORRECT");
          lcdScreen.setCursor(2,2);
          lcdScreen.print("INCORRECT");
          userInput = "";
          for(int i = 0; i < 2; i++){
            tone(buzzer, 300);
            delay(100);
            noTone(buzzer);
            delay(100);
          }
        }
      }
    }
  }
}
