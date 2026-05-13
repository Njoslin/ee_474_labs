#include "driver/gpio.h"
#include "soc/io_mux_reg.h"
#include "soc/gpio_reg.h"
#include "soc/gpio_periph.h"
#include "soc/timer_group_reg.h"


//base address for gpio derived from the manual
#define GPIO_BASE 0x60004000
//pointer to the gpio_enable_reg address
#define GPIO_ENABLE_REG ((volatile uint32_t *)(GPIO_BASE + 0x0020))
//pointer to the gpio_out_reg address
#define GPIO_OUT_REG ((volatile uint32_t *)(GPIO_BASE + 0x0004))
//based address for timer group 0
#define TIMERG0_BASE 0x6001F000
//marco that controls enables flags for the clock rate, counter, and comparator
#define TIMG0_T0CONFIG ((volatile uint32_t *)(TIMERG0_BASE + 0x0000))
//lower 32 bits of the latched current clock time
#define TIMG0_T0LO ((volatile uint32_t *)(TIMERG0_BASE + 0x0004))
//macro that esnapshots the current value of the counter and buses value to lo
#define TIMG0_T0UPDATE ((volatile uint32_t *)(TIMERG0_BASE + 0x000C))
//a million ticks per second, after dividing 80MHz clock by 80(prescalar value)
#define TIMER_INCREMENT_MODE (1 << 30)
//enable the timer to start incrementing
#define TIMER_ENABLE (1 << 31)
#define LED_TOGGLE 1000000
#define PRESCALAR_VALUE 80
#define GPIO_PIN 5


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

  //GPIO PIN SETUP
  //select the 5th pin on the board and set the function to GPIO
  PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO5_U, PIN_FUNC_GPIO);
  //set the i/o to output, this is essentially pinMode(gpio5, output)
  *GPIO_ENABLE_REG |= (1 << GPIO_PIN);
  //snapshot the current time
  *TIMG0_T0UPDATE = 1;
}

void loop() {
  //snapshot the current time
  *TIMG0_T0UPDATE = 1;
  //get the current time after latching
  uint32_t currentTime = *TIMG0_T0LO;
  //if the amount of ticks paddes it greater than or equal to 100000 (number if ticks in 1 sec)
  //then we want to toggle the LED on/off
  if((currentTime - lastToggleTime >= LED_TOGGLE)){
    //turn the LED on/off
    *GPIO_OUT_REG ^= (1 << GPIO_PIN);
    lastToggleTime = currentTime;
  }
}
