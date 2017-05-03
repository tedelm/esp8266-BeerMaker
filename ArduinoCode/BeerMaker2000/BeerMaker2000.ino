/*
 * Copyright (c) 2015, Majenko Technologies
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * 
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 * 
 * * Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 * 
 * * Neither the name of Majenko Technologies nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/* Create a WiFi access point and provide a web server on it. */

/*

Relay     Arduino
GND ->    GND
INT ->    D5
VCC ->    VCC 5v

Analog Temp sensor 100k Ohm
Leg1    ->  GND
Leg2    =>  4.7k resistor -> Vcc 3.3
        =>  A0            

// Add Analog reference: Configures the reference voltage used for analog input
        -> AnalogREF      -> Vcc3.3 // cannot be used on D1 esp
*/

#include <ESP8266WiFi.h>
#include <WiFiClient.h> 
#include <ESP8266WebServer.h>

    // which analog pin to connect
#define THERMISTORPIN A0         
// resistance at 25 degrees C
#define THERMISTORNOMINAL 100000      
// temp. for nominal resistance (almost always 25 C)
#define TEMPERATURENOMINAL 25   
// how many samples to take and average, more takes longer
// but is more 'smooth'
#define NUMSAMPLES 25
// The beta coefficient of the thermistor (usually 3000-4000)
#define BCOEFFICIENT 3950
// the value of the 'other' resistor
#define SERIESRESISTOR 4700 

int relay = 5;
int samples[NUMSAMPLES];
float SetTemp;
float SetTempFull;
float SetTempMin;
float SetTempMax;
float steinhart;
uint8_t i;
float average;
boolean stringComplete = false;  // whether the string is complete  
String tempResult;
String theMessage;
String theMessage2;
String theMessage3;
/* Set these to your desired credentials. */
const char *ssid = "BeerMaker2000";
const char *password = "mead1234";
String webPage = "";
String TheHeatIsON = "";
String HandleThisSubmit = "";

ESP8266WebServer server(80);

/* Just a little test message.  Go to http://192.168.4.1 in a web browser
 * connected to this access point to see it.
 */
