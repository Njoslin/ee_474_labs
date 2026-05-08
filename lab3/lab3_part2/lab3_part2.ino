#include <stdio.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "driver/ledc.h"


//scheduler will check task states and decide what task should run next (1 tick = 1 ms)
#define TICK_PERIOD 1

//task states
#define RUNNING 0
#define READY 1
#define HALT 2
#define SLEEPING 3

//number of functions
#define N_TASKS 5

#define LED_PIN 1
#define BUZZER_PIN 5

//period it 1/f => 1/8 = 0.125s 125ms
//this means peak to peak LED turns on every 125, but to achieve this rate, we must 
//turn on every 63ms and off every 63 roughly (62.5)
#define LED_FREQ 63


uint8_t IC2_BUS_ADDRESS = 0x27;
LiquidCrystal_I2C lcd(IC2_BUS_ADDRESS, 16, 2);

// Frequencies for: A4, B4, C5, D5, E5, F5, G5, A5, B5, C6
int soundsFrequencies[] = {440, 494, 523, 587, 659, 698, 784, 880, 988, 1047};

int priorities[4][3] = {
    {2,5,2}, //task A
    {3,4,2}, //task B
    {4,2,2}, //task C
    {5,3,2}  //task D
};

char *taskNames[] = {"BlinkLed", "Count", "Buzzer", "Alphabet"};

//each tcb struct is stored in an array, the var keeps track of index relative to the current task
int currentTaskIndex;
//used to break ties. if two task hold the same priority, start at (current task + 1) to avoid starvation
//note the offset is skipped on the very first run of the program.
int offset = 0;
//bool to initiate print Initiating scheme: 1 at the very beginning
bool printStart = true;
//bool to start including offset in selecting which task to run IF priorities are equal
bool started = false;
bool led_state = LOW;
//keeps the updater asleep on the first run of the program to ensure we run the first scheme.
bool firstRun = true;

long unsigned int currentTimePassed;
long unsigned int lastScheduleToggle = 0;
long unsigned int lastLightToggle = 0;
int countOnLCD = 1;

//these vars keep track of how many times we ran a particular task
int taskBPasses = 0;
int taskCPasses = 0;
int taskDPasses = 0;
int currentFreqIndex = 0;
//helper var to keep track of the what scheme we are on, compliments the priorities 2D array.
int taskChangePriorityIndex = 1;
char currentChar = 'A';


//defining a TCB struct
typedef struct{
  //pointer to the current tasks function
  void (*fptr)(void *p);
  //instance storing the current state of the task
  short int state;
  //the delay, how long the task wait till ready
  unsigned int sleepTime; // not needed
  int remainingSleepTime;
  //current tasks priority, the higher the number, the higher the priority
  //ranges from. 1-7
  short int priority;
}TCB;


//storing the TCBs in an array
TCB tasklist[N_TASKS];

/*function prototypes*/
void scheduler();
//sets the state of a task back to ready!
void set_state_ready();
//helper function that switches task state it halt, this means that this
//particular task can no longer run UNLESS it is told it can
void halt_me();
//set the task to sleep for some N time.
void sleep_time();
void updateSleepState();
void toggleLED();
void taskA(void *p);
void taskB(void *p);
void taskC(void *p);
void taskD(void *p);
void toggleLED();


void setup() {
  Serial.begin(115200);
  Wire.begin();
  lcd.init();
  lcd.backlight();

  pinMode(LED_PIN, OUTPUT);

  ledcAttach(BUZZER_PIN, 1000, 8);

  //initializing tasks A-E
  tasklist[0].fptr = taskA;
  tasklist[0].state = READY;
  tasklist[0].sleepTime = 0;
  tasklist[0].priority = 2;
  tasklist[0].remainingSleepTime = 0;

  tasklist[1].fptr = taskB;
  tasklist[1].state = READY;
  tasklist[1].sleepTime = 0;
  tasklist[1].priority = 3;
  tasklist[1].remainingSleepTime = 0;

  tasklist[2].fptr = taskC;
  tasklist[2].state = READY;
  tasklist[2].sleepTime = 0;
  tasklist[2].priority = 4;
  tasklist[2].remainingSleepTime = 0;

  tasklist[3].fptr = taskD;
  tasklist[3].state = READY;
  tasklist[3].sleepTime = 0;
  tasklist[3].priority = 5;
  tasklist[3].remainingSleepTime = 0;

  tasklist[4].fptr = taskE;
  tasklist[4].state = READY;
  tasklist[4].sleepTime = 0;
  tasklist[4].priority = 1;
  tasklist[4].remainingSleepTime = 0;
}


void loop() {
  currentTimePassed = millis();
  // since our tick period is 1ms, this means that we must call the schedular every 1ms! so
  // we must have a way to check the current time with the last time we toggled a check!
  // update task states first before running schedular
  // enforcing the schedular to run every TICK_PERIOD  
  if(currentTimePassed - lastScheduleToggle >= TICK_PERIOD){
    lastScheduleToggle = currentTimePassed;
    updateSleepState();
    scheduler();
  }
}


