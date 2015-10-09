//Library for HX711
#include "HX711.h"

//Library for GSM
#include "SIM900.h"
#include <SoftwareSerial.h>
#include "inetGSM.h"

//Library for AM2315
#include <Wire.h>
#include <Adafruit_AM2315.h>

// AM2315 (Temperature and Humidity)
Adafruit_AM2315 am2315;

// ThingSpeak Settings
char thingSpeakAddress[] = "cloud.4Bees.at";
String writeAPIKey = "4OCHQWNUWSE0RTDP";

//HX711.DOUT    - pin #A1
//HX711.PD_SCK  - pin #A0

HX711 scale(A1, A0);      //parameter "gain" is ommited; the default value 128 is used by the library

int SIM900_PWR = 46;

//float offset = 0;
//float scalefactor = 1;

float offset = 299.7;
float scalefactor = 28075.5;

const int powerLED = A2;

InetGSM inet;

//Change to Datapoint you are using
String datapoint = "0001_Gewicht";

float floatGewicht = 0;
char charGewicht[10];
String stringGewicht = "";

float floatTemperature = 0;
char charTemperature[10];
String stringTemperature = "";

float floatHumidity = 0;
char charHumidity[10];
String stringHumidity ="";

char path[200];
char msg[100];
char inSerial[50];
int i = 0;


void setup()
{
  //Initialize the digital pin as an output.
  pinMode(SIM900_PWR, OUTPUT);
  
  pinMode(powerLED, OUTPUT);
  
    
  Serial.begin(9600);
  scale.set_scale(scalefactor);                      //this value is obtained by calibrating the scale with known weights; 
                                                 /*   How to Calibrate Your Scale
                                                      1.Call set_scale() with no parameter.
                                                      2.Call set_tare() with no parameter.
                                                      3.Place a known weight on the scale and call get_units(10).
                                                      4.Divide the result in step 3 to your known weight. You should get about the parameter you need to pass to set_scale.
                                                      5.Adjust the parameter in step 4 until you get an accurate reading. 
                                                  */
                                                  
  Serial.println("AM2315 Test!");

  if (! am2315.begin()) {
     Serial.println("Sensor not found, check wiring & pullups!");
     while (1);
  }
   
}

void loop()
{ 
  digitalWrite(powerLED, HIGH);
  powerUp();
  delay(10000);
  
  scale.power_up();
  delay(1000);
  Serial.print("Average weight from 10 messures:\t");
  float floatGewicht = (scale.get_units(10) - offset);
  Serial.println(floatGewicht);
  dtostrf(floatGewicht, 6, 2, charGewicht);
  stringGewicht = charGewicht;
  scale.power_down();
  
  floatTemperature = am2315.readTemperature();
  dtostrf(floatTemperature, 6, 2, charTemperature);
  stringTemperature = charTemperature;
  
  floatHumidity = am2315.readHumidity();
  dtostrf(floatHumidity, 6, 2, charHumidity);
  stringHumidity = charHumidity;
  
  delay(1000);
   
  
  Serial.println("GSM Shield testing.");
  //Start configuration of shield with baudrate.
  //For http uses is recommended to use 9600.
  gsm.begin(9600);
  delay(1000);
  
  //GPRS attach, put in order APN, useername and password.
  //If not neede auth left them blank.
  if (inet.attachGPRS("A1.net", "ppp@A1plus.at", "ppp"))
    Serial.println("status=ATTACHED");
  else 
    Serial.println("status=ERROR APN Connection");
  delay(1000);
  
  //Read IP address.
  gsm.SimpleWriteln("AT+CIFSR");
  delay(5000);
  //Read until serial buffer is empty.
  gsm.WhileSimpleRead();
  
  String pfad = "key="+writeAPIKey+"&field1="+stringGewicht+"&field2="+stringTemperature+"&field3="+stringHumidity;
  pfad.toCharArray(path,200);
  
  //TCP Client GET, send a GET request to the server and save the reply
  inet.httpPOST("cloud.4bees.at", 3000, "/update", path, msg, 100);
  
  //Read for new byte on serial hardware, and write them on NewSoftSerial.
  serialhwread();
  //Read for new byte on NewSoftSerial.
  serialswread();
  
  powerDown();
  
  for (int minuten = 0; minuten < 58; minuten++) {
  delay(60500);
  }
  
};

void powerUp()
 { 
 pinMode(SIM900_PWR, OUTPUT); 
 digitalWrite(SIM900_PWR,LOW);
 delay(1000);
 digitalWrite(SIM900_PWR,HIGH);
 delay(2000);
 digitalWrite(SIM900_PWR,LOW);
 delay(3000);
 }
 
void powerDown()
{
 pinMode(SIM900_PWR, OUTPUT); 
 digitalWrite(SIM900_PWR,LOW);
 delay(1000);
 digitalWrite(SIM900_PWR,HIGH);
 delay(2000);
 digitalWrite(SIM900_PWR,LOW);
 delay(3000);
}



void serialhwread(){
  i=0;
  if (Serial.available() > 0){            
    while (Serial.available() > 0) {
      inSerial[i]=(Serial.read());
      delay(10);
      i++;      
    }
    
    inSerial[i]='\0';
    if(!strcmp(inSerial,"/END")){
      Serial.println("_");
      inSerial[0]=0x1a;
      inSerial[1]='\0';
      gsm.SimpleWriteln(inSerial);
    }
    //Send a saved AT command using serial port.
    if(!strcmp(inSerial,"TEST")){
      Serial.println("SIGNAL QUALITY");
      gsm.SimpleWriteln("AT+CSQ");
    }
    //Read last message saved.
    if(!strcmp(inSerial,"MSG")){
      Serial.println(msg);
    }
    else{
      Serial.println(inSerial);
      gsm.SimpleWriteln(inSerial);
    }    
    inSerial[0]='\0';
  }
}

void serialswread(){
  gsm.SimpleRead();
}

