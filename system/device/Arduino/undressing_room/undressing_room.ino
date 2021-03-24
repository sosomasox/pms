 #include <XBee.h>
#include <SoftwareSerial.h>

#define THRESHOLD_OF_COUNT_TIME 100 - 60
#define THRESHOLD_OF_CURRENTLY_mA 200
#define PIR_PIN 2
#define CT_SENSOR_PIN A5
#define XBEE_HIGHTER_ADDRESS 0x0013a200
#define XBEE_LOWER_ADDRESS 0x415411ED

const int BITRATE = 9600;
XBee xbee = XBee();
XBeeAddress64 addr64 = XBeeAddress64(XBEE_HIGHTER_ADDRESS, XBEE_LOWER_ADDRESS);
SoftwareSerial mySerial(7, 8); // RX, TX
boolean flag_of_existing = false;
boolean flag_of_turned_on_the_washing_machine = false;
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



inline int getCurrentlymA() {
  int observed_value;
  int max_val = 0;
  int min_val = 32767;

  for(int i=0; i<100; i++) {
     observed_value = analogRead(CT_SENSOR_PIN);

     if(max_val < observed_value) max_val = observed_value;
     if(min_val > observed_value) min_val = observed_value;
  }

  observed_value = max_val;
  Serial.println((int)(observed_value * 103.889627659574));
  return observed_value * 103.889627659574;
}



inline boolean isTurningOnTheWashingMachine()
{
  int count = 0;
  
  for(int i=0; i<10; i++) {
    if(getCurrentlymA() <= THRESHOLD_OF_CURRENTLY_mA) {
      return false;
    }
  }

  return true;
  
}



inline boolean wasTurnedOnTheWashingMachine()
{
  return flag_of_turned_on_the_washing_machine;
}



inline boolean changeFlagOfTheWashingMachine()
{
  flag_of_turned_on_the_washing_machine = !flag_of_turned_on_the_washing_machine;
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
  
  boolean is_existing = isExisting();
  boolean was_existed = wasExisted();
  
  if(is_existing && !was_existed) {
    changeFlagOfExisting();
    count_of_time_s = 0;
    
    char data_json[] = "{'location':\"undressing_room\", 'target':\"subject\", 'subject':1}";
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
      
      char data_json[] = "{'location':\"undressing_room\", 'target':\"subject\", 'subject':0}";
      ZBTxRequest zbTx = ZBTxRequest(addr64, data_json, strlen(data_json));
      
      xbee.send(zbTx);
      
    }
    else {
      count_of_time_s++;
    }
  }
  
  

  boolean is_turning_on_the_washing_machine = isTurningOnTheWashingMachine();
  boolean was_turned_on_the_washing_machine = wasTurnedOnTheWashingMachine();

  if(is_turning_on_the_washing_machine && !was_turned_on_the_washing_machine) {
    changeFlagOfTheWashingMachine();
    
    char data_json[] = "{'location':\"undressing_room\", 'target':\"washing_machine\", 'washing_machine':1}";
    ZBTxRequest zbTx = ZBTxRequest(addr64, data_json, strlen(data_json));
    
    Serial.println(data_json);
    
    xbee.send(zbTx);
  }
  else if(!is_turning_on_the_washing_machine && was_turned_on_the_washing_machine) {
    changeFlagOfTheWashingMachine();
    
    char data_json[] = "{'location':\"undressing_room\", 'target':\"washing_machine\", 'washing_machine':0}";
    ZBTxRequest zbTx = ZBTxRequest(addr64, data_json, strlen(data_json));   

    Serial.println(data_json);
    
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
