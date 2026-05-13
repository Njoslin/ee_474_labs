
//declaring and defining MAX to simply stop my loop from running forever.
const int MAX = 100;
int i = 1;

void setup() {
  //configure a specific pin on the mc to behave as an output.
  pinMode(5, OUTPUT);
  Serial.begin(9600);
}

void loop() {

  //used to set a digital pin to HIGH or LOW voltage level. (keywords HIGH or LOW)
  while(i <= MAX){
    //turning the LED on
    digitalWrite(5, HIGH);
    //wait for 1 sec
    delay(1000);
    //turn the LED off
    digitalWrite(5, LOW);
    //wait another sec
    delay(1000);
    //Serial.printf("Value: %d\n", (i));
    ++i;
  }
}
