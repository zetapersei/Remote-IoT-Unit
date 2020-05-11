/* 
*************************************************************************
   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS 
   IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED 
   TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A 
   PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT 
   HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
   ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
   OF OR INABILITY TO USE THIS SOFTWARE, EVEN IF THE COPYRIGHT HOLDERS OR 
   CONTRIBUTORS ARE AWARE OF THE POSSIBILITY OF SUCH DAMAGE.
************************************************************************
*/

#include <OneWire.h>
#include <DallasTemperature.h>
#include <avr/wdt.h>
#include <Pushbutton.h>
#include <DS3232RTC.h>


// Select your modem:
#define TINY_GSM_RX_BUFFER 512
#define TINY_GSM_MODEM_A6

//#include <SoftwareSerial.h>
//SoftwareSerial SerialAT(2, 3); // RX, TX

// Set serial for debug console (to the Serial Monitor, default speed 115200)
#define SerialMon Serial

// Set serial for AT commands (to the module)
// Use Hardware Serial on Mega, Leonardo, Micro
#define SerialAT Serial1
// See all AT commands, if wanted
//#define DUMP_AT_COMMANDS

// Define the serial console for debug prints, if needed


#define TINY_GSM_DEBUG SerialMon

// Range to attempt to autobaud
#define GSM_AUTOBAUD_MIN 9600
#define GSM_AUTOBAUD_MAX 115200

#include <TinyGsmClient.h>
#include <PubSubClient.h>

// Define how you're planning to connect to the internet
#define TINY_GSM_USE_GPRS true
#define TINY_GSM_USE_WIFI false

// set GSM PIN, if any
#define GSM_PIN ""

// Your GPRS credentials, if any
const char apn[] = "APN_Code";
const char gprsUser[] = "";
const char gprsPass[] = "";

// Your WiFi connection credentials, if applicable
const char wifiSSID[] = "YourSSID";
const char wifiPass[] = "YourWiFiPass";

// MQTT details
const char* broker = "server.broker";
//const char* broker = "test.mosquitto.org";
//const char* broker = "my_IP_local_broker";

//MQTT Topic String
const char* mqttSub = "mare_gprs/#";
const char* checkTemp01 = "mare_gprs/seguenze/check_temp01";
const char* checkTemp02 = "mare_gprs/seguenze/check_temp02";
const char* checkLevel01 = "mare_gprs/seguenze/check_level01";
const char* topicInit = "mare_gprs/init";
const char* inviaData = "mare_gprs/dispositivi/stato/tempus";
const char* comandoAttuatore_01 = "mare_gprs/comandi/dispositivi/att01";
const char* comandoAttuatore_02 = "mare_gprs/comandi/dispositivi/att02";
const char* analogValue01 = "mare_gprs/segnali/analogici/in01";
const char* temp01 = "mare_gprs/segnali/analogici/temp01";
const char* temp02 = "mare_gprs/segnali/analogici/temp02";
const char* digiValue01 = "mare_gprs/segnali/digitali/di01";
const char* alarmString = "mare_gprs/segnali/digitali/allarmi/intrusione";
const char* pwrString = "mare_gprs/segnali/digitali/allarmi/rete220";
const char* aux01String = "mare_gprs/segnali/digitali/allarmi/aux01";
const char* aux02String = "mare_gprs/segnali/digitali/allarmi/aux02";
const char* aux03String = "mare_gprs/segnali/digitali/allarmi/aux03";
const char* aux04String = "mare_gprs/segnali/digitali/allarmi/aux04";
const char* tempusString = "mare_gprs/segnali/timestamp/tempus";

// String Definition
char analValue[5];
char tmp01[5];
char tmp02[5];
int index = 0;
char tempus_buf[20];

#ifdef DUMP_AT_COMMANDS
  #include <StreamDebugger.h>
  StreamDebugger debugger(SerialAT, SerialMon);
  TinyGsm modem(debugger);
#else
TinyGsm modem(SerialAT);
#endif
TinyGsmClient client(modem);
PubSubClient mqtt(client);

//ATMEGA 2560 I/O definition

// Digital Output
#define ACT01_PIN 11
#define ACT02_PIN 12

// Digital I/O
#define ALARM_PIN 22
#define POWER_PIN 23
#define AUX01_PIN 26
#define AUX02_PIN 27
#define AUX03_PIN 28
#define AUX04_PIN 30

