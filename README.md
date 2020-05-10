# Remote-IoT-Unit
Remote units with Digital/analog Input and Output for IoT application.
Communication is based on MQTT protocol.

## Hardware
The unit is composed by ATmega 2560 Arduino board connected to a A6 gprs module.
### I/O capability:
 N° 6 digital input
 
 N° 2 temperature input DS18B20 sensors
 
 N° 1 analog input (Volt)
 
 N° 2 digital output

## Software
The software is based with TinyGsm library https://github.com/vshymanskyy/TinyGSM.
A real-time clock is implemented using DS3232 module and related library.
A MQTT function is derived by a PubSubClient library https://github.com/knolleary/pubsubclient.
Digital input is controlled by a PushButton library https://github.com/pololu/pushbutton-arduino.
Temperature input use a DS18B20 probe controlled by a related library.
