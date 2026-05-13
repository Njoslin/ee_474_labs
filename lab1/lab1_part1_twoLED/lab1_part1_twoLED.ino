//symbolic constants for better readability and debugging
const int WHITE_LED_PIN = 4;
const int BLUE_LED_PIN = 5;

const int TEN_HZ = 100;
const int ONE_H_HZ = 10;


void setup() {
  //configure a specific pin on the mc to behave as an output.
  pinMode(WHITE_LED_PIN, OUTPUT);
  pinMode(BLUE_LED_PIN, OUTPUT);
  //opens i/o stream between home computer and microcontroller through serial port.
  Serial.begin(9600);
}

void loop() 
{
    //blink blue LEDS
    //10HZ
    blinkLED(WHITE_LED_PIN, ONE_H_HZ);
    //100HZ
    blinkLED(BLUE_LED_PIN, TEN_HZ);
}


void blinkLED(int pinValue, int delayTime)
{
  //used to set a digital pin to HIGH or LOW voltage level. (keywords HIGH or LOW)
  digitalWrite(pinValue, HIGH);
  delay(delayTime);
  digitalWrite(pinValue, LOW);
  //wait 1 sec
  delay(delayTime);
}


