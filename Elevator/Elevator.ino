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
#define OPEN_THE_DOOR 2

unsigned short stepsPerRevolution = 64;
Stepper myStepper(stepsPerRevolution, 8,10,9,11);
LiquidCrystal_I2C lcd(0x27,16,2);

short registeredFloor;
short arrayFloor[N] = {-1,-1,-1,-1};
short ledFloor[N] = {7,6,5,4};
short executableFloor = -1; // -1 it means not called floors
boolean canWork = false;
boolean landed = false;
boolean waitForNext = true;
short flagMoving = 0;

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
  
  registerFloor();
  writeTurn();
   
  if(waitForNext)
    nextFloor();
 
  if(executableFloor != -1 && canWork)
    moveElevator();
  delay(10);
}

void nextFloor()
{ 
    executableFloor = arrayFloor[0];

    if(executableFloor != -1)
    {
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

  executableFloor = -1;
  turnOffLedOnFloor(errivedOnFloor);
  sendCommandToPanelControl(ELEVATOR_IS_ERIVED);
  flagMoving = 0;
  waitForNext = true;
  landed = true;
}

void turnOffLedOnFloor(short errivedOnFloor)
{
  digitalWrite(ledFloor[errivedOnFloor-1],LOW);
}

void writeTurn()
{
  short localCalledFloorExpected = calledFloor();

  if( localCalledFloorExpected == registeredFloor)
  {
    sendCommandToPanelControl(OPEN_THE_DOOR);
    return;
  }
  
  if(localCalledFloorExpected != 0)
  {
    for (short i = 0; i < 4;i++)
      if (localCalledFloorExpected == arrayFloor[i])
          return;
          
      for (short i = 0; i < 4; i++)
        if (arrayFloor[i] == -1)
        {
          arrayFloor[i] = localCalledFloorExpected;
          digitalWrite(ledFloor[abs(localCalledFloorExpected)-1],HIGH);// Turn on led on floor which called
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

void registerFloor()
{
  unsigned int value = analogRead(FLOOR_SENSORS);
  short bufRegisteredFloor = 0;
  if(value >= 221 && value <= 231) bufRegisteredFloor = 4;
  else if(value >= 300 && value <= 310) bufRegisteredFloor = 3;
  else if(value >= 395 && value <= 405) bufRegisteredFloor = 2;
  else if(value >= 691 && value <= 701) bufRegisteredFloor = 1;
  
  if(bufRegisteredFloor != 0 && bufRegisteredFloor != registeredFloor)
  {
    registeredFloor = bufRegisteredFloor;
    printFloor();
  }
}

short calledFloor()
{
  short value = analogRead(CALL_BUTTONS);

  if(value >= 85 && value <= 95) return 4;
  else if(value >= 333 && value <= 345) return 3;
  else if(value >= 175 && value <= 185) return 3;
  else if(value >= 505 && value <= 515) return 2;
  else if(value >= 402 && value <= 412) return 2;  
  else if(value >= 690 && value <= 705) return 1;
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
  char str[2] = {'0','0'};
  switch(command)
  {
    case ELEVATOR_IS_ERIVED:
      str[0] = 'e';
      str[1] = 'f'; 
        break;
        
    case OPEN_THE_DOOR:
      str[0] = 'e';
      str[1] = 'o'; 
        break;
  }

  Serial.write(str,2);
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
