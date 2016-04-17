/*
    This sketch sends data via HTTP GET requests to data.sparkfun.com service.

    You need to get streamId and privateKey at data.sparkfun.com and paste them
    below. Or just customize this script to talk to other HTTP servers.

https://www.pushingbox.com/
http://www.instructables.com/id/Post-to-Google-Docs-with-Arduino/
https://docs.google.com/forms/d/1cRJwf9MiV4jKkmAiYpPgx1Uzce4K-DZQ2v0l0ywWUZY/viewform
https://docs.google.com/forms/d/1cRJwf9MiV4jKkmAiYpPgx1Uzce4K-DZQ2v0l0ywWUZY/formResponse
http://www.electroschematics.com/11291/arduino-dht22-am2302-tutorial-library/
https://productforums.google.com/forum/#!msg/docs/f4hJKF1OQOw/9k2oJ-kLULsJ
*/

#include <ESP8266WiFi.h>
#include <Wire.h>

//// BMP085: Librería para el barometro BMP085
//#include "Barometer.h"

// BMP180: Library to manage the BMP180 barometer
#include <SFE_BMP180.h>

// Variable declaration to manage the WIFI networt
const char* WIFI_ssid     = "Hotspot";
const char* password = "%yatengowifi%";
// Use WiFiClient class to create TCP connections
WiFiClient client;
// Variable Setup
long lastConnectionTime = 0;
int failedCounter = 0;
boolean lastConnected     = false;
boolean uploadTemperature = true;

//const char* host = "data.sparkfun.com";
//const char* streamId   = "....................";
//const char* privateKey = "....................";

// Variable declaration to work with thingspeak
// ThingSpeak Settings
char thingSpeakAddress[] = "api.thingspeak.com";
String writeAPIKey_C = "7J5F3NW8FDLOJDX8";
// Google forms Settings
char Google_form_Address[] = "www.google.es";
char Google_form_key[] = "1cRJwf9MiV4jKkmAiYpPgx1Uzce4K-DZQ2v0l0ywWUZY"; //Replace with your Key
//https://docs.google.com/forms/d/1cRJwf9MiV4jKkmAiYpPgx1Uzce4K-DZQ2v0l0ywWUZY/e
//byte Google_form_Address[] = { 209,85,229,101 }; // Google IP
//byte Google_form_Address[] = { 216,58,209,163 }; // Google IP Alemania

char pushingbox_Address[] = "api.pushingbox.com";
//char pushingbox_ID[] = "vE76BCC4BE2C5A8C";  // THIS IS THE DEVICE ID FROM PUSHINGBOX
char pushingbox_ID[] = "vC6A8513890F3CEF";
char pushingbox_msg[100];


// Time interval in milliseconds to update ThingSpeak (number of seconds * 1000 = interval)
const int updateThingSpeakInterval_C = 16 * 1000;

// BMP180: Variable declaration for the BMP180 barometer.
//Se declara una instancia de la librería
SFE_BMP180 pressure;


typedef struct BMP180_type
{
   char    BMP180_status    = 0; // Whether the sensor give data or not.
   double  BMP180_Pa        = 0; // Read pressure in mBar.
   double  BMP180_H         = 0; // Elevation in m.
   double  BMP180_T         = 0; // Read Temperature ºC.
   double  BMP180_Initial_P = 0; // Reference pressure to calculate elevation
                                 // differences.
};
BMP180_type BMP180_data;


void setup() {
   //Serial.begin(115200);
   Serial.begin(9600);
   delay(10);

   // We start by connecting to a WiFi network
   // Se comienza conectando a la WiFi
   Serial.println();
   Serial.println();
   Serial.print("Connecting to ");
   Serial.println(WIFI_ssid);

   WiFi.begin(WIFI_ssid, password);

   // Repeat until the conexion is established
   while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
   }

   //The Wifi connection has been opened.
   Serial.println("");
   Serial.println("WiFi connected");
   Serial.print("IP address: ");
   Serial.println(WiFi.localIP());

   //Initialize the sensor BMP180
//   Start_BMP180_Sensor();
  pinMode(BUILTIN_LED, OUTPUT); // Onboard LED

}

