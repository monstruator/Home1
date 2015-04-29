#include <IRremote.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#define ONE_WIRE_BUS 10
#define DHT11_PIN 4
#include "DHT.h"

#define DHTPIN 2     // what pin we're connected to
#define DHTTYPE DHT11   // DHT 11 

DHT dht(DHTPIN, DHTTYPE);
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

int RECV_PIN = 11;

IRrecv irrecv(RECV_PIN);

int Relay1 = 6, rele1=0;
int Relay2 = 7, rele2=0;
decode_results results;

char data[10]={0,0,0};
char index;
int i=0;

unsigned char incomingByte;
//-------------------------------------------------------------------------------

void Temp ()
{
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  
  //Serial.print("HUM ");
  //Serial.print(h, DEC);
  //Serial.print("%   TEMP ");
  //Serial.print(t, DEC);
  //Serial.print("C  ");
  data[0]=t;
  data[2]=h;
 //-----------------------------
  sensors.requestTemperatures(); // Send the command to get temperatures

  //Serial.print("TEMP2 ");
 // Serial.print(sensors.getTempCByIndex(0));
 // Serial.println("C"); 

  data[1]=sensors.getTempCByIndex(0);
 }


void setup()
{
  Serial.begin(115200);
  irrecv.enableIRIn(); // Start the receiver
  pinMode(Relay1, OUTPUT);
  pinMode(Relay2, OUTPUT);
  digitalWrite(Relay1, HIGH);
  digitalWrite(Relay2, HIGH);
  
  DDRC |= _BV(DHT11_PIN);
  PORTC |= _BV(DHT11_PIN);
  sensors.begin();
  dht.begin();
}

void loop() {
  if (irrecv.decode(&results)) {
    //Serial.println(results.value,HEX);
    if (results.value==0xFFB04F) 
    {
      //Serial.println(" 1 ");
      if (rele1==0)   
      {
        digitalWrite(Relay1, LOW);rele1=1;//Serial.println(" 1 ");
      }
      else {digitalWrite(Relay1, HIGH);rele1=0;}
    }    
    if (results.value==0xFFF807) 
    {
      //Serial.println(" 2 ");
     if (rele2==0) 
     {
       digitalWrite(Relay2, LOW);rele2=1;//Serial.println(" 2 ");
     }
        else {digitalWrite(Relay2, HIGH);rele2=0;}
    }    
    //if (results.value==0xFFF807) Serial.println(" 2 ");
    

    //Serial.print(results.value, HEX);
    //Serial.print("  ");
    //Serial.println(results.decode_type);
    irrecv.resume(); // Receive the next value
  }
  
  if (Serial.available() > 0)   
  {
     incomingByte = Serial.read(); // read the incoming byte:
     if (incomingByte >> 4 == 0x0F)     index=incomingByte&0x0F;
     else data[index]=incomingByte;
     if ((data[5]== 1)&&(rele1==0))   
          {digitalWrite(Relay1, LOW);rele1=1;}
     if ((data[5]!= 1)&&(rele1==1))   
          {digitalWrite(Relay1, HIGH);rele1=0;}
     //if (data[5]== 1)   
       //   {digitalWrite(Relay1, LOW);rele1=1;}
     //if (data[5]!= 1)   
       //   {digitalWrite(Relay1, HIGH);rele1=1;}
     
     //if (incomingByte>>6 == 0x02)     data[2]=incomingByte&0x3F;
     //data[1]=incomingByte;
     //data[2]=incomingByte>>4;
     //Serial.write(data,10);
  }
  //Serial.write(data,10);
  delay(200); 
  //data[0]=data[0]+1;  
  i=i+1;
  //Serial.write(data,10);
  // for (int j=0;j<10;j++) //{Serial.write(&data[j],1); delay(1);}
  //if (i>2) 
  {
    data[9]=0;
    data[8]=i;
    for (int i1=0;i1<9;i1++) data[9]+=data[i1];
    Temp();
    Serial.write(data,10);
  }
  
}