int act01Status = LOW;
int act02Status = LOW;
const int buttonPin = 2;
int buttonState = LOW;

//Temperature input
#define ONE_WIRE_TEMP01 3
#define ONE_WIRE_TEMP02 2

// Analog Input
int sensorPin = A0;

// Interface Object allocation
OneWire oneWire01(ONE_WIRE_TEMP01);
DallasTemperature sensors_temp01(&oneWire01);
OneWire oneWire02(ONE_WIRE_TEMP02);
DallasTemperature sensors_temp02(&oneWire02);

Pushbutton alm_button(ALARM_PIN);
Pushbutton pwr_button(POWER_PIN);
Pushbutton aux01_button(AUX01_PIN);
Pushbutton aux02_button(AUX02_PIN);
Pushbutton aux03_button(AUX03_PIN);
Pushbutton aux04_button(AUX04_PIN);

// RTC Time-Date

DS3232RTC gprs_RTC;

long lastReconnectAttempt = 0;

void mqttCallback(char* topic, byte* payload, unsigned int len) {

 /* SerialMon.print("Message arrived [");
  SerialMon.print(topic);
  SerialMon.print("]: ");
  SerialMon.write(payload, len);
  SerialMon.println();
  SerialMon.print(analValue);
  SerialMon.println();
  SerialMon.print(tempC);
  SerialMon.println(); */

  if (String(topic) == checkTemp01) {
  sensors_temp01.requestTemperatures();
  dtostrf(sensors_temp01.getTempCByIndex(0), 4, 1,tmp01);
  //mqtt.publish(analogValue01,analValue);
  //mqtt.publish(digiValue01, buttonState ? "1" : "0");
  mqtt.publish(temp01, tmp01);
  //mqtt.publish(temp02, tmp02);
  }

if (String(topic) == checkTemp02) {
  sensors_temp02.requestTemperatures();
  dtostrf(sensors_temp02.getTempCByIndex(0), 4, 1,tmp02);
  //mqtt.publish(analogValue01,analValue);
  //mqtt.publish(digiValue01, buttonState ? "1" : "0");
  //mqtt.publish(temp01, tmp01);
  mqtt.publish(temp02, tmp02);
  }

if (String(topic) == checkLevel01) {
  sprintf(analValue,"%d",analogRead(sensorPin));
  mqtt.publish(analogValue01,analValue);
  //mqtt.publish(digiValue01, buttonState ? "1" : "0");
  //mqtt.publish(temp01, tmp01);
  //mqtt.publish(temp02, tmp02);
  }

  if (String(topic) == comandoAttuatore_01) {
    act01Status = !act01Status;
    digitalWrite(ACT01_PIN, act01Status);
    //mqtt.publish(statoPompa, ledStatus ? "1" : "0");
  }
  if (String(topic) == comandoAttuatore_02) {
    act02Status = !act02Status;
    digitalWrite(ACT02_PIN, act02Status);
  }

  if (String(topic) == inviaData) {
     time_t t = gprs_RTC.get();
     sprintf(tempus_buf, "%.2d\:%.2d\:%.2d %.2d %s %d",hour(t), minute(t), second(t), day(t), monthShortStr(month(t)), year(t));
     mqtt.publish(tempusString,tempus_buf);
  }

}

