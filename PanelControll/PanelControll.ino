#include<Servo.h>
#define SIGNAL_PIN 8
#define BUTTON_SIGNAL A0
#define DOOR_PIN 3

///////COMANDS FOR ELEVATOR//////
#define DOOR_IS_CLOSED 1
#define DOOR_IS_OPENED 2
#define CALL_ON_FLOOR 3

char command[2]={'0','0'};

Servo door;

void setup() {

  pinMode(SIGNAL_PIN,OUTPUT);
  pinMode(BUTTON_SIGNAL,INPUT);
  
  door.attach(DOOR_PIN);
  
  Serial.begin(9600);
  closeDoor();

  for(int i = 0; i < 5; i++)
  {
    digitalWrite(SIGNAL_PIN,HIGH);
    delay(100);
    digitalWrite(SIGNAL_PIN,LOW);
    delay(100);
  }

}

void loop() {
   readSerial(command);
   decodeCommand(command);
}

void decodeCommand(char str[2])
{
  if(str[0] == '0' && str[1] == '0')
    return;
  
  if(str[0] == 'e' && str[1] == 'f')
  {
    elevatorIsArrived();
        str[0] = '0';
        str[1] = '0';
  }
  
}

void elevatorIsArrived()
{
  digitalWrite(SIGNAL_PIN,HIGH);
  delay(500);
  digitalWrite(SIGNAL_PIN,LOW);
  openDoor();
  delay(5000);
  closeDoor();
}

void readSerial(char str[2])
{
  str[0] = '0';
  str[1] = '0';

  if(Serial.available())
  {
    delay(100);
    int i = 0;
    while(Serial.available() && i<2)
      str[i++] = Serial.read();
  }   
}

void sendCommand(short codeOfCommand)
{
  char com[2];
  switch(codeOfCommand)
  {
    case DOOR_IS_CLOSED:
      com[0] = 'd';
      com[1] = 'c';
      Serial.write(com,2);
    break;

    case DOOR_IS_OPENED:
      com[0] = 'd';
      com[1] = 'o';
      Serial.write(com,2);
      break;
  }
}

void openDoor()
{
  door.write(180);
  sendCommand(DOOR_IS_OPENED);

}

void closeDoor()
{
  door.write(0);
  sendCommand(DOOR_IS_CLOSED);
}

short whatFloorCall()
{
  short value = analogRead(BUTTON_SIGNAL);
  
  if(925 <= value && value <= 935) return 1;
  else if(885 <= value && value <= 895) return 2;
  else if(759 <= value && value <= 769) return 3;
  else if(696 <= value && value <= 706) return 4;
  else return -1;
}
