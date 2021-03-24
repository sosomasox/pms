#include <XBee.h>
#include <SoftwareSerial.h>

#define THRESHOLD_OF_COUNT_TIME 100 - 20
#define PIR_PIN 2
#define SENSOR_PIN 11
#define XBEE_HIGHTER_ADDRESS 0x0013a200
#define XBEE_LOWER_ADDRESS 0x415411ED

const int BITRATE = 9600;
XBee xbee = XBee();
XBeeAddress64 addr64 = XBeeAddress64(XBEE_HIGHTER_ADDRESS, XBEE_LOWER_ADDRESS);
SoftwareSerial mySerial(7, 8); // RX, TX
boolean flag_of_existing = false;
boolean flag_of_turned_on_the_water_supply = false;
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



inline boolean isTurningOnTheWaterSupply()
{
  if(digitalRead(SENSOR_PIN) == HIGH) {
    return true;
  }
  else{
    return false;
  }
}



inline boolean wasTurnedOnTheWaterSupply()
{
  return flag_of_turned_on_the_water_supply;
}



inline boolean changeFlagOfTheWaterSupply()
{
  flag_of_turned_on_the_water_supply = !flag_of_turned_on_the_water_supply;
}



void setup() {
  // put your setup code here, to run once:
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
    count_of_time_s = 0;
    
    char data_json[] = "{'location':\"wash_room\", 'target':\"subject\", 'subject':1}";
    ZBTxRequest zbTx = ZBTxRequest(addr64, data_json, strlen(data_json));
    
    xbee.send(zbTx);
    
  }
  else if(is_existing && was_existing) {
    count_of_time_s = 0;
  }
  else if(!is_existing && was_existing) {
    if(count_of_time_s >= THRESHOLD_OF_COUNT_TIME) {
      changeFlagOfExisting();
      count_of_time_s = 0;
      
      char data_json[] = "{'location':\"wash_room\", 'target':\"subject\", 'subject':0}";
      ZBTxRequest zbTx = ZBTxRequest(addr64, data_json, strlen(data_json));
      
      xbee.send(zbTx);
      
    }
    else {
      count_of_time_s++;
    }
  }
  

  
  boolean is_turning_on_the_water_supply = isTurningOnTheWaterSupply();
  boolean was_turned_on_the_water_supply = wasTurnedOnTheWaterSupply();

  if(is_turning_on_the_water_supply && !was_turned_on_the_water_supply) {
    changeFlagOfTheWaterSupply();
    
    char data_json[] = "{'location':\"wash_room\", 'target':\"water_supply\", 'water_supply':1}";
    ZBTxRequest zbTx = ZBTxRequest(addr64, data_json, strlen(data_json));
    
    xbee.send(zbTx);
  }
  else if(!is_turning_on_the_water_supply && was_turned_on_the_water_supply) {
    changeFlagOfTheWaterSupply();
    
    char data_json[] = "{'location':\"wash_room\", 'target':\"water_supply\", 'water_supply':0}";
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
