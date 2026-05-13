//driver header for LEDC functions
#include "driver/ledc.h"
#include <stdio.h>
#include <math.h>


//defining key macros
#define RESOLUTION 12
//ledcAttach() takes HZ
#define FREQUENCY 5000
//max votlage of the specific pin being used on the esp-32
#define MAX_PIN_VOLTAGE 4095
//read room ambiance
#define PHOTO_R_PIN 1
#define LED_PIN 5
float duty = 0;

void setup() {
  ledcAttach(LED_PIN, FREQUENCY, RESOLUTION);
  Serial.begin(115200);
}

void loop() {
  //used to get the duty cycle
  int averageVoltage = analogRead(PHOTO_R_PIN);
  int targetValue = MAX_PIN_VOLTAGE-averageVoltage;
  

  
  //truncate to nearest decimal val. ie if it line returns 25.67 we want 25.
  //float dutyCycle = ((float)averageVoltage/(float)MAX_PIN_VOLTAGE);
  //get the levels the LED is on for
  //int duty = (pow(2,RESOLUTION) - 1) * dutyCycle;
  //duty = MAX_PIN_VOLTAGE - averageVoltage;
  
  duty =  duty + (targetValue - duty) * 0.00025;//duty=duty+(max-ave)*0.2
  ledcWrite(LED_PIN, (int)duty);
}
