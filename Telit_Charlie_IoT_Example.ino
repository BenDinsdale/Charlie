/*======================================================================================
Title:            Charlie Board Testing.ino
Date created:     07-02-2021
Author:           Ben Dinsdale
Company:          Sargent Electrical Services Ltd.
Address:          Unit 35, Tokenspire Business Park, Beverley, East Yorkshire, HU17 0TB
Enviroment:       Written in Arduino IDE 1.8.0 with Arduino SAM boards added in boards
                  manager and BlueDot BMA400 library added from the Library Manager.
Target/BOARD:     Arduino MKRZERO (for arduino config)
                  Telit Charlie IoT Evaluation kit REV C, PN:200515
Useful Links:
 * Telit Charlie IoT Board: https://contact.telit.com/charlie-evaluation-kit
 * ME310G1 product page:    https://www.telit.com/me310g1/
 * Accelerometer library:   https://github.com/BlueDot-Arduino/BlueDot_BMA400
 * Eclipse MQTT Broker:     https://mosquitto.org 
//======================================================================================
// LICENSE
/*======================================================================================
Copyright (c) 2021 Sargent Electrical Services Ltd.

Permission is hereby granted, free of charge, to any person obtaining a copy of this 
software and associated documentation files (the "Software"), to deal in the Software 
without restriction, including without limitation the rights to use, copy, modify, merge,
publish, distribute, sublicense, and/or sell copies of the Software, and to permit 
persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, 
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,FITNESS FOR A PARTICULAR 
PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE 
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT
OR OTHERWISE, ARISING FROM,OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.              
//======================================================================================
// VERSION HISTORY
/*======================================================================================
Ver.  Date:       Description  
----------------------------------------------------------------------------------------
0.0.1 07/02/2021  *Toggle user LED with user button
                  *send to USB Serial Terminal window 
                  *Switch modem on and off
0.0.2 08/02/2021  *Send commands from teminal input to modem
                  *recieve from modem and pass to terminal
                  *Implemented sercom 4 to use real USART instead of software serial 
                   this also means that the standard variant file can be used without 
                   the need for modification.
                  *Added TWI for Bosch BMA400 Accelerometer
                  *Added BlueDot BMA400 library & configured for 2g range @ 12.5Hz read
                  *Added terminal send of BMA400 x, y, z data every second    
0.0.3 09/02/2021  *Added MQTT test script to be typed into the terminal monitor
                  *Added GNSS test script to be typed into the terminal monitor       
                  *Added Licence information
1.0.0 10/02/2021  *Intitial release
//======================================================================================
// SOFTWARE DESCRIPTION
//======================================================================================
After setting up the peripherals, it tests to see if Modem is already switched on, if it
isn't on, it will switch it on and check again, reporting the state to the terminal
window.

If the user button is pressed the user LED is on, otherwise its off.

The BMA400 Accelermeter runs in the background and puts is data out to the terminal 
window every second. This can be easly commented out if required at the top of the 
definitions section below.

The ME310G1 Modem is directly linked to the terminal window, entering AT commands in the
Terminal will result in them being sent to the Modem, the modems reponse will then be 
sent back to the terminal.
SETTINGS FOR TERMINAL: Baud: 9600, 8 data bits, No parity, 1 stop bit, add carriage 
return on to the of line on send.

Below is an example test script for sending some data over MQTT using the Eclipse 
Mosquito test server, Also is an example script for seting up and getting GNSS data back
from the modem. 

GNSS Note: The onboard antenna is a little on the weak side and wont get a signal 
indoors (even near a window), a clear open view of the sky is best for testing.
//======================================================================================
// EXAMPLE MQTT Script - to be typed into the terminal window
//======================================================================================
// Connect to Network [SIM required - charlie board takes nano SIM]
AT+CFUN=1                                                                               // ensure modem is on and running with all feature enabled
AT+COPS=2                                                                               // disable network registation
AT+CGDCONT=1,"IP","[APN from SIM provider]"                                             // define a PDP context for data connection using internet protocol
// e.g. AT+CGDCONT=1,"IP","telit123.net"
AT+COPS=0                                                                               // register to network (give it 30 secs or so to find a connection before next step)
AT+COPS?                                                                                // check connection (should have some network data back)
AT#SERVINFO                                                                             // check connection infomation (should have network and mast data back)
// if you don't get a network connection within 2 minutes try rebooting the modem: 
// AT#REBOOT then try again, if that doesn't work try forcing the connection to a known
// Network such as: AT+COPS=4,1,"O2 - UK",8
AT#SGACT=1,1                                                                            // Enable the access technology, connection (should spit back an IP address, may take a few seconds first)
AT+CGPADDR=1                                                                            // IP address can also be read back with this command when connected if SGACT doesn't give it and returns ERROR
//+++++++++++++++++++++++++++++++++
// Setup MQTT
AT#MQEN=1,1                                                                             // enable MQTT features on client instance 1
AT#MQCFG=1,”test.mosquitto.org”,1883,1                                                  // MQTT config 1: instance no, host name/address, port, client no
                                                                                        // unsecured port / connection see test.mosquitto.org for details
AT#MQCFG2=1,3600,1                                                                      // MQTT config 2: instance no, keep alive time (in sec), session type (0 persistent, 1 clean)
AT#MQWCFG=1,1,1,2,system_status,connection_dead                                         // MQTT last will config: instance no, will flag, will retain, will QoS, topic, message 
// Connect to MQTT
AT#MQCONN=1,myClientID,myUser,myPassword                                                // MQTT connect: instance no, client ID, username, password
// Subscribe to a topic
AT#MQSUB=1,myTestTopic                                                                  // MQTT subcribe: instance no, topic
// Publish to that topic
AT#MQPUBS=1,myTestTopic,0,1,test_message                                                // MQTT publish: instance no, topic, retain, QoS, message 
// read the topic back after a RING                                                     // should recive somthing like this #MQRING: <instanceNumber>,<mId>,<topic>,<len>
AT#MQREAD=1,1                                                                           // MQTT read: instance no, Message Id (1 - 30)
                                                                                        // will return <<< \n "read message" 
// Disconnect from MQTT
AT#MQDISC=1                                                                             // disconnect from broker (clean, last will and testiment won't be triggered)
AT#MQEN=1,0                                                                             // disable MQTT on instance 1
//======================================================================================
// EXAMPLE GNSS & GTP Script - to be typed into the terminal window
//======================================================================================
AT$GPSCFG=0,0                                                                           // set WWAN/GNSS priority at start up to GNSS
AT$GPSCFG=1,5                                                                           // set time between fix to 5 seconds
AT$GPSCFG=2,0                                                                           // set GNSS constellation to GPS + Default based on network connection
AT$GPSCFG=3,0                                                                           // set WWAN/GNSS priority runtime
AT#REBOOT                                                                               // reboot the modem for the changes to take affect (give it 20 seconds or so to reboot)
AT                                                                                      // check modem has rebooted should get OK back
AT$GPSP?                                                                                // check if the GNSS modules power is on? should be off after a reboot
AT$GPSNMUN=1,1,0,0,0,0,0                                                                // enable stream, enable GCA packet in stream
AT$GPSP=1                                                                               // switch on the GNSS modules power also enables the LNA out to power the antenna
// should now be getting data back every 5 seconds, 
// when you have had enough, disable the stream.
AT$GPSNMUN=0                                                                            // disable GNSS data stream 
AT$GPSACP                                                                               // you can also use the get last aquired position command with the steam diabled
AT$GPSP=0                                                                               // power down the GNSS module and LNA output

// GTP WWAN uses the network to work out a location, this ties up the modem for 30 to 90
// seconds while getting the position, as below to test:
AT$GPSCFG=3,1                                                                           // set WWAN/GNSS priority runtime to WWAN
AT#GTPENA=1                                                                             // enable the Global Terrestrial Postioning service (must be on network with IP address to work)
AT#GTP                                                                                  // get position from the GTP WWAN service
// returns: Lat, Long, altitude(m), accuracy(m)   
AT#GTPENA=0                                                                             // disable the GTP WWAN service
//======================================================================================
// INCLUDE FILES
//======================================================================================*/
#include <Wire.h>                                                                       // For I2C / TWI marcos and definitions
#include "BlueDot_BMA400.h"                                                             // For BMA400 Accelerometer macros and definitions
BlueDot_BMA400 bma400 = BlueDot_BMA400();                                               
//======================================================================================
// DEFINITIONS
//======================================================================================
//#define BMA400_ENA                                                                    // uncomment to add the BMA400 stream to the terminal output