void Start_BMP180_Sensor() {

   // Sensor initialization sequence.
   if (pressure.begin())
      Serial.println("BMP180 init success");
   else
   {

      Serial.println("BMP180 init fail (disconnected?)\n\n");
      while (1);
   }

   // Temperature initialization.
   BMP180_data.BMP180_status = pressure.startTemperature();
   if (BMP180_data.BMP180_status != 0)  {
      delay(BMP180_data.BMP180_status);
      //Initial Temperature.
      BMP180_data.BMP180_status = pressure.getTemperature(BMP180_data.BMP180_T);
      if (BMP180_data.BMP180_status != 0)    {
        // Pressure initialization.
         BMP180_data.BMP180_status = pressure.startPressure(3);
         if (BMP180_data.BMP180_status != 0)      {
            delay(BMP180_data.BMP180_status);
            //Initial Pressure in the sensor at initialization.
            BMP180_data.BMP180_status =
               pressure.getPressure
                  (BMP180_data.BMP180_Initial_P, BMP180_data.BMP180_T);
         }
         else Serial.println("Error 3. Presure is not initialised.\n");
      }
      else Serial.println("Error 2. Temperature is not available.\n");
   }
   else Serial.println("Error 1. Temperature is not initialised.\n");
/*
   // Write results
   Serial.print(" BMP085 T: ");
   Serial.print(BMP180_data.BMP180_T, 2); //display 2 decimal places
   Serial.print(" C  Pres Inicial: ");
   Serial.print(BMP180_data.BMP180_Initial_P, 0); //whole number only.
   Serial.println();
*/
}

