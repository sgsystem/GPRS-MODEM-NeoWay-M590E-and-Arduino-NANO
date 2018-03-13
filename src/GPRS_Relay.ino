// Created by Tsvetomir Gotsov
// 7.3.2018
// Version 0.0.2 //Тествана-> Работи!
#define DEBUG
 
#include <SoftwareSerial.h>

// Закъснение на реле в секунди
const int RELAYTimeoutSecs = 3; // seconds
// Добави телефонен номер както е показано "359 телефонен номер",
const char* phoneNumbers[] = {
  "359884663750"
  };
  
int numberPoolSize = sizeof(phoneNumbers) / sizeof(phoneNumbers[0]);
 
// Pin definitions
#define SRX        2  // Rx--> D2 Свържи към Tx на GPRS
#define STX        3  // Tx--> D3 Свържи към Rx на GPRS
#define RELAY      4  // D4--> Relay
 
SoftwareSerial m590Serial(SRX, STX);
int ch = 0;
String val = "";
String phoneNumber = "";
unsigned int baudrateSelect[] = {
2400,   // 0
4800,   // 1
9600,   // 2
14400,  // 3
19200,  // 4
28800,  // 5
38400,  // 6
57600,  // 7 Maximum speed for Arduino Nano!
115200, // 8 
230400, // 9
460800};// 10
unsigned int baudRateM590S = baudrateSelect[2];  // BaudRate Serial Port GPRS Module
unsigned int baudRateSerial = baudrateSelect[2];  // BaudRate Serial Port Comunication
unsigned long RELAYTimeout = (unsigned long)RELAYTimeoutSecs * 1000;
 
void setup() {
  pinMode(RELAY, OUTPUT);
  digitalWrite(RELAY, 1);
  InitModem();
}
 
void loop() {
      if (m590Serial.available()) {         
        while (m590Serial.available()) {
          ch = m590Serial.read();
          val += char(ch);
          delay(20);
        }
        if (val.indexOf("+PBREADY") > -1)                     //Връща +PBREADY // after the module power-on, startup response
          InitModem();                                        //Инициализира модема
        if (val.indexOf("RING") > -1) {                       //Звънене
          // Проверява номера и ако е от списъка активира релето; Затваря линията!
          if (CheckPhone() == true) {                         // Проверка на номера
            m590Serial.println("ATH0");                       //Прекъсване на линията
#ifdef DEBUG
            Serial.println("--- MASTER CALL DETECTED ---");
#endif
            MasterCall();
          } else {
            // Прекъсва линията за непознати номера!
#ifdef DEBUG
            Serial.println("--- UNKNOWN CALL DETECTED ---");
#endif
            m590Serial.println("ATH0");                       //Прекъсване на линията
          }
          // Ако има RING проверява за SMS от номер в листата
        } else if (val.indexOf("+CMT") > -1) {                // GSM / GPRS модем или мобилен телефон използва + CMT за препращане на новоприето SMS съобщение към компютъра.
          if (CheckPhone() == true) {
#ifdef DEBUG
            Serial.println("--- MASTER SMS DETECTED ---");
#endif
            //MasterSms();
          }
        }// else
        val = "";
      }   //if (m590Serial.available())
      // Serial port Begin
      if (Serial.available()) {
        while (Serial.available()) {
          ch = Serial.read();
          val += char(ch);
          delay(20);
        }
        ConsolePrint(); // Изтрива всички SMS-и
        val = "";
      }
}   //void loop()

 
void InitModem() {
  delay(2000);
  m590Serial.begin(baudRateM590S);
  m590Serial.println("AT+CLIP=1");        // Инциализация изписва и номера !
  delay(100);
  m590Serial.println("AT+CMGF=1");        // Текстов режим
  delay(100);                             // Set text mode
  m590Serial.println("AT+CSCS=\"GSM\"");  //Set Alphabet, Select TE charter string
  delay(100);                             //AT comand need to set when sending text mode
  m590Serial.println("AT+CNMI=2,2");      //Нови SMS съобщения се показват директно, не се запаметяват в SIM картата
  delay(100);                             // Indicating method for new SMS
#ifdef DEBUG
  Serial.begin(baudRateSerial);
  Serial.println("M590 Modem initialized!");  
#endif

}
 
bool CheckPhone() {
  for (int i = 0; i < numberPoolSize; i++) {
    if (val.indexOf(phoneNumbers[i]) > -1) {
      phoneNumber = phoneNumbers[i];
      
#ifdef DEBUG
      Serial.println("Phone number: +" + phoneNumber);
#endif
      return true;
    }
  }
  return false;
}
 
void MasterCall() {
    digitalWrite(RELAY, 0);
    delay(RELAYTimeout);
    digitalWrite(RELAY, 1);
    
#ifdef DEBUG
    Serial.println("Relay switched on  (Call from " + phoneNumber + ").");
#endif
 
}
 
void MasterSms() {
    digitalWrite(RELAY, 0);
    delay(RELAYTimeout);
    digitalWrite(RELAY, 1);
    m590Serial.println("AT+CMGD=0,4");    //ИЗтрива всички SMS
#ifdef DEBUG
    Serial.println("Relay switched on  (SMS from " + phoneNumber + ").");
    Serial.println("Clear: All messages removed");
#endif
}
 
void ConsolePrint() {
  if ((val.indexOf("clear") > -1) or (val.indexOf("Clear") > -1) or (val.indexOf("CLEAR") > -1)) {
    
#ifdef DEBUG
    Serial.println("Clear: All messages removed");
#endif
    m590Serial.println("AT+CMGD=0,4");    //ИЗтрива всички SMS
  }
}
