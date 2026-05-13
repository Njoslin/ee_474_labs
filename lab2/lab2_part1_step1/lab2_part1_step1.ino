#include "driver/gpio.h"
#include "soc/io_mux_reg.h"
#include "soc/gpio_reg.h"
#include "soc/gpio_periph.h"

//base address for gpio derived from the manual
#define GPIO_BASE 0x60004000
//pointer to the gpio_enable_reg address
#define GPIO_ENABLE_REG ((volatile uint32_t *)(GPIO_BASE + 0x0020))
//pointer to the gpio_out_reg address
#define GPIO_OUT_REG ((volatile uint32_t *)(GPIO_BASE + 0x0004))
//macro for pin5
#define GPIO_PIN 5

void setup() {
//select the 5th pin on the board and set the function to GPIO
  PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO5_U, PIN_FUNC_GPIO);
  //set the i/o to output, this is essentially pinMode(gpio5, output)
  *GPIO_ENABLE_REG |= (1 << GPIO_PIN);
}

void loop() {
  //set the output to high using XOR 
  *GPIO_OUT_REG  ^= (1 << GPIO_PIN);
  delay(1000);
}