// Subprogram to read the BMP085 barometer: Pressure, Temperature and Elevation
void Read_BMP180( ){
 
   if (BMP180_data.BMP180_status == 0){
      //Barometer sensor status is not available. Initialize the sensor BMP180
      Serial.print("Initialize the sensor BMP180");
      Start_BMP180_Sensor();

   }
   else
   {

      //Se inicia la lectura de temperatura
      BMP180_data.BMP180_status = pressure.startTemperature();
      if (BMP180_data.BMP180_status != 0)
      {
         delay(BMP180_data.BMP180_status);
         //Read Temperature.
         BMP180_data.BMP180_status = pressure.getTemperature(BMP180_data.BMP180_T);
         if (BMP180_data.BMP180_status != 0)
         {
            // Init Pressure read.
            BMP180_data.BMP180_status = pressure.startPressure(3);
            if (BMP180_data.BMP180_status != 0)
            {
               delay(BMP180_data.BMP180_status);

               // The read of the Pressure is affected by the temperature affecting
               // to the sensor readings).
               BMP180_data.BMP180_status =
                  pressure.getPressure(BMP180_data.BMP180_Pa, BMP180_data.BMP180_T);
               if (BMP180_data.BMP180_status != 0)
               {
                  //Elevation difference is calculated related to the initial Pressure.
                  BMP180_data.BMP180_H =
                     pressure.altitude
                        (BMP180_data.BMP180_Pa, BMP180_data.BMP180_Initial_P);
               }
               else Serial.println("Error en la lectura de presion\n");
            }
            else Serial.println("Error iniciando la lectura de presion\n");
         }
         else Serial.println("Error en la lectura de temperatura\n");
      }
      else Serial.println("Error iniciando la lectura de temperatura\n");
   }
}
//void updatepushingbox(String)
//void update_pushingbox(String puData)
void update_pushingbox()
{
   //Connect to wifi
   Serial.println("");
   Serial.println("");
   Serial.print(" Pushingbox: ");
   Serial.print("Connecting to ");
   Serial.print(WIFI_ssid);
   Serial.print(" IP address: ");
   Serial.println(WiFi.localIP());

   if (client.connect(pushingbox_Address, 80))
   {
/* Codigo del get
      Serial.println("connected");
      static char CHAR_BMP180_T[15];
      dtostrf(BMP180_data.BMP180_T,5, 2, CHAR_BMP180_T); // Pasa el double a char.
      sprintf(pushingbox_msg,"GET /pushingbox?devid=%s&status=%s HTTP/1.1",pushingbox_ID,CHAR_BMP180_T);
      client.println(pushingbox_msg);
      client.println("Host: api.pushingbox.com");
      client.println("Connection: close");
      client.println();
      Serial.println(pushingbox_msg);
      Serial.println("Host: api.pushingbox.com");
      Serial.println("Connection: close");
      Serial.println();
*/
//*********************************************
String data="";
char str1[10];   
char str2[10];  
char str3[10];  
char str4[10];  
char str5[10]; 
char str6[10];
char str7[10];
char str8[10];
char str9[10];
char str10[10]; 
      Serial.println("........1..");

  data="";
  data+="";
  data+="&Temperatura=";
      static char CHAR_BMP180_T[10];
      dtostrf(BMP180_data.BMP180_T,5, 2, CHAR_BMP180_T); // Pasa el double a char.
  data+=CHAR_BMP180_T;     
  data+="&Presion=";
      static char CHAR_BMP180_Pa[10];
      dtostrf(BMP180_data.BMP180_Pa,5, 2, CHAR_BMP180_Pa); // Pasa el double a char.
  data+=CHAR_BMP180_Pa;
  data+="&Altitud=";
      static char CHAR_BMP180_H[10];
  dtostrf(BMP180_data.BMP180_H,4,2,CHAR_BMP180_H);
  data+=CHAR_BMP180_H; 

  data+="&status=25";
/*  dtostrf(InHum,4,2,str4);
  data+=str4;

 data+="&BMPTemp=";
  dtostrf(BMPTemp,4,2,str5);
  data+=str5;    
  data+="&BarometricPress=";
  dtostrf(BarometerPress,4,2,str6);
  data+=str6;
 data+="&CrawlTemp=";
  dtostrf(CrawlTemp,4,2,str7);
  data+=str7; 
  data+="&CrawlHum=";
  dtostrf(CrawlHum,4,2,str8);
  data+=str8;

 data+="&OutTemp=";
  dtostrf(OutTemp,4,2,str9);
  data+=str9; 
  data+="&OutHum=";
  dtostrf(OutHum,4,2,str10);
  data+=str10;
*/
  data+= "&&submit=Submit";
  
    Serial.println ("connecting ...");
    Serial.print ("data ...");
    Serial.println (data);
    
  // Send the HTTP PUT request:
    client.print("POST /pushingbox/pushingbox?devid=");
    client.print(pushingbox_ID);
    client.println(" HTTP/1.1");
    client.println("Host: api.pushingbox.com");
    client.println("Content-Type: application/x-www-form-urlencoded");  
    client.println("Connection: close");
    client.print("Content-Length: ");
    client.println(data.length());
    client.println();
    client.print(data);
    client.println();
//    delay(100);
//    client.stop(); 
/*
      Serial.println("connected");
      static char CHAR_BMP180_T[15];
      dtostrf(BMP180_data.BMP180_T,5, 2, CHAR_BMP180_T); // Pasa el double a char.
      sprintf(pushingbox_msg,"GET /pushingbox?devid=%s&status=%s HTTP/1.1",pushingbox_ID,CHAR_BMP180_T);
      client.println(pushingbox_msg);
      client.println("Host: api.pushingbox.com");
      client.println("Connection: close");
      client.println();
      Serial.println(pushingbox_msg);
      Serial.println("Host: api.pushingbox.com");
      Serial.println("Connection: close");
      Serial.println();
*/

//*********************************************
 
      delay(1000);
      lastConnectionTime = millis();
      delay(1000);
      if (client.connected())
      {
         Serial.println("Connecting to Pushingbox ...");
         Serial.println();
         failedCounter = 0;
      }
      else
      {
         failedCounter++;
         Serial.println("Error 1: Connection to Pushingbox failed (" + String(failedCounter, DEC) + ")");
         Serial.println();
      }
      delay(1000);
    }
    else
    {
       failedCounter++;
       Serial.println("Error 2: Connection to Pushingbox Failed (" + String(failedCounter, DEC) + ")");
       Serial.println();
       lastConnectionTime = millis();
    }
    //***************************************************************************
    int timeout = millis() + 5000;
    while (client.available() == 0) {
       if (timeout - millis() < 0) {
          Serial.println(">>> Client Timeout !");
          client.stop();
          return;
       }
    }

    // Read all the lines of the reply from server and print them to Serial
    while (client.available()) {
       String line = client.readStringUntil('\r');
       Serial.print(line);
    }

    Serial.println("closing connection");
    Serial.println();
    Serial.println();
}


//void updateThingSpeak(String tsData)
void updateThingSpeak(String tsData, String writeAPIKey)
{
   //Connect to wifi
   Serial.println("");
   Serial.println("");
   Serial.print(" ThingSpeak: ");
   Serial.print("Connecting to ");
   Serial.print(WIFI_ssid);
   Serial.print(" IP address: ");
   Serial.println(WiFi.localIP());

   if (client.connect(thingSpeakAddress, 80))
   {
      client.print("POST /update HTTP/1.1\n");
      client.print("Host: api.thingspeak.com\n");
      client.print("Connection: close\n");
      client.print("X-THINGSPEAKAPIKEY: " + writeAPIKey + "\n");
      client.print("Content-Type: application/x-www-form-urlencoded\n");
      client.print("Content-Length: ");
      client.print(tsData.length());
      client.print("\n\n");
      client.print(tsData);
      lastConnectionTime = millis();
      delay(1000);
      if (client.connected())
      {
 /*        Serial.println("Connecting to ThingSpeak...");
         Serial.println();
*/
         failedCounter = 0;
      }
      else
      {
         failedCounter++;
         Serial.println("Connection to ThingSpeak failed (" + String(failedCounter, DEC) + ")");
         Serial.println();
      }
      delay(1000);
   }
   else
   {
      failedCounter++;
      Serial.println("Connection to ThingSpeak Failed (" + String(failedCounter, DEC) + ")");
      Serial.println();
      lastConnectionTime = millis();
   }
   //***************************************************************************
   int timeout = millis() + 5000;
   while (client.available() == 0) {
     if (timeout - millis() < 0) {
       Serial.println(">>> Client Timeout !");
       client.stop();
       return;
     }
   }

   // Read all the lines of the reply from server and print them to Serial
   while (client.available()) {
     String line = client.readStringUntil('\r');
//     Serial.print(line);
   }

   Serial.println("closing connection");
   Serial.println();
   Serial.println();

}


