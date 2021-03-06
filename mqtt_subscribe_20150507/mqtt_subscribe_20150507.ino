#include <NanodeUNIO.h>
#include <NanodeUIP.h>
#include <NanodeMQTT.h>
#include <dht11.h>
#include <string.h>

//#define DEBUG 1

#define mq2Pin A5
#define pirPin 4
#define photoPin A7
#define dht11Pin 3
#define relayPin 5
#define keyPin 6
#define diodaPin 2

#define Light_Time 10   //czas swiecenia sie swiatla w sekundach

//MQTT Vars
NanodeMQTT mqtt(&uip);

//DHT11 Vars
//dht11 DHT11;
//signed int temperatura;
//unsigned int wilgotnosc;

//PIR Vars
boolean motion = false; 

//Photo Vars
int photo = 0;

//Gas vars
int gas = 0;

void message_callback(const char* topic, uint8_t* payload, int payload_length)
{
  int data = 0;
  
  #ifdef DEBUG
  Serial.print(topic);
  Serial.print(" => ");
  Serial.write(payload, payload_length);
  #endif
    
  payload[payload_length] = 0;
  data = atoi((char *) payload);

  switch(atoi(topic))
  {
    case 1010:
    {
      digitalWrite(relayPin,data);
      break;
    }
    case 1020:
    {
      analogWrite(keyPin, map(data,0,100,0,255));
      break;
    }
    default:
    break;
  }
  
  #ifdef DEBUG
  Serial.print(" Zmienna " );
  Serial.print(relay);
  Serial.println();
  #endif
}



void setup() {
  byte macaddr[6];
  NanodeUNIO unio(NANODE_MAC_DEVICE);
  
  #ifdef DEBUG
  Serial.begin(9600);
  #endif
  
  //PIR section
  pinMode(pirPin, INPUT);
  digitalWrite(pirPin, LOW);
  
  #ifdef DEBUG
  Serial.println("MQTT Subscribe test");
  #endif
  
  unio.read(macaddr, NANODE_MAC_ADDRESS, 6);
  uip.init(macaddr);

  // FIXME: use DHCP instead
  uip.set_ip_addr(192, 168, 137, 101);
  uip.set_netmask(255, 255, 255, 0);

  uip.wait_for_link();
  
  #ifdef DEBUG
  Serial.println("Link is up");
  #endif

  // FIXME: resolve using DNS instead
  mqtt.set_server_addr(192, 168, 137, 2);
  mqtt.set_callback(message_callback);
  mqtt.connect();
  
  #ifdef DEBUG
  Serial.println("Connected to MQTT server");
  #endif
  
  mqtt.subscribe("1010");
  mqtt.subscribe("1020");
  
  #ifdef DEBUG
  Serial.println("Subscribed.");
  Serial.println("setup() done");
  #endif
  
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, 0);
  pinMode(diodaPin, OUTPUT);
  digitalWrite(relayPin, 0);
  pinMode(keyPin, OUTPUT);
  analogWrite(keyPin, 0);
}


void loop() {
  uip.poll();
  DHT11Pomiar();
   
  PirPomiar();
  
  PhotoPomiar();
  
  Mq2Pomiar();
  
  #ifdef DEBUG
  Serial.print("Przekaznik: ");
  Serial.println(relay);
  #endif
  
  if(mqtt.connected()){
    #ifdef DEBUG    
    Serial.println("Publishing...");
    #endif
    
    //SendMQTT(temperatura,"1030");
    //SendMQTT(wilgotnosc,"1090");
    SendMQTT(photo,"1070");
    SendMQTT(motion,"1040");
    SendMQTT(gas,"1091");
    SendMQTT(millis()/60000,"1000");
    
    #ifdef DEBUG    
    Serial.println("Published.");
    #endif
    
    digitalWrite(diodaPin,HIGH);
  }
  else
  {    
    digitalWrite(diodaPin,LOW);
    
    static long int PIR_Timer = 0;
    
    if(motion==1) 
    {
      PIR_Timer = millis();
      analogWrite(keyPin, 255);
    }
    else if((millis()-PIR_Timer)/1000 > Light_Time) 
    {
      analogWrite(keyPin, 0);
    }
    
    
    #ifdef DEBUG
    Serial.println("PDALO POLACZENIE Z MQTT !!!!!!!!");
    #endif
    
    mqtt.connect();
    
    #ifdef DEBUG
    Serial.println("Connected to MQTT server");
    #endif
    
    mqtt.subscribe("test");
  }
  //delay(15);
}


void SendMQTT(int data, char* topic){
  char charBuf[4];
  itoa(data, charBuf, 10);
  mqtt.publish(topic, charBuf);
  
  #ifdef DEBUG
  Serial.println(charBuf);
  #endif
}


void Mq2Pomiar(){
  gas = analogRead(mq2Pin);
  //gas = map(gas,0,1024,0,100);
  
  #ifdef DEBUG
  Serial.print("Gaz: ");
  Serial.println(gas);
  #endif
  
  if(gas > 25) 
  {
    #ifdef DEBUG
    Serial.println("UWAGA - WYKRYTO GAZ LUB DYM");
    #endif
  }
}


void PhotoPomiar() {
  photo = analogRead(photoPin);
  //photo = map(photo,0,1024,0,100);
  
  #ifdef DEBUG
  Serial.print("Swiatlo: ");
  Serial.println(photo);
  #endif
}

/*
void DHT11Pomiar(){
  switch (DHT11.read(dht11Pin))
  {
    case DHTLIB_OK: 
                #ifdef DEBUG
                Serial.println("\n");
                Serial.print("DHT 11 Read sensor: ");
                #endif
                
                wilgotnosc = (int)DHT11.humidity;
                temperatura = (int)DHT11.temperature;
                
                #ifdef DEBUG
                Serial.print("Humidity (%): ");
                Serial.println((float)DHT11.humidity, 2);
              
                Serial.print("Temperature (Â°C): ");
                Serial.println((float)DHT11.temperature, 2);
                #endif
                
		//Serial.println("OK"); 
		break;
    case DHTLIB_ERROR_CHECKSUM: 
		//Serial.println("Checksum error"); 
		break;
    case DHTLIB_ERROR_TIMEOUT: 
		//Serial.println("Time out error"); 
		break;
    default: 
		//Serial.println("Unknown error"); 
		break;
  }
}
*/

void PirPomiar (){
   if(digitalRead(pirPin) == HIGH){
        motion = true;
       }
   else if(digitalRead(pirPin) == LOW){       
        motion = false;
       } 
   #ifdef DEBUG
   Serial.print("Ruch: ");
   Serial.println(motion);
   #endif  
}


