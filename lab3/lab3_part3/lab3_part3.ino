#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <BLEUtils.h>
#include <BLEDevice.h>
#include <BLEServer.h>

#define SERVICE_UUID        "7d1b315a-4ed9-4006-b058-5926eca3367c"
#define CHARACTERISTIC_UUID "5bb15597-484e-4b03-8ac2-c49c033bf1e8"

#define BUTTON_PIN 1

volatile bool bleMessageReceived = false;

class MyCallbacks: public BLECharacteristicCallbacks {
   void onWrite(BLECharacteristic *pCharacteristic) {
    bleMessageReceived = true;
   }
};

//LCD setup
uint8_t IC2_BUS_ADDRESS = 0x27;
LiquidCrystal_I2C lcd(IC2_BUS_ADDRESS, 16, 2);


volatile bool buttonPressed = false;
volatile bool counterUpdated = false;

bool messageActive = false;
unsigned long messageStartTime = 0;

volatile int counter = 0;
hw_timer_t *timer = NULL;

void ISR_buttonPress();
void ISR_timer();

void setup() {


 BLEDevice::init("MYESP32YEAH"); // Change this to an unique name
 BLEServer *pServer = BLEDevice::createServer();
 BLEService *pService = pServer->createService(SERVICE_UUID);
 BLECharacteristic *pCharacteristic = pService->createCharacteristic(
                                       CHARACTERISTIC_UUID,
                                       BLECharacteristic::PROPERTY_READ |
                                       BLECharacteristic::PROPERTY_WRITE
                                     );
 pCharacteristic->setCallbacks(new MyCallbacks());
 pService->start();
 BLEAdvertising *pAdvertising = pServer->getAdvertising();
 pAdvertising->start();


  Wire.begin();
  lcd.init();
  lcd.backlight();
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), &ISR_buttonPress, FALLING);
  // 1Mhz -> 1 tick per second
  //setting the speed of the clock ie scaling it
  timer = timerBegin(1000000);
  timerAttachInterrupt(timer, &ISR_timer);
  //start periodic countdown behavior
  timerAlarm(timer, 1000000, true, 0); 
}

void loop() {
  //button pressed interrupt
  if(buttonPressed){
    buttonPressed = false;
    //reset lcd cursor position
    lcd.setCursor(0, 0);
    //clear the lcd screen
    lcd.print("                ");
    lcd.setCursor(0, 0);
    lcd.print("Button Pressed");

    messageActive = true;
    messageStartTime = millis();
  }

  //message received interrupt
  if(bleMessageReceived){
    bleMessageReceived = false;

    lcd.setCursor(0, 0);
    lcd.print("                ");
    lcd.setCursor(0, 0);
    lcd.print("New Message!");

    messageActive = true;
    //get current time to clear screen after two seconds
    messageStartTime = millis();
  }
  //if we updated the counter and the message is not active, then print the counter
  if(counterUpdated && !messageActive){
    counterUpdated = false;
    lcd.setCursor(0, 1);
    lcd.print("                ");
   
    lcd.setCursor(0, 1);
    lcd.print(counter);
  }
  
  //if the message is active, then check to see if two seconds has pasted,
  //if two seconds have passed, then clear the lcd
  if(messageActive){
    if(millis() - messageStartTime >= 2000){
      lcd.setCursor(0, 0);
      lcd.print("                ");

      messageActive = false;
    }
  }
}

void ISR_buttonPress(){
  buttonPressed = true;
}

// IRAM places ISR in the instruction ram so interrupt executrs reliably/quickly
void IRAM_ATTR ISR_timer(){
  counter++;
  counterUpdated = true;
}




