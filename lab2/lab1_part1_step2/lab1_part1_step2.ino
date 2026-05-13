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
bool leave = true;

void setup() {
  pinMode(GPIO_PIN, OUTPUT);
//select the 5th pin on the board and set the function to GPIO
  PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO5_U, PIN_FUNC_GPIO);
  //set the i/o to output, this is essentially pinMode(gpio5, output)
  *GPIO_ENABLE_REG |= (1 << GPIO_PIN);
  Serial.begin(115200);
}

void loop() {
  unsigned long start = micros();
  while(leave){
    for(int i = 0; i < 1000; i++){
    //turn LED on and off 1000 times
    //*GPIO_OUT_REG ^= (1 >> GPIO_PIN);
    //*GPIO_OUT_REG ^= (1 >> GPIO_PIN);

    digitalWrite(GPIO_PIN, HIGH);
    digitalWrite(GPIO_PIN, LOW);
  }
    unsigned long end = micros();
    unsigned long finalTime = end - start;

    Serial.println("Total time for 1000 iterations of HIGH->LOW: ");
    Serial.print(finalTime);
    Serial.println(" us");
  leave = false;
  }
}