// Define IO pins
#define userLED 7                                                                       // User LED
#define userButton 6                                                                    // User Button
#define ME310_on_off_wake 32                                                            // define pin for modem on/off/wake line - PB08 - ARD 32

// UART for Modem
#define PIN_SERIAL2_RX  (27ul)                                                          // define pin for uart TX to modem
#define PIN_SERIAL2_TX  (26ul)                                                          // define pin for uart RX to modem
#define PAD_SERIAL2_TX  (UART_TX_PAD_0)                                                 // define sercom pad type for uart TX to modem
#define PAD_SERIAL2_RX  (SERCOM_RX_PAD_1)                                               // define sercom pad type for uart RX to modem

Uart Serial2(&sercom4, PIN_SERIAL2_RX, PIN_SERIAL2_TX, PAD_SERIAL2_RX, PAD_SERIAL2_TX); // set up sercom4 as serial 2 for UART to modem
//======================================================================================
// GLOBAL VARIABLES
//======================================================================================
uint8_t count=10, fail=0, user_button=0, button_cnt=0, second_cnt=0;
String ME310str1;
//======================================================================================
// SUBROUTINES
//======================================================================================
// SETUP
//**************************************************************************************
void setup() 
{               
// UART TO THE TERMINAL WINDOW VIA USB                                                  
Serial.begin(9600);                                                                     // start the USB serial (default is 8 bit, no parity and 1 stop bit)
while(!Serial);                                                                         // wait for USB to start (native)

// UART TO THE MODEM
Serial2.begin(115200);                                                                  // start the modem serial
while(!Serial2);                                                                        // wait for USB to start (native)

// ACCELEROMETER SETUP PERAMETERS
bma400.parameter.I2CAddress = 0x14;                                                     // BMA400 default address is 0x15, alt. address is [0x14]
bma400.parameter.powerMode = 0x02;                                                      // power mode: 0 is sleep mode, 1 is low power mode, 2 is normal mode
bma400.parameter.measurementRange = 0x00;                                               // Set the BMA400 measurement range 0 is 2g, 1 is 4g, 2 is 8g and 3 is 16g
bma400.parameter.outputDataRate = 0x05;                                                 // meaurement frequency (12.5Hz)
bma400.parameter.oversamplingRate = 0x03;                                               // over sampling rate 0 is lowest & lowest power 3 is highest and highest power

// INITIALIZE THE ACCELEROMETER
Serial.print(F("Communication with BMA400: "));                                         // print text to terminal
if (bma400.init()==0x90)Serial.println("Successful");                                   // if connected ok print to terminal 
else Serial.println("Failed");                                                          // if failed prin to terminal
 
// CHECK IF MODEM IS ALREADY ON IF NOT SWITCH ON
Serial2.print("AT\r");                                                                  // Test write, expect OK
delay(250);                                                                             // allow time for response
ME310str1=Serial2.readString();                                                         // read the modem message back
if (ME310str1.indexOf("OK")==-1)                                                        // if sting doesn't contain OK
  {
  Serial.print("Modem is OFF, retry\n");                                                // modem is off
  digitalWrite(ME310_on_off_wake,1);                                                    // switch on the modem 
  delay(5000);                                                                          // switch the line for 5 seconds
  digitalWrite(ME310_on_off_wake,0);                                                    // switch off the modem 
  Serial.print("Modem Should be ON\n");
  delay(2000);                                                                          // a little time for modem to finish booting
  Serial2.print("AT\r");                                                                // Test write, expect OK back
  delay(250);                                                                           // allow some time for a response
  ME310str1=Serial2.readString();                                                       // read the modem message back
  if (ME310str1.indexOf("OK")==-1)                                                      // if sting doesn't contain OK
    {
    Serial.print("Modem is OFF\n");                                                     // modem has failed after retry
    }
  else
    {
    Serial.print("Modem is on\n");                                                      // modem is on after retry
    }
  } 
else
  {
  Serial.print("Modem is on\n");                                                        // modem is on first attempt
  }
}
//======================================================================================
// MAIN PROGRAM
//======================================================================================
void loop()                                                                             // never ending loop
{
//+++++++++++++++++++++++++++++++
// Bi-directional debounce of User Button
if (digitalRead(userButton)!=user_button) button_cnt++;                                 // if button state has changed start counting
if (digitalRead(userButton)==user_button) button_cnt=0;                                 // if button state is the same as button varible reset counter
if (button_cnt>5)user_button = digitalRead(userButton);                                 // make states match as 25ms+ has passed
// User LED state based on User button state
if (!user_button) digitalWrite(userLED,1);                                              // if user button presed, LED on
else digitalWrite(userLED,0);                                                           // if user button not pressed, LED off
//+++++++++++++++++++++++++++++++
// MODEM / TERMINAL
if (Serial.available())                                                                 // if serial data received from the terminal, read the string, then transmit to the modem
  {
  String USB_str = Serial.readString();                                                 // read the string from the terminal
  Serial2.print(USB_str);                                                               // transmit string to the modem   
  Serial.print("\n");                                                                   // add a new line for the terminal
  }

if (Serial2.available())                                                                // if serial data received from the Modem, read the string, then transmit to the terminal 
  {
  String ME310_str = Serial2.readString();                                              // read the string from the modem
  Serial.print(ME310_str);                                                              // transmit the string to the terminal
  }
//+++++++++++++++++++++++++++++++
// BMA400 ACCELEROMETER READING EVERY SECOND
#ifdef BMA400_ENA                                                                       // if BMA400_ENA is defined
if (second_cnt>=199)                                                                    // roughly every second read accelerometer data and send to terminal
  {  
  second_cnt=0;                                                                         // reset second count (0 to 199) 200 * 5ms is 1 sec
  bma400.readData();                                                                    // read the accelerometer data
  Serial.print(bma400.parameter.acc_x);                                                 // write X data to terminal
  Serial.print("\t");                                                                   // tab accross
  Serial.print(bma400.parameter.acc_y);                                                 // write Y data to terminal
  Serial.print("\t");                                                                   // tab accross
  Serial.print(bma400.parameter.acc_z);                                                 // write Z data to terminal
  Serial.print("\n\r");                                                                 // new line & carriage return for next data set
  }
second_cnt++;                                                                           // increment second count
#endif                                                                                  // end BMA400_ENA ifdef
//*/
//+++++++++++++++++++++++++++++++
delay(5);                                                                               // ensure 1 code cycle takes at least 5ms for some rough timing
                                                                                        // could be improved by using pace routine and timer interrupt set to 5ms, but this will do for quick test.
}                                                                                       // end of never ending loop
//======================================================================================
// INTERUPTS
//======================================================================================
// Interrupt handeler for added SERCOM 4 - Serial 2 to Modem
//**************************************************************************************
void SERCOM4_Handler()                                                                  
{
Serial2.IrqHandler();                                                                   // Interrupt handler for SERCOM4
}
//**************************************************************************************
