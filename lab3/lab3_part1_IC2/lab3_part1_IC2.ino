#include <Wire.h>
#include <LiquidCrystal_I2C.h>

//defining macros for clarity. these initialize marcos of 1 byte unsigned integers for masking 
//later
#define RS  ((uint8_t)(1 << 0))
#define RW  ((uint8_t)(1 << 1))
#define EN  ((uint8_t)(1 << 2))
#define BL  ((uint8_t)(1 << 3))

uint8_t IC2_BUS_ADDRESS = 0x27;

LiquidCrystal_I2C lcd(IC2_BUS_ADDRESS, 16, 2);
//function will seperate the high and low nibbles and stream them into 
//another function that will create an 8bit value that inclues the high/low nibble and the 
//corresponding control bits
void sendBytes(uint8_t data, bool isData);
//creates a single 8 bit control line to be written to the SDA -> PCF8574T -> LCD 1-byte register
void setNibbleAndControls(uint8_t data, uint8_t controls);
//write the current command to the lcd (with some pulse to process and complete the process)
void writeBytes(uint8_t nibbleData);
//set the pulse ie. en->1 to en->0 when finishing each
void setPulse(uint8_t nibbleData);


void setup() {
  Serial.begin(115200);
  //leaving the arugments empty ensures that the esp is set to the master
  Wire.begin();
  lcd.init();
  delay(2);
}

void loop() {
  if(Serial.available() > 0){
    String input = Serial.readString();
    //trimming input to avoid weird end chars like \n or \s
    input.trim();
    //clear screen
    sendBytes(0x01, false); 
    //wait
    delay(2);
    //cursor home, ie the first column and row
    sendBytes(0x80, false); 
    //get the length of the current input
    int len = input.length();
    //traverse each char in the string
    for (int i = 0; i < len; i++) {
      //send each char (1 byte) to the data flow. note we want RS to be 1 (for data)
      //because we are now sending data to the PCF -> LCD and not commanding it to do anything
      sendBytes((uint8_t)input[i], true);
    }
  }
}

//helper method initializes the high and low nibbles, and sets the control bits
void sendBytes(uint8_t data, bool isData){
  //turn the backlight on by default, always want it on
  uint8_t control = BL;
  //set to write to the LCD, note that RW = 1 -> read only and RW = 0 -> write only
  control &= ~RW; 
  //get high nibble
  uint8_t highNibble = (data & 0xF0);
  //get low nibble
  uint8_t lowNibble = (data & 0x0F) << 4;
  //if the data is 1 ie. informs us whether. the most sig nibble (bits 4-7) being sent are part of a command
  //or data!
  if(isData){
    control |= RS;
  }

  setNibbleAndControls(highNibble, control);
  setNibbleAndControls(lowNibble, control);
}

//helper method that combines the low and high nibbles (two seperate calls) into on command like ie.
//-> (data/command bits 4-7)(controls bits 0-3)
void setNibbleAndControls(uint8_t nibble, uint8_t controls){
  //combine the high/low nibble with control signals
  uint8_t command = nibble | controls;
  setPulse(command);
}

//helper method that sets the pulse. for each 8 bit command line we must first stage the data onto the PCF,
//then inform the LCD that it should expect data. once the data has been transmitted, we close it and 
//set enable = 0, we store the data in the 8-bit register onboard the LCD. 
void setPulse(uint8_t command){
  //send the byte over the I2C, set the PCF output pins. the data lines now contain nibble with EN set to high (meaning we are processing data)
  writeBytes(command | EN);
  //do the same thing, but set EN to low (the falling edge) and latch the data to the LCD
  writeBytes(command & ~EN);
}

//controls the I2C bit stream. when we begin, it opens the valve for communication. writign the data sends the data
//over the SDA line into the PCF. Once in the PCF, we end end the transmission, setting the output pins in the
//pcf
void writeBytes(uint8_t data){
  Wire.beginTransmission(IC2_BUS_ADDRESS);
  //places the byte into the I2C transmit buffer, prepping to be sent
  Wire.write(data);
  //send the data and clear the buffer
  Wire.endTransmission();
}


