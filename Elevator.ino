#include<Stepper.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

#define CALL_BUTTONS A1
#define FLOOR_SENSORS A0
#define DEFAULT_SPEED 300
#define SLOW_SPEED 120
#define N 4
#define CALIBRATION_SENSOR 2
#define MOVING_UP 1
#define MOVING_DOWN -1

///////COMANDS FOR PANEL_CONTROL ///////
#define ELEVATOR_IS_ERIVED 1

unsigned short stepsPerRevolution = 64;
Stepper myStepper(stepsPerRevolution, 8,10,9,11);
LiquidCrystal_I2C lcd(0x27,16,2);

short calledFloorExpected;
short registeredFloor;
short arrayFloor[N] = {-1,-1,-1,-1};
short ledFloor[N] = {7,6,5,4};
short executableFloor = -1; // -1 it means not called floors
boolean canWork = false;
boolean landed = false;
boolean waitForNext = true;
short flagMoving = MOVING_UP;

void setup() {
   Serial.begin(9600);
  pinMode(CALL_BUTTONS,INPUT);
  pinMode(FLOOR_SENSORS,INPUT);
  pinMode(CALIBRATION_SENSOR, INPUT);
  
  for(int i = 0; i < N; i++)
    pinMode(ledFloor[i],OUTPUT);
  
  myStepper.setSpeed(DEFAULT_SPEED);

  lcd.begin();                            
  lcd.backlight();                     
  //printInf();
  calibration();
}
void loop() 
{
  readComFromPanelControl();
  if(!canWork)
  {
    lcd.setCursor(0,1);
    lcd.print("Door is opened!");
    delay(100);
    return;
  }
    
  
 registerFloor();
 writeTurn();
  if(waitForNext)
    nextFloor();
  moveElevator();
  delay(10);  
}

void nextFloor()
{
  SortArray(arrayFloor); 
  
  if(arrayFloor[0] != -1)
  {
    executableFloor = arrayFloor[0];
    swipeArray(arrayFloor);
    landed = false;
    waitForNext = false;
  }
}

void swipeArray(short * arrayFloor){
  for(int i = 0; i < N - 1; i++)
    arrayFloor[i] = arrayFloor[i+1];
  arrayFloor[N-1] = -1;  
}

void SortArray(short * arrayFloor)
{
  if(flagMoving == MOVING_UP)
    return;
  for(int i = 0; i < N; i++)
    for(int j = 0; j < N-1; j++)   
       if(arrayFloor[j] < arrayFloor[j+1])//4,3,2,1...
       {
          short bufferNum = arrayFloor[j];
          arrayFloor[j] = arrayFloor[j+1];
          arrayFloor[j+1] = bufferNum;   
       }
}

void moveElevator()
{

  if(executableFloor == -1)
    return;

    
  if(executableFloor > registeredFloor)
  {
    moveUp();
    flagMoving = MOVING_UP;
  }
  
  if(executableFloor == registeredFloor && !landed)
  {
    if(flagMoving == MOVING_UP)
      moveUpMiddle();
    else if(flagMoving == MOVING_DOWN)
       moveDownMiddle();
       
    ElevatorErrived(executableFloor);    
         
  }

  if(executableFloor < registeredFloor)
  {
    moveDown();
    flagMoving = MOVING_DOWN;
  }
}

void ElevatorErrived(short errivedOnFloor)
{
  landed = true;
  executableFloor = -1;
  waitForNext = true;
  turnOffLedOnFloor(errivedOnFloor);
  sendCommandToPanelControl(ELEVATOR_IS_ERIVED);
}

void turnOffLedOnFloor(short errivedOnFloor)
{
  digitalWrite(ledFloor[errivedOnFloor-1],LOW);
}

void writeTurn()
{
  calledFloorExpected = calledFloor();

 
  if(calledFloorExpected != 0)
  {
    for (short i = 0; i < 4;i++)
      if (calledFloorExpected == arrayFloor[i])
          return;
          
      for (short i = 0; i < 4; i++)
        if (arrayFloor[i] == -1)
        {
          arrayFloor[i] = calledFloorExpected;
          digitalWrite(ledFloor[abs(calledFloorExpected)-1],HIGH);// Turn on led on floor which called
          return;
        }
  }
}
void calibration()
{
  for (int i = 0; i < N; i++)
    digitalWrite(ledFloor[i], HIGH);
  lcd.setCursor(0,0); 
  lcd.print("  CALIBRATION...");
  
  while(!digitalRead(CALIBRATION_SENSOR))
    myStepper.step(-10);
    
    
  for (int i = 0; i < N; i++)
    digitalWrite(ledFloor[i], LOW);
  registeredFloor = 1;  
  lcd.clear();
  lcd.print("Floor:1");
}

void moveUp()
{
  myStepper.step(50);
}

void moveUpMiddle()
{
  myStepper.setSpeed(SLOW_SPEED);
  myStepper.step(900);
  myStepper.setSpeed(DEFAULT_SPEED);
}

void moveDownMiddle()
{
  myStepper.setSpeed(SLOW_SPEED);
  myStepper.step(-650);
  myStepper.setSpeed(DEFAULT_SPEED);
}

void moveDown()
{
  myStepper.step(-50);
}
/*
short onFloor()
{
  unsigned int value = analogRead(FLOOR_SENSORS);
  if(value >= 221 && value <= 231) return 4;
  else if(value >= 300 && value <= 310) return 3;
  else if(value >= 395 && value <= 405) return 2;
  else if(value >= 691 && value <= 701) return 1;
  else return 0;
}
*/

void registerFloor()
{
  unsigned int value = analogRead(FLOOR_SENSORS);
  if(value >= 221 && value <= 231) registeredFloor = 4;
  else if(value >= 300 && value <= 310) registeredFloor = 3;
  else if(value >= 395 && value <= 405) registeredFloor = 2;
  else if(value >= 691 && value <= 701) registeredFloor = 1;
  if(value != 0)
    printFloor();
}

short calledFloor()
{
  unsigned int value = analogRead(CALL_BUTTONS);
  if(value >= 85 && value <= 95) return 4;
  else if(value >= 333 && value <= 345) return 3;
  else if(value >= 175 && value <= 185) return 3;
  else if(value >= 505 && value <= 515) return 2;
  else if(value >= 402 && value <= 412) return 2;  
  else if(value >= 695 && value <= 705) return 1;
  else return 0;
  
}

void printFloor()
{
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Floor:");
  lcd.print(registeredFloor);
}

void printInf()
{
  lcd.print("    Elevalor");
  lcd.setCursor(0,1);
  lcd.print("Y.Alieksieiev");
  delay(2000);
  lcd.setCursor(0,1);
  lcd.println("S.Michalewski   ");
  delay(2000);
  lcd.clear();
}

void sendCommandToPanelControl(short command)
{
  switch(command)
  {
    case ELEVATOR_IS_ERIVED:
        char str[2] = {'e','f'};//e - elevator   f - finished
        Serial.write(str,2);
        break;
  }
}

void readComFromPanelControl()
{
  if(!Serial.available())
    return;

    int i = 0;
    char str[2] = {'0','0'};
    delay(100);
    while(Serial.available())
      str[i++] = Serial.read();  

   if(str[0] == 'd' && str[1] == 'o')
    canWork = false;
   else if(str[0] == 'd' && str[1] == 'c')
    canWork = true;
}