void loop() {

//   //Read the BMP085 barometer
//   BMP085_data = Read_BMP085();
//
//   // Write received data
//   Serial.print(" BMP085 T: ");
//   Serial.print(BMP085_data.BMP085_T, 2); //display 2 decimal places
//   Serial.print(" ºC  P: ");
//   Serial.print(BMP085_data.BMP085_Pa, 0); //whole number only.
//   Serial.print(" Pa = ");
//   Serial.print(BMP085_data.BMP085_ATM, 4); //display 4 decimal places
//   Serial.print("Atm.  Altitude: ");
//   Serial.print(BMP085_data.BMP085_H, 2); //display 2 decimal places
//   Serial.println(" m");
//   Serial.println();

   //Read the BMP180 barometer
   Read_BMP180();
   // Write received data
   Serial.print(" BMP180 T: ");
   Serial.print(BMP180_data.BMP180_T, 2); //display 2 decimal places
   Serial.print(" °C  P: ");
   Serial.print(BMP180_data.BMP180_Pa, 0); //whole number only.
   Serial.print(" mBar.  Altitude: ");
   Serial.print(BMP180_data.BMP180_H, 2); //display 2 decimal places
   Serial.println(" m");
//   Serial.println();
   delay(5000);

//******************************************************************************
   // Update ThingSpeak
   if(!client.connected() && (millis() - lastConnectionTime > updateThingSpeakInterval_C))
   {
      if (uploadTemperature == false)
      {
         // Upload the Temperatures.
         updateThingSpeak("field1="+String(BMP180_data.BMP180_T,DEC)
                         +"&field2="+String(BMP180_data.BMP180_Pa,DEC)
                         ,writeAPIKey_C);
  //***********************
  // Prueba para ver si funciona la subida alimentando con un cargador.
  //***********************
  // initialize digital pin BUILD_LED as an output.
  digitalWrite(BUILTIN_LED, LOW);   // turn on LED with voltage LOW
//  Serial.println("");
//  Serial.println("******************");
//  Serial.println("** BLINK LOW *****");
//  Serial.println("******************");
//  Serial.println("");
  delay(1000);                      // wait one second
  digitalWrite(BUILTIN_LED, HIGH);  // turn off LED with voltage HIGH
//  Serial.println("");
//  Serial.println("******************");
//  Serial.println("** BLINK HIGH ****");
//  Serial.println("******************");
//  Serial.println("");
  delay(1000);                      // wait one second
         // Reset the variable.
         uploadTemperature = true;
      }
      else
      {
         // Upload the Monitor data.
        update_pushingbox();   
        // initialize digital pin BUILD_LED as an output.
        digitalWrite(BUILTIN_LED, LOW);   // turn on LED with voltage LOW
        delay(400);                       // wait 300 ms
        digitalWrite(BUILTIN_LED, HIGH);  // turn off LED with voltage HIGH
        delay(300);                       // wait 300 ms
        digitalWrite(BUILTIN_LED, LOW);  // turn on LED with voltage LOW
        delay(400);                       // wait 300 ms
        digitalWrite(BUILTIN_LED, HIGH);  // turn off LED with voltage HIGH
        delay(1000);                      // wait one second
 /*        updateGoogleForms("entry.0.single="+String(BMP180_data.BMP180_T,DEC)
                        +"&entry.2.single="+String(BMP180_data.BMP180_Pa,DEC)
                        +"&submit=Submit");
 */
         // Change variable value.
         uploadTemperature = false;
 
      }
   }
   // Check if Arduino Wifi needs to be restarted
   if (failedCounter > 3 )
   {
      WiFi.begin(WIFI_ssid, password);
   }

   lastConnected = client.connected();
}

  