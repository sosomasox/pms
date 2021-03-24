#include <XBee.h>
#include <SoftwareSerial.h>

#define THRESHOLD_OF_COUNT_EXISTING_TIME 100 - 20
#define THRESHOLD_OF_COUNT_SLEEPING_TIME 100 - 20
#define PIR_PIN 2
#define PRESS_SENSOR_PIN A5
#define THRESHOLD_OF_FLAME_SENSOR_VALUE 20
#define XBEE_HIGHTER_ADDRESS 0x0013a200
#define XBEE_LOWER_ADDRESS 0x415411ED

const int BITRATE = 9600;
XBee xbee = XBee();
XBeeAddress64 addr64 = XBeeAddress64(XBEE_HIGHTER_ADDRESS, XBEE_LOWER_ADDRESS);
SoftwareSerial mySerial(7, 8); // RX, TX

boolean flag_of_existing = false;
boolean flag_of_sleeping = false;

uint8_t count_of_existing_time_s;
uint8_t count_of_sleeping_time_s;



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



inline boolean isSleeping()
{
  int press_val = 0;
  
  for(int i=0; i<10; i++) {
    press_val += analogRead(PRESS_SENSOR_PIN);
    delay(10);
  }
  
  Serial.println(press_val);
  
  if(press_val <= 10) {
    return true;
  }
  else{
    return false;
  }
}



inline boolean wasSleeped()
{
  return flag_of_sleeping;
}



inline boolean changeFlagOfSleeping()
{
  flag_of_sleeping = !flag_of_sleeping;
}



void setup()
{
  // put your setup code here, to run once:
  Serial.begin(BITRATE);
  mySerial.begin(BITRATE);
  xbee.setSerial(mySerial);
  pinMode(PIR_PIN, INPUT);
  delay(60000);//wait for 60s until the PIR sensor voltage becomes stable
}

void loop() {
  // put your main code here, to run repeatedly:
  //int start_time_m = millis();
  
  boolean is_existing  = isExisting();
  boolean was_existing = wasExisted();
  
  if(is_existing && !was_existing) {
    changeFlagOfExisting();
    count_of_existing_time_s = 0;
    
    char data_json[] = "{'location':\"bed_room\", 'target':\"subject\", 'subject':1}";
    ZBTxRequest zbTx = ZBTxRequest(addr64, data_json, strlen(data_json));
    
    xbee.send(zbTx);
    
  }
  else if(is_existing && was_existing) {
    count_of_existing_time_s = 0;
  }
  else if(!is_existing && was_existing) {
    if(count_of_existing_time_s >= THRESHOLD_OF_COUNT_EXISTING_TIME) {
      changeFlagOfExisting();
      count_of_existing_time_s = 0;
      
      char data_json[] = "{'location':\"bed_room\", 'target':\"subject\", 'subject':0}";
      ZBTxRequest zbTx = ZBTxRequest(addr64, data_json, strlen(data_json));
      
      xbee.send(zbTx);
      
    }
    else {
      count_of_existing_time_s++;
    }
  }
  

  
  boolean is_sleeping = isSleeping();
  boolean was_sleeped = wasSleeped();

  if(is_sleeping && !was_sleeped) {
    changeFlagOfSleeping();
    count_of_sleeping_time_s = 0;
    
    char data_json[] = "{'location':\"bed_room\", 'target':\"bed\", 'bed':1}";
    ZBTxRequest zbTx = ZBTxRequest(addr64, data_json, strlen(data_json));

    Serial.println(data_json);
    
    xbee.send(zbTx);
    
  }
  else if(is_sleeping && was_sleeped) {
    count_of_sleeping_time_s = 0;
  }
  else if(!is_sleeping && was_sleeped) {
    if(count_of_sleeping_time_s >= THRESHOLD_OF_COUNT_SLEEPING_TIME) {
      changeFlagOfSleeping();
      count_of_sleeping_time_s = 0;
      
      char data_json[] = "{'location':\"bed_room\", 'target':\"bed\", 'bed':0}";
      ZBTxRequest zbTx = ZBTxRequest(addr64, data_json, strlen(data_json));

      Serial.println(data_json);
      
      xbee.send(zbTx);
      
    }
    else {
      count_of_sleeping_time_s++;
    }
  }

  
  delay(10);


  /*** mesuring run time ***
  int finish_time_m = millis();
  Serial.print(finish_time_m - start_time_m);
  Serial.print(finish_time_m - start_time_m);
  Serial.print(" + ");
  */


   
}
