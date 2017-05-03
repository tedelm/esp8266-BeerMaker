# esp8266-BeerMaker
ESP8266 (Wemos D1) with Bielmeier or other kind of boiler/mash tun that uses SSR relay

What you need:
1 x Wemos D1 or other ESP8266
1 x 100kOhm NTC (analog temp)
1 x Resistor (sketch uses 4.7kOhm)
1 x SSR Relay

How to connect:

Relay     Arduino
GND ->    GND
INT ->    D5

Analog Temp sensor 100k Ohm
Leg1    ->  GND
Leg2    =>  4.7k resistor -> Vcc 3.3
        =>  A0            

// Add Analog reference: Configures the reference voltage used for analog input
        -> AnalogREF      -> Vcc3.3 // cannot be used on D1 esp
*/


Use:
Arduino IDE 1.6.9
https://github.com/esp8266/Arduino.git
https://github.com/wemos/Arduino_XI.git