void handleRoot() {
    CheckTemp(); // Get current temp
                 
         webPage += "<html><head><meta http-equiv='refresh' content='30'><style>.submit {    background-color: #4CAF50;    border: none;    color: white;    padding: 16px 32px;    text-align: center;    text-decoration: none;    display: inline-block;    font-size: 100px;    margin: 4px 2px;  width: 80%;  height: 160px;    -webkit-transition-duration: 0.4s;    transition-duration: 0.6s;    cursor: pointer;}.button2 {    background-color: #008CBA;     color: white;     border: 2px solid #008CBA;}.button2:hover {    background-color: white;    color: #008CBA;}</style></head>";
         webPage += "<body style='font-size: 45px;font-family:'Lucida Sans Unicode';color: #616161;'>";
         webPage += "<center><font style='font-size: 125px;font-family:'Lucida Sans Unicode';color: #616161;'>BeerMaker2000</font><br/><br/>";
         webPage += "Temp (c): <b>" + tempResult + "</b> Target (c):<b>" + SetTemp + "</b>";
         webPage += "<br /><br /><br />";
         webPage += "<form action='http://192.168.4.1/' method='POST'>";
         webPage += "<input type='number' class='submit' name='settemp' value=''>";
         webPage += "<br/>";
         webPage += "<input type='submit' class='submit' value='Submit'>";
         webPage += "</form></br>TheHeatIsON?... " + TheHeatIsON +"</br></br><H1>&#x1f37a;</H1></center></body></html>";          
    
  if (server.args() > 0 ) {
    for ( uint8_t i = 0; i < server.args(); i++ ) {
      if (server.argName(i) == "settemp") {
         // do something here with value from server.arg(i);
         SetTemp = server.arg(i).toInt();
         webPage = "";
         webPage += "<html><head><meta http-equiv='refresh' content='30'><style>.submit {    background-color: #4CAF50;    border: none;    color: white;    padding: 16px 32px;    text-align: center;    text-decoration: none;    display: inline-block;    font-size: 100px;    margin: 4px 2px;  width: 80%;  height: 160px;    -webkit-transition-duration: 0.4s;    transition-duration: 0.6s;    cursor: pointer;}.button2 {    background-color: #008CBA;     color: white;     border: 2px solid #008CBA;}.button2:hover {    background-color: white;    color: #008CBA;}</style></head>";
         webPage += "<body style='font-size: 45px;font-family:'Lucida Sans Unicode';color: #616161;'>";
         webPage += "<center><font style='font-size: 125px;font-family:'Lucida Sans Unicode';color: #616161;'>BeerMaker2000</font><br/><br/>";
         webPage += "Temp (c): <b>" + tempResult + "</b> Target (c):<b>" + SetTemp + "</b>";
         webPage += "<br /><br /><br />";
         webPage += "<form action='http://192.168.4.1/' method='POST'>";
         webPage += "<input type='number' class='submit' name='settemp' value=''>";
         webPage += "<br/>";
         webPage += "<input type='submit' class='submit' value='Submit'>";
         webPage += "</form></br>TheHeatIsON?... " + TheHeatIsON +"</br></br><H1>&#x1f37a;</H1></center></body></html>";        
         server.send(200, "text/html", webPage);
         webPage = "";
      }
   }
  }else{
    server.send(200, "text/html", webPage);
    webPage = "";
  }

  //Run program to set temp
  CheckTemp(); // Get current temp
  
        //If temp over
    if (steinhart > SetTempMax){
        digitalWrite(7,LOW);      
        delay(7000);
        TheHeatIsON = "Nope...";
        //If temp under by much
      }else if (steinhart < SetTempFull) {
        Serial.println("ON");
        digitalWrite(7,HIGH);
        delay(7000);
        digitalWrite(7,LOW);
        delay(500);
        digitalWrite(7,LOW);
        TheHeatIsON = "Yes it is :)";        
        
        //IF temp under by minimum
      }else if (steinhart < SetTempMin){
        Serial.println("ON");
        digitalWrite(7,HIGH);
        delay(1000);
        digitalWrite(7,LOW);
        delay(300);
        digitalWrite(7,LOW);
        TheHeatIsON = "Yes... beer-ly";  
      }   
}


void setup() {
  delay(1000);
  Serial.begin(115200);
  Serial.println();
  Serial.print("Configuring access point...");
  /* You can remove the password parameter if you want the AP to be open. */
  WiFi.softAP(ssid, password);

  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
  server.on("/", handleRoot);
  server.begin();
  Serial.println("HTTP server started");

  pinMode(relay,OUTPUT);
  // connect AREF to 3.3V and use that as VCC, less noisy!
 //analogReference(EXTERNAL); 
}

void loop() {
  server.handleClient();
}

void CheckTemp(){
 
  // take N samples in a row, with a slight delay
  for (i=0; i< NUMSAMPLES; i++) {
   samples[i] = analogRead(THERMISTORPIN);
   delay(10);
  }
 
  // average all the samples out
  average = 0;
  for (i=0; i< NUMSAMPLES; i++) {
     average += samples[i];
  }
  average /= NUMSAMPLES;
  
  // convert the value to resistance
  average = 1023 / average - 1;
  average = SERIESRESISTOR / average;
 

  steinhart = average / THERMISTORNOMINAL;     // (R/Ro)
  steinhart = log(steinhart);                  // ln(R/Ro)
  steinhart /= BCOEFFICIENT;                   // 1/B * ln(R/Ro)
  steinhart += 1.0 / (TEMPERATURENOMINAL + 273.15); // + (1/To)
  steinhart = 1.0 / steinhart;                 // Invert
  steinhart -= 273.15;                         // convert to C
 

  //SetTemp = inputString.toInt();
  SetTempFull = SetTemp - 2;
  SetTempMin = SetTemp - 0.2;  
  SetTempMax = SetTemp + 0.5;
    
  tempResult = String(steinhart);
  theMessage = "Temp: " + tempResult;
  theMessage2 = " Min: " + String(SetTempMin);
  theMessage3 = " Max: " + String(SetTempMax);
  
  }
