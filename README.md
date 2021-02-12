# Charlie
Telit Charlie IoT

This is an Example project for the Telit Charlie IoT Evaluation board written in Arduino IDE.
Telits Charlie has been designed for developers to get an asset management / tracking project up and running quickly.

The Evaluation board features a user button, user LED, ME310G1-WW modem with Built in GNSS and Bosch BMA400 accelerometer.
The Board is compatable with MKRZero Shields.
The Modem operates LTE CAT M1 & LTE NB-IoT with GSM fall back
The user will need to provide a data SIM (nano), you will also need to ask the SIM provider for the APN address, to be able to connect to the internet with it.

For more information on the board please see the Telit website: https://contact.telit.com/charlie-evaluation-kit
For more information on the Modem: https://www.telit.com/me310g1/
For more information in the Accelerometer: https://www.bosch-sensortec.com/products/motion-sensors/accelerometers/bma400.html

I bought my board from Easby Electronics based here in the UK: https://easby.com

My Project uses the Teminal Window to write AT commands to the modem and display the responces.
SETTINGS FOR TERMINAL: Baud: 9600, 8 data bits, No parity, 1 stop bit, add carriage return on to the of line on send.

First the device will set up the Accelerometer
The device will check to see if the modem is on, if it isn't it will switch it on.
At this point you should be able to send AT commands, there are a few examples in the comments of the ino, for connecting to the networks, setting up MQTT, using MQTT and taking GNSS readings as well as using the GTP WWAN feature (Golbal Terestrial Positioning).

If the #define BMA400_ENA is commented in, the accelerometer data will stream to the terminal window every second.

Pressing the user button will switch on the user LED while pressed.