// the schedular keeps everything in check, its job is to iterate through all the tasks in the 
// data structure, check first who is ready to go, then their priorities and turn on the correct 
// signals. this includes changing the state of tasks, and actaully running them!
void scheduler(){
  // need to keep track of the selected task to run! 
  short int selectedTaskIndex = -1;
  short int highestPriority = -1;

  //round robin, if we have started running tasks, then ensure we get the correct starting 
  //position.
  if(started){
    offset = (currentTaskIndex + 1) % N_TASKS;
  }

  for(int i = 0; i < N_TASKS; i++){
    int index = (i + offset) % N_TASKS;
    //if the task is ready then we want to continue
    if(tasklist[index].state == READY){
      // if the current task index priority is greater than the highest one so far,so
      // save the value of the prio, and save the selectedIndex
      if(tasklist[index].priority > highestPriority){
        highestPriority = tasklist[index].priority;
        selectedTaskIndex = index;
      }
    }
  }
  // but what if no tasks were selected? ie. none were ready? then we need to break.
  // we dont want to try and run a task that is not ready, this could lead to some unexpected 
  // behavior!
  if(selectedTaskIndex == -1){
    return;
  }
  // latch the selected task index to the global currentTaskIndex for later use in other helper 
  // methods
  currentTaskIndex = selectedTaskIndex;

  if(printStart){
    Serial.println("Initiating scheme: 1");
    printStart = false;
  }
  //set the state of the task to running
  tasklist[selectedTaskIndex].state = RUNNING;
  //run the actually task
  tasklist[selectedTaskIndex].fptr(NULL);

  if(tasklist[selectedTaskIndex].state == HALT){
    Serial.print("Completed: ");
    Serial.print(taskNames[selectedTaskIndex]);
    Serial.print(": ");
    Serial.println(tasklist[selectedTaskIndex].priority);
  }

  started = true;
  //check ensures that the task's state IS RUNNING and not intentionally set to halt or else.
  if(tasklist[selectedTaskIndex].state == RUNNING){
    //since the schedular's job is to not detemrine the state of tasks we need a helper function to 
    //change the state back to ready!
    set_state_ready(selectedTaskIndex);
  }
}

void set_state_ready(int index){
  //need some global variable that keeps trach of the current task index!
  if(tasklist[index].state != HALT){
    tasklist[index].state = READY;
  }
}

//set the currentTaskIndex state to halt.
void halt_me(){
  tasklist[currentTaskIndex].state = HALT;
}


void sleep_time(long int ticks){
  //Serial.println("in sleep_time");
  //first set the state of the current task
  tasklist[currentTaskIndex].state = SLEEPING;
  tasklist[currentTaskIndex].sleepTime = ticks;
  tasklist[currentTaskIndex].remainingSleepTime = ticks;
}


void updateSleepState(){
  //Serial.println("in updatesleep");
  for(int i = 0; i < N_TASKS; i++){
    //if the task has waited its full sleep time
    if(tasklist[i].remainingSleepTime <= 0 && tasklist[i].state == SLEEPING){
      tasklist[i].sleepTime = 0;
      set_state_ready(i);
      // only decrement the sleep time if the task state is sleeping.
    }else if(tasklist[i].state == SLEEPING){
      tasklist[i].remainingSleepTime -= TICK_PERIOD;
    }
  }
}
// blink LED task that blinks forever.
void taskA(void *p){
      toggleLED();
      sleep_time(LED_FREQ);
      return;
}
//helper method for TaskA
void toggleLED(){
  led_state = !led_state;
  digitalWrite(LED_PIN, led_state);
}

// count on the lcd task
void taskB(void *p){
  lcd.setCursor(0, 0);

  //if the task has yet to be haulted, then continue
  if(tasklist[currentTaskIndex].state != HALT){
    //after two passes then halt the task
    if(taskBPasses == 2){
      halt_me();
      return;
    }
    //if the count is greater than one, reset the counter and increment its pass
    if(countOnLCD > 10){
      countOnLCD = 1;
      taskBPasses++;
      lcd.setCursor(0, 0);
      lcd.print("                ");
      return;
    }else{
      //else print the number and sleep for two seconds
      lcd.print(countOnLCD);
      countOnLCD++;
      sleep_time(2000);
      //jump do another task till done
      return;
    }
  }
}

// buzzer task
void taskC(void *p){
  lcd.setCursor(0, 1);

  if(tasklist[currentTaskIndex].state != HALT){
    if(taskCPasses == 2){
      ledcWriteTone(BUZZER_PIN, 0);
      halt_me();
      return;
    }

    // if the current index is not out of bounds, continue printing
    if(currentFreqIndex < 10){
      ledcWriteTone(BUZZER_PIN, soundsFrequencies[currentFreqIndex]);
      lcd.print(soundsFrequencies[currentFreqIndex]);
      currentFreqIndex++;
      sleep_time(500);
      return;

    }else{
      lcd.setCursor(0, 1);
      lcd.print("                ");
      taskCPasses += 1;
      currentFreqIndex = 0;
      sleep_time(500);
      return;
    }
  }
}
// alphabet task 
void taskD(void *p){
  if(tasklist[currentTaskIndex].state != HALT){
    if(taskDPasses == 2){
      halt_me();
      return;
    }
    if(currentChar <= 'Z'){
      Serial.println(currentChar);
      currentChar += 1;
      sleep_time(1000);
      return;
    }else{
      Serial.println();
      taskDPasses += 1;
      currentChar = 'A';
      sleep_time(1000);
      return;
    }
  }
}

void taskE(void *p){
  //on the first scheme (very first run of the program) sleep the updater for 53s
  if(firstRun){
    firstRun = false;
    sleep_time(53000);
    return;
  }
  if(taskChangePriorityIndex > 2){
    taskChangePriorityIndex = 0;
  }
  //skip taskE
  for(int i = 0; i < N_TASKS - 1; i++){
    tasklist[i].state = READY;
    tasklist[i].priority = priorities[i][taskChangePriorityIndex];
  }
  //reset all the counters for each task to ensure then can run after changing state to READY
  countOnLCD = 1;
  taskBPasses = 0;
  taskCPasses = 0;
  taskDPasses = 0;
  currentChar = 'A';

  Serial.print("Initiating scheme: ");
  Serial.println(taskChangePriorityIndex + 1);
  taskChangePriorityIndex++;
  sleep_time(53000);
  return;
  }
