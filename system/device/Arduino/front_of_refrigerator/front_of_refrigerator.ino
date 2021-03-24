#include <XBee.h>
#include <SoftwareSerial.h>

#define THRESHOLD_OF_COUNT_TIME 100 - 20
#define PIR_PIN 2
#define MAGNETIC_PIN1 4
#define MAGNETIC_PIN2 5
#define XBEE_HIGHTER_ADDRESS 0x0013a200
#define XBEE_LOWER_ADDRESS 0x415411ED

const int BITRATE = 9600;
XBee xbee = XBee();
XBeeAddress64 addr64 = XBeeAddress64(XBEE_HIGHTER_ADDRESS, XBEE_LOWER_ADDRESS);
SoftwareSerial mySerial(7, 8); // RX, TX

boolean flag_of_existing = false;
boolean flag_of_opened_the_door1 = false;
boolean flag_of_opened_the_door2 = false;

uint8_t count_of_time_s;



inline boolean isExisting()
{
  if(digitalRead(PIR_PIN) == HIGH) {
    return true;
  }
  else{
    return false;
  }
}



inline boolean wasExisted()
{
  return flag_of_existing;
}



inline boolean changeFlagOfExisting()
{
  flag_of_existing = !flag_of_existing;
}



inline boolean isOpeningTheDoor1()
{
  if(digitalRead(MAGNETIC_PIN1) == HIGH) {
    return true;
  }
  else{
    return false;
  }
}



inline boolean wasOpenedTheDoor1()
{
  return flag_of_opened_the_door1;
}



inline boolean changeFlagOfTheDoor1()
{
  flag_of_opened_the_door1 = !flag_of_opened_the_door1;
}



inline boolean isOpeningTheDoor2()
{
  if(digitalRead(MAGNETIC_PIN2) == HIGH) {
    return true;
  }
  else{
    return false;
  }
}



inline boolean wasOpenedTheDoor2()
{
  return flag_of_opened_the_door2;
}



inline boolean changeFlagOfTheDoor2()
{
  flag_of_opened_the_door2 = !flag_of_opened_the_door2;
}



void setup()
{
  // put your setup code here, to run once:
  mySerial.begin(BITRATE);
  xbee.setSerial(mySerial);
  pinMode(PIR_PIN, INPUT);
  pinMode(MAGNETIC_PIN1, INPUT_PULLUP);
  pinMode(MAGNETIC_PIN2, INPUT_PULLUP);
  delay(60000);//wait for 60s until the PIR sensor voltage becomes stable
}

void loop() {
  // put your main code here, to run repeatedly:
  //int start_time_m = millis();
  

  boolean is_existing  = isExisting();
  boolean was_existed = wasExisted();
  
  if(is_existing && !was_existed) {
    changeFlagOfExisting();
    count_of_time_s = 0;
    
    char data_json[] = "{'location':\"front_of_refrigerator\", 'target':\"subject\", 'subject':1}";
    ZBTxRequest zbTx = ZBTxRequest(addr64, data_json, strlen(data_json));
    
    xbee.send(zbTx);
    
  }
  else if(is_existing && was_existed) {
    count_of_time_s = 0;
  }
  else if(!is_existing && was_existed) {
    if(count_of_time_s >= THRESHOLD_OF_COUNT_TIME) {
      changeFlagOfExisting();
      count_of_time_s = 0;
      
      char data_json[] = "{'location':\"front_of_refrigerator\", 'target':\"subject\", 'subject':0}";
      ZBTxRequest zbTx = ZBTxRequest(addr64, data_json, strlen(data_json));
      
      xbee.send(zbTx);
      
    }
    else {
      count_of_time_s++;
    }
  }
  

  
  boolean is_opening_the_door1  = isOpeningTheDoor1();
  boolean was_opened_the_door1 = wasOpenedTheDoor1();

  // when someone open the door
  if(is_opening_the_door1 && !was_opened_the_door1) {
    changeFlagOfTheDoor1();
    
    char data_json[] = "{'location':\"front_of_refrigerator\", 'target':\"upper_door_of_refrigerator\", 'upper_door_of_refrigerator':1}";
    ZBTxRequest zbTx = ZBTxRequest(addr64, data_json, strlen(data_json));
    
    xbee.send(zbTx);
  }
  // when someone close the door
  else if(!is_opening_the_door1 && was_opened_the_door1) {
    changeFlagOfTheDoor1();
    
    char data_json[] = "{'location':\"front_of_refrigerator\", 'target':\"upper_door_of_refrigerator\", 'upper_door_of_refrigerator':0}";
    ZBTxRequest zbTx = ZBTxRequest(addr64, data_json, strlen(data_json));   
    
    xbee.send(zbTx);
  }



  boolean is_opening_the_door2  = isOpeningTheDoor2();
  boolean was_opening_the_door2 = wasOpenedTheDoor2();

  // when someone open the door
  if(is_opening_the_door2 && !was_opening_the_door2) {
    changeFlagOfTheDoor2();
    
    char data_json[] = "{'location':\"front_of_refrigerator\", 'target':\"lower_door_of_refrigerator\", 'lower_door_of_refrigerator':1}";
    ZBTxRequest zbTx = ZBTxRequest(addr64, data_json, strlen(data_json));
    
    xbee.send(zbTx);
  }
  // when someone close the door
  else if(!is_opening_the_door2 && was_opening_the_door2) {
    changeFlagOfTheDoor2();
    
    char data_json[] = "{'location':\"front_of_refrigerator\", 'target':\"lower_door_of_refrigerator\", 'lower_door_of_refrigerator':0}";
    ZBTxRequest zbTx = ZBTxRequest(addr64, data_json, strlen(data_json));   
    
    xbee.send(zbTx);
  }



  delay(100);


  
  /*** mesuring run time ***
  int finish_time_m = millis();
  Serial.print(finish_time_m - start_time_m);
  Serial.print(finish_time_m - start_time_m);
  Serial.print(" + ");
  */


   
}
