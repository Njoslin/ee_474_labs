//define constants for clarity
const int FIRST_BLUE_LED_PIN = 4;
const int SECOND_BLUE_LED_PIN = 5;
//button port
const int YELLOW_BUTTON_1 = 6; //normal button

const int TEN_HZ = 50000;
//change to 10 for demo
const int ONE_HUN_HZ = 5000;

//defining and declaring each LED to correctly turn on or off later
bool blue1_state = LOW;
bool blue2_state = LOW;

//defining and declaring time stamps of each time an LED was last toggled (on or off)
long lastBlue1Toggle  = 0;
long lastBlue2Toggle = 0;
//total time passed from start-up
long currentTimePassed = 0;

void setup() {
  //configure a specific pin on the mc to behave as an output.
  pinMode(FIRST_BLUE_LED_PIN, OUTPUT);
  pinMode(SECOND_BLUE_LED_PIN, OUTPUT);
  pinMode(YELLOW_BUTTON_1, INPUT_PULLUP);

 
  //opens i/o stream between home computer and microcontroller through serial port.
  Serial.begin(9600);
}

void loop()
{
  currentTimePassed = micros();
  //because the esp32 has a built in PULL-UP resistor for each GPIO, when the button is pressed, 
  //it drains to zero (LOW) ,and when it is not pressed, it is HIGH
  if(digitalRead(YELLOW_BUTTON_1) == LOW){
    //switch the freq if the button has been pressed
    switchFreq(ONE_HUN_HZ, TEN_HZ);

  //else do not switch the LED freq.
  }else{
    switchFreq(TEN_HZ, ONE_HUN_HZ); 
  }
}

//helper method that: 1. switches LED freq. based on arguments passed, 2. toggle the LED IF the
//the correct amount of time is past. An LED is toggled depeneding on its freq. For example, at 10HZ
//currentTimePassed - lastBlue1Toggle >= 100ms (or 50ms since the time an LED is on is 50ms and the time 
//it is off is 50ms)
void switchFreq(int freq1, int freq2)
{
  if(currentTimePassed - lastBlue1Toggle >= freq1){
      toggleLED1();
      lastBlue1Toggle = currentTimePassed;
    }
    
  if(currentTimePassed - lastBlue2Toggle >= freq2){
    //Serial.printf("Current blue2_STATE = %d\n", blue2_state);
    toggleLED2();
    lastBlue2Toggle = currentTimePassed;
  }
}

//helper method to turn on/off the LED.
void toggleLED1()
{
  blue1_state = !blue1_state;
  digitalWrite(FIRST_BLUE_LED_PIN, blue1_state);
}

void toggleLED2()
{
  blue2_state = !blue2_state;
  //Serial.printf("Current 2 state: %d\n", blue2_state);
  digitalWrite(SECOND_BLUE_LED_PIN, blue2_state);
}