boolean mqttConnect() {
  SerialMon.print("Connecting to ");
  SerialMon.print(broker);

  // Connect to MQTT Broker
  //boolean status = mqtt.connect("Remote_location");

  // Or, if you want to authenticate MQTT:
  boolean status = mqtt.connect("Remote_location", "user", "pwd");

  if (status == false) {
    SerialMon.println(" fail");
    return false;
  }
  SerialMon.println(" success");
  mqtt.publish(topicInit, "Gprs Remote location started");
  mqtt.subscribe(mqttSub);
  return mqtt.connected();
}
void setup() {


  // Set console baud rate
  SerialMon.begin(115200);
  delay(10);

  pinMode(ACT01_PIN, OUTPUT);
  pinMode(ACT02_PIN, OUTPUT);
  pinMode(buttonPin, INPUT);
  sensors_temp01.begin();
  sensors_temp02.begin();

//display date-Time

  gprs_RTC.begin();

  time_t t = gprs_RTC.get();
   sprintf(tempus_buf, "%.2d\:%.2d\:%.2d %.2d %s %d\n",
        hour(t), minute(t), second(t), day(t), monthShortStr(month(t)), year(t));
   SerialMon.println(tempus_buf);

  
  SerialMon.println("Wait...");

  // Set GSM module baud rate
  TinyGsmAutoBaud(SerialAT,GSM_AUTOBAUD_MIN,GSM_AUTOBAUD_MAX);
  //SerialAT.begin(9600);
  delay(3000);




  // Restart takes quite some time
  // To skip it, call init() instead of restart()
  SerialMon.println("Initializing modem...");
  modem.restart();
  //modem.init();


  wdt_enable(WDTO_8S);
    String modemInfo = modem.getModemInfo();
  SerialMon.print("Modem Info: ");
  SerialMon.println(modemInfo);
  //SerialMon.println(modem.getBattPercent());

#if TINY_GSM_USE_GPRS
  // Unlock your SIM card with a PIN if needed
  if ( GSM_PIN && modem.getSimStatus() != 3 ) {
    modem.simUnlock(GSM_PIN);
  }
#endif

  SerialMon.println(" success");
//#endif

// wdt_reset();

  SerialMon.print("Waiting for network...");
  if (!modem.waitForNetwork()) {
    SerialMon.println(" fail");
    delay(10000);
    return;
  }
  SerialMon.println(" success");

  if (modem.isNetworkConnected()) {
    SerialMon.println("Network connected");
  }

 wdt_reset();
   

#if TINY_GSM_USE_GPRS
  // GPRS connection parameters are usually set after network registration
    SerialMon.print(F("Connecting to "));
    SerialMon.print(apn);
    if (!modem.gprsConnect(apn, gprsUser, gprsPass)) {
      SerialMon.println(" fail");
      delay(10000);
      return;
    }
    SerialMon.println(" success");

  if (modem.isGprsConnected()) {
    SerialMon.println("GPRS connected");

  }
#endif

 wdt_reset();
// wdt_enable(WDTO_8S);

  // MQTT Broker setup
  mqtt.setServer(broker, 16198);
  //mqtt.setServer(broker, 1883);
  mqtt.setCallback(mqttCallback);

  
   wdt_reset();
}

void loop() {

  //SerialMon.println(modem.getLocalIP());
   
// digital input management   
// Burglary alarm - Ready
 if (alm_button.getSingleDebouncedPress())
    {
      mqtt.publish(alarmString, "Alert");
    }
 if (alm_button.getSingleDebouncedRelease())
    {
      mqtt.publish(alarmString, "Ready");
    }
// 230 Vac test
 if (pwr_button.getSingleDebouncedPress())
    {
      mqtt.publish(pwrString, "Inizio");
    }
 if (pwr_button.getSingleDebouncedRelease())
    {
      mqtt.publish(pwrString, "Rientro");
    }
// Input AUX01 
  if (aux01_button.getSingleDebouncedPress())
    {
      mqtt.publish(aux01String, "Inizio");
    }
  if (aux01_button.getSingleDebouncedRelease())
    {
      mqtt.publish(aux01String, "Rientro");
    }
 // Input AUX02 
  if (aux02_button.getSingleDebouncedPress())
    {
      mqtt.publish(aux02String, "Inizio");
    }
  if (aux02_button.getSingleDebouncedRelease())
    {
      mqtt.publish(aux02String, "Rientro");
    }
// Input AUX03 
     if (aux03_button.getSingleDebouncedPress())
    {
      mqtt.publish(aux03String, "Inizio");
    }
 if (aux03_button.getSingleDebouncedRelease())
    {
      mqtt.publish(aux03String, "Rientro");
    }
// Input AUX04 
 if (aux04_button.getSingleDebouncedPress())
    {
      mqtt.publish(aux04String, "Inizio");
    }
 if (aux04_button.getSingleDebouncedRelease())
    {
      mqtt.publish(aux04String, "Rientro");
    }
   
  if (!mqtt.connected()) {
    SerialMon.println("=== MQTT NOT CONNECTED ===");
    unsigned long t = millis();
    if (t - lastReconnectAttempt > 10000L) {
      lastReconnectAttempt = t;
      if (mqttConnect()) {
        lastReconnectAttempt = 0;
      }
    }

    delay(20);
    return;
  }

  wdt_reset();

  mqtt.loop();
}
