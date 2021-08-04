# PizzaOvenThermometer

I have a wood fire pizza oven and wanted to create a sensor that I can use to monitor the temperature inside of the oven while I am not in front of it. So I have acquired some boards on Amazon and put together this example.

From a HW perspective I have used the following boards:

Comidox 1PCS MAX31865 PT100/PT1000 RTD Temperature Thermocouple Sensor Amplifier Board Temperature Detector Module for Arduino
https://www.amazon.com/Comidox-MAX31865-Temperature-Thermocouple-Amplifier/dp/B07PDFNZYX

Waterproof RTD PT100 Temperature Sensor - Jaybva Stainless Steel 3 Wire Thermistor Probe for PID Temperature Controller NPT 1/4" Inch Pipe Thread with Insulation Lead Shield Wire -50~500℃ with 2m Wire
 https://www.amazon.com/dp/B07DP3LYPX/ref=cm_sw_em_r_mt_dp_G2APBZG0VBZGNT9B7DST?_encoding=UTF8&psc=1

ThunderBoard BG22:
https://www.silabs.com/development-tools/thunderboard/thunderboard-bg22-kit

This is a PTD100 connected to MAX31685 and the EFR32BG22.

I have ported the CPP Arduino Library developed by 2015 Ole Wolf <wolf@blazingangles.com> to the EFR32BG22 from Silicon Labs and have used the Health Thermometer example as a starting point.

https://github.com/hallard/arduino-max31865/

Hardware connections:

*    ThunderBoard BG22     -->  MAX31865
*    ------------------------------------
*    CS: pin 10            -->  CS
*    MOSI: pin 4           -->  SDI
*    MISO: pin 6           -->  SDO
*    SCK: pin 8            -->  SCLK



Software:

Gecko SDK Suite : Bluetooth 3.2.0
GNU ARM C Compiler 10.2.1
