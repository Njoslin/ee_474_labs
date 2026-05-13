//declaring and defining const int for LED and BUTTON pins to avoid magic numbers
const int LED_PIN = 4;
const int BUTTON_PIN = 6;


void setup()
{
  //assign led_pin to be output
  pinMode(LED_PIN, OUTPUT);
  /*
  B/C each GPIO pin on the ESP has a built in PULL-UP resistor, we must initilaize the pinMode
  to use this built in feature use the INPUT_PULLUP key word. This ensures that when the button is
  not pressed, output is HIGH, and when it is pressed that the output is LOW
  */
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  Serial.begin(9600);
}

void loop()
{
  //if the button is not pressed, set freq to 10
  if(digitalRead(BUTTON_PIN) == HIGH){
    blink(LED_PIN, 100);
    //Serial.printf("FAST\n");
  }else{
    //if it is pressed, set freq to 1
    blink(LED_PIN, 1000);
    //Serial.printf("NOT FAST\n");
  }
}


//helper method that turns an LEDs on and off as at nHZ depending on the timeDelay argument.
void blink(int pin, int timeDelay)
{
  digitalWrite(pin, HIGH);
  delay(timeDelay);
  digitalWrite(pin, LOW);
  delay(timeDelay);
}