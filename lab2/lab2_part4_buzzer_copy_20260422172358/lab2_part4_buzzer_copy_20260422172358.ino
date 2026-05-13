//driver header for LEDC functions
#include "driver/ledc.h"
#include <stdio.h>
#include <math.h>
#include "soc/timer_group_reg.h"

//-------TIMER MACROS-------
//based address for timer group 0
#define TIMERG0_BASE 0x6001F000
//marco that controls enables flags for the clock rate, counter, and comparator
#define TIMG0_T0CONFIG ((volatile uint32_t *)(TIMERG0_BASE + 0x0000))
//lower 32 bits of the latched current clock time
#define TIMG0_T0LO ((volatile uint32_t *)(TIMERG0_BASE + 0x0004))
//macro that esnapshots the current value of the counter and buses value to lo
#define TIMG0_T0UPDATE ((volatile uint32_t *)(TIMERG0_BASE + 0x000C))
//a million ticks per second, after dividing 80MHz clock by 80(prescalar value)
//enable the timer to start incrementing every clock tick
#define TIMER_INCREMENT_MODE (1 << 30)
//enable the timer to start incrementing
#define TIMER_ENABLE (1 << 31)
//if 1000000 ticks = 1 second then 250000 is 250ms
#define SOUND_TOGGLE 250000
#define PRESCALAR_VALUE 80
//-------------------------

//defining key macros
#define RESOLUTION 8
//ledcAttach() takes HZ
#define FREQUENCY 1000
//max votlage of the specific pin being used on the esp-32
#define MAX_PIN_VOLTAGE 4095
//read room ambiance
#define PHOTO_R_PIN 1
#define SOUND_PIN 5
#define THRESHOLD 1000
#define SOUND_SEQEUENCE_ARRAY_LENGTH 3

int soundsFrequencies[] = {500,1000,2000};

int currentFreq = 0;

//declare and define last toggle of a LED
uint32_t lastToggleTime = 0;

void setup() {
   //TIMER SETUP
  //declare and define 32 bit config, that will set the TIMG0_TOCONFIG register
  uint32_t config = *TIMG0_T0CONFIG;
  //declare and define 32 bit mask, so clear the prescalar indeces of the config
  uint32_t mask = 0xFFFF;
  //shift the bits to start AT index 13
  mask = mask << 13;
  //clear the bits
  config &= ~(mask);
  //apply new prescalar
  config |= (PRESCALAR_VALUE << 13);
  //flip the enable bit to start clock
  config |= TIMER_ENABLE;
  //flip the enable bit to start the incrementing
  config |= TIMER_INCREMENT_MODE;
  //set the config reg bits repres. prescalar to 80
  *TIMG0_T0CONFIG = config;

  ledcAttach(SOUND_PIN, FREQUENCY, RESOLUTION);
  Serial.begin(115200);

  //snapshot the current time
  *TIMG0_T0UPDATE = 1;
}

void loop() {
  int averageVoltage = analogRead(PHOTO_R_PIN);
   //snapshot the current time
  *TIMG0_T0UPDATE = 1;
  //get the current time after latching
  uint32_t currentTime = *TIMG0_T0LO;
  //if we have hit the threshold
  if(averageVoltage <= THRESHOLD){
    if(currentTime - lastToggleTime >= SOUND_TOGGLE){
      Serial.print("Current Difference: ");
      Serial.println(currentTime - lastToggleTime);
      ledcWriteTone(SOUND_PIN, soundsFrequencies[currentFreq]);
      currentFreq++;
      //reset the sequence
      if(currentFreq >= SOUND_SEQEUENCE_ARRAY_LENGTH){
        currentFreq = 0;
      }
      lastToggleTime = currentTime;
    }
  }else{
    //if we are above the threshold, do not play a sound
    ledcWriteTone(SOUND_PIN, 0);
    //reset the the sequence
    currentFreq = 0;
  }
}
